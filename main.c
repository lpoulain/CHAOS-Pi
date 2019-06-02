#include "libc.h"
#include "uart.h"
#include "mbox.h"
#include "display.h"
#include "process.h"

extern void scheduler();
extern uint *buffer;

void touchscreen_init();

void kernel_main()
{
    uart_init();
    display_init();

    touchscreen_init();

	uart_puts("Welcome to CHAOS!\r\n");

	// read the current level from system register
	unsigned long el;
    asm volatile ("mrs %0, CurrentEL" : "=r" (el));

    uart_puts("Current EL is: ");
    uart_hex((el>>2)&3);
    uart_puts("\n");		
 
	mbox_add4(MBOX_GET_MAC, 0, 8, 0, 0);
	mbox_send();
	uart_puts("MAC address: ");
    uart_hex(mbox_get(6));
    uart_hex(mbox_get(5));
    uart_puts("\n");

    mbox_add4(MBOX_GET_CLOCK_RATE, 4, 8, 0x000000005, 0);
    mbox_send();
    uart_puts("Clock Rate: 0x");
    uart_hex(mbox_get(4));
    uart_puts("\n");

    uart_puts("Display address: 0x");
    uart_hex_long((uint64)get_display_buffer());
    uart_puts("\n");

    draw_mandelbrot();
    char *sp = (char*)buffer;

	while (1) {
        char c = uart_getc();

        if (c == 0x41) {
            sp -= 512;
            draw_rect(0, 0, 684, 264, 0);
            display_dump_mem(sp, 512, 0, 0);
        }
        else if (c == 0x42) {
            sp += 512;
            draw_rect(0, 0, 684, 264, 0);
            display_dump_mem(sp, 512, 0, 0);
        } else {
            uart_putc(c);
        }
	}
}
