#include "libc.h"
#include "mbox.h"
#include "uart.h"
#include "font.h"
#include "stdarg.h"

#define WIDTH       800
#define HEIGHT      480

uint width, height, pitch;
uint *buffer;
uint backup_buffer[WIDTH][HEIGHT];

uint cursor_x, cursor_y;

void display_init() {
	mbox_add4(MBOX_SET_PHYSICAL_DISPLAY, 8, 8, 800, 480);  // Resolution: 800x480
	mbox_add4(MBOX_SET_VIRTUAL_DISPLAY, 8, 8, 800, 480);   // Virtual resolution: 800x480
	mbox_add4(MBOX_SET_VIRTUAL_OFFSET, 8, 8, 0, 0);        // Virtual offset: 0x0
	mbox_add3(MBOX_SET_DEPTH, 4, 4, 32);                   // Number of colors: true color
	mbox_add3(MBOX_SET_PIXEL_ORDER, 4, 4, 0);              // BGR instead of RGB (because for some reason it doesn't work for the RPi but does for qemu)
	mbox_add4(MBOX_ALLOCATE_BUFFER, 8, 8, 4096, 0);        // Allocate the buffer memory for the screen (and retrieve its location)
	mbox_add3(MBOX_GET_PITCH, 4, 4, 0);                    // Get the number of bytes per line

	mbox_send();

	width = mbox_get(5);
	height = mbox_get(6);
	pitch = mbox_get(33);
	buffer = (uint*)(uint64)(mbox_get(28) & 0x3FFFFFFF);
    cursor_x = 0;
    cursor_y = 0;
}

void set_cursor(uint x, uint y) {
    cursor_x = x;
    cursor_y = y;
}

void cursor_cr() {
    cursor_x = 0;
    cursor_y += get_font_height();
}

uint get_display_width() { return WIDTH; }
uint get_display_height() { return HEIGHT; }
uint get_display_pitch() { return pitch; }
uint *get_display_buffer() { return buffer; }

void draw_pixel(uint x, uint y, uint color) {
    int off = (y)*pitch/4 + x;
    buffer[off] = color;
}

void draw_frame(uint tx, uint ty, uint lx, uint ly, uint color) {
    for (int i=tx; i<=lx; i++) {
        draw_pixel(i, ly, color);
        draw_pixel(i, ty, color);
    }

    for (int i=ty; i<ly; i++) {
        draw_pixel(lx, i, color);
        draw_pixel(tx, i, color);
    }
}

void undo_frame(uint tx, uint ty, uint lx, uint ly) {
    for (int i=tx; i<=lx; i++) {
        draw_pixel(i, ly, backup_buffer[i][ly]);
        draw_pixel(i, ty, backup_buffer[i][ty]);
    }

    for (int i=ty; i<ly; i++) {
        draw_pixel(lx, i, backup_buffer[lx][i]);
        draw_pixel(tx, i, backup_buffer[tx][i]);
    }
}

void display_screen_backup() {
    for (int x=0; x<width; x++) {
        for (int y=0; y<height; y++) {
            backup_buffer[x][y] = buffer[y*pitch/4 + x];
        }
    }
}

void draw_rect_multi(int x, int y, unsigned int r, unsigned int g, unsigned int b) {
    unsigned int R, G, B;

    for (unsigned int i=0; i<256; i++) {
        for (unsigned int j=0; j<256; j++) {
            if (r == 0) {
                R = i;
                if (g == 0) {
                    G = j;
                    B = b;
                }
                else {
                    G = g;
                    B = j;
                }
            }
            else {
                R = r;
                G = i;
                B = j;
            }
            int off = (j+y)*pitch/4 + i + x;
            unsigned int color = (B << 16) | (G << 8) | R;

            buffer[off] = color;
        }
    }
}

void animate(int x) {
    for (int y=0; y<1000; y++) {
        wait_msec(8000);
        draw_proportional_font(x, y, 'A');
    }
}

uint draw_string(const char *str, int x, int y) {
    int len = strlen(str);
    int x_pos = x;

    for (int i=0; i<len; i++) {
        x_pos += draw_font(str[i], x_pos, y);
    }

    return x_pos;
}

uint draw_string_n(const char *str, int x, int y, int len) {
    int x_pos = x;

    for (int i=0; i<len; i++) {
        x_pos += draw_font(str[i], x_pos, y);
    }

    return x_pos;
}

uint draw_char(char c, int x, int y) {
    return draw_font(c, x, y);
}

uint draw_hex(char b, int x, int y) {
    unsigned char str[5];
    ctoa_hex_0x(b, (unsigned char *)&str);
    return draw_string((char *)&str, x, y);
}

// Prints a character in hex format, without the "0x"
uint draw_hex2(char b, int x, int y) {
    unsigned char str[3];
    ctoa_hex(b, (unsigned char *)&str);
    return draw_string((char *)&str, x, y);
}

// Prints the value of a pointer (in hex) at a particular
// row and column on the screen
uint draw_ptr(void *ptr, int x, int y) {
    unsigned char str[16];
    itoa_hex_0x64((uint64)ptr, (unsigned char *)&str);
    return draw_string((char *)&str, x, y);
}

uint draw_int(int nb, int x, int y) {
    char tmp[12];
    itoa(nb, (char *)&tmp);
    return draw_string(tmp, x, y);
}

void draw_rect(int tx, int ty, int lx, int ly, uint color) {
    for (int i=tx; i<lx; i++) {
        for (int j=ty; j<ly; j++) {
            int off = (j)*pitch/4 + i;
            buffer[off] = color;
        }
    }
}

void display_dump_mem(void *ptr, int nb_bytes, uint x, uint y) {
    unsigned char *addr = (unsigned char *)ptr;

    int offset = (uint64)addr % 16;
    int i, j;

    int row = 0;
    draw_ptr(addr - offset, 0, 0);

//    draw_string("                                                  ", 10*8, row*8);
    for (i=0; i< 16-offset; i++) {
        draw_hex2(*addr, (offset * 3 + 20 + i*3)*8, 0);
        draw_char(ascii[*addr++], (69 + i)*8, 0);
    }

    int nb_rows = (nb_bytes - 16 + offset) / 16 + 1;
    for (j=0; j<nb_rows; j++) {
        draw_ptr(addr - 0, 0, (++row)*8);
//        draw_string("                                                  ", 10*8, row*8);
        for (i=0; i<16; i++) {
            draw_hex2(*addr, (20 + i*3)*8, row*8);
            draw_char(ascii[*addr++], (69 + i)*8, row*8);
        }
    }

}

void printf(const char *format, va_list args) {
    int x = 0, y;
    char *str;
    char tmp[3], ch;
    uint ip;
    uint8 *ip_ptr;

    for (; *format != 0; ++format) {
        if (*format == '\n') {
            cursor_cr();
        } else if (*format == '%') {
            ++format;
            switch(*format) {
                case 'd':
                    x+= draw_int(va_arg( args, int ), cursor_x, cursor_y);
                    break;
                case 's':
                    str = (char*)(uint64)va_arg( args, int );
//                    win->action->puts(win, str);
                    x = 0;
                    y = 0;
                    ch = str[y];
                    while (ch != 0) {
                        while (ch != 0 && ch != 0x0A) ch = str[++y];
  
                        if (ch == 0x0A) {
                            cursor_x += draw_string_n(str + x, y - x, cursor_x, cursor_y);

                            x = ++y;
//                            y = x;
                            ch = str[y];
                            cursor_cr();
                        } else {
                            cursor_x += draw_string(str + x, cursor_x, cursor_y);
                        }
                    }
                    break;
                case 'x':
                    cursor_x += draw_int(va_arg(args, long long), cursor_x, cursor_y);
                    break;
                case 'X':
                    ctoa_hex(va_arg(args, int), (unsigned char*)&tmp);
                    cursor_x += draw_string(tmp, cursor_x, cursor_y);
                    break;
                case 'i':
                    ip = va_arg(args, unsigned int);
                    ip_ptr = (uint8*)&ip;
                    cursor_x += draw_int(ip_ptr[0], cursor_x, cursor_y);
                    cursor_x += draw_char('.', cursor_x, cursor_y);
                    cursor_x += draw_int(ip_ptr[1], cursor_x, cursor_y);
                    cursor_x += draw_char('.', cursor_x, cursor_y);
                    cursor_x += draw_int(ip_ptr[2], cursor_x, cursor_y);
                    cursor_x += draw_char('.', cursor_x, cursor_y);
                    cursor_x += draw_int(ip_ptr[3], cursor_x, cursor_y);
                    break;
            }
        }
        else {
            cursor_x += draw_char(*format, cursor_x, cursor_y);
        }
    }
}


void draw_map_backup_screen(int x1_from, int y1_from, int x2_from, int y2_from,
                            int x1_to, int y1_to, int x2_to, int y2_to) {
    if (x1_from == x2_from) return;
    if (x1_to == x2_to) return;
    if (y1_from == y2_from) return;
    if (y1_to == y2_to) return;

    int x_backup, y_backup;

    float x_delta = 1.0 * (x2_from - x1_from) / (x2_to - x1_to);
    float y_delta = 1.0 * (y2_from - y1_from) / (y2_to - y1_to);

    for (int x=0; x<width; x++) {
        for (int y=0; y<height; y++) {
            x_backup = (int)(x_delta * (x - x1_to)) + x1_from;
            y_backup = (int)(y_delta * (y - y1_to)) + y1_from;

            if (x_backup < 0 || y_backup < 0 || x_backup >= width || y_backup >= height) {
                draw_pixel(x, y, 0);
            } else {
                draw_pixel(x, y, backup_buffer[x_backup][y_backup]);
            }
        }
    }
}

void draw_backup_screen(int x_offset, int y_offset) {
    int x_backup, y_backup;

    for (int x=0; x<width; x++) {
        for (int y=0; y<height; y++) {
            x_backup = x - x_offset;
            y_backup = y - y_offset;

            if (x_backup < 0 || y_backup < 0 || x_backup >= width || y_backup >= height) {
                draw_pixel(x, y, 0);
            } else {
                draw_pixel(x, y, backup_buffer[x_backup][y_backup]);
            }
        }
    }
}

void draw_restore_backup(int x_left, int y_bottom, int width, int height) {
    for (int x=x_left; x<x_left + width; x++) {
        for (int y=y_bottom; y<y_bottom + height; y++) {
            draw_pixel(x, y, backup_buffer[x][y]);
        }
    }
}
