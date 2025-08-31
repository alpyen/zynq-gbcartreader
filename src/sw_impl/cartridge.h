#pragma once

#include <cstdint>

const uint16_t HEADER_BASE_ADDRESS = 0x0100;

const unsigned ROM_BANK_SIZE = 0x4000;
const unsigned RAM_BANK_SIZE = 0x2000;

extern uint8_t cartridge_buffer[ROM_BANK_SIZE];

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

struct rtc_data
{
    uint8_t seconds;
    uint8_t minutes;
    uint8_t hours;
    uint8_t day_lower;

    union
    {
        uint8_t value;

        struct
        {
            uint8_t day_higher: 1;
            uint8_t: 5;
            uint8_t halt: 1;
            uint8_t day_counter_carry: 1;
        };
    } day_halt_carry;
} __attribute__((packed));

namespace mbc1
{
    void read_rom(uint8_t bank);
    // TODO: void read_ram(uint8_t bank);
    // TODO: void write_ram();
}

namespace mbc3
{
    void read_rom(uint8_t bank);
    void read_ram(uint8_t bank);
    void write_ram(uint8_t bank);
    void read_rtc();
    // TODO: void write_rtc();
}

namespace mbc5
{
    void read_rom(uint16_t bank);
    void read_ram(uint8_t bank);
    // TODO: void write_ram();
}
