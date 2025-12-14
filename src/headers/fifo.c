#include "basic_sched.h"
#include <stdlib.h>

// Process state tracking for FIFO with concurrent I/O-CPU execution
typedef struct {
    process_t proc;
    int op_idx;           // Current operation index
    int op_remaining;     // Remaining time for current operation
    int io_until;         // Time when I/O will complete (-1 if not doing I/O)
    int terminated;       // 1 if process is terminated
    int fifo_order;       // FIFO queue position
} fifo_state_t;

void fifo_sched(process_queue *p, process_descriptor_t **descriptor, int* size) {
    if (p->size == 0) return;

    int n = p->size;
    fifo_state_t *states = calloc(n, sizeof(fifo_state_t));
    
    // Initialize process states and sort by arrival time (FIFO order)
    int idx = 0;
    for (node_t *node = p->head; node != NULL; node = node->next) {
        states[idx].proc = node->proc;
        states[idx].op_idx = 0;
        states[idx].op_remaining = (node->proc.operations_count > 0) ? 
                                     node->proc.descriptor_p[0].duration_op : 0;
        states[idx].io_until = -1;
        states[idx].terminated = 0;
        states[idx].fifo_order = idx;
        idx++;
    }

    // Sort by arrival time to establish FIFO order
    for (int i = 0; i < n - 1; i++) {
        for (int j = 0; j < n - i - 1; j++) {
            if (states[j].proc.arrival_time_p > states[j + 1].proc.arrival_time_p) {
                fifo_state_t temp = states[j];
                states[j] = states[j + 1];
                states[j + 1] = temp;
            }
        }
    }

    int current_time = 0;
    int finished = 0;
    int max_time = 10000; // Safety timeout

    while (finished < n && current_time < max_time) {
        int cpu_assigned = -1;
        int io_assigned = -1;

        // Find first ready process for CPU (FIFO order, only CALC operations)
        for (int k = 0; k < n; k++) {
            if (states[k].terminated) continue;
            if (states[k].proc.arrival_time_p > current_time) continue;
            if (states[k].io_until >= 0) continue; // Doing I/O
            
            // Check if has CALC operation
            if (states[k].op_idx < states[k].proc.operations_count &&
                states[k].proc.descriptor_p[states[k].op_idx].operation_p == calc_p) {
                cpu_assigned = k;
                break; // FIFO: take the first ready process
            }
        }

        // Find process for I/O device (only one I/O at a time)
        for (int k = 0; k < n; k++) {
            if (states[k].terminated) continue;
            if (states[k].proc.arrival_time_p > current_time) continue;
            if (states[k].io_until >= 0 && states[k].io_until > current_time) {
                io_assigned = k; // Continue existing I/O
                break;
            }
        }

        // If no ongoing I/O, assign to first process needing it (FIFO order)
        if (io_assigned == -1) {
            for (int k = 0; k < n; k++) {
                if (states[k].terminated) continue;
                if (states[k].proc.arrival_time_p > current_time) continue;
                if (states[k].io_until >= 0) continue; // Already counted
                
                if (states[k].op_idx < states[k].proc.operations_count &&
                    states[k].proc.descriptor_p[states[k].op_idx].operation_p == IO_p) {
                    io_assigned = k;
                    // Start I/O
                    states[k].io_until = current_time + states[k].op_remaining;
                    break; // FIFO: take the first ready process
                }
            }
        }

        // Execute CPU operation
        if (cpu_assigned >= 0) {
            fifo_state_t *ps = &states[cpu_assigned];
            
            process_descriptor_t entry;
            entry.process_name = ps->proc.process_name;
            entry.date = current_time;
            entry.state = running_p;
            entry.operation = calc_p;
            append_descriptor(descriptor, entry, size);

            ps->op_remaining--;

            // Check if operation completed
            if (ps->op_remaining == 0) {
                ps->op_idx++;
                if (ps->op_idx < ps->proc.operations_count) {
                    ps->op_remaining = ps->proc.descriptor_p[ps->op_idx].duration_op;
                }
            }
        }

        // Execute I/O operation
        if (io_assigned >= 0) {
            fifo_state_t *ps = &states[io_assigned];
            
            process_descriptor_t entry;
            entry.process_name = ps->proc.process_name;
            entry.date = current_time;
            entry.state = running_p;
            entry.operation = IO_p;
            append_descriptor(descriptor, entry, size);

            // Check if I/O completed
            if (ps->io_until <= current_time + 1) {
                ps->op_idx++;
                ps->io_until = -1;
                if (ps->op_idx < ps->proc.operations_count) {
                    ps->op_remaining = ps->proc.descriptor_p[ps->op_idx].duration_op;
                }
            }
        }

        // Mark other processes as waiting
        for (int k = 0; k < n; k++) {
            if (k == cpu_assigned || k == io_assigned) continue;
            if (states[k].terminated) continue;
            if (states[k].proc.arrival_time_p > current_time) continue;

            process_descriptor_t entry;
            entry.process_name = states[k].proc.process_name;
            entry.date = current_time;
            entry.state = waiting_p;
            entry.operation = none;
            append_descriptor(descriptor, entry, size);
        }

        // Check for terminated processes
        for (int k = 0; k < n; k++) {
            if (states[k].terminated) continue;
            if (states[k].proc.arrival_time_p > current_time) continue;
            
            if (states[k].op_idx >= states[k].proc.operations_count && states[k].io_until < 0) {
                states[k].terminated = 1;
                finished++;
                
                process_descriptor_t entry;
                entry.process_name = states[k].proc.process_name;
                entry.date = current_time + 1;
                entry.state = terminated_p;
                entry.operation = none;
                append_descriptor(descriptor, entry, size);
            }
        }

        current_time++;
    }

    free(states);
}
