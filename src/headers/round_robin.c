#include "basic_sched.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>



void round_robin_sched(process_queue* p, process_descriptor_t** descriptor, int *size, int quantum)
{
    if (p == NULL || p->size == 0) return;
    if (quantum <= 0) return;

    process_queue *rr_queue = copy_queue(p);
    if (rr_queue == NULL) return;

    int current_time = 0;

    while (rr_queue->size > 0) {
        process_t proc = rr_queue->head->proc;

        if (current_time < proc.arrival_time_p) current_time = proc.arrival_time_p;
        if (current_time < proc.begining_date) current_time = proc.begining_date;

        int op_index = -1;
        for (int j = 0; j < proc.operations_count; ++j) {
            if (proc.descriptor_p[j].duration_op > 0) { op_index = j; break; }
        }

        if (op_index == -1) {
            process_descriptor_t term;
            term.process_name = proc.process_name;
            term.date = current_time;
            term.state = terminated_p;
            term.operation = none;
            append_descriptor(descriptor, term, size);

            remove_head(rr_queue);
            current_time++;
            continue;
        }

        operation_t op = proc.descriptor_p[op_index].operation_p;
        int *remaining = &rr_queue->head->proc.descriptor_p[op_index].duration_op;
        int used = 0;

        while (used < quantum && *remaining > 0) {
            process_descriptor_t run_entry;
            run_entry.process_name = rr_queue->head->proc.process_name;
            run_entry.date = current_time;
            run_entry.state = (op != none) ? running_p : ready_p;
            run_entry.operation = op;
            append_descriptor(descriptor, run_entry, size);

            for (node_t *o = rr_queue->head->next; o != NULL; o = o->next) {
                if (o->proc.arrival_time_p <= current_time &&
                    o->proc.begining_date <= current_time) {
                    process_descriptor_t wait_entry;
                    wait_entry.process_name = o->proc.process_name;
                    wait_entry.date = current_time;
                    wait_entry.state = waiting_p;
                    wait_entry.operation = none;
                    append_descriptor(descriptor, wait_entry, size);
                }
            }

            current_time++;
            (*remaining)--;
            used++;
        }

        int still_has_work = 0;
        for (int j = 0; j < rr_queue->head->proc.operations_count; ++j) {
            if (rr_queue->head->proc.descriptor_p[j].duration_op > 0) { still_has_work = 1; break; }
        }

        if (still_has_work) {
            process_t tmp = rr_queue->head->proc;
            remove_head(rr_queue);
            add_tail(rr_queue, tmp);
        } else {
            process_descriptor_t term;
            term.process_name = rr_queue->head->proc.process_name;
            term.date = current_time;
            term.state = terminated_p;
            term.operation = none;
            append_descriptor(descriptor, term, size);

            remove_head(rr_queue);
            current_time++;
        }
    }

    free_queue(rr_queue);
    free(rr_queue);
}
