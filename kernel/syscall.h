#ifndef __SYSCALL_H
#define __SYSCALL_H

#include "libc.h"

#define SVC_UART_PUTS		0x0001
#define SVC_UART_HEX		0x0002
#define SVC_UART_HEX_LONG	0x0003
#define SVC_UART_PUTC		0x0004

#define SVC_CLEAR_SCREEN	0x0010
#define SVC_DRAW_WALLPAPER  0x0011
#define SVC_DRAW_PIXEL		0x0012

void sys_uart_puts(char *);
void sys_uart_hex(uint32);
void sys_uart_hex_long(uint64);
void sys_uart_putc(char);
void sys_clear_screen();
void sys_draw_wallpaper();
void sys_draw_pixel(uint, uint);

#endif
