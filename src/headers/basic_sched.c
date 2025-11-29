#define _POSIX_C_SOURCE 200809L
#include "basic_sched.h"
#include <stdbool.h>
#include <ctype.h>

/* ============== Queue Operations ============== */

process_queue* create_queue(void) {
    process_queue *q = (process_queue *)malloc(sizeof(process_queue));
    if (q == NULL) return NULL;
    q->head = NULL;
    q->tail = NULL;
    q->size = 0;
    return q;
}

void enqueue(process_queue *q, process_t proc) {
    if (q == NULL) return;
    
    node_t *new_node = (node_t *)malloc(sizeof(node_t));
    if (new_node == NULL) return;
    
    new_node->proc = proc;
    new_node->next = NULL;
    
    if (q->tail == NULL) {
        q->head = new_node;
        q->tail = new_node;
    } else {
        q->tail->next = new_node;
        q->tail = new_node;
    }
    q->size++;
}

process_t* dequeue(process_queue *q) {
    if (q == NULL || q->head == NULL) return NULL;
    
    node_t *temp = q->head;
    process_t *proc = (process_t *)malloc(sizeof(process_t));
    if (proc == NULL) return NULL;
    
    *proc = temp->proc;
    q->head = temp->next;
    
    if (q->head == NULL) {
        q->tail = NULL;
    }
    
    free(temp);
    q->size--;
    return proc;
}

int is_queue_empty(process_queue *q) {
    return (q == NULL || q->head == NULL);
}

void free_queue(process_queue *q) {
    if (q == NULL) return;
    
    node_t *current = q->head;
    while (current != NULL) {
        node_t *next = current->next;
        if (current->proc.process_name) free(current->proc.process_name);
        if (current->proc.descriptor_p.operations) free(current->proc.descriptor_p.operations);
        free(current);
        current = next;
    }
    free(q);
}

process_queue* copy_queue(process_queue *q) {
    if (q == NULL) return NULL;
    
    process_queue *new_q = create_queue();
    if (new_q == NULL) return NULL;
    
    node_t *current = q->head;
    while (current != NULL) {
        process_t proc_copy;
        proc_copy.process_name = strdup(current->proc.process_name);
        proc_copy.arrival_time_p = current->proc.arrival_time_p;
        proc_copy.priority_p = current->proc.priority_p;
        proc_copy.burst_time = current->proc.burst_time;
        proc_copy.remaining_time = current->proc.remaining_time;
        proc_copy.quantum_p = current->proc.quantum_p;
        proc_copy.begining_date = current->proc.begining_date;
        proc_copy.descriptor_p.count = current->proc.descriptor_p.count;
        proc_copy.descriptor_p.operations = NULL;
        
        if (current->proc.descriptor_p.operations && current->proc.descriptor_p.count > 0) {
            proc_copy.descriptor_p.operations = (operation_t *)malloc(
                sizeof(operation_t) * current->proc.descriptor_p.count);
            if (proc_copy.descriptor_p.operations) {
                memcpy(proc_copy.descriptor_p.operations, 
                       current->proc.descriptor_p.operations,
                       sizeof(operation_t) * current->proc.descriptor_p.count);
            }
        }
        
        enqueue(new_q, proc_copy);
        current = current->next;
    }
    return new_q;
}

/* ============== Process Operations ============== */

process_t* create_process(const char *name, int arrival, int priority, int burst) {
    process_t *p = (process_t *)malloc(sizeof(process_t));
    if (p == NULL) return NULL;
    
    p->process_name = strdup(name);
    p->arrival_time_p = arrival;
    p->priority_p = priority;
    p->burst_time = burst;
    p->remaining_time = burst;
    p->quantum_p = 0;
    p->begining_date = 0;
    p->descriptor_p.operations = NULL;
    p->descriptor_p.count = 0;
    
    return p;
}

void free_process(process_t *p) {
    if (p == NULL) return;
    if (p->process_name) free(p->process_name);
    if (p->descriptor_p.operations) free(p->descriptor_p.operations);
    free(p);
}

int get_total_burst_time(process_t *p) {
    if (p == NULL) return 0;
    return p->burst_time;
}

/* ============== Simulation Result Operations ============== */

simulation_result_t* create_simulation_result(void) {
    simulation_result_t *result = (simulation_result_t *)malloc(sizeof(simulation_result_t));
    if (result == NULL) return NULL;
    
    result->capacity = MAX_DESCRIPTOR_ENTRIES;
    result->entries = (process_descriptor_t **)malloc(
        sizeof(process_descriptor_t *) * result->capacity);
    if (result->entries == NULL) {
        free(result);
        return NULL;
    }
    result->count = 0;
    return result;
}

void add_result_entry(simulation_result_t *result, int date, const char *name,
                      process_state state, process_operation_t op, int duration) {
    if (result == NULL || result->count >= result->capacity) return;
    
    process_descriptor_t *entry = (process_descriptor_t *)malloc(sizeof(process_descriptor_t));
    if (entry == NULL) return;
    
    entry->date = date;
    entry->process_name = strdup(name);
    entry->state = state;
    entry->operation = op;
    entry->duration = duration;
    
    result->entries[result->count++] = entry;
}

void free_simulation_result(simulation_result_t *result) {
    if (result == NULL) return;
    
    for (int i = 0; i < result->count; i++) {
        if (result->entries[i]) {
            if (result->entries[i]->process_name) 
                free(result->entries[i]->process_name);
            free(result->entries[i]);
        }
    }
    free(result->entries);
    free(result);
}

void print_simulation_result(simulation_result_t *result) {
    if (result == NULL) {
        printf("No simulation results.\n");
        return;
    }
    
    printf("\n========== Simulation Results ==========\n");
    printf("%-6s %-15s %-12s %-10s %-10s\n", 
           "Time", "Process", "State", "Operation", "Duration");
    printf("--------------------------------------------------\n");
    
    for (int i = 0; i < result->count; i++) {
        process_descriptor_t *e = result->entries[i];
        const char *state_str;
        const char *op_str;
        
        switch (e->state) {
            case running_p: state_str = "Running"; break;
            case ready_p: state_str = "Ready"; break;
            case blocked_p: state_str = "Blocked"; break;
            case waiting_p: state_str = "Waiting"; break;
            case terminated_p: state_str = "Terminated"; break;
            default: state_str = "Unknown"; break;
        }
        
        switch (e->operation) {
            case calc_p: op_str = "CPU"; break;
            case IO_p: op_str = "I/O"; break;
            default: op_str = "None"; break;
        }
        
        printf("%-6d %-15s %-12s %-10s %-10d\n",
               e->date, e->process_name, state_str, op_str, e->duration);
    }
    printf("==========================================\n");
}

/* ============== Configuration File Parsing ============== */

static char* trim_whitespace(char *str) {
    while (isspace((unsigned char)*str)) str++;
    if (*str == 0) return str;
    
    char *end = str + strlen(str) - 1;
    while (end > str && isspace((unsigned char)*end)) end--;
    end[1] = '\0';
    return str;
}

process_queue* parse_config_file(const char *filename) {
    FILE *file = fopen(filename, "r");
    if (file == NULL) {
        fprintf(stderr, "Error: Cannot open config file '%s'\n", filename);
        return NULL;
    }
    
    process_queue *q = create_queue();
    if (q == NULL) {
        fclose(file);
        return NULL;
    }
    
    char line[MAX_LINE_LEN];
    int line_num = 0;
    
    while (fgets(line, sizeof(line), file)) {
        line_num++;
        char *trimmed = trim_whitespace(line);
        
        /* Skip empty lines and comments */
        if (*trimmed == '\0' || *trimmed == '#' || *trimmed == ';') {
            continue;
        }
        
        /* Parse process line: name arrival_time burst_time priority [quantum] */
        char name[MAX_NAME_LEN];
        int arrival, burst, priority, quantum = 0;
        
        int parsed = sscanf(trimmed, "%63s %d %d %d %d", 
                           name, &arrival, &burst, &priority, &quantum);
        
        if (parsed < 4) {
            fprintf(stderr, "Warning: Line %d: Invalid format, skipping\n", line_num);
            continue;
        }
        
        process_t proc;
        proc.process_name = strdup(name);
        proc.arrival_time_p = arrival;
        proc.burst_time = burst;
        proc.remaining_time = burst;
        proc.priority_p = priority;
        proc.quantum_p = quantum;
        proc.begining_date = 0;
        
        /* Create a simple CPU operation for the burst time */
        proc.descriptor_p.count = 1;
        proc.descriptor_p.operations = (operation_t *)malloc(sizeof(operation_t));
        if (proc.descriptor_p.operations) {
            proc.descriptor_p.operations[0].operation_p = calc_p;
            proc.descriptor_p.operations[0].duration_op = burst;
        }
        
        enqueue(q, proc);
    }
    
    fclose(file);
    printf("Loaded %d processes from '%s'\n", q->size, filename);
    return q;
}

/* ============== Helper Functions ============== */

static void rotate_queue(process_queue *p) {
    if (p == NULL || p->head == NULL || p->head->next == NULL) {
        return;
    }
    
    node_t *first = p->head;
    p->head = first->next;
    first->next = NULL;
    
    if (p->tail == NULL) {
        p->tail = first;
    } else {
        p->tail->next = first;
        p->tail = first;
    }
}

__attribute__((unused))
static node_t* find_earliest_arrival(process_queue *q, int current_time) {
    if (q == NULL || q->head == NULL) return NULL;
    
    node_t *earliest = NULL;
    node_t *current = q->head;
    
    while (current != NULL) {
        if (current->proc.arrival_time_p <= current_time && current->proc.remaining_time > 0) {
            if (earliest == NULL || 
                current->proc.arrival_time_p < earliest->proc.arrival_time_p) {
                earliest = current;
            }
        }
        current = current->next;
    }
    return earliest;
}

static node_t* find_highest_priority(process_queue *q, int current_time) {
    if (q == NULL || q->head == NULL) return NULL;
    
    node_t *highest = NULL;
    node_t *current = q->head;
    
    while (current != NULL) {
        if (current->proc.arrival_time_p <= current_time && current->proc.remaining_time > 0) {
            if (highest == NULL || 
                current->proc.priority_p < highest->proc.priority_p) {
                highest = current;
            }
        }
        current = current->next;
    }
    return highest;
}

static int all_processes_done(process_queue *q) {
    if (q == NULL || q->head == NULL) return 1;
    
    node_t *current = q->head;
    while (current != NULL) {
        if (current->proc.remaining_time > 0) return 0;
        current = current->next;
    }
    return 1;
}

static int get_next_arrival(process_queue *q, int current_time) {
    if (q == NULL || q->head == NULL) return -1;
    
    int next = -1;
    node_t *current = q->head;
    while (current != NULL) {
        if (current->proc.arrival_time_p > current_time && current->proc.remaining_time > 0) {
            if (next == -1 || current->proc.arrival_time_p < next) {
                next = current->proc.arrival_time_p;
            }
        }
        current = current->next;
    }
    return next;
}

/* ============== FIFO Scheduler ============== */

void fifo_sched(process_queue* p, process_descriptor_t* descriptor[], int mode) {
    (void)descriptor;  /* Unused in this version */
    if (p == NULL) return;
    
    if (mode == 1) {
        rotate_queue(p);
        return;
    }
    
    /* Legacy mode support - not used in new implementation */
}

simulation_result_t* run_fifo(process_queue *q) {
    if (q == NULL || q->head == NULL) return NULL;
    
    simulation_result_t *result = create_simulation_result();
    if (result == NULL) return NULL;
    
    process_queue *work_q = copy_queue(q);
    if (work_q == NULL) {
        free_simulation_result(result);
        return NULL;
    }
    
    int current_time = 0;
    
    /* Sort by arrival time (simple insertion sort) */
    node_t *sorted = NULL;
    while (work_q->head != NULL) {
        node_t *current = work_q->head;
        work_q->head = work_q->head->next;
        
        if (sorted == NULL || current->proc.arrival_time_p < sorted->proc.arrival_time_p) {
            current->next = sorted;
            sorted = current;
        } else {
            node_t *s = sorted;
            while (s->next != NULL && s->next->proc.arrival_time_p <= current->proc.arrival_time_p) {
                s = s->next;
            }
            current->next = s->next;
            s->next = current;
        }
    }
    work_q->head = sorted;
    
    /* Run FIFO scheduling */
    node_t *proc_node = work_q->head;
    while (proc_node != NULL) {
        process_t *proc = &proc_node->proc;
        
        /* Wait for process arrival */
        if (current_time < proc->arrival_time_p) {
            current_time = proc->arrival_time_p;
        }
        
        /* Record the execution */
        add_result_entry(result, current_time, proc->process_name, 
                        running_p, calc_p, proc->burst_time);
        
        current_time += proc->burst_time;
        proc->remaining_time = 0;
        
        /* Record completion */
        add_result_entry(result, current_time, proc->process_name,
                        terminated_p, none, 0);
        
        proc_node = proc_node->next;
    }
    
    free_queue(work_q);
    return result;
}

/* ============== Round Robin Scheduler ============== */

simulation_result_t* run_round_robin(process_queue *q, int quantum) {
    if (q == NULL || q->head == NULL || quantum <= 0) return NULL;
    
    simulation_result_t *result = create_simulation_result();
    if (result == NULL) return NULL;
    
    process_queue *work_q = copy_queue(q);
    if (work_q == NULL) {
        free_simulation_result(result);
        return NULL;
    }
    
    int current_time = 0;
    process_queue *ready_q = create_queue();
    if (ready_q == NULL) {
        free_queue(work_q);
        free_simulation_result(result);
        return NULL;
    }
    
    /* Initialize: add all arrived processes to ready queue */
    node_t *current = work_q->head;
    while (current != NULL) {
        current->proc.remaining_time = current->proc.burst_time;
        current = current->next;
    }
    
    while (!all_processes_done(work_q)) {
        /* Add newly arrived processes to ready queue */
        current = work_q->head;
        while (current != NULL) {
            if (current->proc.arrival_time_p <= current_time && 
                current->proc.remaining_time > 0) {
                /* Check if already in ready queue */
                int in_ready = 0;
                node_t *r = ready_q->head;
                while (r != NULL) {
                    if (strcmp(r->proc.process_name, current->proc.process_name) == 0) {
                        in_ready = 1;
                        break;
                    }
                    r = r->next;
                }
                if (!in_ready) {
                    process_t proc_copy;
                    proc_copy.process_name = current->proc.process_name;
                    proc_copy.arrival_time_p = current->proc.arrival_time_p;
                    proc_copy.burst_time = current->proc.burst_time;
                    proc_copy.remaining_time = current->proc.remaining_time;
                    proc_copy.priority_p = current->proc.priority_p;
                    proc_copy.quantum_p = current->proc.quantum_p;
                    proc_copy.descriptor_p.operations = NULL;
                    proc_copy.descriptor_p.count = 0;
                    enqueue(ready_q, proc_copy);
                }
            }
            current = current->next;
        }
        
        if (is_queue_empty(ready_q)) {
            /* No process ready, advance time to next arrival */
            int next_arr = get_next_arrival(work_q, current_time);
            if (next_arr == -1) break;
            current_time = next_arr;
            continue;
        }
        
        /* Get front process from ready queue */
        node_t *front = ready_q->head;
        if (front == NULL) break;
        
        /* Find corresponding process in work queue */
        node_t *work_proc = work_q->head;
        while (work_proc != NULL) {
            if (strcmp(work_proc->proc.process_name, front->proc.process_name) == 0) {
                break;
            }
            work_proc = work_proc->next;
        }
        
        if (work_proc == NULL || work_proc->proc.remaining_time <= 0) {
            /* Remove from ready queue */
            ready_q->head = front->next;
            if (ready_q->head == NULL) ready_q->tail = NULL;
            free(front);
            ready_q->size--;
            continue;
        }
        
        /* Execute for quantum or remaining time */
        int exec_time = (work_proc->proc.remaining_time < quantum) ? 
                        work_proc->proc.remaining_time : quantum;
        
        add_result_entry(result, current_time, work_proc->proc.process_name,
                        running_p, calc_p, exec_time);
        
        current_time += exec_time;
        work_proc->proc.remaining_time -= exec_time;
        
        /* Remove from front of ready queue */
        ready_q->head = front->next;
        if (ready_q->head == NULL) ready_q->tail = NULL;
        free(front);
        ready_q->size--;
        
        if (work_proc->proc.remaining_time <= 0) {
            /* Process completed */
            add_result_entry(result, current_time, work_proc->proc.process_name,
                            terminated_p, none, 0);
        } else {
            /* Add back to ready queue (will be added in next iteration) */
        }
    }
    
    /* Cleanup ready queue without freeing process names (they belong to work_q) */
    while (ready_q->head != NULL) {
        node_t *temp = ready_q->head;
        ready_q->head = temp->next;
        free(temp);
    }
    free(ready_q);
    free_queue(work_q);
    
    return result;
}

/* ============== Priority Preemptive Scheduler ============== */

void priority_sched(process_queue* p, int preemptive, process_descriptor_t* descriptor[]) {
    (void)p;
    (void)preemptive;
    (void)descriptor;
    /* Legacy interface - use run_priority_preemptive instead */
}

simulation_result_t* run_priority_preemptive(process_queue *q) {
    if (q == NULL || q->head == NULL) return NULL;
    
    simulation_result_t *result = create_simulation_result();
    if (result == NULL) return NULL;
    
    process_queue *work_q = copy_queue(q);
    if (work_q == NULL) {
        free_simulation_result(result);
        return NULL;
    }
    
    int current_time = 0;
    node_t *running = NULL;
    int running_start = 0;
    
    while (!all_processes_done(work_q)) {
        /* Find highest priority ready process */
        node_t *highest = find_highest_priority(work_q, current_time);
        
        if (highest == NULL) {
            /* No process ready, advance time */
            int next_arr = get_next_arrival(work_q, current_time);
            if (next_arr == -1) break;
            
            if (running != NULL) {
                /* Record partial execution */
                int exec_time = current_time - running_start;
                if (exec_time > 0) {
                    add_result_entry(result, running_start, running->proc.process_name,
                                    running_p, calc_p, exec_time);
                }
                running = NULL;
            }
            current_time = next_arr;
            continue;
        }
        
        /* Check for preemption */
        if (running != NULL && running != highest) {
            /* Preempt current process */
            int exec_time = current_time - running_start;
            if (exec_time > 0) {
                add_result_entry(result, running_start, running->proc.process_name,
                                running_p, calc_p, exec_time);
            }
            running = highest;
            running_start = current_time;
        } else if (running == NULL) {
            running = highest;
            running_start = current_time;
        }
        
        /* Find next event time (next arrival or completion) */
        int next_arr = get_next_arrival(work_q, current_time);
        int completion = current_time + running->proc.remaining_time;
        
        int next_event;
        if (next_arr == -1 || completion <= next_arr) {
            next_event = completion;
        } else {
            next_event = next_arr;
        }
        
        /* Execute until next event */
        int exec_time = next_event - current_time;
        running->proc.remaining_time -= exec_time;
        current_time = next_event;
        
        /* Check if process completed */
        if (running->proc.remaining_time <= 0) {
            int total_exec = current_time - running_start;
            add_result_entry(result, running_start, running->proc.process_name,
                            running_p, calc_p, total_exec);
            add_result_entry(result, current_time, running->proc.process_name,
                            terminated_p, none, 0);
            running = NULL;
        }
    }
    
    free_queue(work_q);
    return result;
}

/* ============== Multi-Level Queue Scheduler ============== */

simulation_result_t* run_multilevel_queue(process_queue *q, int num_levels, int aging_threshold) {
    if (q == NULL || q->head == NULL || num_levels <= 0) return NULL;
    
    simulation_result_t *result = create_simulation_result();
    if (result == NULL) return NULL;
    
    process_queue *work_q = copy_queue(q);
    if (work_q == NULL) {
        free_simulation_result(result);
        return NULL;
    }
    
    /* Create multiple queues for different priority levels */
    process_queue **level_queues = (process_queue **)malloc(sizeof(process_queue *) * num_levels);
    if (level_queues == NULL) {
        free_queue(work_q);
        free_simulation_result(result);
        return NULL;
    }
    
    for (int i = 0; i < num_levels; i++) {
        level_queues[i] = create_queue();
        if (level_queues[i] == NULL) {
            for (int j = 0; j < i; j++) free_queue(level_queues[j]);
            free(level_queues);
            free_queue(work_q);
            free_simulation_result(result);
            return NULL;
        }
    }
    
    /* Track waiting time for aging */
    int *waiting_time = NULL;
    if (aging_threshold > 0) {
        waiting_time = (int *)calloc(work_q->size, sizeof(int));
    }
    
    int current_time = 0;
    int quantum_per_level[] = {2, 4, 8, 16, 32};  /* Increasing quantum per level */
    
    while (!all_processes_done(work_q)) {
        /* Distribute processes to level queues based on priority */
        node_t *current = work_q->head;
        int proc_idx = 0;
        
        while (current != NULL) {
            if (current->proc.arrival_time_p <= current_time && 
                current->proc.remaining_time > 0) {
                
                /* Apply aging: promote process if waited too long */
                int level = current->proc.priority_p % num_levels;
                if (aging_threshold > 0 && waiting_time && proc_idx < work_q->size) {
                    if (waiting_time[proc_idx] >= aging_threshold && level > 0) {
                        level--;
                        waiting_time[proc_idx] = 0;
                    }
                }
                
                /* Check if already in level queue */
                int found = 0;
                node_t *q_node = level_queues[level]->head;
                while (q_node != NULL) {
                    if (strcmp(q_node->proc.process_name, current->proc.process_name) == 0) {
                        found = 1;
                        break;
                    }
                    q_node = q_node->next;
                }
                
                if (!found) {
                    process_t proc_copy;
                    proc_copy.process_name = current->proc.process_name;
                    proc_copy.arrival_time_p = current->proc.arrival_time_p;
                    proc_copy.burst_time = current->proc.burst_time;
                    proc_copy.remaining_time = current->proc.remaining_time;
                    proc_copy.priority_p = current->proc.priority_p;
                    proc_copy.quantum_p = quantum_per_level[level < 5 ? level : 4];
                    proc_copy.descriptor_p.operations = NULL;
                    proc_copy.descriptor_p.count = 0;
                    enqueue(level_queues[level], proc_copy);
                }
            }
            current = current->next;
            proc_idx++;
        }
        
        /* Find first non-empty level queue (higher priority = lower index) */
        int active_level = -1;
        for (int i = 0; i < num_levels; i++) {
            if (!is_queue_empty(level_queues[i])) {
                active_level = i;
                break;
            }
        }
        
        if (active_level == -1) {
            /* No process ready, advance time */
            int next_arr = get_next_arrival(work_q, current_time);
            if (next_arr == -1) break;
            current_time = next_arr;
            
            /* Update waiting times */
            if (waiting_time) {
                for (int i = 0; i < work_q->size; i++) {
                    waiting_time[i]++;
                }
            }
            continue;
        }
        
        /* Get process from active level queue */
        node_t *front = level_queues[active_level]->head;
        if (front == NULL) continue;
        
        /* Find in work queue */
        node_t *work_proc = work_q->head;
        while (work_proc != NULL) {
            if (strcmp(work_proc->proc.process_name, front->proc.process_name) == 0) {
                break;
            }
            work_proc = work_proc->next;
        }
        
        if (work_proc == NULL || work_proc->proc.remaining_time <= 0) {
            /* Remove from level queue */
            level_queues[active_level]->head = front->next;
            if (level_queues[active_level]->head == NULL) 
                level_queues[active_level]->tail = NULL;
            free(front);
            level_queues[active_level]->size--;
            continue;
        }
        
        /* Execute for quantum or remaining time */
        int quantum = quantum_per_level[active_level < 5 ? active_level : 4];
        int exec_time = (work_proc->proc.remaining_time < quantum) ?
                        work_proc->proc.remaining_time : quantum;
        
        add_result_entry(result, current_time, work_proc->proc.process_name,
                        running_p, calc_p, exec_time);
        
        current_time += exec_time;
        work_proc->proc.remaining_time -= exec_time;
        
        /* Remove from level queue */
        level_queues[active_level]->head = front->next;
        if (level_queues[active_level]->head == NULL)
            level_queues[active_level]->tail = NULL;
        free(front);
        level_queues[active_level]->size--;
        
        if (work_proc->proc.remaining_time <= 0) {
            add_result_entry(result, current_time, work_proc->proc.process_name,
                            terminated_p, none, 0);
        } else {
            /* Demote to lower priority level (if not already lowest) */
            if (active_level < num_levels - 1) {
                work_proc->proc.priority_p = active_level + 1;
            }
        }
        
        /* Update waiting times for other processes */
        if (waiting_time) {
            node_t *w = work_q->head;
            int idx = 0;
            while (w != NULL) {
                if (w != work_proc && w->proc.remaining_time > 0 &&
                    w->proc.arrival_time_p <= current_time) {
                    waiting_time[idx] += exec_time;
                }
                w = w->next;
                idx++;
            }
        }
    }
    
    /* Cleanup */
    for (int i = 0; i < num_levels; i++) {
        while (level_queues[i]->head != NULL) {
            node_t *temp = level_queues[i]->head;
            level_queues[i]->head = temp->next;
            free(temp);
        }
        free(level_queues[i]);
    }
    free(level_queues);
    if (waiting_time) free(waiting_time);
    free_queue(work_q);
    
    return result;
}

/* ============== Display Functions ============== */

void print_gantt_chart(simulation_result_t *result) {
    if (result == NULL || result->count == 0) {
        printf("No data to display.\n");
        return;
    }
    
    printf("\n=============== Gantt Chart ===============\n\n");
    
    /* Print timeline header */
    printf("Time: ");
    int last_time = 0;
    for (int i = 0; i < result->count; i++) {
        if (result->entries[i]->state == running_p) {
            int end_time = result->entries[i]->date + result->entries[i]->duration;
            if (end_time > last_time) last_time = end_time;
        }
    }
    
    for (int t = 0; t <= last_time && t <= 50; t += 5) {
        printf("%-5d", t);
    }
    printf("\n");
    
    /* Print execution bars for each process */
    char *printed[MAX_PROCESSES];
    int printed_count = 0;
    
    for (int i = 0; i < result->count; i++) {
        if (result->entries[i]->state != running_p) continue;
        
        /* Check if already printed */
        int found = 0;
        for (int j = 0; j < printed_count; j++) {
            if (strcmp(printed[j], result->entries[i]->process_name) == 0) {
                found = 1;
                break;
            }
        }
        
        if (!found && printed_count < MAX_PROCESSES) {
            printed[printed_count++] = result->entries[i]->process_name;
            
            printf("%-6s", result->entries[i]->process_name);
            
            int col = 0;
            for (int j = 0; j < result->count; j++) {
                if (strcmp(result->entries[j]->process_name, result->entries[i]->process_name) == 0 &&
                    result->entries[j]->state == running_p) {
                    
                    /* Print gaps */
                    while (col < result->entries[j]->date && col < 50) {
                        printf(" ");
                        col++;
                    }
                    
                    /* Print execution */
                    for (int k = 0; k < result->entries[j]->duration && col < 50; k++) {
                        printf("#");
                        col++;
                    }
                }
            }
            printf("\n");
        }
    }
    
    printf("\n# = Process executing\n");
    printf("===========================================\n");
}

void print_process_stats(process_queue *original, simulation_result_t *result) {
    if (original == NULL || result == NULL) return;
    
    printf("\n=========== Process Statistics ===========\n");
    printf("%-12s %-10s %-10s %-10s %-12s %-12s\n",
           "Process", "Arrival", "Burst", "Completion", "Turnaround", "Waiting");
    printf("--------------------------------------------------------------------\n");
    
    float total_turnaround = 0;
    float total_waiting = 0;
    int count = 0;
    
    node_t *current = original->head;
    while (current != NULL) {
        const char *name = current->proc.process_name;
        int arrival = current->proc.arrival_time_p;
        int burst = current->proc.burst_time;
        int completion = 0;
        
        /* Find completion time */
        for (int i = 0; i < result->count; i++) {
            if (strcmp(result->entries[i]->process_name, name) == 0 &&
                result->entries[i]->state == terminated_p) {
                completion = result->entries[i]->date;
                break;
            }
        }
        
        if (completion == 0) {
            /* Find last running entry */
            for (int i = result->count - 1; i >= 0; i--) {
                if (strcmp(result->entries[i]->process_name, name) == 0 &&
                    result->entries[i]->state == running_p) {
                    completion = result->entries[i]->date + result->entries[i]->duration;
                    break;
                }
            }
        }
        
        int turnaround = completion - arrival;
        int waiting = turnaround - burst;
        
        printf("%-12s %-10d %-10d %-10d %-12d %-12d\n",
               name, arrival, burst, completion, turnaround, waiting);
        
        total_turnaround += turnaround;
        total_waiting += waiting;
        count++;
        
        current = current->next;
    }
    
    printf("--------------------------------------------------------------------\n");
    if (count > 0) {
        printf("Average Turnaround Time: %.2f\n", total_turnaround / count);
        printf("Average Waiting Time: %.2f\n", total_waiting / count);
    }
    printf("===========================================\n");
}
