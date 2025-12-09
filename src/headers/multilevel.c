#include "basic_sched.h"
#include <stdlib.h>
#include <limits.h>

void multilevel_rr_sched(process_queue* p, process_descriptor_t** descriptor, int *size) {
    if (!p || p->size == 0) return;

    int n = p->size;

    process_t *procs = malloc(n * sizeof(process_t));
    int *op_idx  = malloc(n * sizeof(int));
    int *op_left = malloc(n * sizeof(int));
    int *done    = malloc(n * sizeof(int));

    if (!procs || !op_idx || !op_left || !done) {
        free(procs); free(op_idx); free(op_left); free(done);
        return;
    }

    int i = 0;
    for (node_t *cur = p->head; cur; cur = cur->next, i++) {
        procs[i] = cur->proc;
        op_idx[i] = 0;
        op_left[i] = (procs[i].operations_count > 0)
                        ? procs[i].descriptor_p[0].duration_op : 0;
        done[i] = (procs[i].operations_count == 0);
    }

    int finished = 0;
    int current_time = 0;

    int max_priority = INT_MIN;
    for (int k = 0; k < n; k++)
        if (procs[k].priority_p > max_priority) max_priority = procs[k].priority_p;

    int *rr_index = malloc((max_priority+1) * sizeof(int));
    for (int k = 0; k <= max_priority; k++)
        rr_index[k] = -1;

    while (finished < n) {
        int pick = -1;
        int best_priority = INT_MIN;

        /* --- Find highest ready priority --- */
        for (int k = 0; k < n; k++) {
            if (!done[k] &&
                procs[k].arrival_time_p <= current_time &&
                procs[k].begining_date  <= current_time)
            {
                if (procs[k].priority_p > best_priority) {
                    best_priority = procs[k].priority_p;
                }
            }
        }

        /* --- If nobody ready --- */
        if (best_priority == INT_MIN) {
            current_time++;
            continue;
        }

        /* --- ROUND-ROBIN among processes of SAME priority --- */
        int last = rr_index[best_priority];

        /* Phase 1: search after last index */
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

        /* Phase 2: wrap around */
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

        if (pick == -1) {
            current_time++;
            continue;
        }

        rr_index[best_priority] = pick;

        /* LOG RUN */
        process_descriptor_t r;
        r.process_name = procs[pick].process_name;
        r.date = current_time;
        r.state = running_p;
        r.operation = procs[pick].descriptor_p[op_idx[pick]].operation_p;
        append_descriptor(descriptor, r, size);

        /* LOG WAITING */
        for (int k = 0; k < n; k++) {
            if (k != pick && !done[k] &&
                procs[k].arrival_time_p <= current_time &&
                procs[k].begining_date <= current_time)
            {
                process_descriptor_t w;
                w.process_name = procs[k].process_name;
                w.date = current_time;
                w.state = waiting_p;
                w.operation = none;
                append_descriptor(descriptor, w, size);
            }
        }

        /* Run 1 tick */
        op_left[pick]--;
        current_time++;

        /* Operation finished? */
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

    free(procs); free(op_idx); free(op_left); free(done); free(rr_index);
}

