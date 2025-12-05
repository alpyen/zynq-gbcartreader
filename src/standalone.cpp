#include "standalone.h"

#include <xuartps.h>
#include "common.h"

/* NOTE: This function fills up the buffer and overwrites only the last character
         if more arrive than the buffer can handle. It breaks upon receciving '\r'. */
void uart_readline(char* buffer, uint8_t buffer_size)
{
    uint8_t num_received = 0;

    while (true)
    {
        char received = XUartPs_RecvByte(STDOUT_BASEADDRESS);
        num_received++;

        // Transform to lower case for strcmp
        if (received >= 'A' && received <= 'Z')
            received += 'a' - 'A';

        buffer[num_received - 1] = received;

        // Command complete?
        if (received == '\r') break;

        // Decrement buffer pointer to point to last element again.
        if (num_received == buffer_size)
            num_received--;
    }

    // Overwrite the \r line break with null-terminator for strcmp.
    buffer[num_received - 1] = '\0';
}
