#include "pmod.h"

#include <sleep.h>

PmodState pmod_state;
XGpio pmod_gpio;

int init_pmod(uint32_t base_address)
{
    int result = XGpio_Initialize(&pmod_gpio, base_address);
    if (result != XST_SUCCESS) return result;

    // The GPIO IP Core starts in tri-state and needs its inputs/outputs configured.
    // All signals are outputs except for DATA_IN_SDATA which is the data line
    // from the PISO shift register from the PCB to the FPGA.

    // However, before going out of the tri-state mode,
    // we need to make sure some lines like (RDn, WRn, OEn, PLn) are turned off
    // otherwise we may corrupt the cartridge data or even break it completely.

    // Note that this could have been done with the default output values
    // in the IPI but doing this here is much clearer as the signal names
    // are used rather than cryptic magic numbers.
    
    pmod_state = {
        .RDn = 1,
        .CSn = 1,
        .ADDR_SDATA = 0,
        .ADDR_RCLK = 0,
        .WRn = 1,
        .ADDR_SCLK = 0,
        
        .DATA_OUT_SDATA = 0,
        .DATA_OUT_RCLK = 0,
        .DATA_IN_SDATA = 0,
        .DATA_IN_RCLK = 0,
        .DATA_OUT_OEn = 1,
        .DATA_OUT_SCLK = 0,
        .DATA_IN_PLn = 1,
        .DATA_IN_SCLK = 0
    };
    
    write_pmod();

    // Now it's safe to enable all ports as outputs except DATA_IN_SDATA.
    XGpio_SetDataDirection(&pmod_gpio, 1, 1 << PmodSignals::DATA_IN_SDATA);
    
    return XST_SUCCESS;
}

/*
    NOTE: The PMOD IP core is clocked with 100 MHz but the cartridge
    can handle at most 4.19 MHz (x2 in double speed mode) so we slow
    it down to ~1MHz for now. Once the reading/writing routine
    are done we can measure the IOs and sleep less to increase throughput.
*/
void write_pmod()
{
    XGpio_DiscreteWrite(&pmod_gpio, 1, pmod_state.value);
    usleep(1);
}

void read_pmod()
{
    pmod_state.value = (uint16_t)XGpio_DiscreteRead(&pmod_gpio, 1);
    usleep(1);
}
