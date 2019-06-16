#ifndef __TOUCHSCREEN_H
#define __TOUCHSCREEN_H

#include "libc.h"

enum TouchStatus { NOTOUCH, TAP, SWIPE, PINCH, EXIT };

enum TouchStatus touchscreen_poll();
int touchscreen_x1();
int touchscreen_x2();
int touchscreen_y1();
int touchscreen_y2();

void no_op2(int x, int y);
void no_op4(int x, int y, int z, int t);
void no_op_status(enum TouchStatus status);

#endif
