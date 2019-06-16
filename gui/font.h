#ifndef __FONT_H
#define __FONT_H

#include "libc.h"

#define FONT_CHICAGO_12PT       0
#define FONT_CHICAGO_8PT        1
#define FONT_OLDWIZARD          2
#define FONT_TIMESNEW           3
#define FONT_ORIGINALBY         4
#define FONT_HELVETICA          5

typedef const unsigned char(FontBitmap)[128][35];

typedef struct {
        uint8 top;
        uint8 bottom;
        FontBitmap *bitmap;
} Font;

uint draw_font(unsigned char c, int x, int y);
uint draw_proportional_font(unsigned char c, int x, int y);
int set_font(int idx);
uint get_font_height();
uint get_font_string_size(char *str);

#endif
