#pragma once

#include <cstdint>

const uint16_t HEADER_BASE_ADDRESS = 0x0100;

const uint32_t ROM_BANK_SIZE = 0x4000;
const uint32_t RAM_BANK_SIZE = 0x2000;

const uint8_t NINTENDO_LOGO[0x30] = {
    0xCE, 0xED, 0x66, 0x66, 0xCC, 0x0D, 0x00, 0x0B,
    0x03, 0x73, 0x00, 0x83, 0x00, 0x0C, 0x00, 0x0D,
    0x00, 0x08, 0x11, 0x1F, 0x88, 0x89, 0x00, 0x0E,
    0xDC, 0xCC, 0x6E, 0xE6, 0xDD, 0xDD, 0xD9, 0x99,
    0xBB, 0xBB, 0x67, 0x63, 0x6E, 0x0E, 0xEC, 0xCC,
    0xDD, 0xDC, 0x99, 0x9F, 0xBB, 0xB9, 0x33, 0x3E
};

extern uint8_t cartridge_buffer[ROM_BANK_SIZE];

enum cartridge_type: uint8_t
{
    ROM =                            0x00,
    MBC1 =                           0x01,
    MBC1_RAM =                       0x02,
    MBC1_RAM_BATTERY =               0x03,
    MBC2 =                           0x05,
    MBC2_BATTERY =                   0x06,
    ROM_RAM =                        0x08,
    ROM_RAM_BATTERY =                0x09,
    MMM01 =                          0x0b,
    MMM01_RAM =                      0x0c,
    MMM01_RAM_BATTERY =              0x0d,
    MBC3_RTC_BATTERY =               0x0f,
    MBC3_RTC_RAM_BATTERY =           0x10,
    MBC3 =                           0x11,
    MBC3_RAM =                       0x12,
    MBC3_RAM_BATTERY =               0x13,
    MBC5 =                           0x19,
    MBC5_RAM =                       0x1a,
    MBC5_RAM_BATTERY =               0x1b,
    MBC5_RUMBLE =                    0x1c,
    MBC5_RUMBLE_RAM =                0x1d,
    MBC5_RUMBLE_RAM_BATTERY =        0x1e,
    MBC6 =                           0x20,
    MBC7_SENSOR_RUMBLE_RAM_BATTERY = 0x22,
    POCKETCAMERA =                   0xfc,
    BANDAITAMA5 =                    0xfd,
    HUC3 =                           0xfe,
    HUC1_RAM_BATTERY =               0xff
};

struct cartridge_header
{
    uint8_t entry_point[4];         // 0x100 - 0x103
    uint8_t nintendo_logo[48];      // 0x104 - 0x133
    uint8_t title[16];              // 0x134 - 0x143
    uint8_t new_licensee_code[2];   // 0x144 - 0x145
    uint8_t sgb_flag;               // 0x146
    uint8_t cartridge_type;         // 0x147
    uint8_t rom_size;               // 0x148
    uint8_t ram_size;               // 0x149
    uint8_t destination_code;       // 0x14A
    uint8_t old_licensee_code;      // 0x14B
    uint8_t rom_version;            // 0x14C
    uint8_t header_checksum;        // 0x14D
    uint8_t global_checksum[2];     // 0x14E - 0x14F
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
    cartridge_header* read_header();
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
    void write_ram(uint8_t bank);
}

const char* get_cartridge_type_string(uint8_t cartridge_type);
const char* get_new_licensee_code_string(uint8_t code[2]);
const char* get_old_licensee_code_string(uint8_t code);
