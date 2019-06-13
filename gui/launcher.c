#include "libc.h"
#include "display.h"
#include "touchscreen.h"
#include "kheap.h"
#include "font.h"
#include "touchscreen.h"
#include "mandelbrot.h"
#include "disk.h"

void no_op2(int x, int y) { }
void no_op4(int x, int y, int z, int t) { }
void no_op_status(enum TouchStatus status) { }

////////////////////////////////////////////////////////
// Applications
////////////////////////////////////////////////////////

void screen_clear();

typedef struct {
	char *name;

	void (*init)();
	void (*first_swipe)(int, int);
	void (*swipe)(int, int);
	void (*first_pinch)(int, int, int, int);
  	void (*pinch)(int, int, int, int);
  	void (*process_touch_event)(enum TouchStatus);
} Application;

Application *app_register(char *name,
						  void (*init)(), void (*first_swipe)(int, int),
						  void (*swipe)(int, int),
						  void (*first_pinch)(int, int, int, int),
						  void (*pinch)(int, int, int, int),
						  void (*process_touch_event)(enum TouchStatus)) {

	Application *app = (Application *)kmalloc(sizeof(Application));
	app->name = name;
	app->init = init;
	app->first_swipe = first_swipe;
	app->swipe = swipe;
	app->first_pinch = first_pinch;
	app->pinch = pinch;
	app->process_touch_event = process_touch_event;

	return app;
}

Application *app_mandelbrot;
Application *app_filesystem;

void filesystem_init() {
	disk_load_mbr();

    File *files = disk_ls(0);

    for (int i=0; i<30; i++) {
        if (files[i].filename[0] == 0) continue;

        printf("%s\n", files[i].filename);

        uart_puts("[");
        uart_puts(files[i].filename);
        uart_puts("]\n");
    }
}

void app_init() {
	app_mandelbrot = app_register("Mandelbrot", mandelbrot_init, mandelbrot_first_swipe, mandelbrot_swipe, mandelbrot_first_pinch, mandelbrot_pinch, mandelbrot_process_touch_event);
	app_filesystem = app_register("Filesystem", filesystem_init, no_op2, no_op2, no_op4, no_op4, no_op_status);
}

void app_launch(Application *app) {
	screen_clear();
	app->init();

	while (1) {
		enum TouchStatus status = touchscreen_poll(app->first_swipe, app->swipe, app->first_pinch, app->pinch);

		if (status == EXIT) return;

		app->process_touch_event(status);
	}
}

void mandelbrot_callback() {
	app_launch(app_mandelbrot);
}

void filesystem_callback() {
	app_launch(app_filesystem);
}

///////////////////////////////////////////////////////////////////////
// Main Launcher
///////////////////////////////////////////////////////////////////////

struct button_struct {
	uint x;
	uint y;
	uint width;
	uint height;
	char *label;
	void (*callback)();

	struct button_struct *next;
} button;

typedef struct button_struct Button;

Button *first_button = 0;
Button *last_button = 0;

uint screen_width, screen_height;

void screen_clear() {
	draw_rect(0, 0, screen_width, screen_height, 0);
}

void screen_draw_button (Button *button) {
	draw_frame(button->x, button->y, button->x + button->width - 1, button->y + button->height - 1, 0xFFFFFF);
	draw_string(button->label, button->x + (button->width - get_font_string_size(button->label)) / 2, button->y + (button->height - get_font_height()) / 2);
}

void screen_add_button(char *label, void (*callback)()) {
	Button *button = (Button*)kmalloc(sizeof(Button));

	if (first_button == 0) {
		first_button = button;
		last_button = button;

		button->y = 0;
	} else {
		last_button->next = button;
		button->y = last_button->y + last_button->height + 5;
	}

	button->x = 0;
	button->width = screen_width;
	button->height = 80;
	button->label = label;
	button->callback = callback;
	button->next = 0;
	last_button = button;
}

int screen_pos_x = -1;
int screen_pos_y = -1;

void screen_first_swipe(int x, int y) {
	screen_pos_x = x;
	screen_pos_y = y;
}

void launcher_init_screen() {
	screen_clear();

	Button *button = first_button;

	while (button != 0) {
		screen_draw_button(button);
		button = button->next;
	}
}

void launcher_init() {
	screen_width = get_display_width();
	screen_height = get_display_height();

	app_init();

	screen_add_button("Mandelbrot", mandelbrot_callback);
	screen_add_button("Filesystem", filesystem_callback);

	launcher_init_screen();

	while (1) {
		enum TouchStatus status = touchscreen_poll(screen_first_swipe, no_op2, no_op2, no_op4);

		if (status == TAP) {
			Button *button = first_button;

			while (button != 0) {
				if (screen_pos_x >= button->x &&
					screen_pos_x < button->x + button->width &&
					screen_pos_y >= button->y &&
					screen_pos_y < button->y + button->height) {

					button->callback();

					launcher_init_screen();

					break;
				}

				button = button->next;
			}
		}
	}
}
