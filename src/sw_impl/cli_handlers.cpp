#include "cli_handlers.h"

#include <cstdint>
#include <xuartps.h>
#include <xil_printf.h>
#include <string.h>

#include "cartridge.h"
#include "../common.h"

// TODO: Replace all bytewise xil_printfs with custom implementation.

void cli_help()
{
    xil_printf(
        "ZYNQ GBCartReader - Read & Write Gameboy cartridges\r\n"
        "written by alpyen - visit the project on github.com/alpyen/zynq-gbcartreader\r\n"
        "\r\n"
        "Available commands:\r\n"
        "-------------------\r\n"
        "help          Display this help page\r\n"
        "parse header  Read cartridge header and parse into readable form\r\n"
        "read rom      Read cartridge rom and echo it in binary\r\n"
        "read ram      Read cartridge ram (if available) and echo it in binary\r\n"
        "write ram     Write cartridge ram (if available) from binary terminal data\r\n"
        "read rtc      Read cartridge RTC (if available) and echo it in binary\r\n"
        "write rtc     Write cartridge RTC (if available) from binary terminal data\r\n"
    );
}

void cli_parse_header()
{
    // All MBCs power up in such a state that mbc1::read_rom(0) will read the first bank
    // which contains the header for further identification (when reading other banks or RAM).
    mbc1::read_rom(0);

    cartridge_header* header = (cartridge_header*)&cartridge_buffer[HEADER_BASE_ADDRESS];

    xil_printf("Overview:\r\n");

    xil_printf("  Entry Point:      ");
    for (unsigned i = 0; i < arraysizeof(header->entry_point); ++i)
        xil_printf(" %02x", header->entry_point[i]);
    xil_printf("\r\n");


    xil_printf(
        "  Nintendo Logo:     %s\r\n",
        memcmp(header->nintendo_logo, NINTENDO_LOGO, arraysizeof(NINTENDO_LOGO)) ? "Bad" : "Good"
    );


    // The title can include the "Manufacturer Code" and the "CGB flag" depending on the age of the game/cartridge.
    // We only display the printable characters.
    xil_printf("  Title:             ");
    for (unsigned i = 0; i < arraysizeof(header->title); ++i)
    {
        if (!is_printable(header->title[i])) break;
        xil_printf("%c", header->title[i]);
    }
    xil_printf("\r\n");
    xil_printf(
        "  Manufac. Code (?): %c%c%c%c\r\n",
        header->title[11], header->title[12], header->title[13], header->title[14]
    );

    xil_printf("  CGB Flag:          ");
    if (header->title[15] == 0x80)
        xil_printf("CGB supported, but backwards compatible");
    else if (header->title[15] == 0xc0)
        xil_printf("CGB exclusive");
    else
        xil_printf("No");
    xil_printf("\r\n");


    xil_printf(
        "  New Licensee Code: %s\r\n",
        get_new_licensee_code_string(header->new_licensee_code)
    );


    xil_printf("  SGB Flag:          %s\r\n", header->sgb_flag ? "Yes": "No");


    xil_printf("  Type:              %s\r\n", get_cartridge_type_string(header->cartridge_type));


    xil_printf("  ROM Size:          ", header->rom_size);
    if (header->rom_size >= 0x00 and header->rom_size <= 0x08)
        xil_printf("%d KiB (%d banks)", 1 << (5 + header->rom_size), 1 << (1 + header->rom_size));
    else
        xil_printf("%02x not recognized.", header->rom_size);
    xil_printf("\r\n");


    xil_printf("  RAM Size:          ", header->ram_size);
    switch (header->ram_size)
    {
        case 0x00: xil_printf("No RAM"); break;
        case 0x02: xil_printf("8 KiB (1 banks"); break;
        case 0x03: xil_printf("32 KiB (4 banks)"); break;
        case 0x04: xil_printf("128 KiB (16 banks)"); break;
        case 0x05: xil_printf("64 KiB (8 banks)"); break;
    }
    xil_printf("\r\n");


    xil_printf(
        "  Destination Code:  %s\r\n",
        (header->destination_code == 0x00) ? "Japan": "Overseas"
    );


    xil_printf(
        "  Old Licensee Code: %s\r\n",
        get_old_licensee_code_string(header->old_licensee_code)
    );


    xil_printf("  Version:           %02x\r\n", header->rom_version);


    xil_printf("  Header Checksum:   %02x\r\n", header->header_checksum);


    xil_printf(
        "  Global Checksum:   %02x %02x\r\n",
        header->global_checksum[0], header->global_checksum[1]
    );

    xil_printf("\r\n");

    xil_printf("Full Header:");
    for (unsigned i = 0; i < sizeof(*header); ++i)
    {
        if (i % 0x10 == 0) xil_printf("\r\n  0x%04x: ", HEADER_BASE_ADDRESS + i);
        xil_printf(" %02x", ((uint8_t*)header)[i]);
    }
    xil_printf("\r\n");
}

void cli_read_rom()
{
    mbc1::read_rom(0);
    cartridge_header* header = (cartridge_header*)&cartridge_buffer[0x0100];

    uint8_t cartridge_type = header->cartridge_type;
    unsigned num_banks = 1 << (header->rom_size + 1);

    if (!(header->rom_size >= 0x00 and header->rom_size <= 0x08))
    {
        // TODO: Invalid num_banks handling
        return;
    }

    // Technically we already read bank 0, but can't hurt to read it again.
    for (unsigned bank = 0; bank < num_banks; ++bank)
    {
        switch (cartridge_type)
        {
            // Even though the ROM-only has no MBC and therefore no registers,
            // we can simply use the MBC1 methods as the register writes will
            // not cause any harm since the WRn pin is unconnected on these cartridges.
            case cartridge_type::ROM:

            case cartridge_type::MBC1:
            case cartridge_type::MBC1_RAM:
            case cartridge_type::MBC1_RAM_BATTERY:
                mbc1::read_rom(bank);
                break;

            case cartridge_type::MBC3:
            case cartridge_type::MBC3_RAM:
            case cartridge_type::MBC3_RAM_BATTERY:
            case cartridge_type::MBC3_RTC_BATTERY:
            case cartridge_type::MBC3_RTC_RAM_BATTERY:
                mbc3::read_rom(bank);
                break;

            case cartridge_type::MBC5:
            case cartridge_type::MBC5_RAM:
            case cartridge_type::MBC5_RAM_BATTERY:
            case cartridge_type::MBC5_RUMBLE:
            case cartridge_type::MBC5_RUMBLE_RAM:
            case cartridge_type::MBC5_RUMBLE_RAM_BATTERY:
                mbc5::read_rom(bank);
                break;
        }

        for (unsigned address = 0; address < ROM_BANK_SIZE; ++address)
            xil_printf("%c", cartridge_buffer[address]);
    }
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

        default:
            // TODO: Invalid num_banks handling
            return;
    }

    for (unsigned bank = 0; bank < num_banks; ++bank)
    {
        switch (cartridge_type)
        {
            case cartridge_type::MBC3_RAM:
            case cartridge_type::MBC3_RAM_BATTERY:
            case cartridge_type::MBC3_RTC_RAM_BATTERY:
                mbc3::read_ram(bank);
                break;

            case cartridge_type::MBC5_RAM:
            case cartridge_type::MBC5_RAM_BATTERY:

            // Bit 3 of RAMB controls the rumble motor on these cartridges but since
            // the cartridge will not have so many banks to accidentally trigger the rumble
            // this works just as well.
            case cartridge_type::MBC5_RUMBLE_RAM:
            case cartridge_type::MBC5_RUMBLE_RAM_BATTERY:
                mbc5::read_ram(bank);
                break;

            default:
                // TODO: Invalid cartridge type handling
                return;
        }

        for (unsigned address = 0; address < RAM_BANK_SIZE; ++address)
            xil_printf("%c", cartridge_buffer[address]);
    }
}

void cli_write_ram()
{
    mbc1::read_rom(0);

    cartridge_header* header = (cartridge_header*)&cartridge_buffer[HEADER_BASE_ADDRESS];
    uint8_t cartridge_type = header->cartridge_type;
    uint8_t num_banks = 0;

    switch (header->ram_size)
    {
        case 0x02: num_banks = 1; break;
        case 0x03: num_banks = 4; break;
        case 0x04: num_banks = 16; break;
        case 0x05: num_banks = 8; break;

        default:
            // TODO: Invalid num_banks handling
            return;
    }

    switch (cartridge_type)
    {
        case cartridge_type::MBC3_RAM:
        case cartridge_type::MBC3_RAM_BATTERY:
        case cartridge_type::MBC3_RTC_RAM_BATTERY:
            for (unsigned bank = 0; bank < num_banks; ++bank)
            {
                for (unsigned address = 0; address < RAM_BANK_SIZE; ++address)
                {
                    /*
                        TODO: This is just a rudimentary flow control implementation
                            to not overrun the RX buffer on the Zynq when waiting
                            for the GPIO to write the data.

                        TODO: Do not hardcore XPAR_UART0_BASEADDR
                    */
                    uint8_t byte = XUartPs_RecvByte(XPAR_UART0_BASEADDR);
                    cartridge_buffer[address] = byte;
                    XUartPs_SendByte(XPAR_UART0_BASEADDR, byte);
                }

                mbc3::write_ram(bank);
            }
            break;

        case cartridge_type::MBC5_RAM:
        case cartridge_type::MBC5_RAM_BATTERY:
        case cartridge_type::MBC5_RUMBLE_RAM:
        case cartridge_type::MBC5_RUMBLE_RAM_BATTERY:
            for (unsigned bank = 0; bank < num_banks; ++bank)
            {
                for (unsigned address = 0; address < RAM_BANK_SIZE; ++address)
                {
                    // NOTE: See MBC3 case!
                    uint8_t byte = XUartPs_RecvByte(XPAR_UART0_BASEADDR);
                    cartridge_buffer[address] = byte;
                    XUartPs_SendByte(XPAR_UART0_BASEADDR, byte);
                }

                mbc5::write_ram(bank);
            }
            break;

        default:
            // TODO: Invalid cartridge type handling
            return;
    }
}

void cli_read_rtc()
{
    mbc1::read_rom(0);

    cartridge_header* header = (cartridge_header*)&cartridge_buffer[HEADER_BASE_ADDRESS];

    switch (header->cartridge_type)
    {
        case cartridge_type::MBC3_RTC_BATTERY:
        case cartridge_type::MBC3_RTC_RAM_BATTERY:
            mbc3::read_rtc();
            break;

        default:
            // TODO: Invalid cartridge type handling
            return;
    }

    // Registers are selected and then appear on the whole address range.
    // mbc3::read_rtc just lists them sequentially.
    for (unsigned address = 0; address < 5; ++address)
        xil_printf("%c", cartridge_buffer[address]);
}

void cli_write_rtc()
{
    xil_printf("Write RTC handler called.\r\n");
}
