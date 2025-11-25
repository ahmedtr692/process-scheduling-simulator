#include "basic_sched.h"

void fifo_sched(process_queue* p, process_descriptor_t* descriptor[], int mode) {
    int calc_time = 0;
    int io_time = 0;
    int index = 0;
    node_t *current = p->head;

    if (mode == 0) {
        while (current != NULL) {
            process_t proc = current->proc;

            for (int i = 0; i < proc.descriptor_p.count; i++) {
                if (proc.descriptor_p.operations[i].operation_p == calc_p) {
                    descriptor[index]->date = calc_time;
                    descriptor[index]->state = running_p;
                    descriptor[index]->operation = calc_p;
                    calc_time += proc.descriptor_p.operations[i].duration_op;
                }
                else if (proc.descriptor_p.operations[i].operation_p == IO_p) {
                    descriptor[index]->date = io_time;
                    descriptor[index]->state = running_p;
                    descriptor[index]->operation = IO_p;
                    io_time += proc.descriptor_p.operations[i].duration_op;
                }
                index++;
            }

            current = current->next;
        }
    }
    else if (mode == 1) {
        if (p->head != NULL) {
            node_t *first = p->head;
            p->head = p->head->next;
            first->next = NULL;
            p->tail->next = first;
            p->tail = first;
        }
    }
}

void round_robin(process_queue* p, int quantum, process_descriptor_t* descriptor[]) {
    int calc_time = 0;
    int io_time = 0;
    int index = 0;

    while (p->size > 0) {
        node_t *current = p->head;
        process_t proc = current->proc;

        int op_idx = -1;
        for (int i = 0; i < proc.descriptor_p.count; i++) {
            if (proc.descriptor_p.operations[i].duration_op > 0) {
                op_idx = i;
                break;
            }
        }

        if (op_idx == -1) {
            fifo_sched(p, descriptor, 1);
            continue;
        }

        int time_slice = (proc.descriptor_p.operations[op_idx].duration_op > quantum)
                         ? quantum
                         : proc.descriptor_p.operations[op_idx].duration_op;

        if (proc.descriptor_p.operations[op_idx].operation_p == calc_p) {
            descriptor[index]->date = calc_time;
            descriptor[index]->state = running_p;
            descriptor[index]->operation = calc_p;
            calc_time += time_slice;
        }
        else if (proc.descriptor_p.operations[op_idx].operation_p == IO_p) {
            descriptor[index]->date = io_time;
            descriptor[index]->state = running_p;
            descriptor[index]->operation = IO_p;
            io_time += time_slice;
        }

        current->proc.descriptor_p.operations[op_idx].duration_op -= time_slice;
        index++;

        fifo_sched(p, descriptor, 1);
    }
}

void shortest_job_first(process_queue* p, process_descriptor_t* descriptor[]) {
}

void priority_sched(process_queue* p, int preemptive, process_descriptor_t* descriptor[]) {
}
