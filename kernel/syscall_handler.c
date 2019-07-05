#include "libc.h"
#include "syscall.h"
#include "uart.h"
#include "display.h"

void syscall_handler(uint call_nb, uint64 arg1, uint64 arg2) {
	switch(call_nb) {
		case SVC_UART_PUTS:
			uart_puts((char*)arg1);
			break;
		case SVC_UART_HEX:
			uart_hex((uint)arg1);
			break;
		case SVC_UART_HEX_LONG:
			uart_hex_long(arg1);
			break;
		case SVC_CLEAR_SCREEN:
			break;
		case SVC_DRAW_WALLPAPER:
			draw_wallpaper();
			break;
		case SVC_DRAW_PIXEL:
			draw_pixel((uint32)arg1, (uint32)arg2, 0xFFFFFF);
			break;
		default:
			break;
	}
}
