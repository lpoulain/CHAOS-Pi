#include "libc.h"
#include "uart.h"
#include "display.h"
#include "font.h"

#define MMIO_BASE       	0x3F000000
#define IRQ_PENDING_1		(MMIO_BASE+0x0000B204)

extern unsigned char __EL1_stack_core0;

void syscall_handler(uint call_nb, uint64 arg1, uint64 arg2);

const char *entry_error_messages[] = {
	"SYNC_INVALID_EL1t",
	"IRQ_INVALID_EL1t",		
	"FIQ_INVALID_EL1t",		
	"ERROR_INVALID_EL1T",		

	"SYNC_INVALID_EL1h",		
	"IRQ_INVALID_EL1h",		
	"FIQ_INVALID_EL1h",		
	"ERROR_INVALID_EL1h",		

	"SYNC_INVALID_EL0_64",		
	"IRQ_INVALID_EL0_64",		
	"FIQ_INVALID_EL0_64",		
	"ERROR_INVALID_EL0_64",	

	"SYNC_INVALID_EL0_32",		
	"IRQ_INVALID_EL0_32",		
	"FIQ_INVALID_EL0_32",		
	"ERROR_INVALID_EL0_32"	
};

const char *get_interrupt_type(uint type) {
	switch(type % 4) {
		case 0: return "Synchronous";
		case 1: return "IRQ";
		case 2: return "Fast IRQ";
		case 3: return "SError";
		default: return "n/a";
	}
}

const char *get_ESR_message(uint esr) {
	switch(esr >> 26) {
		case 0x0: return "Unknown reason";
		case 0x1: return "Trapped WFI or WFE";
		case 0x2: return "Trapped MCR or MRC access";
		case 0x4: return "Trapped MCRR or MRRC access";
		case 0x5: return "Trapped MCR or MRC access";
		case 0x6: return "Trapped LDC or STC access";
		case 0x20: return "Instruction Abort from lower EL";
		case 0x21: return "Instruction Abort in same EL";
		case 0x22: return "PC alignment fault exception";
		case 0x24: return "Data Abort from lower EL";
		case 0x25: return "Data Abort in same EL";
		case 0x26: return "SP alignment fault exception";
		case 0x2C: return "Floating point exception";
		default: return "n/a";
	}
}

const char *get_ESR_extra_message(uint esr) {
	switch(esr >> 26) {
		case 0x24:
		case 0x25:
			switch (esr & 0x3C) {
				case 0x0: return " - Address size fault";
				case 0x4: return " - Translation fault";
				case 0x8: return " - Access flag fault";
				case 0xC: return " - Permission fault";
				case 0x10:
				case 0x14:
					return " - Synchronous External abort";
				case 0x1C: return " - Memory access error";
				case 0x20: return " - Alignment fault";
				case 0x30: return " - TLB conflict fault";
			}
			return "";
		default: return "";
	}
}

void show_invalid_entry_message(uint type, uint64 esr, uint64 address, uint64 spsr, uint64 far)
{
	if ((esr & 0xFF000000) == 0x0000000056000000) {

		uint32 call_nb = esr & 0xFFFF;

		char **arg1 = (char**)(&esr + 1);
		char **arg2 = (char**)(&esr + 2);
//		uart_printf("System call %d 0x%X 0x%X\n", call_nb, *arg1, *arg2);
		syscall_handler(call_nb, (uint64)*arg1, (uint64)*arg2);
		return;
	} else {

	uart_puts("******************\nException: ");
	uart_puts(get_interrupt_type(type));
	uart_puts("\nESR:  0x");
	uart_hex_long(esr);
	uart_puts(" (");
	uart_puts(get_ESR_message(esr));
	uart_puts(get_ESR_extra_message(esr));
	uart_puts(")\nELR:  0x");
	uart_hex_long(address);
	uart_puts(" (address of the crash)\nSPSR: 0x");
	uart_hex_long(spsr);
	uart_puts(" (Saved Program Status Register)\nFAR:  0x");
	uart_hex_long(far);
	uart_puts(" (problematic address)\n");

//	uart_dump_mem(get_sp(), 128);

	print_set_cursor(0, 430);
	set_font(-1);
	printf("Exception: %s\n", get_interrupt_type(type));
	printf("ESR:  0x%X (%s%s)\n", esr, get_ESR_message(esr), get_ESR_extra_message(esr));
	printf("ELR:  0x%X (address of the crash)\n", address);
	printf("SPSR: 0x%X (Saved Program Status Register)\n", spsr);
	printf("FAR:  0x%X (problematic address)\n", far);
  }
for (;;);
/*	if ((esr & 0xFF000000) == 0x0000000056000000) {
		uint32 arg = esr & 0xFFFF;
		printf("Argument: %d", arg);
		uart_puts("Argument: 0x");
		uart_hex(arg);
		uart_putc('\n');

		uart_dump_mem(&__EL1_stack_core0 - 512, 512);

		char **sys_arg = (char**)(&esr + 1);
		uart_puts(*sys_arg);

		asm volatile("eret");
	}*/
}

void handle_irq(void)
{
	uart_puts("IRQ ");
	unsigned int irq = get32(IRQ_PENDING_1);
	uart_hex(irq);
	uart_putc('\n');
}

void RPi_FiqFuncAddr(void)
{
	unsigned int irq = get32(IRQ_PENDING_1);
	uart_puts("IRQ ");
	uart_hex(irq);
	uart_putc('\n');
}
