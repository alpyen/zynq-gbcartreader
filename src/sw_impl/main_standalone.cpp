#include "xparameters.h"
#include <xil_printf.h>
#include <xuartps.h>

#include "pmod.h"
#include "cli_handlers.h"
#include "../common.h"
#include "../standalone.h"

#include <string.h>

int main()
{
    if (init_pmod(XPAR_AXI_PMOD_GPIO_BASEADDR) != XST_SUCCESS)
        die("PMOD GPIO Initialization failed.\r\n");

    const char* commands[] = {
        "help", "echo", "read header", "read rom",
        "read ram", "write ram", "read rtc", "write rtc"
    };

    void (* const handlers[])(void) = {
        cli_help, cli_echo, cli_read_header, cli_read_rom,
        cli_read_ram, cli_write_ram, cli_read_rtc, cli_write_rtc
    };

    char line_buffer[16];

    while (true)
    {
        if (echo) xil_printf("> ");
        uart_readline(XPAR_UART0_BASEADDR, line_buffer, sizeof(line_buffer), echo);

        bool valid_command = false;
        for (uint8_t i = 0; i < arraysizeof(commands); ++i)
            if (!strcmp(line_buffer, commands[i]))
            {
                valid_command = true;
                handlers[i]();
            }

        if (!valid_command && strcmp(line_buffer, ""))
            xil_printf(
                "\"%s\" command not recognized."
                " Use \"help\" to see available commands.\r\n",
                line_buffer
            );
    }

    return 0;
}
