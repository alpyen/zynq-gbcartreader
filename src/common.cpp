#include "common.h"

#include <xil_printf.h>
#include <xstatus.h>
#include <cstdlib>

void die(const char* message)
{
    xil_printf("%s%s", message, "Critical Failure - Exiting Application!\r\n");
    exit(XST_FAILURE);
}

bool is_printable(const char letter)
{
    return (letter >= 0x20) && (letter <= 0x7e);
}
