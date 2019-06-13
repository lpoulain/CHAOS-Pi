#ifndef __UART_H
#define __UART_H

#include "libc.h"

void uart_init();
void uart_putc(unsigned char c);
unsigned char uart_getc();
void uart_puts(const char* str);
void uart_hex(uint d);
void uart_hex_long(uint64 d);
void uart_dump_mem(void *ptr, int nb_bytes);
void uart_print_current_stack();

#endif
