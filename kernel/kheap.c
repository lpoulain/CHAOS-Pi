// This is the kernel heap - the heap used by the kernel to allocate
// kernel structures, as well as process heaps

#include "libc.h"
#include "heap.h"
#include "mbox.h"

Heap kheap;
extern uint64 _end;

void init_kheap() {
    // - end of the used memory -> 10 Mb used for small object allocations
    // - The rest is used for page allocations
//    mbox_add4(0x00010005, 0, 8, 0, 0);
//    mbox_send();

//    printf("Max mem: %X\n", mbox_get(6));

	init_heap(&kheap, (uint64)&_end, (uint64)&_end + 0xA00000, 0x3C000000);
}

void* kmalloc_pages(uint nb_pages, const char *name) {
	return heap_alloc_pages(nb_pages, name, &kheap);
}

void* kmalloc(uint nb_bytes) {
	uint ptr = *(&nb_bytes - 1);
	return heap_alloc(nb_bytes, &kheap, ptr);
}

void kfree(void *ptr) {
	heap_free(ptr, &kheap);
}

uint kheap_free_space() {
	return heap_free_space(&kheap);
}

void kheap_check_for_corruption(const char *msg) {
	heap_check_for_corruption(&kheap, msg);
}
