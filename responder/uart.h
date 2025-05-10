#ifndef __UART_H__
#define __UART_H__

#include <stm32f10x.h>
#include <stdint.h>
#include <stdarg.h>
#include <string.h>
#include <stddef.h>
#include <stdio.h>

void UART3_Init(void);
void UART3_IDLE_Init(void);
void UART3_SendChar(char c);
void UART3_SendString(const char *str);
void UART3_printf(const char *format, ...);
void UART3_SendBuffer(uint8_t *data, uint16_t len);

#endif
