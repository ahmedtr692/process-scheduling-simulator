#include "basic_sched.h"

process_queue fifo_sched(process_queue p) {
    process_queue sorted = p;
    
    for (node_t *i = sorted.head; i != NULL; i = i->next) {
        for (node_t *j = i->next; j != NULL; j = j->next) {
            if (i->proc.arrival_time_p > j->proc.arrival_time_p) {
                process_t temp = i->proc;
                i->proc = j->proc;
                j->proc = temp;
            }
        }
    }
    
    return sorted;
}

