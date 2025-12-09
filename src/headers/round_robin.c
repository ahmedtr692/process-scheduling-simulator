#include "basic_sched.h"
#include <stdlib.h>
#include <string.h>

// Time-driven simulation for concurrent I/O and CPU execution
void round_robin_sched(process_queue *p, process_descriptor_t **descriptor, int *size, int quantum) {
    if (!p || p->size == 0 || quantum <= 0) return;

    int n = p->size;
    
    // Copy all processes
    process_t *procs = malloc(n * sizeof(process_t));
    int *op_idx = malloc(n * sizeof(int));      // Current operation index
    int *op_ticks = malloc(n * sizeof(int));    // Ticks left in current operation
    int *io_end_time = malloc(n * sizeof(int)); // When I/O completes (-1 if not doing I/O)
    int *done = malloc(n * sizeof(int));        // Process completed
    int *quantum_left = malloc(n * sizeof(int)); // CPU quantum remaining
    
    if (!procs || !op_idx || !op_ticks || !io_end_time || !done || !quantum_left) {
        free(procs); free(op_idx); free(op_ticks); free(io_end_time); free(done); free(quantum_left);
        return;
    }
    
    int i = 0;
    for (node_t *cur = p->head; cur; cur = cur->next, i++) {
        procs[i] = cur->proc;
        op_idx[i] = 0;
        op_ticks[i] = (procs[i].operations_count > 0) ? procs[i].descriptor_p[0].duration_op : 0;
        io_end_time[i] = -1;
        done[i] = (procs[i].operations_count == 0);
        quantum_left[i] = 0;
    }
    
    int current_time = 0;
    int finished = 0;
    int last_cpu_proc = -1; // Track which process last used CPU for round-robin
    
    while (finished < n) {
        // Check if any I/O operations completed
        for (int k = 0; k < n; k++) {
            if (io_end_time[k] == current_time) {
                io_end_time[k] = -1;
                // Move to next operation
                op_idx[k]++;
                if (op_idx[k] >= procs[k].operations_count) {
                    done[k] = 1;
                    finished++;
                    // Mark terminated
                    process_descriptor_t term;
                    term.process_name = procs[k].process_name;
                    term.date = current_time;
                    term.state = terminated_p;
                    term.operation = none;
                    append_descriptor(descriptor, term, size);
                } else {
                    op_ticks[k] = procs[k].descriptor_p[op_idx[k]].duration_op;
                    quantum_left[k] = quantum; // Reset quantum for new operation
                }
            }
        }
        
        // Find next process for CPU (round-robin among ready processes)
        int cpu_proc = -1;
        int start_search = (last_cpu_proc + 1) % n;
        
        for (int offset = 0; offset < n; offset++) {
            int k = (start_search + offset) % n;
            if (done[k] || io_end_time[k] != -1) continue;
            if (procs[k].arrival_time_p > current_time) continue;
            if (procs[k].begining_date > current_time) continue;
            if (op_ticks[k] == 0) continue;
            
            // Check if current operation is CPU
            if (procs[k].descriptor_p[op_idx[k]].operation_p == calc_p) {
                cpu_proc = k;
                break;
            }
        }
        
        // Execute CPU operation if available
        if (cpu_proc != -1) {
            if (quantum_left[cpu_proc] == 0) {
                quantum_left[cpu_proc] = quantum;
            }
            
            process_descriptor_t entry;
            entry.process_name = procs[cpu_proc].process_name;
            entry.date = current_time;
            entry.state = running_p;
            entry.operation = calc_p;
            append_descriptor(descriptor, entry, size);
            
            op_ticks[cpu_proc]--;
            quantum_left[cpu_proc]--;
            last_cpu_proc = cpu_proc;
            
            // Check if operation completed or quantum expired
            if (op_ticks[cpu_proc] == 0) {
                op_idx[cpu_proc]++;
                if (op_idx[cpu_proc] >= procs[cpu_proc].operations_count) {
                    done[cpu_proc] = 1;
                    finished++;
                    process_descriptor_t term;
                    term.process_name = procs[cpu_proc].process_name;
                    term.date = current_time + 1;
                    term.state = terminated_p;
                    term.operation = none;
                    append_descriptor(descriptor, term, size);
                } else {
                    op_ticks[cpu_proc] = procs[cpu_proc].descriptor_p[op_idx[cpu_proc]].duration_op;
                    quantum_left[cpu_proc] = quantum;
                }
            } else if (quantum_left[cpu_proc] == 0) {
                // Quantum expired, will switch to next process
                quantum_left[cpu_proc] = quantum;
            }
        }
        
        // Start I/O operations for processes that need it (concurrent with CPU)
        for (int k = 0; k < n; k++) {
            if (done[k] || io_end_time[k] != -1) continue;
            if (procs[k].arrival_time_p > current_time) continue;
            if (procs[k].begining_date > current_time) continue;
            if (op_ticks[k] == 0) continue;
            
            // Check if current operation is I/O and not already doing I/O
            if (procs[k].descriptor_p[op_idx[k]].operation_p == IO_p) {
                // Start I/O operation
                io_end_time[k] = current_time + op_ticks[k];
                
                // Add I/O entries for the duration
                for (int t = 0; t < op_ticks[k]; t++) {
                    process_descriptor_t io_entry;
                    io_entry.process_name = procs[k].process_name;
                    io_entry.date = current_time + t;
                    io_entry.state = running_p;
                    io_entry.operation = IO_p;
                    append_descriptor(descriptor, io_entry, size);
                }
                
                op_ticks[k] = 0; // Mark as in progress
            }
        }
        
        // Mark other ready processes as waiting
        for (int k = 0; k < n; k++) {
            if (k == cpu_proc || done[k] || io_end_time[k] != -1) continue;
            if (procs[k].arrival_time_p > current_time) continue;
            if (procs[k].begining_date > current_time) continue;
            
            process_descriptor_t wait_entry;
            wait_entry.process_name = procs[k].process_name;
            wait_entry.date = current_time;
            wait_entry.state = waiting_p;
            wait_entry.operation = none;
            append_descriptor(descriptor, wait_entry, size);
        }
        
        current_time++;
        
        // Safety check to prevent infinite loops
        if (current_time > 10000) break;
    }
    
    free(procs);
    free(op_idx);
    free(op_ticks);
    free(io_end_time);
    free(done);
    free(quantum_left);
}
