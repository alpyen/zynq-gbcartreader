#include "common.h"

#include <xil_printf.h>
#include <cstdio>
#include <cstdlib>

void die(const char* message)
{
    xil_printf("%s%s", message, "Critical Failure - Exiting Application!");
    fflush(stdout);
    exit(1);
}
