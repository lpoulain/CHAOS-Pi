#ifndef __MANDELBROT_H
#define __MANDELBROT_H

#include "libc.h"
#include "touchscreen.h"
#include "widgets.h"

void mandelbrot_load(UserInterface *ui);
void mandelbrot_wake_up(UserInterface *ui);
void mandelbrot_first_touch(int x_px, int y_px);
void mandelbrot_swipe(int x_px, int y_px);
void mandelbrot_first_pinch(int x1_px, int y1_px, int x2_px, int y2_px);
void mandelbrot_pinch(int x1_px, int y1_px, int x2_px, int y2_px);
void mandelbrot_process_touch_event(enum TouchStatus status);

#endif
