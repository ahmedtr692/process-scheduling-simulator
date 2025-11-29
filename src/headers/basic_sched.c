#include "basic_sched.h"
#include <stdbool.h>

// Think of this helper as the person filling out a logbook.
// Every time some work happens, we hand it the time, who did it, what state they were in,
// and what kind of work it was so the next free page can be updated consistently.
static int add_descriptor_entry(process_descriptor_t* descriptor[],
							  int index,
							  int date,
							  const char *process_name,
							  process_state state,
							  process_operation_t operation) {
	if (descriptor == NULL) {
		return index;
	}
	process_descriptor_t *slot = descriptor[index];
	if (slot == NULL) {
		return index;
	}
    
	slot->date = date;
	slot->process_name = (char *)process_name;
	slot->state = state;
	slot->operation = operation;
	return index + 1;
}

// Quick translator between operation type and the visible state
// we want to display on the grid of processes.
static process_state state_from_operation(process_operation_t operation) {
	switch (operation) {
		case calc_p:
			return running_p; // CPU burst actively running
		case IO_p:
			return blocked_p; // Waiting on IO device
		case none:
		default:
			return ready_p; // Default to "ready" when no specific activity
	}
}

// This function simply moves the front process to the back, like people taking turns in a queue.
static void rotate_queue(process_queue *p) {
	if (p == NULL || p->head == NULL || p->head->next == NULL) {
		return;
	}

	node_t *first = p->head;
	p->head = first->next;
	first->next = NULL;

	if (p->tail == NULL) {
		p->tail = first;
	} else {
		p->tail->next = first;
		p->tail = first;
	}
}

// Runs every step of a process without interruption, updating separate clocks
// for CPU work (calculations) and IO work (waiting on devices).
static void schedule_process_fifo(process_t *proc,
								  process_descriptor_t* descriptor[],
								  int *index,
								  int *calc_time,
								  int *io_time) {
	if (proc == NULL || proc->descriptor_p.operations == NULL || proc->descriptor_p.count <= 0) {
		return;
	}

	int proc_clock = proc->arrival_time_p;

	for (int i = 0; i < proc->descriptor_p.count; i++) {
		operation_t *op = &proc->descriptor_p.operations[i];

		if (op->duration_op <= 0) {
			continue;
		}

		if (op->operation_p == calc_p) {
			int start = (*calc_time > proc_clock) ? *calc_time : proc_clock;
			*index = add_descriptor_entry(descriptor,
											 *index,
											 start,
											 proc->process_name,
											 state_from_operation(calc_p),
											 calc_p);
			*calc_time = start + op->duration_op;
			proc_clock = *calc_time;
		} else if (op->operation_p == IO_p) {
			int start = (*io_time > proc_clock) ? *io_time : proc_clock;
			*index = add_descriptor_entry(descriptor,
											 *index,
											 start,
											 proc->process_name,
											 state_from_operation(IO_p),
											 IO_p);
			*io_time = start + op->duration_op;
			proc_clock = *io_time;
		}
	}
}

// Checks whether any pieces of work still have time left so that
// round-robin mode knows when it can stop looping.
static bool process_has_work(int remaining_ops[], int count) {
	for (int i = 0; i < count; i++) {
		if (remaining_ops[i] > 0) {
			return true;
		}
	}
	return false;
}

// Gives each process equal-sized turns, repeatedly looping until all of its steps finish.
// The "quantum" is the maximum amount of time a process is allowed to work before yielding.
static void schedule_process_rr(process_t *proc,
								process_descriptor_t* descriptor[],
								int *index,
								int *calc_time,
								int *io_time) {
	if (proc == NULL || proc->descriptor_p.operations == NULL || proc->descriptor_p.count <= 0) {
		return;
	}

	if (proc->quantum_p <= 0) {
		schedule_process_fifo(proc, descriptor, index, calc_time, io_time);
		return;
	}

	int count = proc->descriptor_p.count;
	int *remaining = (int *)malloc(sizeof(int) * count);
	if (remaining == NULL) {
		schedule_process_fifo(proc, descriptor, index, calc_time, io_time);
		return;
	}

	for (int i = 0; i < count; i++) {
		remaining[i] = proc->descriptor_p.operations[i].duration_op;
	}

	int proc_clock = proc->arrival_time_p;

	while (process_has_work(remaining, count)) {
		bool progress = false; // Tracks whether at least one slice was executed in this pass.

		for (int i = 0; i < count; i++) {
			if (remaining[i] <= 0) {
				continue;
			}

			operation_t *op = &proc->descriptor_p.operations[i];
			int slice = (remaining[i] > proc->quantum_p) ? proc->quantum_p : remaining[i];

			if (op->operation_p == calc_p) {
				int start = (*calc_time > proc_clock) ? *calc_time : proc_clock;
				*index = add_descriptor_entry(descriptor,
												 *index,
												 start,
												 proc->process_name,
												 state_from_operation(calc_p),
												 calc_p);
				*calc_time = start + slice;
				proc_clock = *calc_time;
			} else if (op->operation_p == IO_p) {
				int start = (*io_time > proc_clock) ? *io_time : proc_clock;
				*index = add_descriptor_entry(descriptor,
												 *index,
												 start,
												 proc->process_name,
												 state_from_operation(IO_p),
												 IO_p);
				*io_time = start + slice;
				proc_clock = *io_time;
			} else {
				remaining[i] = 0;
				continue;
			}

			remaining[i] -= slice;
			progress = true;
		}

		if (!progress) {
			break; // Safety guard: avoids infinite loops if data is malformed.
		}
	}

	free(remaining);
}

// The public FIFO scheduler: decides whether to run processes non-stop, round-robin,
// or simply rotate the line depending on the selected mode and each process's quantum.
void fifo_sched(process_queue* p, process_descriptor_t* descriptor[], int mode) {
	if (p == NULL) {
		return;
	}

	if (mode == 1) { // Helper mode requested: only move the queue along.
		rotate_queue(p);
		return;
	}

	int calc_time = 0;
	int io_time = 0;
	int index = 0;

	for (node_t *current = p->head; current != NULL; current = current->next) {
		process_t *proc = &current->proc;

		if (proc == NULL || proc->descriptor_p.operations == NULL || proc->descriptor_p.count <= 0) {
			continue;
		}

		if (proc->quantum_p <= 0) {
			schedule_process_fifo(proc, descriptor, &index, &calc_time, &io_time); // No quantum means classic FIFO.
		} else {
			schedule_process_rr(proc, descriptor, &index, &calc_time, &io_time); // Positive quantum triggers per-process RR.
		}
	}
}

// Placeholder: future implementation will reuse the above helpers to run every process in RR order.
void round_robin(process_queue* p, int quantum, process_descriptor_t* descriptor[]) {
	(void)p;
	(void)quantum;
	(void)descriptor;
}

// Placeholder: planned shortest-job-first scheduler will pick the process with the smallest remaining work next.
void shortest_job_first(process_queue* p, process_descriptor_t* descriptor[]) {
	(void)p;
	(void)descriptor;
}

// Placeholder: priority scheduler will eventually respect the "priority" field and optional preemption flag.
void priority_sched(process_queue* p, int preemptive, process_descriptor_t* descriptor[]) {
	(void)p;
	(void)preemptive;
	(void)descriptor;
}
