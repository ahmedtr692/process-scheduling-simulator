#include "basic_sched.h"
#include <stdlib.h>

void round_robin_sched(process_queue *p, process_descriptor_t **descriptor, int *size, int quantum) {
    if (!p || p->size == 0 || quantum <= 0) return;

    // Create RR queue
    process_queue *rr_queue = malloc(sizeof(process_queue));
    rr_queue->head = rr_queue->tail = NULL;
    rr_queue->size = 0;

    // Copy all processes into RR queue
    for (node_t *n = p->head; n != NULL; n = n->next) {
        add_tail(rr_queue, n->proc);
    }

    int current_time = 0;

    while (rr_queue->size > 0) {
        node_t *cur_node = rr_queue->head; // pointer to actual node
        process_t *proc = &cur_node->proc;
        remove_head(rr_queue); // remove from head

        // Sync start time
        if (current_time < proc->arrival_time_p) current_time = proc->arrival_time_p;
        if (current_time < proc->begining_date) current_time = proc->begining_date;

        // Find first operation with remaining duration
        int op_index = -1;
        for (int j = 0; j < proc->operations_count; j++) {
            if (proc->descriptor_p[j].duration_op > 0) {
                op_index = j;
                break;
            }
        }

        // If all operations done, mark terminated
        if (op_index == -1) {
            process_descriptor_t term;
            term.process_name = proc->process_name;
            term.date = current_time;
            term.state = terminated_p;
            term.operation = none;
            append_descriptor(descriptor, term, size);
            continue;
        }

        // Execute current operation for up to quantum
        int remaining = proc->descriptor_p[op_index].duration_op;
        int slice = (remaining < quantum) ? remaining : quantum;
        process_operation_t op_type = proc->descriptor_p[op_index].operation_p;

        for (int t = 0; t < slice; t++) {
            proc->descriptor_p[op_index].duration_op--;

            // Add running entry
            process_descriptor_t run_entry;
            run_entry.process_name = proc->process_name;
            run_entry.date = current_time;
            run_entry.state = (op_type != none) ? running_p : ready_p;
            run_entry.operation = op_type;
            append_descriptor(descriptor, run_entry, size);

            // Mark other processes waiting
            for (node_t *o = rr_queue->head; o != NULL; o = o->next) {
                if (o->proc.arrival_time_p <= current_time && o->proc.begining_date <= current_time) {
                    process_descriptor_t wait_entry;
                    wait_entry.process_name = o->proc.process_name;
                    wait_entry.date = current_time;
                    wait_entry.state = waiting_p;
                    wait_entry.operation = none;
                    append_descriptor(descriptor, wait_entry, size);
                }
            }

            current_time++;
        }

        // Re-add process to queue if it still has remaining operations
        int has_work = 0;
        for (int j = 0; j < proc->operations_count; j++)
            if (proc->descriptor_p[j].duration_op > 0) has_work = 1;

        if (has_work) add_tail(rr_queue, *proc);
        else {
            process_descriptor_t term;
            term.process_name = proc->process_name;
            term.date = current_time;
            term.state = terminated_p;
            term.operation = none;
            append_descriptor(descriptor, term, size);
        }
    }

    free(rr_queue);
}

