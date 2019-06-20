#include "libc.h"
#include "uart.h"
#include "display.h"
#include "touchscreen.h"
#include "math.h"
#include "font.h"
#include "widgets.h"

/*
void bounce(int nb) {
    int dir_x = (nb & 0x1) * 2 - 1;
    int dir_y = (nb & 0x2) - 1;
    char c = 'A' + nb;
    uint * lbuffer[8][8];

    int x = rand(nb + 5) % 600 + 200;
    int y = rand(nb + 100) % 400 + 100;
    int offset, limit = width * 5;

//    if (nb == 2) crash();

    for (int nb=0;; nb++) {
        for (int i=x; i<x+8; i++) {
            for (int j=y; j<y+8; j++) {
                offset = (j)*pitch/4 + i;
                lbuffer[i-x][j-y] = buffer[offset];
            }
        }
        draw_char(c, x, y);

        wait_msec(8000);
        for (int i=x; i<x+8; i++) {
            for (int j=y; j<y+8; j++) {
                offset = (j)*pitch/4 + i;
                buffer[offset] = lbuffer[i-x][j-y];
            }
        }


        if (x == 0 && dir_x == -1) { if (nb > limit) break; dir_x = 1; }
        if (y == 0 && dir_y == -1) { if (nb > limit) break; dir_y = 1; }
        if (x == width - 8 - 1 && dir_x == 1) { if (nb > limit) break; dir_x = -1; }
        if (y == height - 8 - 1 && dir_y == 1) { if (nb > limit) break; dir_y = -1; }
        x += dir_x;
        y += dir_y;
    }
}
*/
float x1, x2, y1, y2, x_delta, y_delta;
int max_iter = 100;

int width_px;
int height_px;

void mandelbrot_calibrate() {
	x_delta = (x2 - x1) / get_display_width();
	y_delta = (y2 - y1) / get_display_height();
}

void mandelbrot_reset() {
	width_px = get_display_width();
    height_px = get_display_height();

	x1 = -2.4;
	x2 = 1.6;
	y1 = -1.2;
	y2 = 1.2;
    max_iter = 100;

	mandelbrot_calibrate();
}

int ref_x1_px, ref_y1_px, ref_x2_px, ref_y2_px;
int new_x1_px, new_y1_px, new_x2_px, new_y2_px;

// We want to make sure the screen keeps the same ratio
void set_proportional_rect(int *x1_px, int *y1_px, int *x2_px, int *y2_px) {
	int frame_width = *x2_px - *x1_px;
	int frame_height = *y2_px - *y1_px;
	int new_value, frame_delta;

	// If the frame is too wide
	if (1.0 * frame_width / frame_height < 1.0 * width_px / height_px) {
		new_value = (int)(1.0 * frame_width * height_px / width_px);
		frame_delta = (new_value - frame_height) / 2;
		*y1_px -= frame_delta;
		*y2_px += frame_delta;
	} else
	// If the frame is too thin
	{
		new_value = (uint)(1.0 * frame_height * width_px / height_px);
		frame_delta = (new_value - frame_width) / 2;
		*x1_px = *x1_px - frame_delta;
		*x2_px = *x2_px - frame_delta;
	}

    if (*x2_px < *x1_px) {
        new_value = *x1_px;
        *x1_px = *x2_px;
        *x2_px = new_value;
    }

    if (*y2_px < *y1_px) {
        new_value = *y1_px;
        *y1_px = *y2_px;
        *y2_px = new_value;
    }
}

void mandelbrot_first_pinch(int x1_px, int y1_px, int x2_px, int y2_px) {
	ref_x1_px = min(x1_px, x2_px);
	ref_y1_px = min(y1_px, y2_px);
	ref_x2_px = max(x1_px, x2_px);
	ref_y2_px = max(y1_px, y2_px);

	set_proportional_rect(&ref_x1_px, &ref_y1_px, &ref_x2_px, &ref_y2_px);
}

void mandelbrot_pinch(int x1_px, int y1_px, int x2_px, int y2_px) {
	new_x1_px = min(x1_px, x2_px);
	new_y1_px = min(y1_px, y2_px);
	new_x2_px = max(x1_px, x2_px);
	new_y2_px = max(y1_px, y2_px);

	set_proportional_rect(&new_x1_px, &new_y1_px, &new_x2_px, &new_y2_px);
	draw_map_backup_screen(ref_x1_px, ref_y1_px, ref_x2_px, ref_y2_px, new_x1_px, new_y1_px, new_x2_px, new_y2_px);
}

void mandelbrot_first_touch(int x_px, int y_px) {
	ref_x1_px = x_px;
	ref_y1_px = y_px;
}

void mandelbrot_swipe(int x_px, int y_px) {
	new_x1_px = x_px;
	new_y1_px = y_px;

	// Left hand side: change the number of iterations
	if (new_x1_px < 50 && ref_x1_px < 50) {
		max_iter += (ref_y1_px - new_y1_px);
        if (max_iter < 100) max_iter = 100;

		draw_restore_backup(100, 5, 300, 20);
        int px = draw_string("Iterations: ", 100, 5);
        draw_int(max_iter, 100 + px, 5);
		return;
	}

	draw_backup_screen(new_x1_px - ref_x1_px, new_y1_px - ref_y1_px);
}

// Computes the Mandelbrot set for a particular core
// Because the RPi 3 has 4 cores, the function divides the screen in 4
void mandelbrot(int core) {
    int k;
    double xi, xj, ci, cj, tmpi, tmpj;
    uint color;
    uint i, j = 0;
    uint i_start = core & 0x1, j_start = (core & 0x2) / 2;
    double abs_x = 0;

    for (i=i_start; i<width_px; i+=2) {
        for (j=j_start; j<height_px; j+=2) {
            xi = 0;
            xj = 0;

            ci = x1 + i * x_delta;
            cj = y1 + j * y_delta;
            color = 0;

            for (k=0; k<max_iter; k++) {
                tmpi = ((xi * xi)) - ((xj*xj)) + ci;
                tmpj = ((2 * xi * xj)) + cj;

                xi = tmpi;
                xj = tmpj;

                abs_x = (xi * xi + xj * xj);
                if (abs_x >= 4) break;
            }

            if (k >= max_iter) {
                color = 0;
            } else {
                color = 0xFF + (k + 1 - ln(log2((uint64)abs_x))) * 5;
/*                // Switch red and blue
                color = ((color & 0xFF) << 16) |
                    (color & 0xFF00) |
                    ((color & 0xFF0000) >> 8);*/
            }
    
    		draw_pixel(i, j, color);
        }
    }

    core_complete(core);
//    bounce(nb);
}

// Once the use has completed a swipe operation, compute the new coordinates
void mandelbrot_move_coordinates() {
	// If the swipe was on the left hand side, do nothing
	if (new_x1_px < 50 && ref_x1_px < 50) return;

	x1 += x_delta * (ref_x1_px - new_x1_px);
	x2 += x_delta * (ref_x1_px - new_x1_px);
	y1 += y_delta * (ref_y1_px - new_y1_px);
	y2 += y_delta * (ref_y1_px - new_y1_px);

	mandelbrot_calibrate();
}

// Once the user has completed a pinch operation, compute the new coordinates
void mandelbrot_zoom() {
    float zoom_x_delta = 1.0 * (ref_x2_px - ref_x1_px) / (new_x2_px - new_x1_px);
    float zoom_y_delta = 1.0 * (ref_y2_px - ref_y1_px) / (new_y2_px - new_y1_px);

    x2 = x1 + (zoom_x_delta * (width_px-new_x1_px) + ref_x1_px) * x_delta;
    x1 = x1 + (zoom_x_delta * (-new_x1_px) + ref_x1_px) * x_delta;

//    y2 = y1 + (zoom_y_delta * (height_px-new_y1_px) + ref_y1_px) * y_delta;
    y1 = y1 + (zoom_y_delta * (-new_y1_px) + ref_y1_px) * y_delta;
    // Make sure to keep the height / width ratio
    y2 = y1 + (x2 - x1) * height_px / width_px;

    mandelbrot_calibrate();
}

void mandelbrot_launch1() { mandelbrot(1); }
void mandelbrot_launch2() { mandelbrot(2); }
void mandelbrot_launch3() { mandelbrot(3); }

// Displays the Mandelbrot set using all 4 cores
void draw_mandelbrot_multicore() {
    init_sync_cores();

    CoreExecute(1, mandelbrot_launch1);
    CoreExecute(2, mandelbrot_launch2);
    CoreExecute(3, mandelbrot_launch3);
    mandelbrot(0);

    sync_cores();

    display_screen_backup();
    draw_string("Welcome to CHAOS!", 350, 460);
}


void mandelbrot_tap() {
    if (ref_x1_px > width_px - 50 && ref_y1_px < 50) {
        mandelbrot_reset();
        draw_mandelbrot_multicore();
    }
}

void mandelbrot_load(UserInterface *ui) { }

void mandelbrot_wake_up(UserInterface *ui) {
    mandelbrot_reset();
    set_font(0);

    draw_mandelbrot_multicore();
}

void mandelbrot_process_touch_event(enum TouchStatus status) {
    switch(status) {
        case TAP:
            mandelbrot_tap();
            break;
        case SWIPE:
            mandelbrot_move_coordinates();
            draw_mandelbrot_multicore();
            break;
        case PINCH:
            mandelbrot_zoom();
            draw_mandelbrot_multicore();
            break;
        default:
            break;
    }
}
