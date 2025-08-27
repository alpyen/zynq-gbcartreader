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
    xil_printf("Hello, world! from my GBCartReader Application!\r\n");

    if (init_pmod(XPAR_AXI_PMOD_GPIO_BASEADDR) != XST_SUCCESS)
        die("PMOD GPIO Initialization failed.\r\n");
    
    const char* commands[] = {
        "help", "show header", "show crc32", "read rom", "read ram", "write ram"
    };

    void (* const handlers[])(void) = {
        cli_help, cli_show_header, cli_show_crc32,
        cli_read_rom, cli_read_ram, cli_write_ram
    };

    char line_buffer[16];

    while (true)
    {
        xil_printf("> ");
        uart_readline(XPAR_UART0_BASEADDR, line_buffer, sizeof(line_buffer));

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
