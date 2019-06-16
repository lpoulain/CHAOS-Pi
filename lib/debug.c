#include "libc.h"
#include "kheap.h"
#include "disk.h"
#include "debug_info.h"
#include "display.h"
#include "elf.h"

#define ROOT_DIR_CLUSTER    2

Elf *kernel_elf;
/*
unsigned char *symbols;
unsigned char *kernel_debug_line;
uint kernel_debug_line_size;
unsigned char *kernel_debug_info;
uint kernel_debug_info_size;
unsigned char *kernel_debug_abbrev;
uint kernel_debug_abbrev_size;
unsigned char *kernel_debug_str;*/

uint8 debug_flag = 0;

uint8 switch_debug() {
    debug_flag = 1 - debug_flag;
    return debug_flag;
}

uint8 is_debug() {
    return debug_flag;
}

// Loads the kernel symbols in memory
void init_debug() {
    debug_info_load(kernel_elf);
}
