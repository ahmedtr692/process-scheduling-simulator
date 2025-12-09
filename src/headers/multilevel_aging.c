#include "basic_sched.h"
#include <stdlib.h>
#include <limits.h>

#define AGING_THRESHOLD 5   // Number of ticks before priority increases

void multilevel_rr_aging_sched(process_queue* p, process_descriptor_t** descriptor, int *size) {
    if (!p || p->size == 0) return;

    int n = p->size;

    process_t *procs = malloc(n * sizeof(process_t));
    int *op_idx  = malloc(n * sizeof(int));
    int *op_left = malloc(n * sizeof(int));
    int *done    = malloc(n * sizeof(int));
    int *wait_time = malloc(n * sizeof(int));
    int *io_until = malloc(n * sizeof(int)); // Track when I/O completes

    if (!procs || !op_idx || !op_left || !done || !wait_time || !io_until) {
        free(procs); free(op_idx); free(op_left); free(done); free(wait_time); free(io_until);
        return;
    }

    int i = 0;
    for (node_t *cur = p->head; cur; cur = cur->next, i++) {
        procs[i] = cur->proc;
        op_idx[i] = 0;
        op_left[i] = (procs[i].operations_count > 0)
                        ? procs[i].descriptor_p[0].duration_op : 0;
        done[i] = (procs[i].operations_count == 0);
        wait_time[i] = 0;
        io_until[i] = -1;
    }

    int finished = 0;
    int current_time = 0;
    int max_time = 10000; // Safety limit to prevent infinite loops

    /* dynamic Round-Robin table (resizable) */
    int rr_cap = 32;
    int *rr_index = malloc(rr_cap * sizeof(int));
    for (int k = 0; k < rr_cap; k++) rr_index[k] = -1;

    while (finished < n && current_time < max_time) {

        /* --- AGING STEP --- */
        for (int k = 0; k < n; k++) {
            if (done[k]) continue;

            /* Process must be READY (not arrived? no aging) */
            if (procs[k].arrival_time_p > current_time) continue;
            if (procs[k].begining_date  > current_time) continue;
            if (io_until[k] > current_time) continue; // Doing I/O, not waiting

            wait_time[k]++;

            if (wait_time[k] >= AGING_THRESHOLD) {
                procs[k].priority_p++;   // age
                wait_time[k] = 0;

                /* expand rr_index if needed */
                if (procs[k].priority_p >= rr_cap) {
                    rr_cap *= 2;
                    rr_index = realloc(rr_index, rr_cap * sizeof(int));
                    for (int t = procs[k].priority_p; t < rr_cap; t++)
                        rr_index[t] = -1;
                }
            }
        }

        /* --- SELECT PROCESS WITH HIGHEST PRIORITY (for CPU) --- */
        int best_priority = INT_MIN;
        for (int k = 0; k < n; k++) {
            if (!done[k] &&
                procs[k].arrival_time_p <= current_time &&
                procs[k].begining_date  <= current_time &&
                io_until[k] <= current_time && // Not doing I/O
                op_idx[k] < procs[k].operations_count &&
                procs[k].descriptor_p[op_idx[k]].operation_p == calc_p) // Needs CPU
            {
                if (procs[k].priority_p > best_priority)
                    best_priority = procs[k].priority_p;
            }
        }

        /* --- CORRECT ROUND ROBIN --- */
        int pick = -1;
        if (best_priority != INT_MIN) {
            int last = rr_index[best_priority];

            /* phase 1: search after last */
            for (int k = last + 1; k < n; k++) {
                if (!done[k] &&
                    procs[k].priority_p == best_priority &&
                    procs[k].arrival_time_p <= current_time &&
                    procs[k].begining_date  <= current_time &&
                    io_until[k] <= current_time &&
                    op_idx[k] < procs[k].operations_count &&
                    procs[k].descriptor_p[op_idx[k]].operation_p == calc_p)
                {
                    pick = k;
                    break;
                }
            }

            /* phase 2: wrap around */
            if (pick == -1) {
                for (int k = 0; k <= last; k++) {
                    if (!done[k] &&
                        procs[k].priority_p == best_priority &&
                        procs[k].arrival_time_p <= current_time &&
                        procs[k].begining_date  <= current_time &&
                        io_until[k] <= current_time &&
                        op_idx[k] < procs[k].operations_count &&
                        procs[k].descriptor_p[op_idx[k]].operation_p == calc_p)
                    {
                        pick = k;
                        break;
                    }
                }
            }

            if (pick != -1) {
                rr_index[best_priority] = pick;
                wait_time[pick] = 0; // Reset wait time when running
            }
        }

        /* Execute all active processes (CPU + I/O in parallel) */
        for (int k = 0; k < n; k++) {
            if (done[k]) continue;
            if (procs[k].arrival_time_p > current_time) continue;
            if (procs[k].begining_date > current_time) continue;
            if (op_idx[k] >= procs[k].operations_count) continue;

            process_operation_t cur_op = procs[k].descriptor_p[op_idx[k]].operation_p;

            /* Execute CPU operation (only picked process) */
            if (k == pick && cur_op == calc_p) {
                process_descriptor_t run;
                run.process_name = procs[k].process_name;
                run.date = current_time;
                run.state = running_p;
                run.operation = calc_p;
                append_descriptor(descriptor, run, size);

                op_left[k]--;

                /* END OF OPERATION? */
                if (op_left[k] == 0) {
                    op_idx[k]++;
                    if (op_idx[k] >= procs[k].operations_count) {
                        done[k] = 1;
                        finished++;
                    } else {
                        op_left[k] = procs[k].descriptor_p[op_idx[k]].duration_op;
                        /* If next is I/O, start it */
                        if (procs[k].descriptor_p[op_idx[k]].operation_p == IO_p) {
                            io_until[k] = current_time + op_left[k];
                        }
                    }
                }
            }
            /* Execute I/O operation (concurrent with CPU) */
            else if (cur_op == IO_p && io_until[k] > current_time) {
                process_descriptor_t run;
                run.process_name = procs[k].process_name;
                run.date = current_time;
                run.state = running_p;
                run.operation = IO_p;
                append_descriptor(descriptor, run, size);

                /* Check if I/O completed */
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
            /* Waiting for CPU */
            else if (k != pick && io_until[k] <= current_time) {
                process_descriptor_t w;
                w.process_name = procs[k].process_name;
                w.date = current_time;
                w.state = waiting_p;
                w.operation = none;
                append_descriptor(descriptor, w, size);
            }
        }

        /* Add termination markers */
        for (int k = 0; k < n; k++) {
            if (done[k] && op_idx[k] == procs[k].operations_count) {
                process_descriptor_t t;
                t.process_name = procs[k].process_name;
                t.date = current_time;
                t.state = terminated_p;
                t.operation = none;
                append_descriptor(descriptor, t, size);
                op_idx[k]++; // Mark as already terminated
            }
        }

        current_time++;
    }

    free(procs); free(op_idx); free(op_left); free(done); free(wait_time);
    free(rr_index); free(io_until);
}

