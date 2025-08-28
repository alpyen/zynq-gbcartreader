#include "cli_handlers.h"

#include <xuartps.h>
#include <xil_printf.h>

#include "cartridge.h"
#include "../common.h"

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
    // All MBCs power up in such a state that mbc1::read_rom(0) will read the first bank
    // which contains the header for further identification (when reading other banks or RAM).
    mbc1::read_rom(0);

    cartridge_header* header = (cartridge_header*)&cartridge_buffer[HEADER_BASE_ADDRESS];

    xil_printf("Overview:\r\n");

    // The title can include the "Manufacturer Code" and the "CGB flag"
    // depending on the age of the game/cartridge.
    // We only display the printable characters.
    xil_printf("  Title:    ");
    for (unsigned i = 0; i < arraysizeof(header->title); ++i)
    {
        if (!is_printable(header->title[i])) break;
        xil_printf("%c", header->title[i]);
    }
    xil_printf("\r\n");

    xil_printf("  Version:  %02x\r\n", header->rom_version);

    xil_printf("  Type:     ", header->cartridge_type);
    switch (header->cartridge_type)
    {
        case 0x00: xil_printf("ROM"); break;
        case 0x01: xil_printf("MBC1"); break;
        case 0x02: xil_printf("MBC1 + RAM"); break;
        case 0x03: xil_printf("MBC1 + RAM + Battery"); break;
        case 0x05: xil_printf("MBC2"); break;
        case 0x06: xil_printf("MBC2 + Battery"); break;
        case 0x08: xil_printf("ROM + RAM"); break;
        case 0x09: xil_printf("ROM + RAM + Battery"); break;
        case 0x0b: xil_printf("MMM01"); break;
        case 0x0c: xil_printf("MMM01 + RAM"); break;
        case 0x0d: xil_printf("MMM01 + RAM + Battery"); break;
        case 0x0f: xil_printf("MBC3 + Timer + Battery"); break;
        case 0x10: xil_printf("MBC3 + Timer + RAM + Battery"); break;
        case 0x11: xil_printf("MBC3"); break;
        case 0x12: xil_printf("MBC3 + RAM"); break;
        case 0x13: xil_printf("MBC3 + RAM + Battery"); break;
        case 0x19: xil_printf("MBC5"); break;
        case 0x1a: xil_printf("MBC5 + RAM"); break;
        case 0x1b: xil_printf("MBC5 + RAM + Battery"); break;
        case 0x1c: xil_printf("MBC5 + Rumble"); break;
        case 0x1d: xil_printf("MBC5 + Rumble + RAM"); break;
        case 0x1e: xil_printf("MBC5 + Rumble + RAM + Battery"); break;
        case 0x20: xil_printf("MBC6"); break;
        case 0x22: xil_printf("MBC7 + Sensor + Rumble + RAM + Battery"); break;
        case 0xfc: xil_printf("Pocket Camera"); break;
        case 0xfd: xil_printf("Bandai TAMA5"); break;
        case 0xfe: xil_printf("HuC3"); break;
        case 0xff: xil_printf("HuC1 + RAM + Battery"); break;
    }
    xil_printf("\r\n");

    xil_printf("  ROM Size: ", header->rom_size);
    switch (header->rom_size)
    {
        case 0x00: xil_printf("32 KiB (2 banks)"); break;
        case 0x01: xil_printf("64 KiB (4 banks)"); break;
        case 0x02: xil_printf("128 KiB (8 banks)"); break;
        case 0x03: xil_printf("256 KiB (16 banks)"); break;
        case 0x04: xil_printf("512 KiB (32 banks)"); break;
        case 0x05: xil_printf("1 MiB (64 banks)"); break;
        case 0x06: xil_printf("2 MiB (128 banks)"); break;
        case 0x07: xil_printf("4 MiB (256 banks)"); break;
        case 0x08: xil_printf("8 MiB (512 banks)"); break;
        case 0x52: xil_printf("1.1 MiB (72 banks)"); break;
        case 0x53: xil_printf("1.2 MiB (80 banks)"); break;
        case 0x54: xil_printf("1.5 MiB (96 banks)"); break;
    }
    xil_printf("\r\n");

    xil_printf("  RAM Size: ", header->ram_size);
    switch (header->ram_size)
    {
        case 0x00: xil_printf("No RAM"); break;
        case 0x01: xil_printf("Unused"); break;
        case 0x02: xil_printf("8 KiB (1 banks"); break;
        case 0x03: xil_printf("32 KiB (4 banks)"); break;
        case 0x04: xil_printf("128 KiB (16 banks)"); break;
        case 0x05: xil_printf("64 KiB (8 banks)"); break;
    }
    xil_printf("\r\n\r\n");

    xil_printf("Full Header:");
    for (unsigned i = 0; i < sizeof(*header); ++i)
    {
        if (i % 0x10 == 0) xil_printf("\r\n  ");
        xil_printf(" %02x", ((uint8_t*)header)[i]);
    }
    xil_printf("\r\n");
}

void cli_show_crc32()
{
    xil_printf("Show CRC32 handler called.\r\n");
}

void cli_read_rom()
{
    mbc1::read_rom(0);
    cartridge_header* header = (cartridge_header*)&cartridge_buffer[0x0100];

    uint8_t cartridge_type = header->cartridge_type;
    unsigned num_banks = 1 << (header->rom_size + 1);

    // These don't line up nicely, so we gotta check them manually.
    switch (num_banks)
    {
        case 0x52: num_banks = 72; break;
        case 0x53: num_banks = 80; break;
        case 0x54: num_banks = 96; break;
        // TODO: Invalid num_banks handling
    }

    // Technically we already read bank 0, but can't hurt to read it again.
    for (unsigned bank = 0; bank < num_banks; ++bank)
    {
        switch (cartridge_type)
        {
            case 0x01:
            case 0x02:
            case 0x03:
                mbc1::read_rom(bank);
                break;

            case 0x19:
            case 0x1a:
            case 0x1b:
            case 0x1c:
            case 0x1d:
            case 0x1e:
                mbc5::read_rom(bank);
                break;
        }

        for (unsigned address = 0; address < ROM_BANK_SIZE; ++address)
        {
            if (address % 16 == 0)
                xil_printf("\r\n%08x:", (bank << 14) + address);

            xil_printf(" %02x", cartridge_buffer[address]);
        }
    }

    xil_printf("\r\n");
}

void cli_read_ram()
{
    mbc1::read_rom(0);
    cartridge_header* header = (cartridge_header*)&cartridge_buffer[0x0100];

    uint8_t cartridge_type = header->cartridge_type;
    unsigned num_banks = 0;

    switch (header->ram_size)
    {
        case 0x00:
        case 0x01:
            // There is no RAM.
            return;

        case 0x02: num_banks = 1; break;
        case 0x03: num_banks = 4; break;
        case 0x04: num_banks = 16; break;
        case 0x05: num_banks = 8; break;
        // TODO: Invalid num_banks handling
    }

    for (unsigned bank = 0; bank < num_banks; ++bank)
    {
        switch (cartridge_type)
        {
            case 0x19:
            case 0x1a:
            case 0x1b:
            case 0x1c:
            case 0x1d:
            case 0x1e:
                mbc5::read_ram(bank);
                break;
        }

        for (unsigned address = 0; address < RAM_BANK_SIZE; ++address)
        {
            if (address % 16 == 0)
                xil_printf("\r\n%08x:", (bank << 13) + address);

            xil_printf(" %02x", cartridge_buffer[address]);
        }
    }

    xil_printf("\r\n");
}

void cli_write_ram()
{
    xil_printf("Write RAM handler called.\r\n");
}
