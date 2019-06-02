#include "libc.h"
#include "process.h"
#include "display.h"

struct process processes[10];

struct process *current_process;
int nb_processes = 0;

char stack[10][65536];

void init_processes() {
	for (int i=0; i<10; i++) {
		processes[i].pid = i;
		processes[i].stack = &stack[i];
		processes[i].next = &processes[0];
	}

//	processes[9].next = &processes[0];

//	current_process = &processes[0];
}

void fork() {
	processes[nb_processes].next = &processes[0];

	if (nb_processes == 0) {
		current_process = &processes[0];
		processes[0].next = &processes[0];
		return;
	}

	animate(nb_processes * 100);
}
