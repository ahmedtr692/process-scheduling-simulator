#include "basic_sched.h"
#include <stdlib.h>

void priority_sched(process_queue *p, process_descriptor_t **descriptor, int *size) {
    if (p->size == 0) return;

    int n = p->size;

    process_t *procs = malloc(n * sizeof(process_t));
    int *op_idx = malloc(n * sizeof(int));
    int *op_left = malloc(n * sizeof(int));
    int *io_until = malloc(n * sizeof(int));
    int *done = malloc(n * sizeof(int));

    if (!procs || !op_idx || !op_left || !io_until || !done) {
        free(procs); free(op_idx); free(op_left); free(io_until); free(done);
        return;
    }

    int i = 0;
    for (node_t *cur = p->head; cur; cur = cur->next, i++) {
        procs[i] = cur->proc;
        op_idx[i] = 0;
        op_left[i] = (procs[i].operations_count > 0) ? procs[i].descriptor_p[0].duration_op : 0;
        io_until[i] = -1;
        done[i] = (procs[i].operations_count == 0);
    }

    int finished = 0;
    int current_time = 0;
    int max_time = 10000;

    while (finished < n && current_time < max_time) {
        int cpu_pick = -1;
        int io_pick = -1;

        // Find highest priority process for CPU (only CALC operations)
        for (int k = 0; k < n; k++) {
            if (done[k]) continue;
            if (procs[k].arrival_time_p > current_time) continue;
            if (io_until[k] >= 0) continue; // Doing I/O
            
            if (op_idx[k] < procs[k].operations_count &&
                procs[k].descriptor_p[op_idx[k]].operation_p == calc_p) {
                
                if (cpu_pick == -1 || procs[k].priority_p > procs[cpu_pick].priority_p) {
                    cpu_pick = k;
                }
            }
        }

        // Find process for I/O device (only one I/O at a time)
        for (int k = 0; k < n; k++) {
            if (done[k]) continue;
            if (procs[k].arrival_time_p > current_time) continue;
            if (io_until[k] >= 0 && io_until[k] > current_time) {
                io_pick = k; // Continue existing I/O
                break;
            }
        }

        // If no ongoing I/O, assign to highest priority process needing it
        if (io_pick == -1) {
            for (int k = 0; k < n; k++) {
                if (done[k]) continue;
                if (procs[k].arrival_time_p > current_time) continue;
                if (io_until[k] >= 0) continue;
                
                if (op_idx[k] < procs[k].operations_count &&
                    procs[k].descriptor_p[op_idx[k]].operation_p == IO_p) {
                    
                    if (io_pick == -1 || procs[k].priority_p > procs[io_pick].priority_p) {
                        io_pick = k;
                    }
                }
            }
            
            // Start I/O if found
            if (io_pick >= 0) {
                io_until[io_pick] = current_time + op_left[io_pick];
            }
        }

        // Execute CPU operation
        if (cpu_pick >= 0) {
            process_descriptor_t entry;
            entry.process_name = procs[cpu_pick].process_name;
            entry.date = current_time;
            entry.state = running_p;
            entry.operation = calc_p;
            append_descriptor(descriptor, entry, size);

            op_left[cpu_pick]--;
            
            if (op_left[cpu_pick] == 0) {
                op_idx[cpu_pick]++;
                if (op_idx[cpu_pick] < procs[cpu_pick].operations_count) {
                    op_left[cpu_pick] = procs[cpu_pick].descriptor_p[op_idx[cpu_pick]].duration_op;
                }
            }
        }

        // Execute I/O operation
        if (io_pick >= 0) {
            process_descriptor_t entry;
            entry.process_name = procs[io_pick].process_name;
            entry.date = current_time;
            entry.state = running_p;
            entry.operation = IO_p;
            append_descriptor(descriptor, entry, size);

            // Check if I/O completed
            if (io_until[io_pick] <= current_time + 1) {
                op_idx[io_pick]++;
                io_until[io_pick] = -1;
                if (op_idx[io_pick] < procs[io_pick].operations_count) {
                    op_left[io_pick] = procs[io_pick].descriptor_p[op_idx[io_pick]].duration_op;
                }
            }
        }

        // Mark other processes as waiting
        for (int k = 0; k < n; k++) {
            if (k == cpu_pick || k == io_pick) continue;
            if (done[k]) continue;
            if (procs[k].arrival_time_p > current_time) continue;

            process_descriptor_t w;
            w.process_name = procs[k].process_name;
            w.date = current_time;
            w.state = waiting_p;
            w.operation = none;
            append_descriptor(descriptor, w, size);
        }

        // Check for terminated processes
        for (int k = 0; k < n; k++) {
            if (done[k]) continue;
            if (procs[k].arrival_time_p > current_time) continue;
            
            if (op_idx[k] >= procs[k].operations_count && io_until[k] < 0) {
                done[k] = 1;
                finished++;

                process_descriptor_t term;
                term.process_name = procs[k].process_name;
                term.date = current_time + 1;
                term.state = terminated_p;
                term.operation = none;
                append_descriptor(descriptor, term, size);
            }
        }

        current_time++;
    }

    free(procs);
    free(op_idx);
    free(op_left);
    free(io_until);
    free(done);
}

