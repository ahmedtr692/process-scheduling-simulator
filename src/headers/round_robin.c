#include "basic_sched.h"
#include <stdlib.h>
#include <string.h>

void round_robin_sched(process_queue *p, process_descriptor_t **descriptor, int *size, int quantum) {
    if (!p || p->size == 0 || quantum <= 0) return;

    int n = p->size;
    process_t *procs = malloc(n * sizeof(process_t));
    int *op_idx = malloc(n * sizeof(int));
    int *op_left = malloc(n * sizeof(int));
    int *done = malloc(n * sizeof(int));
    int *io_until = malloc(n * sizeof(int)); // Track when I/O completes
    int *cpu_used = malloc(n * sizeof(int)); // Track CPU usage in current quantum

    if (!procs || !op_idx || !op_left || !done || !io_until || !cpu_used) {
        free(procs); free(op_idx); free(op_left); free(done); free(io_until); free(cpu_used);
        return;
    }

    // Initialize process array
    int i = 0;
    for (node_t *cur = p->head; cur; cur = cur->next, i++) {
        procs[i] = cur->proc;
        op_idx[i] = 0;
        op_left[i] = (procs[i].operations_count > 0) ? procs[i].descriptor_p[0].duration_op : 0;
        done[i] = (procs[i].operations_count == 0);
        io_until[i] = -1;
        cpu_used[i] = 0;
    }

    int finished = 0;
    int current_time = 0;
    int cpu_proc = -1; // Which process currently has CPU
    int max_time = 10000; // Safety limit to prevent infinite loops

    while (finished < n && current_time < max_time) {
        // Check if current CPU process needs to switch (quantum expired or operation done)
        if (cpu_proc != -1) {
            if (done[cpu_proc] || cpu_used[cpu_proc] >= quantum || 
                (op_idx[cpu_proc] < procs[cpu_proc].operations_count && 
                 procs[cpu_proc].descriptor_p[op_idx[cpu_proc]].operation_p == IO_p)) {
                cpu_proc = -1; // Release CPU
            }
        }

        // Select next process for CPU (Round-Robin)
        if (cpu_proc == -1) {
            for (int k = 0; k < n; k++) {
                if (done[k]) continue;
                if (procs[k].arrival_time_p > current_time) continue;
                if (procs[k].begining_date > current_time) continue;
                if (io_until[k] > current_time) continue; // Still doing I/O
                if (op_idx[k] >= procs[k].operations_count) continue;
                
                // Check if current operation is CALC (CPU needed)
                if (procs[k].descriptor_p[op_idx[k]].operation_p == calc_p) {
                    cpu_proc = k;
                    cpu_used[k] = 0; // Reset quantum counter
                    break;
                }
            }
        }

        // Execute one time unit

        for (int k = 0; k < n; k++) {
            if (done[k]) continue;
            if (procs[k].arrival_time_p > current_time) continue;
            if (procs[k].begining_date > current_time) continue;
            if (op_idx[k] >= procs[k].operations_count) continue;

            process_operation_t cur_op = procs[k].descriptor_p[op_idx[k]].operation_p;
            
            // Execute CPU operation
            if (k == cpu_proc && cur_op == calc_p) {
                process_descriptor_t entry;
                entry.process_name = procs[k].process_name;
                entry.date = current_time;
                entry.state = running_p;
                entry.operation = calc_p;
                append_descriptor(descriptor, entry, size);

                op_left[k]--;

                // Check if operation completed
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
                        cpu_used[k] = 0; // Reset quantum for next turn
                    }
                }
            }
            // Waiting for CPU or just arrived
            else if (!done[k] && k != cpu_proc && io_until[k] <= current_time) {
                process_descriptor_t entry;
                entry.process_name = procs[k].process_name;
                entry.date = current_time;
                entry.state = waiting_p;
                entry.operation = none;
                append_descriptor(descriptor, entry, size);
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

        // Always advance time
        current_time++;
    }

    free(procs);
    free(op_idx);
    free(op_left);
    free(done);
    free(io_until);
    free(cpu_used);
}
