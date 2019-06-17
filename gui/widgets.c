#include "libc.h"
#include "kheap.h"
#include "widgets.h"
#include "display.h"
#include "font.h"
#include "touchscreen.h"
#include "uart.h"

uint8 is_hardware_emulated();

extern uint screen_width, screen_height;

UserInterface *UI_init() {
	UserInterface *ui = kmalloc(sizeof(UserInterface));
	ui->first_button = 0;
	ui->last_button = 0;

	return ui;
}

uint ui_pos_x, ui_pos_y;
UserInterface *current_ui;

void UI_first_touch(int x, int y) {
	ui_pos_x = x;
	ui_pos_y = y;

	Button *button = UI_find_button(current_ui, ui_pos_x, ui_pos_y);
	if (button != NULL) {
		set_font(0);
		UI_select_button(button);
		set_font(-1);
	}
}

uint UI_nb_buttons(UserInterface *ui) {
	uint nb = 0;

	Button *button = ui->first_button;
	while (button != NULL) {
		nb++;
		button = button->next;
	}

	return nb;
}

void UI_process_touch_event(enum TouchStatus status) {
	if (status == TAP || status == SWIPE)
	{
		Button *button = UI_find_button(current_ui, ui_pos_x, ui_pos_y);
		if (button != NULL) {
			int font_idx = set_font(0);
			UI_draw_button(button);
			set_font(font_idx);

			button->callback();
		}
	}
}

enum TouchStatus UI_process_UART_event(UserInterface *ui, char c) {

	Button *button = ui->first_button;
	while (button != NULL) {
		if (button->hotkey == c) {
			button->callback();
			return TAP;
		}

		button = button->next;
	}

	return NOTOUCH;
}

enum TouchStatus UI_handle_event(UserInterface *ui) {
	current_ui = ui;

	// If there is no touchscreen, use UART input
	if (is_hardware_emulated()) {
		char c = uart_getc();
		uart_putc(c);
		if (c == 27) return EXIT;

		return UI_process_UART_event(ui, c);
	}

	enum TouchStatus status = touchscreen_poll(UI_first_touch, no_op2, no_op4, no_op4, no_op_status);
	UI_process_touch_event(status);

	return status;
}

void UI_draw_button(Button *button) {
	draw_frame(button->x, button->y, button->x + button->width - 1, button->y + button->height - 1, 0xFFFFFF);
	draw_rect(button->x + 1, button->y + 1, button->x + button->width - 2, button->y + button->height - 2, 0x222222);
	draw_string(button->label, button->x + (button->width - get_font_string_size(button->label)) / 2, button->y + (button->height - get_font_height()) / 2);

	if (is_hardware_emulated())
		draw_char(button->hotkey, button->x + 1, button->y + 1);
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

void UI_add_button_x_y(UserInterface *ui, char *label, uint x, uint y, uint width, uint height, void (*callback)(), char hotkey) {
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
	button->hotkey = hotkey;
	ui->last_button = button;
}

void UI_add_button(UserInterface *ui, char *label, void (*callback)(), char hotkey) {
	Button *button = (Button*)kmalloc(sizeof(Button));

	if (ui->first_button == 0) {
		ui->first_button = button;

		button->y = 10;
	} else {
		ui->last_button->next = button;
		button->y = ui->last_button->y + ui->last_button->height + 10;
	}

	button->x = 10;
	button->width = screen_width - 20;
	button->height = 80;
	button->label = label;
	button->callback = callback;
	button->next = 0;
	button->hotkey = hotkey;
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
