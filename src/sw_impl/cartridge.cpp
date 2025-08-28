#include "cartridge.h"

#include <xil_printf.h>
#include "pmod.h"

const uint16_t ROM_BANK_AREA1_BASE_ADDRESS = 0x0000;
const uint16_t ROM_BANK_AREA2_BASE_ADDRESS = 0x4000;
const uint16_t RAM_BANK_BASE_ADDRESS = 0xa000;

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

void _write_register(uint16_t register_address, uint8_t value)
{
    _shiftout_address(register_address);
    _shiftout_data(value);
}

namespace mbc1
{
    enum registers: uint16_t
    {
        RAMG    = 0x0000,
        BANK1   = 0x2000,
        BANK2   = 0x4000,
        MODE    = 0x6000
    };

    void reset_cartridge()
    {
        _write_register(registers::RAMG, 0);
        _write_register(registers::BANK1, 0);
        _write_register(registers::BANK2, 0);
        _write_register(registers::MODE, 0);

        reset_pmod();
    }

    void read_rom(uint8_t bank)
    {
        uint16_t bank_base_address = ROM_BANK_AREA1_BASE_ADDRESS;

        if ((bank & 0b11111) != 0)
            bank_base_address = ROM_BANK_AREA2_BASE_ADDRESS;

        _write_register(registers::MODE, 1);
        _write_register(registers::BANK1, bank & 0b11111);
        _write_register(registers::BANK2, (bank >> 5) & 0b11);

        for (uint16_t address = 0; address < ROM_BANK_SIZE; ++address)
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

namespace mbc5
{
    enum registers: uint16_t
    {
        RAMG    = 0x0000,
        ROMB1   = 0x2000,
        ROMB2   = 0x3000,
        RAMB    = 0x4000
    };

    void reset_cartridge()
    {
        _write_register(registers::RAMG, 0);
        _write_register(registers::ROMB1, 0);
        _write_register(registers::ROMB2, 0);
        _write_register(registers::RAMB, 0);

        reset_pmod();
    }

    void read_rom(uint16_t bank)
    {
        uint16_t bank_base_address = ROM_BANK_AREA2_BASE_ADDRESS;

        _write_register(registers::ROMB1, bank & 0xff);
        _write_register(registers::ROMB2, (bank >> 8) & 0b1);

        for (uint16_t address = 0; address < ROM_BANK_SIZE; ++address)
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

    void read_ram(uint8_t bank)
    {
        uint16_t bank_base_address = RAM_BANK_BASE_ADDRESS;

        const uint8_t RAM_ENABLE_PATTERN = 0b00001010;

        _write_register(registers::RAMG, RAM_ENABLE_PATTERN);
        _write_register(registers::RAMB, bank);

        for (uint16_t address = 0; address < RAM_BANK_SIZE; ++address)
        {
            pmod_state.RDn = 0;
            write_pmod();

            _shiftout_address(bank_base_address + address);

            pmod_state.CSn = 0;
            write_pmod();

            cartridge_buffer[address] = _shiftin_data();

            pmod_state.CSn = 1;
            pmod_state.RDn = 1;
            write_pmod();
        }

        reset_cartridge();
    }
}
