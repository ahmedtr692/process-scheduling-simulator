#include "basic_sched.h"
#include <stdlib.h>

void fifo_sched(process_queue* p, process_descriptor_t** descriptor, int *size)
{
    if (p == NULL || p->size == 0) return;

    process_queue working_queue = { .head = NULL, .tail = NULL, .size = 0 };
    for (node_t *n = p->head; n != NULL; n = n->next) {
        process_t copy = n->proc;
        add_tail(&working_queue, copy);
    }

    int current_time = 0;

    while (working_queue.size > 0) {
        process_t *proc = &working_queue.head->proc;

        if (current_time < proc->arrival_time_p) current_time = proc->arrival_time_p;
        if (current_time < proc->begining_date) current_time = proc->begining_date;

        for (int i = 0; i < proc->operations_count; ++i) {
            operation_t op = proc->descriptor_p[i].operation_p;
            int *remaining = &proc->descriptor_p[i].duration_op;

            while (*remaining > 0) {
                process_descriptor_t run_entry;
                run_entry.process_name = proc->process_name;
                run_entry.date = current_time;
                run_entry.state = (op != none) ? running_p : ready_p;
                run_entry.operation = op;
                append_descriptor(descriptor, run_entry, size);

                for (node_t *o = working_queue.head->next; o != NULL; o = o->next) {
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
            }
        }
        process_descriptor_t term;
        term.process_name = proc->process_name;
        term.date = current_time;
        term.state = terminated_p;
        term.operation = none;
        append_descriptor(descriptor, term, size);

        remove_head(&working_queue);
        current_time++;  
    }

    
}
