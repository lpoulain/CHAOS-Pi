# The Mailbox

The Mailbox is the way to communicate with the GPU, and is fairly simple to implement. The Mailbox has multiple channels (see [https://github.com/raspberrypi/firmware/wiki/Mailboxes]()), but channel 8 is the one channel to use to communicate with the VideoCore (the GPU shipped with the Broadcom ARM processor on the Raspberry Pi board)

Because the ARM Core and the GPU are on a System-On-a-Chip (SoC), they share the same memory. Control of the Mailbox is done through a set of mailbox registers which can be found in memory at the GPIO base address + `0xB880`, so `0x3F00B880`. The messages sent to the mailbox are written in a memory structure (allocated by the software).

For instance, when setting up the video mode, the software needs to:

- Allocate a memory structure that will contain multiple messages to be sent to the VideoCore (see `display_init()` in [display.c](). See also [https://github.com/raspberrypi/firmware/wiki/Mailbox-property-interface]() for details about the messages supported by channel 8
- Use the Mailbox registers to tell the Mailbox the address of those messages, whether the Mailbox is ready to be used, tell it to process the messages and wait for a response code
- Some responses will be in the messages, such as the memory address of the screen
- From here, you can access said address directly to draw on the screen

Further detail on [https://github.com/raspberrypi/firmware/wiki/Accessing-mailboxes]()
