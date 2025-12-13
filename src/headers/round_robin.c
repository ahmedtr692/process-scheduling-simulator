#include "basic_sched.h"
#include <stdlib.h>
#include <string.h>

// Process state tracking for concurrent I/O-CPU execution
typedef struct {
    process_t proc;
    int op_idx;           // Current operation index
    int op_remaining;     // Remaining time for current operation
    int cpu_time_used;    // Time used in current quantum (for CALC only)
    int io_until;         // Time when I/O will complete (-1 if not doing I/O)
    int terminated;       // 1 if process is terminated
} proc_state_t;

void round_robin_sched(process_queue *p, process_descriptor_t **descriptor, int *size, int quantum) {
    if (!p || p->size == 0 || quantum <= 0) return;

    int n = p->size;
    proc_state_t *states = calloc(n, sizeof(proc_state_t));
    
    // Initialize process states
    int idx = 0;
    for (node_t *node = p->head; node != NULL; node = node->next) {
        states[idx].proc = node->proc;
        states[idx].op_idx = 0;
        states[idx].op_remaining = (node->proc.operations_count > 0) ? 
                                     node->proc.descriptor_p[0].duration_op : 0;
        states[idx].cpu_time_used = 0;
        states[idx].io_until = -1;
        states[idx].terminated = 0;
        idx++;
    }

    int current_time = 0;
    int finished = 0;
    int max_time = 10000; // Safety timeout
    int rr_index = 0; // Round-robin queue index

    while (finished < n && current_time < max_time) {
        int cpu_assigned = -1;
        int io_assigned = -1;

        // Find process for CPU (round-robin, only CALC operations)
        for (int i = 0; i < n; i++) {
            int k = (rr_index + i) % n;
            if (states[k].terminated) continue;
            if (states[k].proc.arrival_time_p > current_time) continue;
            if (states[k].io_until >= 0) continue; // Doing I/O
            
            // Check if has CALC operation
            if (states[k].op_idx < states[k].proc.operations_count &&
                states[k].proc.descriptor_p[states[k].op_idx].operation_p == calc_p) {
                cpu_assigned = k;
                break;
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

        // If no ongoing I/O, assign to first process needing it
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
                    break;
                }
            }
        }

        // Execute CPU operation
        if (cpu_assigned >= 0) {
            proc_state_t *ps = &states[cpu_assigned];
            
            process_descriptor_t entry;
            entry.process_name = ps->proc.process_name;
            entry.date = current_time;
            entry.state = running_p;
            entry.operation = calc_p;
            append_descriptor(descriptor, entry, size);

            ps->op_remaining--;
            ps->cpu_time_used++;

            // Check if operation completed
            if (ps->op_remaining == 0) {
                ps->op_idx++;
                ps->cpu_time_used = 0;
                rr_index = (cpu_assigned + 1) % n; // Rotate to next process
                if (ps->op_idx < ps->proc.operations_count) {
                    ps->op_remaining = ps->proc.descriptor_p[ps->op_idx].duration_op;
                }
            }
            // Check if quantum expired for CALC
            else if (ps->cpu_time_used >= quantum) {
                ps->cpu_time_used = 0;
                rr_index = (cpu_assigned + 1) % n;
            }
        }

        // Execute I/O operation
        if (io_assigned >= 0) {
            proc_state_t *ps = &states[io_assigned];
            
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

