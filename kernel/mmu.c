#include "uart.h"
#include "kheap.h"
#include "mbox.h"

void enable_mmu_tables (uint64 *, uint64 *);

#define MMIO_BASE           0x3F000000
#define PAGESIZE    4096

// granularity
#define PT_PAGE     0b11        // 4k granule
#define PT_BLOCK    0b01        // 2M granule
// accessibility
#define PT_KERNEL   (0<<6)      // privileged, supervisor EL1 access only
#define PT_USER     (1<<6)      // unprivileged, EL0 access allowed
#define PT_RW       (0<<7)      // read-write
#define PT_RO       (1<<7)      // read-only
#define PT_AF       (1<<10)     // accessed flag
#define PT_NX       (1UL<<54)   // no execute
// shareability
#define PT_OSH      (2<<8)      // outter shareable
#define PT_ISH      (3<<8)      // inner shareable
// defined in MAIR register
#define PT_MEM      (0<<2)      // normal memory
#define PT_DEV      (1<<2)      // device MMIO
#define PT_NC       (2<<2)      // non-cachable

#define TTBR_CNP    1

// get addresses from linker
extern volatile unsigned char __data_start__;
extern volatile unsigned char _end;
extern volatile unsigned char __rodata_start__;

/**
 * Set up page translation tables and enable virtual memory
 */

uint64 *ttbr0_l1, *ttbr0_l2, *ttbr0_l3_current;
uint64 *ttbr1_l1, *ttbr1_l2, *ttbr1_l3;

uint ttbr0_l2_idx, ttbr0_l3_idx, ttbr1_l2_idx;

void mmu_add_mapping(uint64 value) {
    ttbr0_l3_current[ttbr0_l3_idx++] = value;

    if (ttbr0_l3_idx >= 512) {
        ttbr0_l3_current = (uint64*)kmalloc_pages(1, "TTBR0 Level 3");
        clear_pages(ttbr0_l3_current, 1);

        ttbr0_l2[ttbr0_l2_idx++] = (uint64)ttbr0_l3_current |
            PT_PAGE |     // we have area in it mapped by pages
            PT_AF |       // accessed flag
            PT_USER |     // non-privileged
            PT_ISH |      // inner shareable
            PT_MEM;       // normal memory

        ttbr0_l3_idx = 0;
    }
}

void mmu_init()
{
    mbox_add4(MBOX_GET_VC_MEMORY, 0, 8, 0, 0);
    mbox_send();
    uint64 vc_mem = (uint64)mbox_get(5);

    ttbr0_l1 = (uint64*)kmalloc_pages(1, "TTBR0 Level 1");
    ttbr0_l2 = (uint64*)kmalloc_pages(2, "TTBR0 Level 2");

//    ttbr0_l3_current = (uint64*)kmalloc_pages(1, "TTBR0 Level 3");
    ttbr1_l1 = (uint64*)kmalloc_pages(1, "TTBR1 Level 1");
    ttbr1_l2 = (uint64*)kmalloc_pages(1, "TTBR1 Level 2");
    ttbr1_l3 = (uint64*)kmalloc_pages(1, "TTBR1 Level 3");

    clear_pages(ttbr0_l1, 1);
    clear_pages(ttbr0_l2, 1);
    clear_pages(ttbr0_l3_current, 1);
    clear_pages(ttbr1_l1, 1);
    clear_pages(ttbr1_l2, 1);
    clear_pages(ttbr1_l3, 1);

    ttbr0_l2_idx = 0;
    ttbr0_l3_idx = 0;
    ttbr1_l2_idx = 0;
    uint64 addr;

    for (addr = 0; addr < (uint64)(&__rodata_start__); addr += 0x200000) {
        ttbr0_l2[ttbr0_l2_idx++] = addr |
        PT_BLOCK |
        PT_AF |
        PT_ISH |
        PT_MEM;
    }

    ttbr0_l2[ttbr0_l2_idx++] = (uint64)&__rodata_start__ |
        PT_BLOCK |
        PT_AF |
//        PT_USER |
        PT_ISH |
        PT_MEM;

addr += 0x200000;

    for (;addr < vc_mem; addr += 0x200000) {
        ttbr0_l2[ttbr0_l2_idx++] = addr |
        PT_BLOCK |
        PT_AF |
        PT_ISH |
        PT_MEM;
    }

    for (; addr < 0x3F000000; addr += 0x200000) {
        ttbr0_l2[ttbr0_l2_idx++] = addr |
        PT_BLOCK |
        PT_AF |
        PT_ISH |
        PT_NC;
    }

    for (; addr < 0x40200000; addr += 0x200000) {
        ttbr0_l2[ttbr0_l2_idx++] = addr |
        PT_BLOCK |
        PT_AF |
        PT_ISH;
    }

    ttbr0_l1[0] = (0x8000000000000000) | (uint64)ttbr0_l2 | 3;
    ttbr0_l1[1] = (0x8000000000000000) | (uint64)&ttbr0_l2[512] | 3;

/*    addr = 0xFFFFFFFFFFFFFFF;
    for (int i=0; i<512; i++) {
        ttbr1_l3[i] = addr |
        PT_BLOCK |
        PT_AD
    }*/

    for (addr = 0; addr < vc_mem; addr += 0x200000) {
        ttbr1_l2[ttbr1_l2_idx++] = addr |
        PT_BLOCK |
        PT_USER |
        PT_AF |
        PT_ISH |
        PT_MEM;
    }

//    ttbr1_l2[511] = (0x8000000000000000) | (uint64)ttbr1_l3 | 3;
    ttbr1_l1[0] = (0x8000000000000000) | (uint64)ttbr0_l2 | 3;
/*
    ttbr0_l2[ttbr0_l2_idx++] = 0x40000000 | ttbr0_l2
        PT_BLOCK |
        PT_AF |
        PT_ISH |
        PT_NC;

    ttbr0_l1[0] = (uint64)ttbr0_l2 |
        PT_PAGE |     // it has the "Present" flag, which must be set, and we have area in it mapped by pages
        PT_AF |       // accessed flag. Without this we're going to have a Data Abort exception
        PT_USER |     // non-privileged
        PT_ISH |      // inner shareable
        PT_MEM;       // normal memory

    ttbr0_l2[ttbr0_l2_idx++] = (uint64)ttbr0_l3_current |
        PT_PAGE |     // we have area in it mapped by pages
        PT_AF |       // accessed flag
        PT_USER |     // non-privileged
        PT_ISH |      // inner shareable
        PT_NC;       // non-cachable
*/
    enable_mmu_tables(ttbr0_l1, ttbr1_l1);

    return;


    for (addr=0; addr<0x80000; addr+= 0x1000) {
        mmu_add_mapping(addr |
            PT_PAGE |     // map 4k
            PT_AF |       // accessed flag
            PT_USER |     // non-privileged
            PT_ISH |      // inner shareable
            PT_RW |
            PT_NX);
    }

    for (addr=0x80000; addr<0x200000; addr+= 0x1000) {
        mmu_add_mapping(addr |
            PT_PAGE |     // map 4k
            PT_AF |       // accessed flag
            PT_USER |     // non-privileged
            PT_ISH |      // inner shareable
            PT_MEM);
    }

    for (addr=0x200000; addr<MMIO_BASE; addr += 0x200000) {
        mmu_add_mapping(addr |
            PT_BLOCK |     // map 4k
            PT_AF |       // accessed flag
            PT_NX |       // no execute
            PT_USER |     // non-privileged
            PT_ISH |      // inner shareable
            PT_MEM);
    }

    for (addr=MMIO_BASE; addr<0x40200000; addr+= 0x200000) {
        mmu_add_mapping(addr |
            PT_BLOCK |     // map 4k
            PT_AF |       // accessed flag
            PT_USER |     // non-privileged
            PT_OSH |      // inner shareable
            PT_DEV);
    }

    ttbr1_l1[511] = (uint64)ttbr1_l2 |
        PT_PAGE |     // we have area in it mapped by pages
        PT_AF |       // accessed flag
        PT_KERNEL |   // privileged
        PT_ISH |      // inner shareable
        PT_MEM;       // normal memory
    ttbr1_l2[511] = (uint64)ttbr1_l3 |
        PT_PAGE |     // we have area in it mapped by pages
        PT_AF |       // accessed flag
        PT_KERNEL |   // privileged
        PT_ISH |      // inner shareable
        PT_MEM;       // normal memory;

    ttbr1_l3[0] = (MMIO_BASE+0x00201000) |   // physical address
        PT_PAGE |     // map 4k
        PT_AF |       // accessed flag
        PT_NX |       // no execute
        PT_KERNEL |   // privileged
        PT_OSH |      // outter shareable
        PT_DEV;

/*    unsigned long data_page = (unsigned long)&__data_start__/PAGESIZE;
    unsigned long r, b, *paging=(unsigned long*)&_end;

    // TTBR0, identity L1
    paging[0]=(unsigned long)((unsigned char*)&_end+2*PAGESIZE) |    // physical address
        PT_PAGE |     // it has the "Present" flag, which must be set, and we have area in it mapped by pages
        PT_AF |       // accessed flag. Without this we're going to have a Data Abort exception
        PT_USER |     // non-privileged
        PT_ISH |      // inner shareable
        PT_MEM;       // normal memory

    // identity L2, first 2M block
    paging[2*512]=(unsigned long)((unsigned char*)&_end+3*PAGESIZE) | // physical address
        PT_PAGE |     // we have area in it mapped by pages
        PT_AF |       // accessed flag
        PT_USER |     // non-privileged
        PT_ISH |      // inner shareable
        PT_MEM;       // normal memory

    // identity L2 2M blocks
    b=MMIO_BASE>>21;
    // skip 0th, as we're about to map it by L3
    for(r=1;r<512;r++)
        paging[2*512+r]=(unsigned long)((r<<21)) |  // physical address
        PT_BLOCK |    // map 2M block
        PT_AF |       // accessed flag
        PT_NX |       // no execute
        PT_USER |     // non-privileged
        PT_MEM;
//        (r>=b? PT_OSH|PT_DEV : PT_ISH|PT_MEM); // different attributes for device memory

    // identity L3
    for(r=0;r<512;r++)
        paging[3*512+r]=(unsigned long)(r*PAGESIZE) |   // physical address
        PT_PAGE |     // map 4k
        PT_AF |       // accessed flag
        PT_USER |     // non-privileged
        PT_ISH |      // inner shareable
        PT_MEM;
//        ((r<0x80||r>data_page)? PT_RW : PT_RW); // different for code and data

    // TTBR1, kernel L1
    paging[512+511]=(unsigned long)((unsigned char*)&_end+4*PAGESIZE) | // physical address
        PT_PAGE |     // we have area in it mapped by pages
        PT_AF |       // accessed flag
        PT_KERNEL |   // privileged
        PT_ISH |      // inner shareable
        PT_MEM;       // normal memory

    // kernel L2
    paging[4*512+511]=(unsigned long)((unsigned char*)&_end+5*PAGESIZE) |   // physical address
        PT_PAGE |     // we have area in it mapped by pages
        PT_AF |       // accessed flag
        PT_KERNEL |   // privileged
        PT_ISH |      // inner shareable
        PT_MEM;       // normal memory

    // kernel L3
    paging[5*512]=(unsigned long)(MMIO_BASE+0x00201000) |   // physical address
        PT_PAGE |     // map 4k
        PT_AF |       // accessed flag
        PT_NX |       // no execute
        PT_KERNEL |   // privileged
        PT_OSH |      // outter shareable
        PT_DEV;       // device memory
*/
    /* okay, now we have to set system registers to enable MMU */
    // check for 4k granule and at least 36 bits physical address bus */
    uint64 r, b;

    asm volatile ("mrs %0, id_aa64mmfr0_el1" : "=r" (r));
    b=r&0xF;
    if(r&(0xF<<28)/*4k*/ || b<1/*36 bits*/) {
        uart_puts("ERROR: 4k granule or 36 bit address space not supported\n");
        return;
    }

    uart_puts("Drumrolls....\n");
    enable_mmu_tables(ttbr0_l1, ttbr1_l1);
    uart_puts("Yay!....\n");

    // first, set Memory Attributes array, indexed by PT_MEM, PT_DEV, PT_NC in our example
    r=  (0xFF << 0) |    // AttrIdx=0: normal, IWBWA, OWBWA, NTR
        (0x04 << 8) |    // AttrIdx=1: device, nGnRE (must be OSH too)
        (0x44 <<16);     // AttrIdx=2: non cacheable
    asm volatile ("msr mair_el1, %0" : : "r" (r));

    // next, specify mapping characteristics in translate control register
    r=  (0b00LL << 37) | // TBI=0, no tagging
        (b << 32) |      // IPS=autodetected
        (0b10LL << 30) | // TG1=4k
        (0b11LL << 28) | // SH1=3 inner
        (0b01LL << 26) | // ORGN1=1 write back
        (0b01LL << 24) | // IRGN1=1 write back
        (0b0LL  << 23) | // EPD1 enable higher half
        (25LL   << 16) | // T1SZ=25, 3 levels (512G)
        (0b00LL << 14) | // TG0=4k
        (0b11LL << 12) | // SH0=3 inner
        (0b01LL << 10) | // ORGN0=1 write back
        (0b01LL << 8) |  // IRGN0=1 write back
        (0b0LL  << 7) |  // EPD0 enable lower half
        (25LL   << 0);   // T0SZ=25, 3 levels (512G)
    asm volatile ("msr tcr_el1, %0; isb" : : "r" (r));

    // tell the MMU where our translation tables are. TTBR_CNP bit not documented, but required
    // lower half, user space
    asm volatile ("msr ttbr0_el1, %0" : : "r" (ttbr0_l1 + TTBR_CNP));

    // upper half, kernel space
    asm volatile ("msr ttbr1_el1, %0" : : "r" (ttbr1_l1 + TTBR_CNP));

    // finally, toggle some bits in system control register to enable page translation
    uart_puts("Drumrolls....\n");
    asm volatile ("dsb ish; isb; mrs %0, sctlr_el1" : "=r" (r));
    r|=0xC00800;     // set mandatory reserved bits
    r&=~((1<<25) |   // clear EE, little endian translation tables
         (1<<24) |   // clear E0E
         (1<<19) |   // clear WXN
         (1<<12) |   // clear I, no instruction cache
         (1<<4) |    // clear SA0
         (1<<3) |    // clear SA
         (1<<2) |    // clear C, no cache at all
         (1<<1));    // clear A, no aligment check
    r|=  (1<<0);     // set M, enable MMU

    asm volatile ("msr sctlr_el1, %0; isb" : : "r" (r));
    
    uart_puts("Yay!!!\n");
}
