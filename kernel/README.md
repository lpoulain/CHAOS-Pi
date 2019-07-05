# Heap

The kernel implements two types of heaps: a kernel heap and a process heap. The former is, as its name implies, used whenever the kernel needs to allocate some memory. Process heaps are allocated inside the kernel heap, and are used by processes (not implemented yet).

In order to decrease fragmentation, all heaps are divided in two spaces: pages and small allocations. A lot of memory allocations are indeed for a certain numer of pages: a disk cluster, a call stack, etc. For easier debugging, each page allocation comes with a label, so it's easier to understand who allocated what when looking at the list of pages.

# EL1 / EL0

By default, the Raspberri Pi starts in EL2 (Exception Level 2), aka Hypervisor mode. Code in `start.S` switches all cores to EL1 aka Supervisor mode, or kernel model.

It is however possible to run some code in EL0 aka user mode. For this:

- The MMU needs to be setup
- Set the `spsr_el1` CPU register (see `switch_el0` in `start.S`)
- Because the code is then run with limited access to the hardware, it needs to perform a system call for any I/O operation:
    - Store any argument in registers
    - Call the assembly `svc <syscall nb>`
    - The interrupt handler (run in EL1) will catch this interruption and can process it
    - When the interrupt handler is done, call `eret` to return back to EL0
