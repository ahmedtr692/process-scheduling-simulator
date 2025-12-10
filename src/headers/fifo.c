 #include "basic_sched.h"

// Track process state for concurrent execution
typedef struct {
    node_t *node;
    int op_index;
    int op_remaining;
    int io_until;  // -1 if not doing I/O, otherwise time when I/O completes
    int terminated;
} proc_state_t;

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

    // Sort by arrival time
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

    // Initialize process states
    int n = fifo_queue->size;
    proc_state_t states[100];
    int idx = 0;
    for (node_t *cur = fifo_queue->head; cur != NULL; cur = cur->next) {
        states[idx].node = cur;
        states[idx].op_index = 0;
        states[idx].op_remaining = (cur->proc.operations_count > 0) ? 
                                    cur->proc.descriptor_p[0].duration_op : 0;
        states[idx].io_until = -1;
        states[idx].terminated = 0;
        idx++;
    }

    int current_time = 0;
    int cpu_process = -1;  // Which process currently has CPU (-1 = none)
    int finished = 0;
    
    while (finished < n) {
        process_descriptor_t entry;

        // Check if any I/O operations complete
        for (int i = 0; i < n; i++) {
            if (states[i].io_until == current_time) {
                states[i].io_until = -1;
                states[i].op_index++;
                if (states[i].op_index < states[i].node->proc.operations_count) {
                    states[i].op_remaining = states[i].node->proc.descriptor_p[states[i].op_index].duration_op;
                } else {
                    states[i].op_remaining = 0;
                }
            }
        }

        // Assign CPU if free
        if (cpu_process == -1 || states[cpu_process].op_remaining == 0 || 
            states[cpu_process].io_until != -1) {
            cpu_process = -1;
            // Find next process that needs CPU
            for (int i = 0; i < n; i++) {
                if (states[i].terminated) continue;
                if (states[i].node->proc.arrival_time_p > current_time) continue;
                if (states[i].node->proc.begining_date > current_time) continue;
                if (states[i].io_until != -1) continue;  // Doing I/O
                if (states[i].op_remaining == 0) {
                    // Process finished all operations
                    states[i].terminated = 1;
                    finished++;
                    entry.process_name = states[i].node->proc.process_name;
                    entry.date = current_time;
                    entry.state = terminated_p;
                    entry.operation = none;
                    append_descriptor(descriptor, entry, size);
                    continue;
                }
                // This process gets CPU
                cpu_process = i;
                break;
            }
        }

        // Execute current time tick
        for (int i = 0; i < n; i++) {
            if (states[i].terminated) continue;
            if (states[i].node->proc.arrival_time_p > current_time) continue;
            if (states[i].node->proc.begining_date > current_time) continue;

            entry.process_name = states[i].node->proc.process_name;
            entry.date = current_time;

            if (i == cpu_process) {
                // This process has CPU
                process_operation_t op = states[i].node->proc.descriptor_p[states[i].op_index].operation_p;
                entry.state = running_p;
                entry.operation = op;
                append_descriptor(descriptor, entry, size);

                if (op == IO_p) {
                    // Start I/O operation (releases CPU immediately)
                    states[i].io_until = current_time + states[i].op_remaining;
                    states[i].op_remaining = 0;
                    cpu_process = -1;  // Release CPU
                } else {
                    // CALC operation
                    states[i].op_remaining--;
                    if (states[i].op_remaining == 0) {
                        states[i].op_index++;
                        if (states[i].op_index < states[i].node->proc.operations_count) {
                            states[i].op_remaining = states[i].node->proc.descriptor_p[states[i].op_index].duration_op;
                        }
                    }
                }
            } else if (states[i].io_until != -1) {
                // Doing I/O operation
                entry.state = running_p;
                entry.operation = IO_p;
                append_descriptor(descriptor, entry, size);
            } else {
                // Waiting for CPU
                entry.state = waiting_p;
                entry.operation = none;
                append_descriptor(descriptor, entry, size);
            }
        }

        current_time++;
        if (current_time > 10000) break;  // Safety limit
    }

    while (fifo_queue->size > 0) remove_head(fifo_queue);
    free(fifo_queue);
}
