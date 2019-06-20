#include "libc.h"
#include "uart.h"
#include "mbox.h"
#include "display.h"
#include "kheap.h"
#include "sd.h"
#include "disk.h"
#include "kheap.h"
#include "font.h"
#include "launcher.h"
#include "debug_line.h"
#include "elf.h"
#include "debug.h"

extern void scheduler();
extern uint *buffer;

void touchscreen_init();
extern unsigned char _end;

uint mac;
uint8 is_hardware_emulated() { return (mac != 0); }

void kernel_main()
{
    uart_init();
    display_init();
    uart_puts("Welcome to CHAOS!\r\n");

    init_kheap();
    sd_init();

    touchscreen_init();

	// read the current level from system register
	unsigned long el;
    asm volatile ("mrs %0, CurrentEL" : "=r" (el));

    uart_puts("Current EL is: ");
    uart_hex((el>>2)&3);
    uart_puts("\n");		

    mbox_add4(0x00010005, 0, 8, 0, 0);
    mbox_send();
    uart_puts("ARM memory base address: 0x");
    uart_hex(mbox_get(5));
    uart_puts("\nARM memory size: 0x");
    uart_hex(mbox_get(6));
    uart_puts("\n");
 
	mbox_add4(MBOX_GET_VC_MEMORY, 0, 8, 0, 0);
	mbox_send();
	uart_puts("VC address: ");
    mac = mbox_get(6);
    uart_hex(mac);
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

    set_font(0);
    draw_string("Welcome to CHAOS!", 350, 460);

    switch_debug();
    set_font(-1);
    Elf *kelf = kernel_ELF();
    debug_info_load(kelf);

    launcher_init();

	while (1) {
        char c = uart_getc();
        uart_putc(c);
	}
}
