#ifndef __PROCESS_H
#define __PROCESS_H

#include "libc.h"

struct process {
	uint pid;
	void *stack;

	struct process *next;
};

extern struct process *current_process;

void init_processes();

#endif
