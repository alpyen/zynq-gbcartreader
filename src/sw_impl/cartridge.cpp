#include "cartridge.h"

#include "../print.h"
#include "pmod.h"

const uint16_t ROM_BANK_AREA1_BASE_ADDRESS = 0x0000;
const uint16_t ROM_BANK_AREA2_BASE_ADDRESS = 0x4000;
const uint16_t RAM_BANK_RTC_BASE_ADDRESS = 0xa000;

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

// NOTE: The cartridge and pmod_state are REQUIRED to be reset to a known state before operating on them.
/* NOTE: Some functions like write_ram are identical across multiple MBCs.  Since their
         footprint is rather small, they are deliberately left unbundled.*/
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

    /* NOTE: This function reads the header into the cartridge buffer and returns a pointer to it.
             The pointer is invalid as soon as new data is being written into the cartridge buffer. */
    cartridge_header* read_header()
    {
        reset_cartridge();

        _write_register(registers::MODE, 0);

        for (uint16_t address = HEADER_BASE_ADDRESS;
            address < HEADER_BASE_ADDRESS + sizeof(cartridge_header);
            ++address)
        {
            pmod_state.RDn = 0;
            write_pmod();

            _shiftout_address(address);
            cartridge_buffer[address] = _shiftin_data();

            pmod_state.RDn = 1;
            write_pmod();
        }

        return (cartridge_header*)&cartridge_buffer[HEADER_BASE_ADDRESS];
    }

    void read_rom(uint8_t bank)
    {
        reset_cartridge();

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
    }
}

namespace mbc3
{
    const uint8_t RAM_RTC_ENABLE_PATTERN = 0b00001010;

    enum registers: uint16_t
    {
        RAMG_RTCRG  = 0x0000,
        ROMB        = 0x2000,
        RAMB_RTCRS  = 0x4000,
        RCLK_RTC    = 0x6000
    };

    void reset_cartridge()
    {
        _write_register(registers::RAMG_RTCRG, 0);
        _write_register(registers::ROMB, 0);
        _write_register(registers::RAMB_RTCRS, 0);
        _write_register(registers::RCLK_RTC, 0);

        reset_pmod();
    }

    void read_rom(uint8_t bank)
    {
        reset_cartridge();

        uint16_t bank_base_address = ROM_BANK_AREA1_BASE_ADDRESS;

        if (bank != 0)
            bank_base_address = ROM_BANK_AREA2_BASE_ADDRESS;

        _write_register(registers::ROMB, bank);

        for (uint16_t address = 0; address < ROM_BANK_SIZE; ++address)
        {
            pmod_state.RDn = 0;
            write_pmod();

            _shiftout_address(bank_base_address + address);
            cartridge_buffer[address] = _shiftin_data();

            pmod_state.RDn = 1;
            write_pmod();
        }

    }

    void read_ram(uint8_t bank)
    {
        reset_cartridge();

        _write_register(registers::RAMG_RTCRG, RAM_RTC_ENABLE_PATTERN);
        _write_register(registers::RAMB_RTCRS, bank);

        for (uint16_t address = 0; address < RAM_BANK_SIZE; ++address)
        {
            pmod_state.RDn = 0;
            write_pmod();

            _shiftout_address(RAM_BANK_RTC_BASE_ADDRESS + address);

            pmod_state.CSn = 0;
            write_pmod();

            cartridge_buffer[address] = _shiftin_data();

            pmod_state.CSn = 1;
            pmod_state.RDn = 1;
            write_pmod();
        }
    }

    void write_ram(uint8_t bank)
    {
        reset_cartridge();

        _write_register(registers::RAMG_RTCRG, RAM_RTC_ENABLE_PATTERN);
        _write_register(registers::RAMB_RTCRS, bank);

        for (uint16_t address = 0; address < RAM_BANK_SIZE; ++address)
        {
            _shiftout_address(RAM_BANK_RTC_BASE_ADDRESS + address);

            pmod_state.CSn = 0;
            write_pmod();

            _shiftout_data(cartridge_buffer[address]);

            pmod_state.CSn = 1;
            write_pmod();
        }
    }

    void read_rtc()
    {
        reset_cartridge();

        _write_register(registers::RAMG_RTCRG, RAM_RTC_ENABLE_PATTERN);

        const uint8_t RTC_REGISTERS_COUNT = 5;
        const uint8_t RTC_REGISTERS_BASE_INDEX = 0x08;

        _write_register(registers::RCLK_RTC, 0);
        _write_register(registers::RCLK_RTC, 1);

        for (uint8_t index = 0; index < RTC_REGISTERS_COUNT; ++index)
        {
            _write_register(registers::RAMB_RTCRS, RTC_REGISTERS_BASE_INDEX + index);

            pmod_state.RDn = 0;
            write_pmod();

            /*
                NOTE: It is recommended to wait 4 microsecods when accessing RTC registers
                      which we do automatically by sleeping 1 us per write_pmod().
            */
            _shiftout_address(RAM_BANK_RTC_BASE_ADDRESS);
            cartridge_buffer[index] = _shiftin_data();

            pmod_state.RDn = 1;
            write_pmod();
        }
    }

    // void write_rtc()
    // {
    //     reset_cartridge();
    //     _write_register(registers::RAMG_RTCRG, RAM_RTC_ENABLE_PATTERN);

    //     // TODO: Set HALT bit before writing. Write can clash? Mulitple needed?

    // }
}

namespace mbc5
{
    const uint8_t RAM_ENABLE_PATTERN = 0b00001010;

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
        reset_cartridge();

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

    }

    void read_ram(uint8_t bank)
    {
        reset_cartridge();

        _write_register(registers::RAMG, RAM_ENABLE_PATTERN);
        _write_register(registers::RAMB, bank);

        for (uint16_t address = 0; address < RAM_BANK_SIZE; ++address)
        {
            pmod_state.RDn = 0;
            write_pmod();

            _shiftout_address(RAM_BANK_RTC_BASE_ADDRESS + address);

            pmod_state.CSn = 0;
            write_pmod();

            cartridge_buffer[address] = _shiftin_data();

            pmod_state.CSn = 1;
            pmod_state.RDn = 1;
            write_pmod();
        }

    }

    void write_ram(uint8_t bank)
    {
        reset_cartridge();

        _write_register(registers::RAMG, RAM_ENABLE_PATTERN);
        _write_register(registers::RAMB, bank);

        for (uint16_t address = 0; address < RAM_BANK_SIZE; ++address)
        {
            _shiftout_address(RAM_BANK_RTC_BASE_ADDRESS + address);

            pmod_state.CSn = 0;
            write_pmod();

            _shiftout_data(cartridge_buffer[address]);

            pmod_state.CSn = 1;
            write_pmod();
        }
    }
}

const char* get_cartridge_type_string(uint8_t cartridge_type)
{
    switch (cartridge_type)
    {
        case cartridge_type::ROM:                            return "ROM";
        case cartridge_type::MBC1:                           return "MBC1";
        case cartridge_type::MBC1_RAM:                       return "MBC1 + RAM";
        case cartridge_type::MBC1_RAM_BATTERY:               return "MBC1 + RAM + Battery";
        case cartridge_type::MBC2:                           return "MBC2";
        case cartridge_type::MBC2_BATTERY:                   return "MBC2 + Battery";
        case cartridge_type::ROM_RAM:                        return "ROM + RAM";
        case cartridge_type::ROM_RAM_BATTERY:                return "ROM + RAM + Battery";
        case cartridge_type::MMM01:                          return "MMM01";
        case cartridge_type::MMM01_RAM:                      return "MMM01 + RAM";
        case cartridge_type::MMM01_RAM_BATTERY:              return "MMM01 + RAM + Battery";
        case cartridge_type::MBC3_RTC_BATTERY:               return "MBC3 + RTC + Battery";
        case cartridge_type::MBC3_RTC_RAM_BATTERY:           return "MBC3 + RTC + RAM + Battery";
        case cartridge_type::MBC3:                           return "MBC3";
        case cartridge_type::MBC3_RAM:                       return "MBC3 + RAM";
        case cartridge_type::MBC3_RAM_BATTERY:               return "MBC3 + RAM + Battery";
        case cartridge_type::MBC5:                           return "MBC5";
        case cartridge_type::MBC5_RAM:                       return "MBC5 + RAM";
        case cartridge_type::MBC5_RAM_BATTERY:               return "MBC5 + RAM + Battery";
        case cartridge_type::MBC5_RUMBLE:                    return "MBC5 + Rumble";
        case cartridge_type::MBC5_RUMBLE_RAM:                return "MBC5 + Rumble + RAM";
        case cartridge_type::MBC5_RUMBLE_RAM_BATTERY:        return "MBC5 + Rumble + RAM + Battery";
        case cartridge_type::MBC6:                           return "MBC6";
        case cartridge_type::MBC7_SENSOR_RUMBLE_RAM_BATTERY: return "MBC7 + Sensor + Rumble + RAM + Battery";
        case cartridge_type::POCKETCAMERA:                   return "Pocket Camera";
        case cartridge_type::BANDAITAMA5:                    return "Bandai TAMA5";
        case cartridge_type::HUC3:                           return "HuC3";
        case cartridge_type::HUC1_RAM_BATTERY:               return "HuC1 + RAM + Battery";
        default:                                             return "Not recognized";
    }
}

const char* get_new_licensee_code_string(uint8_t code[2])
{
    uint16_t merged = ((uint16_t)code[0]) << 8 | code[1];

    switch (merged)
    {
        case ('0' << 8) | '0':	return "None";
        case ('0' << 8) | '1':	return "Nintendo Research & Development 1";
        case ('0' << 8) | '8':	return "Capcom";
        case ('1' << 8) | '3':	return "EA (Electronic Arts)";
        case ('1' << 8) | '8':	return "Hudson Soft";
        case ('1' << 8) | '9':	return "B-AI";
        case ('2' << 8) | '0':	return "KSS";
        case ('2' << 8) | '2':	return "Planning Office WADA";
        case ('2' << 8) | '4':	return "PCM Complete";
        case ('2' << 8) | '5':	return "San-X";
        case ('2' << 8) | '8':	return "Kemco";
        case ('2' << 8) | '9':	return "SETA Corporation";
        case ('3' << 8) | '0':	return "Viacom";
        case ('3' << 8) | '1':	return "Nintendo";
        case ('3' << 8) | '2':	return "Bandai";
        case ('3' << 8) | '3':	return "Ocean Software/Acclaim Entertainment";
        case ('3' << 8) | '4':	return "Konami";
        case ('3' << 8) | '5':	return "HectorSoft";
        case ('3' << 8) | '7':	return "Taito";
        case ('3' << 8) | '8':	return "Hudson Soft";
        case ('3' << 8) | '9':	return "Banpresto";
        case ('4' << 8) | '1':	return "Ubi Soft";
        case ('4' << 8) | '2':	return "Atlus";
        case ('4' << 8) | '4':	return "Malibu Interactive";
        case ('4' << 8) | '6':	return "Angel";
        case ('4' << 8) | '7':	return "Bullet-Proof Software";
        case ('4' << 8) | '9':	return "Irem";
        case ('5' << 8) | '0':	return "Absolute";
        case ('5' << 8) | '1':	return "Acclaim Entertainment";
        case ('5' << 8) | '2':	return "Activision";
        case ('5' << 8) | '3':	return "Sammy USA Corporation";
        case ('5' << 8) | '4':	return "Konami";
        case ('5' << 8) | '5':	return "Hi Tech Expressions";
        case ('5' << 8) | '6':	return "LJN";
        case ('5' << 8) | '7':	return "Matchbox";
        case ('5' << 8) | '8':	return "Mattel";
        case ('5' << 8) | '9':	return "Milton Bradley Company";
        case ('6' << 8) | '0':	return "Titus Interactive";
        case ('6' << 8) | '1':	return "Virgin Games Ltd.";
        case ('6' << 8) | '4':	return "Lucasfilm Games";
        case ('6' << 8) | '7':	return "Ocean Software";
        case ('6' << 8) | '9':	return "EA (Electronic Arts)";
        case ('7' << 8) | '0':	return "Infogrames";
        case ('7' << 8) | '1':	return "Interplay Entertainment";
        case ('7' << 8) | '2':	return "Broderbund";
        case ('7' << 8) | '3':	return "Sculptured Software";
        case ('7' << 8) | '5':	return "The Sales Curve Limited";
        case ('7' << 8) | '8':	return "THQ";
        case ('7' << 8) | '9':	return "Accolade";
        case ('8' << 8) | '0':	return "Misawa Entertainment";
        case ('8' << 8) | '3':	return "lozc";
        case ('8' << 8) | '6':	return "Tokuma Shoten";
        case ('8' << 8) | '7':	return "Tsukuda Original";
        case ('9' << 8) | '1':	return "Chunsoft Co.";
        case ('9' << 8) | '2':	return "Video System";
        case ('9' << 8) | '3':	return "Ocean Software/Acclaim Entertainment";
        case ('9' << 8) | '5':	return "Varie";
        case ('9' << 8) | '6':	return "Yonezawa/S'Pal";
        case ('9' << 8) | '7':	return "Kaneko";
        case ('9' << 8) | '9':	return "Pack-In-Video";
        case ('9' << 8) | 'h':	return "Bottom Up";
        case ('a' << 8) | '4':	return "Konami (Yu-Gi-Oh!)";
        case ('b' << 8) | 'l':	return "MTO";
        case ('d' << 8) | 'k':	return "Kodansha";
        default: return "Not recognized";
    }
}

const char* get_old_licensee_code_string(uint8_t code)
{
    switch (code)
    {
        case 0x00: return "None";
        case 0x01: return "Nintendo";
        case 0x08: return "Capcom";
        case 0x09: return "HOT-B";
        case 0x0a: return "Jaleco";
        case 0x0b: return "Coconuts Japan";
        case 0x0c: return "Elite Systems";
        case 0x13: return "EA (Electronic Arts)";
        case 0x18: return "Hudson Soft";
        case 0x19: return "ITC Entertainment";
        case 0x1a: return "Yanoman";
        case 0x1d: return "Japan Clary";
        case 0x1f: return "Virgin Games Ltd.";
        case 0x24: return "PCM Complete";
        case 0x25: return "San-X";
        case 0x28: return "Kemco";
        case 0x29: return "SETA Corporation";
        case 0x30: return "Infogrames";
        case 0x31: return "Nintendo";
        case 0x32: return "Bandai";
        case 0x33: return "Check New Licensee Code";
        case 0x34: return "Konami";
        case 0x35: return "HectorSoft";
        case 0x38: return "Capcom";
        case 0x39: return "Banpresto";
        case 0x3c: return "Entertainment Interactive (stub)";
        case 0x3e: return "Gremlin";
        case 0x41: return "Ubi Soft";
        case 0x42: return "Atlus";
        case 0x44: return "Malibu Interactive";
        case 0x46: return "Angel";
        case 0x47: return "Spectrum HoloByte";
        case 0x49: return "Irem";
        case 0x4a: return "Virgin Games Ltd.";
        case 0x4d: return "Malibu Interactive";
        case 0x4f: return "U.S. Gold";
        case 0x50: return "Absolute";
        case 0x51: return "Acclaim Entertainment";
        case 0x52: return "Activision";
        case 0x53: return "Sammy USA Corporation";
        case 0x54: return "GameTek";
        case 0x55: return "Park Place";
        case 0x56: return "LJN";
        case 0x57: return "Matchbox";
        case 0x59: return "Milton Bradley Company";
        case 0x5a: return "Mindscape";
        case 0x5b: return "Romstar";
        case 0x5c: return "Naxat Soft";
        case 0x5d: return "Tradewest";
        case 0x60: return "Titus Interactive";
        case 0x61: return "Virgin Games Ltd.";
        case 0x67: return "Ocean Software";
        case 0x69: return "EA (Electronic Arts)";
        case 0x6e: return "Elite Systems";
        case 0x6f: return "Electro Brain";
        case 0x70: return "Infogrames";
        case 0x71: return "Interplay Entertainment";
        case 0x72: return "Broderbund";
        case 0x73: return "Sculptured Software";
        case 0x75: return "The Sales Curve Limited";
        case 0x78: return "THQ";
        case 0x79: return "Accolade";
        case 0x7a: return "Triffix Entertainment";
        case 0x7c: return "MicroProse";
        case 0x7f: return "Kemco";
        case 0x80: return "Misawa Entertainment";
        case 0x83: return "LOZC G.";
        case 0x86: return "Tokuma Shoten";
        case 0x8b: return "Bullet-Proof Software";
        case 0x8c: return "Vic Tokai Corp.";
        case 0x8e: return "Ape Inc.";
        case 0x8f: return "I'Max";
        case 0x91: return "Chunsoft Co.";
        case 0x92: return "Video System";
        case 0x93: return "Tsubaraya Productions";
        case 0x95: return "Varie";
        case 0x96: return "Yonezawa/S'Pal";
        case 0x97: return "Kemco";
        case 0x99: return "Arc";
        case 0x9a: return "Nihon Bussan";
        case 0x9b: return "Tecmo";
        case 0x9c: return "Imagineer";
        case 0x9d: return "Banpresto";
        case 0x9f: return "Nova";
        case 0xa1: return "Hori Electric";
        case 0xa2: return "Bandai";
        case 0xa4: return "Konami";
        case 0xa6: return "Kawada";
        case 0xa7: return "Takara";
        case 0xa9: return "Technos Japan";
        case 0xaa: return "Broderbund";
        case 0xac: return "Toei Animation";
        case 0xad: return "Toho";
        case 0xaf: return "Namco";
        case 0xb0: return "Acclaim Entertainment";
        case 0xb1: return "ASCII Corporation or Nexsoft";
        case 0xb2: return "Bandai";
        case 0xb4: return "Square Enix";
        case 0xb6: return "HAL Laboratory";
        case 0xb7: return "SNK";
        case 0xb9: return "Pony Canyon";
        case 0xba: return "Culture Brain";
        case 0xbb: return "Sunsoft";
        case 0xbd: return "Sony Imagesoft";
        case 0xbf: return "Sammy Corporation";
        case 0xc0: return "Taito";
        case 0xc2: return "Kemco";
        case 0xc3: return "Square";
        case 0xc4: return "Tokuma Shoten";
        case 0xc5: return "Data East";
        case 0xc6: return "Tonkin House";
        case 0xc8: return "Koei";
        case 0xc9: return "UFL";
        case 0xca: return "Ultra Games";
        case 0xcb: return "VAP, Inc.";
        case 0xcc: return "Use Corporation";
        case 0xcd: return "Meldac";
        case 0xce: return "Pony Canyon";
        case 0xcf: return "Angel";
        case 0xd0: return "Taito";
        case 0xd1: return "SOFEL (Software Engineering Lab)";
        case 0xd2: return "Quest";
        case 0xd3: return "Sigma Enterprises";
        case 0xd4: return "ASK Kodansha Co.";
        case 0xd6: return "Naxat Soft";
        case 0xd7: return "Copya System";
        case 0xd9: return "Banpresto";
        case 0xda: return "Tomy";
        case 0xdb: return "LJN";
        case 0xdd: return "Nippon Computer Systems";
        case 0xde: return "Human Ent.";
        case 0xdf: return "Altron";
        case 0xe0: return "Jaleco";
        case 0xe1: return "Towa Chiki";
        case 0xe2: return "Yutaka";
        case 0xe3: return "Varie";
        case 0xe5: return "Epoch";
        case 0xe7: return "Athena";
        case 0xe8: return "Asmik Ace Entertainment";
        case 0xe9: return "Natsume";
        case 0xea: return "King Records";
        case 0xeb: return "Atlus";
        case 0xec: return "Epic/Sony Records";
        case 0xee: return "IGS";
        case 0xf0: return "A Wave";
        case 0xf3: return "Extreme Entertainment";
        case 0xff: return "LJN";
        default: return "Not recognized";
    }
}
