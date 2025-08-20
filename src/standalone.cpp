#include "standalone.h"

#include <xuartps.h>

void uart_readline(uint32_t base_address, char* buffer, uint8_t buffer_size)
{
    uint8_t num_received = 0;

    while (true)
    {
        char received = XUartPs_RecvByte(base_address);

        bool is_valid = false;
        is_valid |= received >= 'a' && received <= 'z';
        is_valid |= received >= 'A' && received <= 'Z';
        is_valid |= received >= '0' && received <= '9';
        is_valid |= received == ' ' || received == '\b' || received == '\r';

        if (!is_valid) continue;

        if (received >= 'A' && received <= 'Z')
        {
            // Transform to lower case for strcmp
            received += 'a' - 'A';
        }
        else if (received == '\b')
        {
            // Delete character if buffer is not empty
            if (num_received > 0)
            {
                num_received--;

                XUartPs_SendByte(base_address, '\b');
                XUartPs_SendByte(base_address, ' ');
                XUartPs_SendByte(base_address, '\b');
            }

            continue;
        }

        buffer[num_received] = received;
        num_received++;

        if (num_received > buffer_size - 1)
        {
            XUartPs_SendByte(base_address, '\b');
            num_received--;
        }

        XUartPs_SendByte(base_address, received);

        if (received == '\r') break;
    }

    XUartPs_SendByte(base_address, '\n');

    // Overwrite the \r line break with null-terminator
    buffer[num_received - 1] = '\0';
}
