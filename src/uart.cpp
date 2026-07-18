#include "uart.h"

void Uart_SendByte(UINTPTR BaseAddress, u8 Data)
{
#ifdef UARTLITE
    XUartLite_SendByte(BaseAddress, Data);
#else
    XUartPs_SendByte(BaseAddress, Data);
#endif
}

u8 Uart_RecvByte(UINTPTR BaseAddress)
{
#ifdef UARTLITE
    return XUartLite_RecvByte(BaseAddress);
#else
    return XUartPs_RecvByte(BaseAddress);
#endif
}
