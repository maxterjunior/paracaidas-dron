/* EUSART1 por J1 (RC6 TX, RC7 RX), 115200 8N1. putch() permite usar printf(). */
#ifndef UART_H
#define UART_H

#include <stdbool.h>

void uart_init(void);
void putch(char c);
bool uart_getc(char *c);    /* no bloqueante */

#endif
