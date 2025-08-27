#pragma once

#include <cstdint>

const uint16_t HEADER_BASE_ADDRESS = 0x0100;

extern uint8_t cartridge_buffer[0x4000];

struct cartridge_header
{
    uint8_t entry_point[4];     // 0x100 - 0x103
    uint8_t nintendo_logo[48];  // 0x104 - 0x133
    char title[16];             // 0x134 - 0x143
    char new_licensee_code[2];  // 0x144 - 0x145
    uint8_t sgb_flag;           // 0x146
    uint8_t cartridge_type;     // 0x147
    uint8_t rom_size;           // 0x148
    uint8_t ram_size;           // 0x149
    uint8_t destination_code;   // 0x14A
    uint8_t old_licensee_code;  // 0x14B
    uint8_t rom_version;        // 0x14C
    uint8_t header_checksum;    // 0x14D
    uint8_t global_checksum[2]; // 0x14E - 0x14F
} __attribute__((packed));

namespace mbc1
{
    enum registers: uint16_t
    {
        RAMG    = 0x0000,
        BANK1   = 0x2000,
        BANK2   = 0x4000,
        MODE    = 0x6000
    };

    void write_register(registers reg, uint8_t value);
    void read_rom(uint8_t bank);
    // TODO: void read_ram(uint8_t bank);
}
