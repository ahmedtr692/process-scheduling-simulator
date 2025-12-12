#include "basic_sched.h"
#include <stdlib.h>
#include <limits.h>

void multilevel_rr_sched(process_queue* p, process_descriptor_t** descriptor, int *size) {
    if (!p || p->size == 0) return;

    int n = p->size;

    process_t *procs = malloc(n * sizeof(process_t));
    int *op_idx  = malloc(n * sizeof(int));
    int *op_left = malloc(n * sizeof(int));
    int *io_until = malloc(n * sizeof(int));
    int *done    = malloc(n * sizeof(int));

    if (!procs || !op_idx || !op_left || !io_until || !done) {
        free(procs); free(op_idx); free(op_left); free(io_until); free(done);
        return;
    }

    int i = 0;
    for (node_t *cur = p->head; cur; cur = cur->next, i++) {
        procs[i] = cur->proc;
        op_idx[i] = 0;
        op_left[i] = (procs[i].operations_count > 0)
                        ? procs[i].descriptor_p[0].duration_op : 0;
        io_until[i] = -1;
        done[i] = (procs[i].operations_count == 0);
    }

    int finished = 0;
    int current_time = 0;
    int max_time = 10000;

    int max_priority = INT_MIN;
    for (int k = 0; k < n; k++)
        if (procs[k].priority_p > max_priority) max_priority = procs[k].priority_p;

    int *rr_index = malloc((max_priority+1) * sizeof(int));
    for (int k = 0; k <= max_priority; k++)
        rr_index[k] = -1;

    while (finished < n && current_time < max_time) {
        int cpu_pick = -1;
        int io_pick = -1;
        int best_priority = INT_MIN;

        /* --- Find highest ready priority for CPU (CALC only) --- */
        for (int k = 0; k < n; k++) {
            if (!done[k] &&
                procs[k].arrival_time_p <= current_time &&
                io_until[k] < 0)
            {
                if (op_idx[k] < procs[k].operations_count &&
                    procs[k].descriptor_p[op_idx[k]].operation_p == calc_p) {
                    if (procs[k].priority_p > best_priority) {
                        best_priority = procs[k].priority_p;
                    }
                }
            }
        }

        /* --- ROUND-ROBIN among processes of SAME priority for CPU --- */
        if (best_priority != INT_MIN) {
            int last = rr_index[best_priority];

            /* Phase 1: search after last index */
            for (int k = last + 1; k < n; k++) {
                if (!done[k] &&
                    procs[k].priority_p == best_priority &&
                    procs[k].arrival_time_p <= current_time &&
                    io_until[k] < 0)
                {
                    if (op_idx[k] < procs[k].operations_count &&
                        procs[k].descriptor_p[op_idx[k]].operation_p == calc_p) {
                        cpu_pick = k;
                        break;
                    }
                }
            }

            /* Phase 2: wrap around */
            if (cpu_pick == -1) {
                for (int k = 0; k <= last; k++) {
                    if (!done[k] &&
                        procs[k].priority_p == best_priority &&
                        procs[k].arrival_time_p <= current_time &&
                        io_until[k] < 0)
                    {
                        if (op_idx[k] < procs[k].operations_count &&
                            procs[k].descriptor_p[op_idx[k]].operation_p == calc_p) {
                            cpu_pick = k;
                            break;
                        }
                    }
                }
            }

            if (cpu_pick >= 0) {
                rr_index[best_priority] = cpu_pick;
            }
        }

        /* --- Find process for I/O device --- */
        for (int k = 0; k < n; k++) {
            if (!done[k] &&
                procs[k].arrival_time_p <= current_time &&
                io_until[k] >= 0 && io_until[k] > current_time) {
                io_pick = k; // Continue existing I/O
                break;
            }
        }

        /* If no ongoing I/O, assign to highest priority process needing it */
        if (io_pick == -1) {
            int best_io_priority = INT_MIN;
            for (int k = 0; k < n; k++) {
                if (!done[k] &&
                    procs[k].arrival_time_p <= current_time &&
                    io_until[k] < 0)
                {
                    if (op_idx[k] < procs[k].operations_count &&
                        procs[k].descriptor_p[op_idx[k]].operation_p == IO_p) {
                        if (procs[k].priority_p > best_io_priority) {
                            best_io_priority = procs[k].priority_p;
                            io_pick = k;
                        }
                    }
                }
            }
            
            /* Start I/O if found */
            if (io_pick >= 0) {
                io_until[io_pick] = current_time + op_left[io_pick];
            }
        }

        /* Execute CPU operation */
        if (cpu_pick >= 0) {
            process_descriptor_t r;
            r.process_name = procs[cpu_pick].process_name;
            r.date = current_time;
            r.state = running_p;
            r.operation = calc_p;
            append_descriptor(descriptor, r, size);

            op_left[cpu_pick]--;
            
            if (op_left[cpu_pick] == 0) {
                op_idx[cpu_pick]++;
                if (op_idx[cpu_pick] < procs[cpu_pick].operations_count) {
                    op_left[cpu_pick] = procs[cpu_pick].descriptor_p[op_idx[cpu_pick]].duration_op;
                }
            }
        }

        /* Execute I/O operation */
        if (io_pick >= 0) {
            process_descriptor_t r;
            r.process_name = procs[io_pick].process_name;
            r.date = current_time;
            r.state = running_p;
            r.operation = IO_p;
            append_descriptor(descriptor, r, size);

            /* Check if I/O completed */
            if (io_until[io_pick] <= current_time + 1) {
                op_idx[io_pick]++;
                io_until[io_pick] = -1;
                if (op_idx[io_pick] < procs[io_pick].operations_count) {
                    op_left[io_pick] = procs[io_pick].descriptor_p[op_idx[io_pick]].duration_op;
                }
            }
        }

        /* LOG WAITING */
        for (int k = 0; k < n; k++) {
            if (k == cpu_pick || k == io_pick) continue;
            if (!done[k] && procs[k].arrival_time_p <= current_time)
            {
                process_descriptor_t w;
                w.process_name = procs[k].process_name;
                w.date = current_time;
                w.state = waiting_p;
                w.operation = none;
                append_descriptor(descriptor, w, size);
            }
        }

        /* Check for terminated processes */
        for (int k = 0; k < n; k++) {
            if (!done[k] &&
                procs[k].arrival_time_p <= current_time &&
                op_idx[k] >= procs[k].operations_count && io_until[k] < 0) {
                done[k] = 1;
                finished++;

                process_descriptor_t t;
                t.process_name = procs[k].process_name;
                t.date = current_time + 1;
                t.state = terminated_p;
                t.operation = none;
                append_descriptor(descriptor, t, size);
            }
        }

        current_time++;
    }

    free(procs); free(op_idx); free(op_left); free(io_until); free(done); free(rr_index);
}

