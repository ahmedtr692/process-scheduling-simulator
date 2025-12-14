 #include "basic_sched.h"

<<<<<<< HEAD
=======
// Process state tracking for concurrent execution
/*typedef struct {
    int op_index;          // Current operation index
    int op_remaining;      // Remaining time for current operation
    int arrived;           // Has process arrived?
    int terminated;        // Is process done?
} proc_state_t;

// FIFO: First In First Out with concurrent I/O-CALC support
// - Processes execute in arrival order (FIFO queue discipline)
// - When current process does CALC, next process can do I/O concurrently
// - Only ONE I/O device: processes cannot do I/O simultaneously
>>>>>>> aba06b296a5d4ec8b5ed9497236322f1eb8b512c
 */
void fifo_sched(process_queue *p, process_descriptor_t **descriptor, int* size) {
    if (p->size == 0) return;


    process_queue *fifo_queue = malloc(sizeof(process_queue));
    fifo_queue->head = NULL;
    fifo_queue->tail = NULL;
    fifo_queue->size = 0;


    node_t *current = p->head;
    while (current != NULL) {
        add_tail(fifo_queue, current->proc);
        current = current->next;
    }


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
    
//<<<<<<< HEAD
  int current_time = 0;
    process_descriptor_t entry;
    
    // Track which processes are terminated
    int terminated[100] = {0};
    int proc_count = 0;
    for (node_t *c = fifo_queue->head; c != NULL; c = c->next) {
        proc_count++;
    }

    int completed = 0;
    for (node_t *cur = fifo_queue->head; cur != NULL; cur = cur->next) {
        if (current_time < cur->proc.arrival_time_p) current_time = cur->proc.arrival_time_p;
        if (current_time < cur->proc.begining_date) current_time = cur->proc.begining_date;

        for (int j = 0; j < cur->proc.operations_count; j++) {
            int ticks = cur->proc.descriptor_p[j].duration_op;
            while (ticks-- > 0) {
                entry.process_name = cur->proc.process_name;
                entry.date = current_time;
                entry.state = (cur->proc.descriptor_p[j].operation_p != none) ? running_p : ready_p;
                entry.operation = cur->proc.descriptor_p[j].operation_p;
                append_descriptor(descriptor, entry, size);

                // Mark waiting for other processes that are ready but NOT terminated
                int idx = 0;
                for (node_t *others = fifo_queue->head; others != NULL; others = others->next) {
                    if (others == cur || terminated[idx]) {
                        idx++;
                        continue;
                    }
                    if (others->proc.arrival_time_p <= current_time &&
                        others->proc.begining_date <= current_time) {
                        process_descriptor_t idle_entry;
                        idle_entry.process_name = others->proc.process_name;
                        idle_entry.date = current_time;
                        idle_entry.state = waiting_p;
                        idle_entry.operation = none;
                        append_descriptor(descriptor, idle_entry, size);
                    }
                    idx++;
                }
                current_time++;
            }
        }

        // Mark this process as terminated
        terminated[completed] = 1;
        completed++;
        
        entry.process_name = cur->proc.process_name;
        entry.date = current_time;
        entry.state = terminated_p;
        entry.operation = none;
        append_descriptor(descriptor, entry, size);
        current_time++;
    }


  while (fifo_queue->size > 0) remove_head(fifo_queue);
  free(fifo_queue);
//=======
 //   free(procs);
  //  free(states);
//>>>>>>> aba06b296a5d4ec8b5ed9497236322f1eb8b512c
}
