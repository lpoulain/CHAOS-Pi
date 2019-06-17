#include "libc.h"
#include "elf.h"
#include "kheap.h"

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

	elf->section[ELF_SECTION_DEBUG_INFO].start = &__debug_info__;
	elf->section[ELF_SECTION_DEBUG_INFO].end = &__debug_info_end__;

	elf->section[ELF_SECTION_DEBUG_LINE].start = &__debug_line__;
	elf->section[ELF_SECTION_DEBUG_LINE].end = &__debug_line_end__;

	elf->section[ELF_SECTION_DEBUG_STR].start = &__debug_str__;
	elf->section[ELF_SECTION_DEBUG_STR].end = &__debug_str_end__;

	elf->section[ELF_SECTION_DEBUG_ABBREV].start = &__debug_abbrev__;
	elf->section[ELF_SECTION_DEBUG_ABBREV].end = &__debug_abbrev_end__;

	return elf;
}
