#include "cli_handlers.h"

#include <xuartps.h>
#include <xil_printf.h>

void cli_help()
{
    xil_printf(
        "ZYNQ GBCartReader - Read & Write Gameboy cartridges\r\n"
        "written by alpyen - visit the project on github.com/alpyen/zynq-gbcartreader\r\n"
        "\r\n"
        "Available commands:\r\n"
        "-------------------\r\n"
        "help          Display this help page\r\n"
        "show header   Read header from cartridge rom and display it humanly readable\r\n"
        "show crc32    Read cartridge rom and calculate its crc32\r\n"
        "read rom      Read cartridge rom and echo it in binary form\r\n"
        "read ram      Read cartridge ram (if available) and echo it in binary form\r\n"
        "write ram     Write cartridge ram (if available) from binary terminal data\r\n"
    );
}

void cli_show_header()
{
    xil_printf("Show Header handler called.\r\n");
}

void cli_show_crc32()
{
    xil_printf("Show CRC32 handler called.\r\n");
}

void cli_read_rom()
{
    xil_printf("Read ROM handler called.\r\n");
}

void cli_read_ram()
{
    xil_printf("Read RAM handler called.\r\n");
}

void cli_write_ram()
{
    xil_printf("Write RAM handler called.\r\n");
}
