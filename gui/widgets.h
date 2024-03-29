#ifndef __WIDGETS_H
#define __WIDGETS_H

#include "libc.h"
#include "touchscreen.h"

struct button_struct {
	uint x;
	uint y;
	uint width;
	uint height;
	char *label;
	void (*callback)();
	char hotkey;

	struct button_struct *next;
} button;

typedef struct button_struct Button;

typedef struct {
	Button *first_button;
	Button *last_button;
} UserInterface;

UserInterface *UI_init();
void UI_draw(UserInterface *ui);
void UI_add_button(UserInterface *ui, char *label, void (*callback)(), char hotkey);
void UI_add_button_x_y(UserInterface *ui, char *label, uint x, uint y, uint width, uint height, void (*callback)(), char hotkey);
void UI_draw_button(Button *);
enum TouchStatus UI_handle_event(UserInterface *);
void UI_select_button(Button *button);
Button *UI_find_button(UserInterface *ui, int screen_pos_x, int screen_pos_y);
uint UI_nb_buttons(UserInterface *ui);

#endif
