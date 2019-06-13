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

extern void scheduler();
extern uint *buffer;

void touchscreen_init();
extern unsigned char _end;

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

    set_font(0);
    draw_string("Welcome to CHAOS!", 350, 460);

//    draw_mandelbrot();

//    disk_load_file_index();

/*    disk_ls(0, &_end);
    DirEntry *dirs = (DirEntry*)&_end;

    for (int i=0; i<20; i++) {
        draw_string(dirs[i].filename, 0, i*8);

        uart_puts(dirs[i].filename);
        uart_puts("\n");
    }
*/

    launcher_init();

/*    disk_load_mbr();

    File *files = disk_ls(0);

    for (int i=0; i<30; i++) {
        if (files[i].filename[0] == 0) continue;

        printf("%s\n", files[i].filename);

        uart_puts("[");
        uart_puts(files[i].filename);
        uart_puts("]\n");
    }
*/
    int sector = 0;

	while (1) {
        char c = uart_getc();

        if (c == 0x41) {
            sector--;
            sd_readblock(0,&_end,1);
            uart_dump_mem(&_end, 512);

/*            sp -= 512;
            draw_rect(0, 0, 684, 264, 0);
            display_dump_mem(sp, 512, 0, 0);*/

        }
        else if (c == 0x42) {
            sector++;
            sd_readblock(0,&_end,1);
            uart_dump_mem(&_end, 512);


/*            sp += 512;
            draw_rect(0, 0, 684, 264, 0);
            display_dump_mem(sp, 512, 0, 0);*/

        } else {
            uart_putc(c);
        }
	}
}
