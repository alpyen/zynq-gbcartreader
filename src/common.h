#pragma once

#define sizeof_array(array) sizeof(array) / sizeof(array[0])

void die(const char* message);
