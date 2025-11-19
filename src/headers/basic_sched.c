#include "basic_sched.h"

void fifo_sched(process_queue* p) {
    // Check if queue is NULL or empty
    if (p == NULL || p->head == NULL) {
        return;
    }
    
    // Bubble sort on the linked list based on arrival time
    for (node_t *i = p->head; i != NULL; i = i->next) {
        for (node_t *j = i->next; j != NULL; j = j->next) {
            if (i->proc.arrival_time_p > j->proc.arrival_time_p) {
                // Swap process data
                process_t temp = i->proc;
                i->proc = j->proc;
                j->proc = temp;
            }
        }
    }
}

