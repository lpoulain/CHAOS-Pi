# Drivers

Those files implement access to the hardware, e.g. screen, disk, touchscreen. `sd.c` (reading blocks from the SD card) and `uart.c` (sending and receiving information to UART 0 and UART 1) come from [https://github.com/bztsrc/raspi3-tutorial]()

## The Official RPi 7" Touchscreen

The Official Raspberry Pi 7" Touchscreen is not that hard to take advantage of.

A good reference is its [Linux device driver](https://github.com/raspberrypi/linux/blob/rpi-4.14.y/drivers/input/touchscreen/rpi-ft5406.c), in particular the `ft5406_thread()` function. The Touchscreen driver needs to:

- Use the [Mailbox](mailbox.md) to set the base memory of the touchscreen. The message looks as follows:
    - Request code = `0x0004801F` (`RPI_FIRMWARE_FRAMEBUFFER_SET_TOUCHBUF`)
    - `0x00000008` (size of the request)
    - `0x00000008` (size of the response)
    - The address of the touchscreen base memory. This should point to a structure similar to `ft5406_regs`
- Instead of handling an interrupt each time a touchscreen event occurs, this driver constantly polls the touchscreen. Because the touchscreen buffer can always change, the code copies it and then reads its values
- Part of the values is the number of points, e.g. the number of fingers on the screen (apparently it can handle up to 10)
- The driver then sets the number of points to 99. This allows to see if there is any new information (a number of points set to 99 means no new information)
- If there is at least one point, the driver reads the point information and processes it appropriately

The driver implemented in [touchscreen.c]() takes multiple callback functions as parameters. The driver focuses on determining if there is a new event, if it is a single tap, a swipe, a two-finger pinch and calls the right callback function when need be:

- When the user first touches the screen with one finger (beginning of a swipe movement)
- When the user keeps swiping
- When the user first touches the screen with two+ fingers (beginning of a pinch)
- When the user keeps pinching in/out
