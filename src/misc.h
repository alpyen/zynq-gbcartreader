#pragma once

#include <cstdint>

#define arraysizeof(array) sizeof(array) / sizeof(array[0])

[[noreturn]] void die(const char* message);
bool is_printable(const char letter);
void uart_readline(char* buffer, uint8_t buffer_size);
