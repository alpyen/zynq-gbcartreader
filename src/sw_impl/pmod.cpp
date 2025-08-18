#include "pmod.h"

void init_port_direction(XGpio* xgpio)
{
    // All signals are outputs except for DATA_OUT_SDATA.
    // That is the data line from the PISO shift register from the PCB to the FPGA.

    int direction_mask = 0;
    direction_mask |= 1 << PmodSignals::DATA_OUT_SDATA;

    XGpio_SetDataDirection(xgpio, 1, direction_mask);
}