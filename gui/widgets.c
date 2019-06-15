#include "libc.h"
#include "kheap.h"
#include "widgets.h"
#include "display.h"
#include "font.h"

extern uint screen_width, screen_height;

UserInterface *UI_init() {
	UserInterface *ui = kmalloc(sizeof(UserInterface));
	ui->first_button = 0;
	ui->last_button = 0;

	return ui;
}

void UI_draw_button(Button *button) {
	draw_frame(button->x, button->y, button->x + button->width - 1, button->y + button->height - 1, 0xFFFFFF);
	draw_rect(button->x + 1, button->y + 1, button->x + button->width - 2, button->y + button->height - 2, 0);
	draw_string(button->label, button->x + (button->width - get_font_string_size(button->label)) / 2, button->y + (button->height - get_font_height()) / 2);
}

void UI_draw(UserInterface *ui) {
	Button *button = ui->first_button;

	while (button != 0) {
		UI_draw_button(button);
		button = button->next;
	}
}

void UI_select_button(Button *button) {
	draw_rect(button->x + 1, button->y + 1, button->x + button->width - 2, button->y + button->height - 2, 0x88);
	draw_string(button->label, button->x + (button->width - get_font_string_size(button->label)) / 2, button->y + (button->height - get_font_height()) / 2);
}

void UI_add_button_x_y(UserInterface *ui, char *label, uint x, uint y, uint width, uint height, void (*callback)()) {
	Button *button = (Button*)kmalloc(sizeof(Button));

	if (ui->first_button == 0) {
		ui->first_button = button;
	} else {
		ui->last_button->next = button;
	}

	button->label = label;
	button->x = x;
	button->y = y;
	button->width = width;
	button->height = height;
	button->next = 0;
	button->callback = callback;
	ui->last_button = button;
}

void UI_add_button(UserInterface *ui, char *label, void (*callback)()) {
	Button *button = (Button*)kmalloc(sizeof(Button));

	if (ui->first_button == 0) {
		ui->first_button = button;

		button->y = 0;
	} else {
		ui->last_button->next = button;
		button->y = ui->last_button->y + ui->last_button->height + 5;
	}

	button->x = 0;
	button->width = screen_width;
	button->height = 80;
	button->label = label;
	button->callback = callback;
	button->next = 0;
	ui->last_button = button;
}

Button *UI_find_button(UserInterface *ui, int screen_pos_x, int screen_pos_y) {
	Button *button = ui->first_button;

	while (button != 0) {
		if (screen_pos_x >= button->x &&
			screen_pos_x < button->x + button->width &&
			screen_pos_y >= button->y &&
			screen_pos_y < button->y + button->height) {

			return button;
		}

		button = button->next;
	}

	return NULL;
}
