#ifndef KHEAP_H
#define KHEAP_H

#include "libc.h"

typedef struct {
	uint64 start;
	uint64 ptr;
	uint64 end;
	uint64 page_index_start;
	uint64 page_index_end;
	uint64 page_start;
	uint64 page_end;
	uint nb_pages;
} Heap;

void init_heap(Heap *, uint, uint, uint);
void *heap_alloc(uint, Heap *, uint);
void *heap_alloc_pages(uint, const char *, Heap *);
void heap_free(void *, Heap *);
uint heap_free_space(Heap *h);
void *malloc(uint);
void free(void*);

void heap_check_for_corruption(Heap *h, const char *msg);

#endif // KHEAP_H
