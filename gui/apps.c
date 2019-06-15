#include "libc.h"
#include "heap.h"
#include "display.h"
#include "disk.h"
#include "font.h"
#include "widgets.h"
#include "touchscreen.h"

extern Heap kheap;

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
extern uint _end;

/////////////////////////////////////////////////////////

void app_kernelheap_init() {
	set_font(-1);
	heap_print(&kheap);
	print_cr();
	heap_print_pages(&kheap);
}

/////////////////////////////////////////////////////////

void app_filesystem_init() {
	set_font(0);
	disk_load_mbr();

    File *files = disk_ls(0);

    for (int i=0; i<30; i++) {
        if (files[i].filename[0] == 0) continue;

        printf("%s\n", files[i].filename);

/*        uart_puts("[");
        uart_puts(files[i].filename);
        uart_puts("]\n");*/
    }
}

/////////////////////////////////////////////////////////

UserInterface *memory_ui;
uint32 *app_memory_ptr;
uint app_mem_x, app_mem_y;

void app_memory_refresh() {
	draw_rect(0, 0, 680, 280, 0);
	display_dump_mem(app_memory_ptr, 512, 0, 0);
}

void app_memory_up() {
	if (app_memory_ptr > 0) app_memory_ptr -= 512;
	app_memory_refresh();
}

void app_memory_down() {
	app_memory_ptr += 512;
	app_memory_refresh();
}

void app_memory_text() {
	app_memory_ptr = &__text_start__;
	app_memory_refresh();
}

void app_memory_rodata() {
	app_memory_ptr = &__rodata_start__;
	app_memory_refresh();
}

void app_memory_data() {
	app_memory_ptr = &__data_start__;
	app_memory_refresh();
}

void app_memory_bss() {
	app_memory_ptr = &__bss_start__;
	app_memory_refresh();
}

void app_memory_stack0() {
	app_memory_ptr = &__stack_start_core0__;
	app_memory_refresh();
}

void app_memory_stack1() {
	app_memory_ptr = &__stack_start_core1__;
	app_memory_refresh();
}

void app_memory_stack2() {
	app_memory_ptr = &__stack_start_core2__;
	app_memory_refresh();
}

void app_memory_stack3() {
	app_memory_ptr = &__stack_start_core3__;
	app_memory_refresh();
}

void app_memory_debug_string() {
	app_memory_ptr = &__debug_str__;
	app_memory_refresh();
}

void app_memory_debug_info() {
	app_memory_ptr = &__debug_info__;
	app_memory_refresh();
}

void app_memory_heap() {
	app_memory_ptr = &_end;
	app_memory_refresh();
}

void app_memory_init() {
	set_font(-1);

	app_mem_x = -1;
	app_mem_y = -1;
	memory_ui = UI_init();
	app_memory_ptr = 0;
	app_memory_refresh();

	print_set_cursor(0, 300);
	printf("Text:         0x%X\n", &__text_start__);
	printf("RO Data:      0x%X\n", &__rodata_start__);
	printf("Data:         0x%X\n", &__data_start__);
	printf("BSS:          0x%X\n", &__bss_start__);
	printf("Stack core 0: 0x%X\n", &__stack_start_core0__);
	printf("Stack core 1: 0x%X\n", &__stack_start_core1__);
	printf("Stack core 2: 0x%X\n", &__stack_start_core2__);
	printf("Stack core 3: 0x%X\n", &__stack_start_core3__);
	printf("Debug String: 0x%X\n", &__debug_str__);
	printf("Debug Info:   0x%X\n", &__debug_info__);
	printf("Heap:         0x%X\n", &_end);

	UI_add_button_x_y(memory_ui, "UP", 700, 50, 90, 80, app_memory_up);
	UI_add_button_x_y(memory_ui, "DOWN", 700, 150, 90, 80, app_memory_down);

	UI_add_button_x_y(memory_ui, "Text", 300, 300, 100, 40, app_memory_text);
	UI_add_button_x_y(memory_ui, "RO Data", 420, 300, 100, 40, app_memory_rodata);
	UI_add_button_x_y(memory_ui, "Data", 540, 300, 100, 40, app_memory_data);
	UI_add_button_x_y(memory_ui, "BSS", 660, 300, 100, 40, app_memory_bss);

	UI_add_button_x_y(memory_ui, "Stack 0", 300, 360, 100, 40, app_memory_stack0);
	UI_add_button_x_y(memory_ui, "Stack 1", 420, 360, 100, 40, app_memory_stack1);
	UI_add_button_x_y(memory_ui, "Stack 2", 540, 360, 100, 40, app_memory_stack2);
	UI_add_button_x_y(memory_ui, "Stack 3", 660, 360, 100, 40, app_memory_stack3);

	UI_add_button_x_y(memory_ui, "Debug String", 300, 420, 100, 40, app_memory_debug_string);
	UI_add_button_x_y(memory_ui, "Debug Info", 420, 420, 100, 40, app_memory_debug_info);
	UI_add_button_x_y(memory_ui, "Heap", 540, 420, 100, 40, app_memory_heap);

	set_font(0);
	UI_draw(memory_ui);
	set_font(-1);
}

void app_memory_first_touch(int x, int y) {
	app_mem_x = x;
	app_mem_y = y;

	Button *button = UI_find_button(memory_ui, app_mem_x, app_mem_y);
	if (button != NULL) {
		set_font(0);
		UI_select_button(button);
		set_font(-1);
	}
}

void app_memory_process_touch_event(enum TouchStatus status) {
	if (status == TAP || status == SWIPE)
	{
		Button *button = UI_find_button(memory_ui, app_mem_x, app_mem_y);
		if (button != NULL) {
			button->callback();
			set_font(0);
			UI_draw_button(button);
			set_font(-1);
		}
	}
}
