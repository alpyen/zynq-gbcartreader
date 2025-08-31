#pragma once

#include <cstdint>

extern bool echo;

void cli_help();
void cli_echo();
void cli_read_header();
void cli_read_rom();
void cli_read_ram();
void cli_write_ram();
void cli_read_rtc();
void cli_write_rtc();
