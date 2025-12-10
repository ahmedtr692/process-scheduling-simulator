#include "basic_sched.h"
#include <stdlib.h>

// Process state tracking for concurrent execution
typedef struct {
    int op_index;          // Current operation index
    int op_remaining;      // Remaining time for current operation
    int arrived;           // Has process arrived?
    int terminated;        // Is process done?
} proc_state_t;

// FIFO: First In First Out with concurrent I/O-CALC support
// - Processes execute in arrival order (FIFO queue discipline)
// - When current process does CALC, next process can do I/O concurrently
// - Only ONE I/O device: processes cannot do I/O simultaneously
void fifo_sched(process_queue *p, process_descriptor_t **descriptor, int* size) {
    if (p->size == 0) return;

    int n = p->size;
    
    // Copy processes to array and sort by arrival time
    process_t *procs = malloc(n * sizeof(process_t));
    proc_state_t *states = calloc(n, sizeof(proc_state_t));
    
    node_t *cur = p->head;
    for (int i = 0; i < n; i++) {
        procs[i] = cur->proc;
        cur = cur->next;
    }
    
    // Bubble sort by arrival time
    for (int i = 0; i < n-1; i++) {
        for (int j = 0; j < n-i-1; j++) {
            int arr1 = (procs[j].begining_date > 0) ? procs[j].begining_date : procs[j].arrival_time_p;
            int arr2 = (procs[j+1].begining_date > 0) ? procs[j+1].begining_date : procs[j+1].arrival_time_p;
            if (arr1 > arr2) {
                process_t tmp = procs[j];
                procs[j] = procs[j+1];
                procs[j+1] = tmp;
            }
        }
    }

    int current_time = 0;
    int finished = 0;
    int cpu_process = -1;      // Which process has CPU (-1 = none)
    int io_process = -1;       // Which process has I/O device (-1 = none)
    
    while (finished < n) {
        // Check arrivals
        for (int i = 0; i < n; i++) {
            if (!states[i].arrived && !states[i].terminated) {
                int arr = (procs[i].begining_date > 0) ? procs[i].begining_date : procs[i].arrival_time_p;
                if (current_time >= arr) {
                    states[i].arrived = 1;
                }
            }
        }
        
        // Allocate CPU to first waiting process (FIFO order)
        cpu_process = -1;
        for (int i = 0; i < n; i++) {
            if (states[i].arrived && !states[i].terminated) {
                if (states[i].op_index < procs[i].operations_count) {
                    process_operation_t op = procs[i].descriptor_p[states[i].op_index].operation_p;
                    if (op == calc_p) {
                        cpu_process = i;
                        break;  // First eligible process gets CPU
                    }
                }
            }
        }
        
        // Allocate I/O device to first process needing it (only if no other I/O running)
        io_process = -1;
        for (int i = 0; i < n; i++) {
            if (states[i].arrived && !states[i].terminated) {
                if (states[i].op_index < procs[i].operations_count) {
                    process_operation_t op = procs[i].descriptor_p[states[i].op_index].operation_p;
                    if (op == IO_p) {
                        io_process = i;
                        break;  // First I/O request gets device
                    }
                }
            }
        }
        
        // Execute operations for this time tick
        for (int i = 0; i < n; i++) {
            if (!states[i].arrived || states[i].terminated) continue;
            
            if (states[i].op_index < procs[i].operations_count) {
                process_operation_t op = procs[i].descriptor_p[states[i].op_index].operation_p;
                
                // Check if this process can execute
                int can_execute = 0;
                if (op == calc_p && i == cpu_process) can_execute = 1;
                if (op == IO_p && i == io_process) can_execute = 1;
                
                if (can_execute) {
                    // Initialize remaining time if starting new operation
                    if (states[i].op_remaining == 0) {
                        states[i].op_remaining = procs[i].descriptor_p[states[i].op_index].duration_op;
                    }
                    
                    // Record running state
                    process_descriptor_t entry;
                    entry.process_name = procs[i].process_name;
                    entry.date = current_time;
                    entry.state = running_p;
                    entry.operation = op;
                    append_descriptor(descriptor, entry, size);
                    
                    states[i].op_remaining--;
                    
                    // Move to next operation if current one done
                    if (states[i].op_remaining == 0) {
                        states[i].op_index++;
                        
                        // Check if process is completely done
                        if (states[i].op_index >= procs[i].operations_count) {
                            states[i].terminated = 1;
                        }
                    }
                } else {
                    // Process is waiting
                    process_descriptor_t entry;
                    entry.process_name = procs[i].process_name;
                    entry.date = current_time;
                    entry.state = waiting_p;
                    entry.operation = none;
                    append_descriptor(descriptor, entry, size);
                }
            }
        }
        
        // Mark terminated processes
        for (int i = 0; i < n; i++) {
            if (states[i].arrived && states[i].terminated && states[i].op_index == procs[i].operations_count) {
                process_descriptor_t entry;
                entry.process_name = procs[i].process_name;
                entry.date = current_time;
                entry.state = terminated_p;
                entry.operation = none;
                append_descriptor(descriptor, entry, size);
                
                states[i].op_index++;  // Prevent re-termination
                finished++;
            }
        }
        
        current_time++;
        
        // Safety timeout
        if (current_time > 10000) break;
    }
    
    free(procs);
    free(states);
}
