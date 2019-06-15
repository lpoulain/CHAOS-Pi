#include "libc.h"
#include "display.h"
#include "touchscreen.h"
#include "kheap.h"
#include "font.h"
#include "touchscreen.h"
#include "mandelbrot.h"
#include "apps.h"
#include "widgets.h"

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

Application *app_mandelbrot, *app_filesystem, *app_kernelheap, *app_memory;

void app_init() {
	app_mandelbrot = app_register("Mandelbrot", mandelbrot_init, mandelbrot_first_touch, mandelbrot_swipe, mandelbrot_first_pinch, mandelbrot_pinch, mandelbrot_process_touch_event);
	app_filesystem = app_register("Filesystem", app_filesystem_init, no_op2, no_op2, no_op4, no_op4, no_op_status);
	app_kernelheap = app_register("Kernel Heap", app_kernelheap_init, no_op2, no_op2, no_op4, no_op4, no_op_status);
	app_memory = app_register("Memory", app_memory_init, app_memory_first_touch, no_op2, no_op4, no_op4, app_memory_process_touch_event);
}

void app_launch(Application *app) {
	screen_clear();
	print_set_cursor(0, 0);
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

void kernelheap_callback() {
	app_launch(app_kernelheap);
}

void memory_callback() {
	app_launch(app_memory);
}

///////////////////////////////////////////////////////////////////////
// Main Launcher
///////////////////////////////////////////////////////////////////////

uint screen_width, screen_height;
UserInterface *ui;

void screen_clear() {
	draw_rect(0, 0, screen_width, screen_height, 0);
}

int screen_pos_x = -1;
int screen_pos_y = -1;

void screen_first_swipe(int x, int y) {
	draw_pixel(x, y, 0xFFFFFF);
	screen_pos_x = x;
	screen_pos_y = y;

	Button *button = UI_find_button(ui, x, y);
	if (button != NULL) UI_select_button(button);
}

void launcher_init_screen() {
	screen_clear();
	set_font(0);

	UI_draw(ui);
}

void launcher_init() {
	screen_width = get_display_width();
	screen_height = get_display_height();

	ui = UI_init();
	app_init();

	UI_add_button(ui, "Mandelbrot", mandelbrot_callback);
	UI_add_button(ui, "Filesystem", filesystem_callback);
	UI_add_button(ui, "Kernel Heap", kernelheap_callback);
	UI_add_button(ui, "Memory", memory_callback);

	launcher_init_screen();

	while (1) {
		enum TouchStatus status = touchscreen_poll(screen_first_swipe, no_op2, no_op2, no_op4);

		if (status == TAP || status == SWIPE) {
			Button *button = UI_find_button(ui, screen_pos_x, screen_pos_y);

			if (button != NULL) {
				button->callback();
				launcher_init_screen();
			}

		}
	}
}
