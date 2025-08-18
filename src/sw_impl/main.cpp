#include <xil_printf.h>
#include "xparameters.h"

#include "pmod.h"
#include "../common.h"

int main()
{
    xil_printf("Hello, world! from my GBCartReader Application!\n\r");

    XGpio xgpio;

    if (XGpio_Initialize(&xgpio, XPAR_AXI_PMOD_GPIO_BASEADDR) != XST_SUCCESS)
        die("XGpio_Initialize failed on AXI PMOD IP core.\n\r");
    
    init_port_direction(&xgpio);
    xil_printf("PMOD GPIO pins in/out direction set.\n\r");
    
    PmodState pmod_state;

    return 0;
}