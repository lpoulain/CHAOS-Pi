#include "libc.h"
#include "uart.h"

#define MMIO_BASE       0x3F000000

/* mailbox message buffer */
volatile unsigned int  __attribute__((aligned(16))) mbox[36];

#define VIDEOCORE_MBOX  (MMIO_BASE+0x0000B880)
#define MBOX_READ       ((volatile unsigned int*)(VIDEOCORE_MBOX+0x0))
#define MBOX_POLL       ((volatile unsigned int*)(VIDEOCORE_MBOX+0x10))
#define MBOX_SENDER     ((volatile unsigned int*)(VIDEOCORE_MBOX+0x14))
#define MBOX_STATUS     ((volatile unsigned int*)(VIDEOCORE_MBOX+0x18))
#define MBOX_CONFIG     ((volatile unsigned int*)(VIDEOCORE_MBOX+0x1C))
#define MBOX_WRITE      ((volatile unsigned int*)(VIDEOCORE_MBOX+0x20))
#define MBOX_REQUEST    0x00000000
#define MBOX_RESPONSE   0x80000000
#define MBOX_FULL       0x80000000
#define MBOX_EMPTY      0x40000000
#define MBOX_TAG_LAST   0x00000000

#define MBOX_CHANNEL_FRAMEBUFFER    1
#define MBOX_CHANNEL_PROP           8

/**
 * Make a mailbox call. Returns 0 on failure, non-zero on success
 */
int mbox_call(unsigned char ch)
{
    unsigned int r = (((unsigned int)((unsigned long)&mbox)&~0xF) | (ch&0xF));
    /* wait until we can write to the mailbox */
    do{asm volatile("nop");}while(*MBOX_STATUS & MBOX_FULL);
    /* write the address of our message to the mailbox with channel identifier */
    *MBOX_WRITE = r;
    /* now wait for the response */
    while(1) {
        /* is there a response? */
        do{asm volatile("nop");}while(*MBOX_STATUS & MBOX_EMPTY);
        /* is it a response to our message? */
        if(r == *MBOX_READ)
            /* is it a valid successful response? */
            return mbox[1]==MBOX_RESPONSE;
    }
    return 0;
}

uint nb_messages = 2;

void mbox_add3(uint tag, uint val1, uint val2, uint val3) {
    mbox[nb_messages++] = tag;
    mbox[nb_messages++] = val1;
    mbox[nb_messages++] = val2;
    mbox[nb_messages++] = val3;
}

void mbox_add4(uint tag, uint val1, uint val2, uint val3, uint val4) {
    mbox_add3(tag, val1, val2, val3);
    mbox[nb_messages++] = val4;
}

void mbox_add5(uint tag, uint val1, uint val2, uint val3, uint val4, uint val5) {
    mbox_add4(tag, val1, val2, val3, val4);
    mbox[nb_messages++] = val5;
}

int mbox_send() {
    mbox[nb_messages++] = MBOX_TAG_LAST;

    mbox[0] = nb_messages * 4;
    mbox[1] = MBOX_REQUEST;

    int result = mbox_call(8);
    nb_messages = 2;
    return result;
}

uint mbox_get(int offset) {
    return mbox[offset];
}

uint64 mbox_get_long(int offset) {
    uint64 *ptr = (uint64*)(&(mbox[offset]));
    return *ptr;
}
