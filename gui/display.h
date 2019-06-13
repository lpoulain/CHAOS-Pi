#ifndef __DISPLAY_H
#define __DISPLAY_H

#include "stdarg.h"

void display_init();
uint get_display_width();
uint get_display_height();
uint get_display_pitch();
uint *get_display_buffer();
void draw_rect_multi(int x, int y, unsigned int r, unsigned int g, unsigned int b);
void animate(int x);
void draw_mandelbrot();
void display_dump_mem(void *ptr, int nb_bytes, uint x, uint y);
void draw_pixel(uint x, uint y, uint color);
void draw_rect(int tx, int ty, int lx, int ly, uint color);
void draw_frame(uint tx, uint ty, uint lx, uint ly, uint color);
void undo_frame(uint tx, uint ty, uint lx, uint ly);
void display_screen_backup();
void draw_map_backup_screen(int x1_from, int y1_from, int x2_from, int y2_from, int x1_to, int y1_to, int x2_to, int y2_to);
void draw_backup_screen(int x_offset, int y_offset);
void draw_restore_backup(int x_left, int y_bottom, int width, int height);

void print_cr();
void print_set_cursor(uint x, uint y);
void printf(const char *format, ...);
int draw_string(const char *str, uint x, uint y);
int draw_int(int nb, int x, int y);

#endif
