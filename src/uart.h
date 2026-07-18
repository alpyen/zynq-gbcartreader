#pragma once

#ifdef UARTLITE
#include <xuartlite_l.h>
#else
#include <xuartps.h>
#endif

void Uart_SendByte(UINTPTR BaseAddress, u8 Data);
u8 Uart_RecvByte(UINTPTR BaseAddress);
