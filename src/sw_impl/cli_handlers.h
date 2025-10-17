#pragma once

#include <cstdint>

enum response_t: uint8_t
{
    // General response codes
    OK                      = 0,
    UNKNOWN_COMMAND         = 1,

    // Broken Cartridge
    INVALID_NUM_ROM_BANKS   = 10,
    INVALID_NUM_RAM_BANKS   = 11,
    INVALID_CARTRIDGE_TYPE  = 12,

    // PC tries op that the cart cannot handle
    INVALID_RAM_WRITE_SIZE  = 21,
    CARTRIDGE_HAS_NO_RAM    = 22,
    CARTRIDGE_HAS_NO_RTC    = 23,
    INVALID_RTC_WRITE_SIZE  = 24
};

void cli_unknown();
void cli_help();
void cli_parse_header();
void cli_read_rom();
void cli_read_ram();
void cli_write_ram();
void cli_read_rtc();
void cli_write_rtc();
