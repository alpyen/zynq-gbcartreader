#pragma once

#include <cstdint>
#include <xgpio.h>

// DATA_OUT refers to FPGA->Cart
// DATA_IN refers to Cart->FPGA

union PmodState {
    uint16_t value;

    struct {
        uint16_t RDn             : 1;
        uint16_t CSn             : 1;
        uint16_t ADDR_SDATA      : 1;
        uint16_t ADDR_RCLK       : 1;
        uint16_t WRn             : 1;
        uint16_t ADDR_SCLK       : 1;
        
        uint16_t DATA_OUT_SDATA  : 1;
        uint16_t DATA_OUT_RCLK   : 1;
        uint16_t DATA_IN_SDATA   : 1;
        uint16_t DATA_IN_RCLK    : 1;
        uint16_t DATA_OUT_OEn    : 1;
        uint16_t DATA_OUT_SCLK   : 1;
        uint16_t DATA_IN_PLn     : 1;
        uint16_t DATA_IN_SCLK    : 1;

        uint16_t : 2;
    };
};

enum PmodSignals {
    RDn,
    CSn,
    ADDR_SDATA,
    ADDR_RCLK,
    WRn,
    ADDR_SCLK,
    DATA_OUT_SDATA,
    DATA_OUT_RCLK,
    DATA_IN_SDATA,
    DATA_IN_RCLK,
    DATA_OUT_OEn,
    DATA_OUT_SCLK,
    DATA_IN_PLn,
    DATA_IN_SCLK
};

extern XGpio pmod_gpio;
extern PmodState pmod_state;

int init_pmod(uint32_t base_address);
void write_pmod();
