#include "cartridge.h"

#include <xil_printf.h>
#include "pmod.h"

uint8_t cartridge_buffer[0x4000];

/*
    NOTE: The read/write routines could potentially be sped up by pipelining
    some of the IO assignments. For now it's better to keep them clear
    and separated which also aids debugging.
*/

void _shiftout_address(uint16_t address)
{
    pmod_state.ADDR_RCLK = 0;

    for (unsigned i = 0; i < 16; ++i)
    {
        pmod_state.ADDR_SCLK = 0;
        pmod_state.ADDR_SDATA = (address >> (15-i)) & 0b1;
        write_pmod();

        pmod_state.ADDR_SCLK = 1;
        write_pmod();
    }

    pmod_state.ADDR_RCLK = 1;
    write_pmod();
}

void _shiftout_data(uint8_t data)
{
    pmod_state.DATA_OUT_RCLK = 0;

    for (unsigned i = 0; i < 8; ++i)
    {
        pmod_state.DATA_OUT_SCLK = 0;
        pmod_state.DATA_OUT_SDATA = (data >> (7-i)) & 0b1;
        write_pmod();

        pmod_state.DATA_OUT_SCLK = 1;
        write_pmod();
    }

    pmod_state.DATA_OUT_RCLK = 1;
    write_pmod();

    pmod_state.DATA_OUT_OEn = 0;
    pmod_state.WRn = 0;
    write_pmod();

    pmod_state.DATA_OUT_OEn = 1;
    pmod_state.WRn = 1;
    write_pmod();
}

uint8_t _shiftin_data()
{
    pmod_state.DATA_IN_RCLK = 0;
    write_pmod();

    pmod_state.DATA_IN_RCLK = 1;
    pmod_state.DATA_IN_PLn = 0;
    write_pmod();

    pmod_state.DATA_IN_PLn = 1;
    write_pmod();

    uint8_t byte = 0;
    for (unsigned i = 0; i < 8; ++i)
    {
        read_pmod();
        byte |= pmod_state.DATA_IN_SDATA << (7-i);

        pmod_state.DATA_IN_SCLK = 0;
        write_pmod();

        pmod_state.DATA_IN_SCLK = 1;
        write_pmod();
    }

    return byte;
}

namespace mbc1
{
    void reset_cartridge()
    {
        write_register(registers::RAMG, 0);
        write_register(registers::BANK1, 0);
        write_register(registers::BANK2, 0);
        write_register(registers::MODE, 0);

        reset_pmod();
    }

    void write_register(registers reg, uint8_t value)
    {
        _shiftout_address(reg);
        _shiftout_data(value);
    }

    void read_rom(uint8_t bank)
    {
        uint16_t bank_base_address = 0x0000;

        if ((bank & 0b11111) != 0)
            bank_base_address = 0x4000;
        
        write_register(registers::MODE, 1);
        write_register(registers::BANK1, bank & 0b11111);
        write_register(registers::BANK2, (bank >> 5) & 0b11);

        for (uint16_t address = 0; address < 0x4000; ++address)
        {
            pmod_state.RDn = 0;
            write_pmod();

            _shiftout_address(bank_base_address + address);
            cartridge_buffer[address] = _shiftin_data();

            pmod_state.RDn = 1;
            write_pmod();
        }

        reset_cartridge();
    }
}
