#include "cli_handlers.h"

#include <cstdint>
#include <xuartps.h>
#include <string.h>

#include "cartridge.h"
#include "../common.h"
#include "../print.h"

// TODO: Implement timeout of 3s?
// TODO: Call virtual printf so platform agnostic? (Zynq/Arduino)

inline static void __print_response_header(response_t code, uint32_t payload_size = 0)
{
    XUartPs_SendByte(STDOUT_BASEADDRESS, code);

    if (payload_size > 0)
    {
        for (unsigned i = 0; i < 4; ++i)
            XUartPs_SendByte(STDOUT_BASEADDRESS, (uint8_t)(payload_size >> (i * 8)));
    }
}

void cli_unknown()
{
    __print_response_header(response_t::UNKNOWN_COMMAND);
}

void cli_help()
{
    const char* help_string =
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
    ;

    // Use __builtin_strlen to compute length at compile time.
    // Otherwise the string has to be stored in the stack or moves to the global scope
    // of this cpp-file.
    __print_response_header(response_t::OK, __builtin_strlen(help_string));
    xil_printf("%s", help_string);
}

// Parses and outputs the cartridge header in human readable format.
void cli_parse_header()
{
    // All MBCs power up in such a state that mbc1::read_rom(0) will read the first bank
    // which contains the header for further identification (when reading other banks or RAM).
    // To speed things up mbc1::read_header only reads the range where the header actually sits
    // instead of the whole bank 0.
    cartridge_header* header = mbc1::read_header();

    /*
        NOTE: Since we need the string's length before writing it out to UART we need to assemble
        it and store it somewhere.  The cartridge buffer only contains the header at the default
        location 0x0100 so we can safely start our string at 0x1000 and still have 0x3000 bytes left.
        The assembled string is guaranteed to be less than 1K.  This is the guarantee we need to
        not use vsnprintf and keep track of the number of characters.

        This implementation works just fine but -feels- terrible.  It also uses vsprintf
        instead of a slimmed down version like xil_printf instead of printf.
        Perhaps, modify xil_printf to print to a char buffer instead to the stdout?
    */
    char* header_string = (char*)&cartridge_buffer[0x1000];
    char* header_string_base = header_string;

    xil_sprintf(&header_string, "Overview:\r\n");
    xil_sprintf(&header_string, "  Entry Point:      ");

    for (unsigned i = 0; i < arraysizeof(header->entry_point); ++i)
        xil_sprintf(&header_string, " %02x", header->entry_point[i]);

    xil_sprintf(&header_string, "\r\n");

    xil_sprintf(&header_string,
        "  Nintendo Logo:     %s\r\n",
        memcmp(header->nintendo_logo, NINTENDO_LOGO, arraysizeof(NINTENDO_LOGO)) ? "Bad" : "Good"
    );


    // The title can include the "Manufacturer Code" and the "CGB flag" depending on the
    // age of the game/cartridge.  We only display the printable characters.
    xil_sprintf(&header_string, "  Title:             ");
    for (unsigned i = 0; i < arraysizeof(header->title); ++i)
    {
        if (!is_printable(header->title[i])) break;
        xil_sprintf(&header_string, "%c", header->title[i]);
    }
    xil_sprintf(&header_string, "\r\n");

    xil_sprintf(&header_string, "  Manufac. Code (?): ");
    for (unsigned i = 0; i < 4; ++i)
    {
        char letter = header->title[11 + i];
        if (is_printable(letter))
        xil_sprintf(&header_string, "%c", letter);
    }
    xil_sprintf(&header_string, "\r\n");

    xil_sprintf(&header_string, "  CGB Flag:          ");
    if (header->title[15] == 0x80)
        xil_sprintf(&header_string, "CGB supported, but backwards compatible");
    else if (header->title[15] == 0xc0)
        xil_sprintf(&header_string, "CGB exclusive");
    else
        xil_sprintf(&header_string, "No");
    xil_sprintf(&header_string, "\r\n");


    xil_sprintf(
        &header_string,
        "  New Licensee Code: %s\r\n",
        get_new_licensee_code_string(header->new_licensee_code)
    );


    xil_sprintf(&header_string, "  SGB Flag:          %s\r\n", header->sgb_flag ? "Yes": "No");


    xil_sprintf(&header_string, "  Type:              %s\r\n", get_cartridge_type_string(header->cartridge_type));


    xil_sprintf(&header_string, "  ROM Size:          ", header->rom_size);
    if (header->rom_size <= 0x08)
        xil_sprintf(&header_string, "%d KiB (%d banks)", 1 << (5 + header->rom_size), 1 << (1 + header->rom_size));
    else
        xil_sprintf(&header_string, "%02x not recognized.", header->rom_size);
    xil_sprintf(&header_string, "\r\n");


    xil_sprintf(&header_string, "  RAM Size:          ", header->ram_size);
    switch (header->ram_size)
    {
        case 0x00: xil_sprintf(&header_string, "No RAM"); break;
        case 0x02: xil_sprintf(&header_string, "8 KiB (1 banks"); break;
        case 0x03: xil_sprintf(&header_string, "32 KiB (4 banks)"); break;
        case 0x04: xil_sprintf(&header_string, "128 KiB (16 banks)"); break;
        case 0x05: xil_sprintf(&header_string, "64 KiB (8 banks)"); break;
        default: xil_sprintf(&header_string, "%02x not recognized."); break;
    }
    xil_sprintf(&header_string, "\r\n");


    xil_sprintf(
        &header_string,
        "  Destination Code:  %s\r\n",
        (header->destination_code == 0x00) ? "Japan": "Overseas"
    );


    xil_sprintf(&header_string,
        "  Old Licensee Code: %s\r\n",
        get_old_licensee_code_string(header->old_licensee_code)
    );


    xil_sprintf(&header_string, "  Version:           %02x\r\n", header->rom_version);


    xil_sprintf(&header_string, "  Header Checksum:   %02x\r\n", header->header_checksum);


    xil_sprintf(
        &header_string,
        "  Global Checksum:   %02x %02x\r\n",
        header->global_checksum[0], header->global_checksum[1]
    );

    xil_sprintf(&header_string, "\r\n");

    xil_sprintf(&header_string, "Full Header:");
    for (unsigned i = 0; i < sizeof(*header); ++i)
    {
        if (i % 0x10 == 0) xil_sprintf(&header_string, "\r\n  0x%04x: ", HEADER_BASE_ADDRESS + i);
        xil_sprintf(&header_string, " %02x", ((uint8_t*)header)[i]);
    }

    xil_sprintf(&header_string, "\r\n");

    __print_response_header(response_t::OK, (uint32_t)(header_string - header_string_base));
    xil_printf("%s", header_string_base);
}

void cli_read_rom()
{
    cartridge_header* header = mbc1::read_header();

    uint8_t cartridge_type = header->cartridge_type;
    unsigned num_banks = 1 << (header->rom_size + 1);

    if (header->rom_size > 0x08)
    {
        __print_response_header(response_t::INVALID_NUM_ROM_BANKS);
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

            default:
                __print_response_header(response_t::INVALID_CARTRIDGE_TYPE);
                return;
        }

        if (bank == 0)
        {
            uint32_t bytes_to_send = num_banks * ROM_BANK_SIZE;
            __print_response_header(response_t::OK, bytes_to_send);
        }

        for (unsigned address = 0; address < ROM_BANK_SIZE; ++address)
            XUartPs_SendByte(STDOUT_BASEADDRESS, cartridge_buffer[address]);
    }
}

void cli_read_ram()
{
    cartridge_header* header = mbc1::read_header();

    uint8_t cartridge_type = header->cartridge_type;
    unsigned num_banks = 0;

    switch (header->ram_size)
    {
        case 0x00:
        case 0x01:
            __print_response_header(response_t::CARTRIDGE_HAS_NO_RAM);
            return;

        case 0x02: num_banks = 1; break;
        case 0x03: num_banks = 4; break;
        case 0x04: num_banks = 16; break;
        case 0x05: num_banks = 8; break;

        default:
            __print_response_header(response_t::INVALID_NUM_RAM_BANKS);
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
                __print_response_header(response_t::INVALID_CARTRIDGE_TYPE);
                return;
        }

        if (bank == 0)
        {
            uint32_t bytes_to_write = num_banks * RAM_BANK_SIZE;
            __print_response_header(response_t::OK, bytes_to_write);
        }

        for (unsigned address = 0; address < RAM_BANK_SIZE; ++address)
            XUartPs_SendByte(STDOUT_BASEADDRESS, cartridge_buffer[address]);
    }
}

void cli_write_ram()
{
    cartridge_header* header = mbc1::read_header();
    uint8_t cartridge_type = header->cartridge_type;

    // This comes in handy to collapse multiple for-loops into one.
    void (*write_func)(uint8_t bank);

    switch (cartridge_type)
    {
        case cartridge_type::MBC3_RAM:
        case cartridge_type::MBC3_RAM_BATTERY:
        case cartridge_type::MBC3_RTC_RAM_BATTERY:
            write_func = mbc3::write_ram;
            break;

        case cartridge_type::MBC5_RAM:
        case cartridge_type::MBC5_RAM_BATTERY:
        case cartridge_type::MBC5_RUMBLE_RAM:
        case cartridge_type::MBC5_RUMBLE_RAM_BATTERY:
            write_func = mbc5::write_ram;
            break;

        default:
            __print_response_header(response_t::CARTRIDGE_HAS_NO_RAM);
            return;
    }

    uint8_t num_banks = 0;

    switch (header->ram_size)
    {
        case 0x02: num_banks = 1; break;
        case 0x03: num_banks = 4; break;
        case 0x04: num_banks = 16; break;
        case 0x05: num_banks = 8; break;

        default:
            __print_response_header(response_t::INVALID_NUM_RAM_BANKS);
            return;
    }

    __print_response_header(response_t::OK);

    // How many bytes wants the PC to write?
    uint32_t write_size = 0;
    for (int i = 0; i < 4; ++i)
        write_size |= ((uint32_t)XUartPs_RecvByte(STDOUT_BASEADDRESS)) << (i * 8);

    if (write_size != num_banks * RAM_BANK_SIZE)
    {
        __print_response_header(response_t::INVALID_RAM_WRITE_SIZE);
        return;
    }

    __print_response_header(response_t::OK);

    for (unsigned bank = 0; bank < num_banks; ++bank)
    {
        for (unsigned address = 0; address < RAM_BANK_SIZE; ++address)
        {
            uint8_t byte = XUartPs_RecvByte(STDOUT_BASEADDRESS);
            cartridge_buffer[address] = byte;
            XUartPs_SendByte(STDOUT_BASEADDRESS, byte);
        }

        write_func(bank);
    }
}

void cli_read_rtc()
{
    cartridge_header* header = mbc1::read_header();

    switch (header->cartridge_type)
    {
        case cartridge_type::MBC3_RTC_BATTERY:
        case cartridge_type::MBC3_RTC_RAM_BATTERY:
            mbc3::read_rtc();
            break;

        default:
            __print_response_header(response_t::CARTRIDGE_HAS_NO_RTC);
            return;
    }

    // TODO: Change to sizeof(rtc_data) ?
    __print_response_header(response_t::OK, 5);

    // Registers are selected and then appear on the whole address range.
    // mbc3::read_rtc just lists them sequentially.
    for (unsigned address = 0; address < 5; ++address)
        XUartPs_SendByte(STDOUT_BASEADDRESS, cartridge_buffer[address]);
}

void cli_write_rtc()
{
    cartridge_header* header = mbc1::read_header();
    (void) header;

    // TODO: Implement

    return;
}
