#ifndef __MBOX_H
#define __MBOX_H

#include "libc.h"

int mbox_call(unsigned char ch);

#define MBOX_GET_MAC				0x00010003
#define MBOX_GET_SERIAL				0x00010004
#define MBOX_GET_VC_MEMORY			0x00010006
#define MBOX_SET_CLOCK_RATE			0x00038002
#define MBOX_SET_PHYSICAL_DISPLAY	0x00048003
#define MBOX_GET_PHYSICAL_DISPLAY	0x00040004
#define MBOX_SET_VIRTUAL_DISPLAY	0x00048004
#define MBOX_SET_VIRTUAL_OFFSET		0x00048009
#define MBOX_SET_DEPTH				0x00048005
#define MBOX_SET_PIXEL_ORDER		0x00048006
#define MBOX_ALLOCATE_BUFFER		0x00040001
#define MBOX_GET_PITCH				0x00040008
#define MBOX_GET_CLOCK_RATE			0x00030002

void mbox_add3(uint tag, uint val1, uint val2, uint val3);
void mbox_add4(uint tag, uint val1, uint val2, uint val3, uint val4);
void mbox_add5(uint tag, uint val1, uint val2, uint val3, uint val4, uint val5);

int mbox_send();
uint mbox_get(int offset);
uint64 mbox_get_long(int offset);
int mbox_call(unsigned char ch);

#endif
