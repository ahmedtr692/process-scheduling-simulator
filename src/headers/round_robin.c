#include "basic_sched.h"

void round_robin_sched(process_queue* p, process_descriptor_t** descriptor, int size, quantum)
{
  if (p->size == 0 ) return ;
  
    process_queue *rr_queue = malloc(sizeof(process_queue));
    rr_queue->head = NULL;
    rr_queue->tail = NULL;
    rr_queue->size = 0;


    node_t *current = p->head;
    while (current != NULL) {
        add_tail(rr_queue, current->proc);
        current = current->next;
    }


    if (rr_queue->size > 1) {
        int swapped;
        node_t *cur;
        node_t *last = NULL;

        do {
            swapped = 0;
            cur = rr_queue->head;
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
  
  
}
