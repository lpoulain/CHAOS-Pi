#include "libc.h"
#include "heap.h"

void init_kheap();
void *kmalloc_pages(uint, const char *);
void *kmalloc(uint);
void kfree(void*);
uint kheap_free_space();
void kheap_check_for_corruption(const char *);

extern Heap kheap;
