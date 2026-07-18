#include "xparameters.h"

#include "pmod.h"
#include "cli_handlers.h"
#include "misc.h"
#include "print.h"

#include <string.h>

int main()
{
    if (init_pmod(XPAR_AXI_PMOD_GPIO_BASEADDR) != XST_SUCCESS)
        die("PMOD GPIO Initialization failed.\r\n");

    const char* commands[] = {
        "help", "parse header", "read rom", "read ram", "write ram",
    };

    void (* const handlers[])(void) = {
        cli_help, cli_parse_header, cli_read_rom, cli_read_ram, cli_write_ram,
    };

    char line_buffer[16];

    // TODO: Implemenet timeout mechanism of 3 seconds.

    while (true)
    {
        /* NOTE: This function fills up the buffer and overwrites only the last character
         if more arrive than the buffer can handle. It breaks upon receciving '\r'. */
        uart_readline(line_buffer, sizeof(line_buffer));

        bool valid_command = false;
        for (uint8_t i = 0; i < arraysizeof(commands); ++i)
            if (!strcmp(line_buffer, commands[i]))
            {
                valid_command = true;
                handlers[i]();
            }

        if (!valid_command && strcmp(line_buffer, ""))
            cli_unknown();
    }

    return 0;
}
