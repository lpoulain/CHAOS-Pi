#include "libc.h"
#include "uart.h"
#include "mbox.h"
#include "display.h"
#include "touchscreen.h"

extern volatile unsigned int mbox[36];

#define MAXIMUM_SUPPORTED_POINTS 10
#define RPI_FIRMWARE_FRAMEBUFFER_GET_TOUCHBUF 0x0004000f
#define RPI_FIRMWARE_FRAMEBUFFER_SET_TOUCHBUF 0x0004801f

void no_op2(int x, int y) { }
void no_op4(int x, int y, int z, int t) { }
void no_op_status(enum TouchStatus status) { }

struct ft5406_regs {
	uint8_t device_mode;
	uint8_t gesture_id;
	uint8_t num_points;
	struct ft5406_touch {
		uint8_t xh;
		uint8_t xl;
		uint8_t yh;
		uint8_t yl;
		uint8_t pressure; /* Not supported */
		uint8_t area;     /* Not supported */
	} point[MAXIMUM_SUPPORTED_POINTS];

	uint8_t notused;
};

void *touchscreen_buffer;

struct ft5406_regs actual_regs;

void touchscreen_init() {
	mbox_add4(RPI_FIRMWARE_FRAMEBUFFER_SET_TOUCHBUF, 8, 8, (uint32)(uint64)&actual_regs, 0);
	mbox_send();

	touchscreen_buffer = &actual_regs;
}

int x1_px=0, y1_px=0, x2_px=0, y2_px=0, init_x1_px, init_y1_px, init_x2_px, init_y2_px;
//int nb_fingers = 0;

int touchscreen_x1() { return x1_px; }
int touchscreen_y1() { return y1_px; }
int touchscreen_x2() { return x2_px; }
int touchscreen_y2() { return y2_px; }

/*
The touchscreen_poll() goal is to capture the touch movements, categorize them as either swipe or pinch
(touch not implemented yet) and call the callback functions when appropriate:
- When the user first touches the screen with one finger (beginning of a swipe movement)
- When the user keeps swiping
- When the user first touches the screen with two+ fingers (beginning of a pinch)
- When the user keeps pinching in/out

When the user remove the finger(s) from the screen, the method returns
*/
enum TouchStatus touchscreen_poll(void (*first_touch)(int, int),
								  void (*swipe)(int, int),
								  void (*first_pinch)(int, int, int, int),
								  void (*pinch)(int, int, int, int)) {

	if (touchscreen_buffer == NULL) return NOTOUCH;

	enum TouchStatus status = NOTOUCH;
	struct ft5406_regs regs;
	uint64 *source, *target;
	source = (uint64*)&regs;
	target = (uint64*)touchscreen_buffer;

	for (;;) {

		wait_cycles(1000);

		// Copy the value of the touchscreen buffer
		source[0] = target[0];
		source[1] = target[1];
		source[2] = target[2];
		source[3] = target[3];
		source[4] = target[4];
		source[5] = target[5];
		source[6] = target[6];
		source[7] = target[7];

//		memcpy(&regs, touchscreen_buffer, sizeof(struct ft5406_regs));
		if (actual_regs.num_points == 99) continue;

//		actual_regs.num_points = 99;
		if (regs.num_points == 99) continue;

		if (regs.num_points == 0) {
			// If the user released the finger(s)
			if (status != NOTOUCH) {
				return status;
			}

//			printf("[0] Status=%d\n", status);
			continue;
		}

		int new_x1_px = (((int) regs.point[0].xh & 0xf) << 8) + regs.point[0].xl;
		int new_y1_px = (((int) regs.point[0].yh & 0xf) << 8) + regs.point[0].yl;

//printf("[%d] Status=%d, Ptr=%d %d\n", regs.num_points, status, new_x1_px, new_y1_px);

		if (regs.num_points == 1) {
			if (status == NOTOUCH) {
				if (new_x1_px < 10 && new_y1_px >= 430) return EXIT;

				first_touch(new_x1_px, new_y1_px);
				status = TAP;
			}
			else if (status == SWIPE || status == TAP) {
				swipe(new_x1_px, new_y1_px);
				status = SWIPE;
			}

			continue;
		}

		// Two or more touch points
		// Only look at the first two and handle as a pinch

		int new_x2_px = (((int) regs.point[1].xh & 0xf) << 8) + regs.point[1].xl;
		int new_y2_px = (((int) regs.point[1].yh & 0xf) << 8) + regs.point[1].yl;

		if (status == NOTOUCH) {
			first_pinch(new_x1_px, new_y1_px, new_x2_px, new_y2_px);
			status = PINCH;
			continue;
		}

		status = PINCH;

		pinch(new_x1_px, new_y1_px, new_x2_px, new_y2_px);
	}
}
