# Heap

The kernel implements two types of heaps: a kernel heap and a process heap. The former is, as its name implies, used whenever the kernel needs to allocate some memory. Process heaps are allocated inside the kernel heap, and are used by processes (not implemented yet).

In order to decrease fragmentation, all heaps are divided in two spaces: pages and small allocations. A lot of memory allocations are indeed for a certain numer of pages: a disk cluster, a call stack, etc. For easier debugging, each page allocation comes with a label, so it's easier to understand who allocated what when looking at the list of pages.
