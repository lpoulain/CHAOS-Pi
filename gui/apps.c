#include "libc.h"
#include "heap.h"
#include "display.h"
#include "disk.h"
#include "font.h"
#include "widgets.h"
#include "touchscreen.h"
#include "syscall.h"

void switch_el0();

extern Heap kheap;
extern uint *buffer;

extern uint __text_start__;
extern uint __rodata_start__;
extern uint __data_start__;
extern uint __data1_start__;
extern uint __bss_start__;
extern uint __stack_start_core0__;
extern uint __stack_start_core1__;
extern uint __stack_start_core2__;
extern uint __stack_start_core3__;
extern uint __debug_str__;
extern uint __debug_info__;
extern uint __debug_line__;
extern uint __debug_abbrev__;
extern uint _end;

/////////////////////////////////////////////////////////

void app_kernelheap_load(UserInterface *ui) { }

void app_kernelheap_wake_up(UserInterface *ui) {
	set_font(-1);
	heap_print(&kheap);
	print_cr();
	heap_print_pages(&kheap);
}

/////////////////////////////////////////////////////////

File *app_filesystem_files;

void app_filesystem_load(UserInterface *ui) {
	disk_load_mbr();

    app_filesystem_files = disk_ls(0);
}

void app_filesystem_wake_up(UserInterface *ui) {
	set_font(0);

    for (int i=0; i<30; i++) {
        if (app_filesystem_files[i].filename[0] == 0) continue;

        printf("%s\n", app_filesystem_files[i].filename);

/*        uart_puts("[");
        uart_puts(files[i].filename);
        uart_puts("]\n");*/
    }
}

/////////////////////////////////////////////////////////

uint32 *app_memory_ptr;
uint app_mem_x, app_mem_y;

void app_memory_refresh() {
	draw_rect(0, 0, 680, 266, 0);
	display_dump_mem(app_memory_ptr, 512, 0, 0);
}

void app_memory_up() { if (app_memory_ptr > 0) app_memory_ptr -= 128; app_memory_refresh(); }
void app_memory_down() { app_memory_ptr += 128;	app_memory_refresh(); }
void app_memory_text() { app_memory_ptr = &__text_start__; app_memory_refresh(); }
void app_memory_rodata() { app_memory_ptr = &__rodata_start__; app_memory_refresh(); }
void app_memory_data() { app_memory_ptr = &__data_start__; app_memory_refresh(); }
void app_memory_bss() { app_memory_ptr = &__bss_start__; app_memory_refresh(); }
void app_memory_stack0() { app_memory_ptr = &__stack_start_core0__; app_memory_refresh(); }
void app_memory_stack1() { app_memory_ptr = &__stack_start_core1__; app_memory_refresh(); }
void app_memory_stack2() { app_memory_ptr = &__stack_start_core2__; app_memory_refresh(); }
void app_memory_stack3() { app_memory_ptr = &__stack_start_core3__; app_memory_refresh(); }
void app_memory_debug_string() { app_memory_ptr = &__debug_str__; app_memory_refresh(); }
void app_memory_debug_info() { app_memory_ptr = &__debug_info__; app_memory_refresh(); }
void app_memory_debug_line() { app_memory_ptr = &__debug_line__; app_memory_refresh(); }
void app_memory_debug_abbrev() { app_memory_ptr = &__debug_abbrev__; app_memory_refresh(); }
void app_memory_heap() { app_memory_ptr = &_end; app_memory_refresh(); }

void app_memory_load(UserInterface *ui) {
	set_font(0);
	UI_add_button_x_y(ui, "UP", 700, 50, 90, 80, app_memory_up, '[');
	UI_add_button_x_y(ui, "DOWN", 700, 150, 90, 80, app_memory_down, ']');

	UI_add_button_x_y(ui, "Text", 300, 300, 100, 40, app_memory_text, 't');
	UI_add_button_x_y(ui, "RO Data", 420, 300, 100, 40, app_memory_rodata, 'r');
	UI_add_button_x_y(ui, "Data", 540, 300, 100, 40, app_memory_data, 'd');
//	UI_add_button_x_y(ui, "BSS", 660, 300, 100, 40, app_memory_bss, 'b');
	UI_add_button_x_y(ui, "Heap", 660, 300, 100, 40, app_memory_heap, 'h');

	UI_add_button_x_y(ui, "Stack 0", 300, 360, 100, 40, app_memory_stack0, '0');
	UI_add_button_x_y(ui, "Stack 1", 420, 360, 100, 40, app_memory_stack1, '1');
	UI_add_button_x_y(ui, "Stack 2", 540, 360, 100, 40, app_memory_stack2, '2');
	UI_add_button_x_y(ui, "Stack 3", 660, 360, 100, 40, app_memory_stack3, '3');

	UI_add_button_x_y(ui, "Debug String", 300, 420, 100, 40, app_memory_debug_string, 's');
	UI_add_button_x_y(ui, "Debug Info", 420, 420, 100, 40, app_memory_debug_info, 'i');
	UI_add_button_x_y(ui, "Debug Line", 540, 420, 100, 40, app_memory_debug_line, 'l');
	UI_add_button_x_y(ui, "Debug Abbrev", 660, 420, 100, 40, app_memory_debug_abbrev, 'a');
}

void app_memory_wake_up(UserInterface *ui) {
	set_font(-1);

	app_mem_x = -1;
	app_mem_y = -1;
	app_memory_ptr = 0;
	app_memory_refresh();

	print_set_cursor(0, 300);
	printf("Text:         0x%X\n", &__text_start__);
	printf("RO Data:      0x%X\n", &__rodata_start__);
	printf("Data:         0x%X\n", &__data_start__);
	printf("BSS:          0x%X\n", &__bss_start__);
	printf("Heap:         0x%X\n", &_end);
	printf("Stack core 0: 0x%X\n", &__stack_start_core0__);
	printf("Stack core 1: 0x%X\n", &__stack_start_core1__);
	printf("Stack core 2: 0x%X\n", &__stack_start_core2__);
	printf("Stack core 3: 0x%X\n", &__stack_start_core3__);
	printf("Debug String: 0x%X\n", &__debug_str__);
	printf("Debug Line:   0x%X\n", &__debug_line__);
	printf("Debug Info:   0x%X\n", &__debug_info__);
	printf("Debug Abbrev: 0x%X\n", &__debug_abbrev__);
	printf("Display:      0x%X\n", buffer);

	set_font(0);
	UI_draw(ui);
	set_font(-1);
}

/////////////////////////////////////////////////////////

void app_fern_load(UserInterface *ui) { }

void app_fern_wake_up(UserInterface *ui) {
    switch_el0();

    sys_draw_wallpaper();

    int64 x = 0, y = 0, a, b, c, d, e, f, new_x, new_y;

    uint p;
    uint64 rnd = 234234;
    int x_px, y_px;
    int i;

    for (i=0; i<20000; i++) {
        rnd = (rnd * 214013L + 2531011L) >> 16;

        p = rnd % 100;
        if (p == 0) {
            a = 0;
            b = 0;
            c = 0;
            d = 1600;
            e = 0;
            f = 0;
        } else if (p > 0 && p < 86) {
            a = 8500;
            b = 400;
            c = -400;
            d = 8500;
            e = 0;
            f = 16000;
        } else if (p >= 86 && p < 93) {
            a = 2000;
            b = -2600;
            c = 2300;
            d = 2200;
            e = 0;
            f = 16000;
        } else {
            a = -1500;
            b = 2800;
            c = 2600;
            d = 2400;
            e = 0;
            f = 4400;
        }

        new_x = (x*a + y*b) / 10000 + e;
        new_y = (x*c + y*d) / 10000 + f;

        x = new_x;
        y = new_y;

        y_px = (int)(x*60/10000) + 240;
        x_px = (int)(y*60/10000) + 150;

//printf("x=%d y=%d rnd=%d\n", x_px, y_px, rnd);

        sys_draw_pixel(x_px, y_px);
//        draw_pixel(x_px, y_px, 0xFF00);
    }	
}
