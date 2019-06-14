# Drivers

Those files implement access to the hardware, e.g. screen, disk, touchscreen. `sd.c` (reading blocks from the SD card) and `uart.c` (sending and receiving information to UART 0 and UART 1) come from [https://github.com/bztsrc/raspi3-tutorial]()

## The Mailbox

The Mailbox is the way to communicate with the GPU, and is fairly simple to implement. The Mailbox has multiple channels (see [https://github.com/raspberrypi/firmware/wiki/Mailboxes]()), but channel 8 is the one channel to use to communicate with the VideoCore (the GPU shipped with the Broadcom ARM processor on the Raspberry Pi board)

Because the ARM Core and the GPU are on a System-On-a-Chip (SoC), they share the same memory. Control of the Mailbox is done through a set of mailbox registers which can be found in memory at the GPIO base address + `0xB880`, so `0x3F00B880`. The messages sent to the mailbox are written in a memory structure (allocated by the software).

For instance, when setting up the video mode, the software needs to:

- Allocate a memory structure that will contain multiple messages to be sent to the VideoCore (see `display_init()` in [display.c](). See also [https://github.com/raspberrypi/firmware/wiki/Mailbox-property-interface]() for details about the messages supported by channel 8
- Use the Mailbox registers to tell the Mailbox the address of those messages, whether the Mailbox is ready to be used, tell it to process the messages and wait for a response code
- Some responses will be in the messages, such as the memory address of the screen
- From here, you can access said address directly to draw on the screen

Further detail on [https://github.com/raspberrypi/firmware/wiki/Accessing-mailboxes]()

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
