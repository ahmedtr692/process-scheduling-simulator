#include "basic_sched.h"

void add_tail(process_queue* p, process_t process) {
    node_t *tmp = malloc(sizeof(node_t));
    tmp->proc = process;
    tmp->next = NULL;

    if (p->size == 0) {
        p->head = tmp;
        p->tail = tmp;
    } else {
        p->tail->next = tmp;
        p->tail = tmp;
    }

    p->size++;
}

void remove_head(process_queue *p) {
    if (p->size == 0) return;

    node_t *tmp = p->head;
    p->head = tmp->next;
    p->size--;

    if (p->size == 0)
        p->tail = NULL;

    free(tmp);
}
void append_descriptor(process_descriptor_t **descriptor, process_descriptor_t unit_descriptor, int *size){
  *descriptor = realloc(*descriptor, (*size +1)*sizeof(process_descriptor_t));
  (*descriptor)[*size] = unit_descriptor ;
  (*size)++;

}
