#include "basic_sched.h"
#include <stdlib.h>

void priority_sched(process_queue *p, process_descriptor_t **descriptor, int *size) {
    if (p->size == 0) return;

    int n = p->size;

    process_t *procs = malloc(n * sizeof(process_t));
    int *op_idx = malloc(n * sizeof(int));
    int *op_left = malloc(n * sizeof(int));
    int *done = malloc(n * sizeof(int));

    if (!procs || !op_idx || !op_left || !done) {
        free(procs); free(op_idx); free(op_left); free(done);
        return;
    }

    int i = 0;
    for (node_t *cur = p->head; cur; cur = cur->next, i++) {
        procs[i] = cur->proc;
        op_idx[i] = 0;
        op_left[i] = (procs[i].operations_count > 0) ? procs[i].descriptor_p[0].duration_op : 0;
        done[i] = (procs[i].operations_count == 0);
    }

    int finished = 0;
    int current_time = 0;

    while (finished < n) {
        int pick = -1;

        for (int k = 0; k < n; k++) {
            if (done[k]) continue;
            if (procs[k].arrival_time_p > current_time) continue;
            if (procs[k].begining_date > current_time) continue;

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

        if (pick == -1) {
            current_time++;
            continue;
        }

        process_descriptor_t entry;
        entry.process_name = procs[pick].process_name;
        entry.date = current_time;
        entry.state = running_p;
        entry.operation = procs[pick].descriptor_p[op_idx[pick]].operation_p;
        append_descriptor(descriptor, entry, size);

        for (int k = 0; k < n; k++) {
            if (k == pick || done[k]) continue;

            if (procs[k].arrival_time_p <= current_time &&
                procs[k].begining_date <= current_time) {

                process_descriptor_t w;
                w.process_name = procs[k].process_name;
                w.date = current_time;
                w.state = waiting_p;
                w.operation = none;
                append_descriptor(descriptor, w, size);
            }
        }

        op_left[pick]--;
        current_time++;

        if (op_left[pick] == 0) {
            op_idx[pick]++;

            if (op_idx[pick] >= procs[pick].operations_count) {
                done[pick] = 1;
                finished++;

                process_descriptor_t term;
                term.process_name = procs[pick].process_name;
                term.date = current_time;
                term.state = terminated_p;
                term.operation = none;
                append_descriptor(descriptor, term, size);

                current_time++;
            } else {
                op_left[pick] = procs[pick].descriptor_p[op_idx[pick]].duration_op;
            }
        }
    }

    free(procs);
    free(op_idx);
    free(op_left);
    free(done);
}

