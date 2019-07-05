#include "libc.h"
#include "mbox.h"
#include "stdarg.h"

#define MMIO_BASE       0x3F000000

#define AUX_ENABLE      ((volatile unsigned int*)(MMIO_BASE+0x00215004))
#define AUX_MU_IO       ((volatile unsigned int*)(MMIO_BASE+0x00215040))
#define AUX_MU_IER      ((volatile unsigned int*)(MMIO_BASE+0x00215044))
#define AUX_MU_IIR      ((volatile unsigned int*)(MMIO_BASE+0x00215048))
#define AUX_MU_LCR      ((volatile unsigned int*)(MMIO_BASE+0x0021504C))
#define AUX_MU_MCR      ((volatile unsigned int*)(MMIO_BASE+0x00215050))
#define AUX_MU_LSR      ((volatile unsigned int*)(MMIO_BASE+0x00215054))
#define AUX_MU_MSR      ((volatile unsigned int*)(MMIO_BASE+0x00215058))
#define AUX_MU_SCRATCH  ((volatile unsigned int*)(MMIO_BASE+0x0021505C))
#define AUX_MU_CNTL     ((volatile unsigned int*)(MMIO_BASE+0x00215060))
#define AUX_MU_STAT     ((volatile unsigned int*)(MMIO_BASE+0x00215064))
#define AUX_MU_BAUD     ((volatile unsigned int*)(MMIO_BASE+0x00215068))

/* PL011 UART registers */
#define UART0_DR        ((volatile unsigned int*)(MMIO_BASE+0x00201000))
#define UART0_FR        ((volatile unsigned int*)(MMIO_BASE+0x00201018))
#define UART0_IBRD      ((volatile unsigned int*)(MMIO_BASE+0x00201024))
#define UART0_FBRD      ((volatile unsigned int*)(MMIO_BASE+0x00201028))
#define UART0_LCRH      ((volatile unsigned int*)(MMIO_BASE+0x0020102C))
#define UART0_CR        ((volatile unsigned int*)(MMIO_BASE+0x00201030))
#define UART0_IMSC      ((volatile unsigned int*)(MMIO_BASE+0x00201038))
#define UART0_ICR       ((volatile unsigned int*)(MMIO_BASE+0x00201044))

#define GPFSEL0         ((volatile unsigned int*)(MMIO_BASE+0x00200000))
#define GPFSEL1         ((volatile unsigned int*)(MMIO_BASE+0x00200004))
#define GPFSEL2         ((volatile unsigned int*)(MMIO_BASE+0x00200008))
#define GPFSEL3         ((volatile unsigned int*)(MMIO_BASE+0x0020000C))
#define GPFSEL4         ((volatile unsigned int*)(MMIO_BASE+0x00200010))
#define GPFSEL5         ((volatile unsigned int*)(MMIO_BASE+0x00200014))
#define GPSET0          ((volatile unsigned int*)(MMIO_BASE+0x0020001C))
#define GPSET1          ((volatile unsigned int*)(MMIO_BASE+0x00200020))
#define GPCLR0          ((volatile unsigned int*)(MMIO_BASE+0x00200028))
#define GPLEV0          ((volatile unsigned int*)(MMIO_BASE+0x00200034))
#define GPLEV1          ((volatile unsigned int*)(MMIO_BASE+0x00200038))
#define GPEDS0          ((volatile unsigned int*)(MMIO_BASE+0x00200040))
#define GPEDS1          ((volatile unsigned int*)(MMIO_BASE+0x00200044))
#define GPHEN0          ((volatile unsigned int*)(MMIO_BASE+0x00200064))
#define GPHEN1          ((volatile unsigned int*)(MMIO_BASE+0x00200068))
#define GPPUD           ((volatile unsigned int*)(MMIO_BASE+0x00200094))
#define GPPUDCLK0       ((volatile unsigned int*)(MMIO_BASE+0x00200098))
#define GPPUDCLK1       ((volatile unsigned int*)(MMIO_BASE+0x0020009C))

void uart_init()
{
    register unsigned int r;
    
    /* initialize UART */
    *UART0_CR = 0;         // turn off UART0

    mbox_add5(MBOX_SET_CLOCK_RATE, 12, 8, 2, 4000000, 0);
    mbox_send();

    /* map UART0 to GPIO pins */
    r=*GPFSEL1;
    r&=~((7<<12)|(7<<15)); // gpio14, gpio15
    r|=(4<<12)|(4<<15);    // alt0
    *GPFSEL1 = r;
    *GPPUD = 0;            // enable pins 14 and 15
    r=150; while(r--) { asm volatile("nop"); }
    *GPPUDCLK0 = (1<<14)|(1<<15);
    r=150; while(r--) { asm volatile("nop"); }
    *GPPUDCLK0 = 0;        // flush GPIO setup

    *UART0_ICR = 0x7FF;    // clear interrupts
    *UART0_IBRD = 2;       // 115200 baud
    *UART0_FBRD = 0xB;
    *UART0_LCRH = 0b11<<5; // 8n1
    *UART0_CR = 0x301;     // enable Tx, Rx, FIFO
}


/**
 * Send a character
 */
void uart_putc(unsigned int c) {
    /* wait until we can send */
    do{asm volatile("nop");}while(*UART0_FR&0x20);
    /* write the character to the buffer */
    *UART0_DR=c;
}

/**
 * Receive a character
 */
char uart_getc() {
    char r;
    /* wait until something is in the buffer */
    do{asm volatile("nop");}while(*UART0_FR&0x10);
    /* read it and return */
    r=(char)(*UART0_DR);
    /* convert carrige return to newline */
    return r=='\r'?'\n':r;
}

/**
 * Display a string
 */
void uart_puts(char *s) {
    while(*s) {
        /* convert newline to carrige return + newline */
        if(*s=='\n')
            uart_putc('\r');
        uart_putc(*s++);
    }
}

void uart_puts_n(char *s, int n) {
    for (int i=0; i<n; i++) {
        uart_putc(s[i]);
    }
}

/**
 * Display a binary value in hexadecimal
 */
void uart_hex(unsigned int d) {
    unsigned int n;
    int c;
    for(c=28;c>=0;c-=4) {
        // get highest tetrad
        n=(d>>c)&0xF;
        // 0-9 => '0'-'9', 10-15 => 'A'-'F'
        n+=n>9?0x37:0x30;
        uart_putc(n);
    }
}

void uart_hex_byte(unsigned char d) {
    unsigned int n;
    int c;
    for(c=4;c>=0;c-=4) {
        // get highest tetrad
        n=(d>>c)&0xF;
        // 0-9 => '0'-'9', 10-15 => 'A'-'F'
        n+=n>9?0x37:0x30;
        uart_putc(n);
    }
}

void uart_hex_long(uint64 d) {
	unsigned int n;
    int c;
    for(c=60;c>=0;c-=4) {
        // get highest tetrad
        n=(d>>c)&0xF;
        // 0-9 => '0'-'9', 10-15 => 'A'-'F'
        n+=n>9?0x37:0x30;
        uart_putc(n);
    }
}

void uart_int(int nb) {
    char tmp[12];
    itoa(nb, (char *)&tmp);
    uart_puts(tmp);
}

void uart_dump_mem(void *ptr, int nb_bytes) {
    unsigned char *addr = (unsigned char *)ptr;
    unsigned char *addr2;

    int offset = (uint)(uint64)addr % 16;
    int i, j;

    uart_hex_long((uint64)(addr - offset));
    addr2 = addr;
    uart_puts("  ");
    for (i=0; i< 16-offset; i++) {
        uart_hex_byte(*addr++);
        uart_putc(' ');
    }

    uart_puts("    ");

    for (i=0; i< 16-offset; i++) {
        uart_putc(ascii[*addr2++]);
    }

    uart_putc('\n');

    int nb_rows = (nb_bytes - 16 + offset) / 16 + 1;
    for (j=0; j<nb_rows; j++) {
        uart_hex_long((uint64)addr);
        addr2 = addr;
	    uart_puts("  ");
        for (i=0; i<16; i++) {
	        uart_hex_byte(*addr++);
    	    uart_putc(' ');
    	}

    	uart_puts("    ");
    	for (i=0; i<16; i++) {
            uart_putc(ascii[*addr2++]);
        }
		uart_putc('\n');
    }
}

void uart_printf(const char *format, ...) {
    va_list args;
    va_start(args, format);

    int x = 0, y;
    char *str;
    char tmp[16], ch;
    uint ip;
    uint8 *ip_ptr;

    for (; *format != 0; ++format) {
        if (*format == '\n') {
            uart_putc('\n');
        } else if (*format == '%') {
            ++format;
            switch(*format) {
                case 'd':
                    uart_int(va_arg( args, int ));
                    break;
                case 's':
                    str = (char*)(uint64)va_arg( args, int );
                    x = 0;
                    y = 0;
                    ch = str[y];
                    while (ch != 0) {
                        while (ch != 0 && ch != 0x0A) ch = str[++y];
  
                        if (ch == 0x0A) {
                            uart_puts_n(str + x, y - x);

                            x = ++y;
//                            y = x;
                            ch = str[y];
                            uart_putc('\n');
                        } else {
                            uart_puts(str + x);
                        }
                    }
                    break;
                case 'x':
                    itoa_hex(va_arg(args, uint), (unsigned char*)&tmp);
                    uart_puts(tmp);
                    break;
                case 'X':
                    itoa_hex_64(va_arg(args, uint64), (unsigned char*)&tmp);
                    uart_puts(tmp);
                    break;
                case 'i':
                    ip = va_arg(args, unsigned int);
                    ip_ptr = (uint8*)&ip;
                    uart_int(ip_ptr[0]);
                    uart_putc('.');
                    uart_int(ip_ptr[1]);
                    uart_putc('.');
                    uart_int(ip_ptr[2]);
                    uart_putc('.');
                    uart_int(ip_ptr[3]);
                    break;
            }
        }
        else {
            uart_putc(*format);
        }
    }
}

void uart_print_current_stack() {
    uint64 *stack_end;

//    asm volatile ("mrs %0, SP" : "=r" (stack_end));

//    asm volatile("mrs %[result], SP_EL1" : [result] "=r" (stack_end));
     
//    register int i asm("ebx");
    uint64 *stack_start;
    uint64 fct_ptr;
    uint64 *sp;

    stack_end = get_sp();
//    stack_end -= 4;
    stack_start = stack_end + 1000;
    sp = stack_end;

    uart_dump_mem(stack_end - 32, 512);

    uart_puts("Stack pointer: ");
    uart_hex_long((uint64)stack_end);
    uart_putc('\n');

    while (sp >= stack_end && sp <= stack_start) {
        fct_ptr = sp[1];
        uart_hex_long(fct_ptr);
        uart_putc('\n');
        sp = (uint64*)sp[0];
        uart_hex_long((uint64)sp);
        uart_putc('\n');
    }
}

extern volatile unsigned char __end;
