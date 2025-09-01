#include "standalone.h"

#include <xuartps.h>
#include "common.h"

void uart_readline(uint32_t base_address, char* buffer, uint8_t buffer_size)
{
    uint8_t num_received = 0;

    while (true)
    {
        char received = XUartPs_RecvByte(base_address);

        // Transform to lower case for strcmp
        if (received >= 'A' && received <= 'Z')
            received += 'a' - 'A';

        buffer[num_received] = received;
        num_received++;

        if (num_received > buffer_size - 1)
            num_received--;

        if (received == '\r') break;
    }

    // Overwrite the \r line break with null-terminator
    buffer[num_received - 1] = '\0';
}
