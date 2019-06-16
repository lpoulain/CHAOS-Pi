#include "libc.h"
#include "display.h"
#include "touchscreen.h"
#include "kheap.h"
#include "font.h"
#include "touchscreen.h"
#include "mandelbrot.h"
#include "apps.h"
#include "widgets.h"
#include "uart.h"

////////////////////////////////////////////////////////
// Applications
////////////////////////////////////////////////////////

#define APP_FLAG_LOADED		1

void screen_clear();

typedef struct {
	char *name;
	UserInterface *ui;
	uint8 flags;

	void (*load)(UserInterface *);
	void (*wake_up)(UserInterface *);
	void (*first_touch)(int, int);
	void (*swipe)(int, int);
	void (*first_pinch)(int, int, int, int);
  	void (*pinch)(int, int, int, int);
  	void (*process_touch_event)(enum TouchStatus);
} Application;

Application *app_register(char *name,
						  void (*load)(UserInterface *),
						  void (*wake_up)(UserInterface *),
						  void (*first_touch)(int, int),
						  void (*swipe)(int, int),
						  void (*first_pinch)(int, int, int, int),
						  void (*pinch)(int, int, int, int),
						  void (*process_touch_event)(enum TouchStatus)) {

	Application *app = (Application *)kmalloc(sizeof(Application));

	app->name = name;
	app->flags = 0;
	app->ui = UI_init();
	app->load = load;
	app->wake_up = wake_up;
	app->first_touch = first_touch;
	app->swipe = swipe;
	app->first_pinch = first_pinch;
	app->pinch = pinch;
	app->process_touch_event = process_touch_event;

	return app;
}

Application *app_mandelbrot, *app_filesystem, *app_kernelheap, *app_memory;

void app_init() {
	app_mandelbrot = app_register("Mandelbrot", mandelbrot_load, mandelbrot_wake_up, mandelbrot_first_touch, mandelbrot_swipe, mandelbrot_first_pinch, mandelbrot_pinch, mandelbrot_process_touch_event);
	app_filesystem = app_register("Filesystem", app_filesystem_load, app_filesystem_wake_up, no_op2, no_op2, no_op4, no_op4, no_op_status);
	app_kernelheap = app_register("Kernel Heap", app_kernelheap_load, app_kernelheap_wake_up, no_op2, no_op2, no_op4, no_op4, no_op_status);
	app_memory = app_register("Memory", app_memory_load, app_memory_wake_up, no_op2, no_op2, no_op4, no_op4, no_op_status);
}

void app_launch(Application *app) {
	screen_clear();
	print_set_cursor(0, 0);

	if (!(app->flags & APP_FLAG_LOADED)) {
		app->load(app->ui);
		app->flags |= APP_FLAG_LOADED;
	}

	app->wake_up(app->ui);

	// The app is using a button-based UI
	if (UI_nb_buttons(app->ui) > 0) {
		while (1) {
			enum TouchStatus status = UI_handle_event(app->ui);
			if (status == EXIT) return;
		}
	} else {
		while (1) {
			enum TouchStatus status = touchscreen_poll(app->first_touch, app->swipe, app->first_pinch, app->pinch);
			if (status == EXIT) return;
			app->process_touch_event(status);
		}
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
UserInterface *launcher_ui;

void screen_clear() {
	draw_rect(0, 0, screen_width, screen_height, 0);
}

int screen_pos_x = -1;
int screen_pos_y = -1;

void screen_first_touch(int x, int y) {
	draw_pixel(x, y, 0xFFFFFF);
	screen_pos_x = x;
	screen_pos_y = y;

	Button *button = UI_find_button(launcher_ui, x, y);
	if (button != NULL) UI_select_button(button);
}

void launcher_init_screen() {
	screen_clear();
	set_font(0);

	UI_draw(launcher_ui);
}

void launcher_init() {
	screen_width = get_display_width();
	screen_height = get_display_height();

	launcher_ui = UI_init();
	app_init();

	UI_add_button(launcher_ui, "Mandelbrot", mandelbrot_callback, '1');
	UI_add_button(launcher_ui, "Filesystem", filesystem_callback, '2');
	UI_add_button(launcher_ui, "Kernel Heap", kernelheap_callback, '3');
	UI_add_button(launcher_ui, "Memory", memory_callback, '4');

	launcher_init_screen();

	while (1) {
		enum TouchStatus status = UI_handle_event(launcher_ui);
		if (status == TAP || status == SWIPE)
			launcher_init_screen();
	}
/*		enum TouchStatus status = touchscreen_poll(screen_first_touch, no_op2, no_op2, no_op4);

		if (status == TAP || status == SWIPE) {
			Button *button = UI_find_button(ui, screen_pos_x, screen_pos_y);

			if (button != NULL) {
				button->callback();
				launcher_init_screen();
			}

		}
	}*/
}
