#pragma once

#define arraysizeof(array) sizeof(array) / sizeof(array[0])

[[noreturn]] void die(const char* message);
bool is_printable(const char letter);
