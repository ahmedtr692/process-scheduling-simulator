#include "basic_sched.h"

// FIFO: First In First Out - Simple sequential execution
// Processes execute in arrival order, non-preemptive
void fifo_sched(process_queue *p, process_descriptor_t **descriptor, int* size) {
    if (p->size == 0) return;

    // Copy and sort processes by arrival time
    process_queue *fifo_queue = malloc(sizeof(process_queue));
    fifo_queue->head = NULL;
    fifo_queue->tail = NULL;
    fifo_queue->size = 0;

    node_t *current = p->head;
    while (current != NULL) {
        add_tail(fifo_queue, current->proc);
        current = current->next;
    }

    // Sort by arrival time (begining_date)
    if (fifo_queue->size > 1) {
        int swapped;
        node_t *cur;
        node_t *last = NULL;

        do {
            swapped = 0;
            cur = fifo_queue->head;
            while (cur->next != last) {
                if (cur->proc.begining_date > cur->next->proc.begining_date) {
                    process_t tmp = cur->proc;
                    cur->proc = cur->next->proc;
                    cur->next->proc = tmp;
                    swapped = 1;
                }
                cur = cur->next;
            }
            last = cur;
        } while (swapped);
    }

    int current_time = 0;

    // Execute each process in order
    for (node_t *cur = fifo_queue->head; cur != NULL; cur = cur->next) {
        process_t *proc = &cur->proc;

        // Wait for process to arrive
        if (current_time < proc->arrival_time_p) current_time = proc->arrival_time_p;
        if (current_time < proc->begining_date) current_time = proc->begining_date;

        // Execute all operations of this process
        for (int op_idx = 0; op_idx < proc->operations_count; op_idx++) {
            process_operation_t op_type = proc->descriptor_p[op_idx].operation_p;
            int duration = proc->descriptor_p[op_idx].duration_op;

            // Execute this operation for its full duration
            for (int t = 0; t < duration; t++) {
                process_descriptor_t entry;
                entry.process_name = proc->process_name;
                entry.date = current_time;
                entry.state = running_p;
                entry.operation = op_type;
                append_descriptor(descriptor, entry, size);

                current_time++;
            }
        }

        // Mark process as terminated
        process_descriptor_t term_entry;
        term_entry.process_name = proc->process_name;
        term_entry.date = current_time;
        term_entry.state = terminated_p;
        term_entry.operation = none;
        append_descriptor(descriptor, term_entry, size);
    }

    while (fifo_queue->size > 0) remove_head(fifo_queue);
    free(fifo_queue);
}
