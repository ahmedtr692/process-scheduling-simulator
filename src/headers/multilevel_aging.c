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

    if (!procs || !op_idx || !op_left || !done || !wait_time) {
        free(procs); free(op_idx); free(op_left); free(done); free(wait_time);
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
    }

    int finished = 0;
    int current_time = 0;

    /* dynamic Round-Robin table (resizable) */
    int rr_cap = 32;
    int *rr_index = malloc(rr_cap * sizeof(int));
    for (int k = 0; k < rr_cap; k++) rr_index[k] = -1;

    while (finished < n) {

        /* --- AGING STEP --- */
        for (int k = 0; k < n; k++) {
            if (done[k]) continue;

            /* Process must be READY (not arrived? no aging) */
            if (procs[k].arrival_time_p > current_time) continue;
            if (procs[k].begining_date  > current_time) continue;

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

        /* --- SELECT PROCESS WITH HIGHEST PRIORITY --- */
        int best_priority = INT_MIN;
        for (int k = 0; k < n; k++) {
            if (!done[k] &&
                procs[k].arrival_time_p <= current_time &&
                procs[k].begining_date  <= current_time)
            {
                if (procs[k].priority_p > best_priority)
                    best_priority = procs[k].priority_p;
            }
        }

        if (best_priority == INT_MIN) {
            current_time++;
            continue;
        }

        /* --- CORRECT ROUND ROBIN --- */
        int pick = -1;
        int last = rr_index[best_priority];

        /* phase 1: search after last */
        for (int k = last + 1; k < n; k++) {
            if (!done[k] &&
                procs[k].priority_p == best_priority &&
                procs[k].arrival_time_p <= current_time &&
                procs[k].begining_date  <= current_time)
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
                    procs[k].begining_date  <= current_time)
                {
                    pick = k;
                    break;
                }
            }
        }

        if (pick == -1) { current_time++; continue; }

        rr_index[best_priority] = pick;

        /* LOG running */
        process_descriptor_t run;
        run.process_name = procs[pick].process_name;
        run.date = current_time;
        run.state = running_p;
        run.operation = procs[pick].descriptor_p[op_idx[pick]].operation_p;
        append_descriptor(descriptor, run, size);

        /* LOG waiting */
        for (int k = 0; k < n; k++) {
            if (k != pick && !done[k] &&
                procs[k].arrival_time_p <= current_time &&
                procs[k].begining_date  <= current_time)
            {
                process_descriptor_t w;
                w.process_name = procs[k].process_name;
                w.date = current_time;
                w.state = waiting_p;
                w.operation = none;
                append_descriptor(descriptor, w, size);
            }
        }

        /* RUN TICK */
        op_left[pick]--;
        current_time++;

        /* END OF OPERATION? */
        if (op_left[pick] == 0) {
            op_idx[pick]++;
            if (op_idx[pick] >= procs[pick].operations_count) {
                done[pick] = 1;
                finished++;

                process_descriptor_t t;
                t.process_name = procs[pick].process_name;
                t.date = current_time;
                t.state = terminated_p;
                t.operation = none;
                append_descriptor(descriptor, t, size);
            } else {
                op_left[pick] = procs[pick].descriptor_p[op_idx[pick]].duration_op;
            }
        }
    }

    free(procs); free(op_idx); free(op_left); free(done); free(wait_time);
    free(rr_index);
}

