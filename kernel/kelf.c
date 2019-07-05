#include "libc.h"
#include "elf.h"
#include "kheap.h"
#include "uart.h"

extern uint __debug_str__;
extern uint __debug_str_end__;
extern uint __debug_info__;
extern uint __debug_info_end__;
extern uint __debug_line__;
extern uint __debug_line_end__;
extern uint __debug_abbrev__;
extern uint __debug_abbrev_end__;

#define ELF_SECTION_TEXT            0
#define ELF_SECTION_STRING          1
#define ELF_SECTION_DEBUG_INFO      2
#define ELF_SECTION_DEBUG_LINE      3
#define ELF_SECTION_DEBUG_ABBREV    4
#define ELF_SECTION_DEBUG_STR       5

Elf *kernel_ELF() {
	Elf *elf = (Elf*)kmalloc(sizeof(Elf));

	elf->section[ELF_SECTION_DEBUG_INFO].start = (uint8*)&__debug_info__;
	elf->section[ELF_SECTION_DEBUG_INFO].end = (uint8*)&__debug_info_end__;

	elf->section[ELF_SECTION_DEBUG_LINE].start = (uint8*)&__debug_line__;
	elf->section[ELF_SECTION_DEBUG_LINE].end = (uint8*)&__debug_line_end__;

	elf->section[ELF_SECTION_DEBUG_STR].start = (uint8*)&__debug_str__;
	elf->section[ELF_SECTION_DEBUG_STR].end = (uint8*)&__debug_str_end__;

	elf->section[ELF_SECTION_DEBUG_ABBREV].start = (uint8*)&__debug_abbrev__;
	elf->section[ELF_SECTION_DEBUG_ABBREV].end = (uint8*)&__debug_abbrev_end__;

	uart_printf("Debug abbreb: 0x%X -> 0x%X\n", elf->section[ELF_SECTION_DEBUG_ABBREV].start, elf->section[ELF_SECTION_DEBUG_ABBREV].end);

	return elf;
}
