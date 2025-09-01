#pragma once

#include <cstdint>

void uart_readline(uint32_t base_address, char* buffer, uint8_t buffer_size);
