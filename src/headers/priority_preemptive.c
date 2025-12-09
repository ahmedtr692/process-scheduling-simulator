#include "basic_sched.h"
#include <stdlib.h>

void priority_sched(process_queue *p, process_descriptor_t **descriptor, int *size) {
    if (p->size == 0) return;

    int n = p->size;

    process_t *procs = malloc(n * sizeof(process_t));
    int *op_idx = malloc(n * sizeof(int));
    int *op_left = malloc(n * sizeof(int));
    int *done = malloc(n * sizeof(int));
    int *io_until = malloc(n * sizeof(int)); // Track when I/O completes

    if (!procs || !op_idx || !op_left || !done || !io_until) {
        free(procs); free(op_idx); free(op_left); free(done); free(io_until);
        return;
    }

    int i = 0;
    for (node_t *cur = p->head; cur; cur = cur->next, i++) {
        procs[i] = cur->proc;
        op_idx[i] = 0;
        op_left[i] = (procs[i].operations_count > 0) ? procs[i].descriptor_p[0].duration_op : 0;
        done[i] = (procs[i].operations_count == 0);
        io_until[i] = -1;
    }

    int finished = 0;
    int current_time = 0;

    while (finished < n) {
        // Find highest priority process that needs CPU
        int pick = -1;
        for (int k = 0; k < n; k++) {
            if (done[k]) continue;
            if (procs[k].arrival_time_p > current_time) continue;
            if (procs[k].begining_date > current_time) continue;
            if (io_until[k] > current_time) continue; // Doing I/O
            if (op_idx[k] >= procs[k].operations_count) continue;
            
            // Only consider for CPU if current operation is CALC
            if (procs[k].descriptor_p[op_idx[k]].operation_p != calc_p) continue;

            if (pick == -1) {
                pick = k;
            } else {
                if (procs[k].priority_p > procs[pick].priority_p) {
                    pick = k;
                } else if (procs[k].priority_p == procs[pick].priority_p &&
                           procs[k].begining_date < procs[pick].begining_date) {
                    pick = k;
                } else if (procs[k].priority_p == procs[pick].priority_p &&
                           procs[k].begining_date == procs[pick].begining_date &&
                           procs[k].arrival_time_p < procs[pick].arrival_time_p) {
                    pick = k;
                }
            }
        }

        // Execute all active processes (CPU + I/O in parallel)
        for (int k = 0; k < n; k++) {
            if (done[k]) continue;
            if (procs[k].arrival_time_p > current_time) continue;
            if (procs[k].begining_date > current_time) continue;
            if (op_idx[k] >= procs[k].operations_count) continue;

            process_operation_t cur_op = procs[k].descriptor_p[op_idx[k]].operation_p;

            // Execute CPU operation (only picked process)
            if (k == pick && cur_op == calc_p) {
                process_descriptor_t entry;
                entry.process_name = procs[k].process_name;
                entry.date = current_time;
                entry.state = running_p;
                entry.operation = calc_p;
                append_descriptor(descriptor, entry, size);

                op_left[k]--;

                if (op_left[k] == 0) {
                    op_idx[k]++;
                    if (op_idx[k] >= procs[k].operations_count) {
                        done[k] = 1;
                        finished++;
                    } else {
                        op_left[k] = procs[k].descriptor_p[op_idx[k]].duration_op;
                        // If next is I/O, start it
                        if (procs[k].descriptor_p[op_idx[k]].operation_p == IO_p) {
                            io_until[k] = current_time + op_left[k];
                        }
                    }
                }
            }
            // Execute I/O operation (concurrent with CPU)
            else if (cur_op == IO_p && io_until[k] > current_time) {
                process_descriptor_t entry;
                entry.process_name = procs[k].process_name;
                entry.date = current_time;
                entry.state = running_p;
                entry.operation = IO_p;
                append_descriptor(descriptor, entry, size);

                // Check if I/O completed
                if (current_time + 1 >= io_until[k]) {
                    op_left[k] = 0;
                    io_until[k] = -1;
                    op_idx[k]++;
                    if (op_idx[k] >= procs[k].operations_count) {
                        done[k] = 1;
                        finished++;
                    } else {
                        op_left[k] = procs[k].descriptor_p[op_idx[k]].duration_op;
                    }
                }
            }
            // Waiting for CPU
            else if (k != pick && io_until[k] <= current_time) {
                process_descriptor_t w;
                w.process_name = procs[k].process_name;
                w.date = current_time;
                w.state = waiting_p;
                w.operation = none;
                append_descriptor(descriptor, w, size);
            }
        }

        // Add termination markers
        for (int k = 0; k < n; k++) {
            if (done[k] && op_idx[k] == procs[k].operations_count) {
                process_descriptor_t term;
                term.process_name = procs[k].process_name;
                term.date = current_time;
                term.state = terminated_p;
                term.operation = none;
                append_descriptor(descriptor, term, size);
                op_idx[k]++; // Mark as already terminated
            }
        }

        current_time++;
    }

    free(procs);
    free(op_idx);
    free(op_left);
    free(done);
    free(io_until);
}

