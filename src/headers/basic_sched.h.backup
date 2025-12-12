#ifndef BASIC_SCHED_H
#define BASIC_SCHED_H
#include <stdlib.h>
#include <stdio.h>

typedef enum process_state {
  waiting_p,
  running_p,
  terminated_p,
  ready_p,
  blocked_p
} process_state;

typedef enum {
  calc_p,
  IO_p,
  none
} process_operation_t;

typedef struct operation_t {
  process_operation_t operation_p;
  int duration_op;
} operation_t;

typedef struct process_t {
  char *process_name;
  int begining_date;
  operation_t *descriptor_p;
  int arrival_time_p;
  int operations_count;
  int priority_p;
} process_t;

typedef struct node_t {
  process_t proc;
  struct node_t *next;
} node_t;

typedef struct process_queue {
  node_t *head;
  node_t *tail;
  int size;
} process_queue;

typedef struct process_descriptor_t {
  char* process_name ;
  int date ;
  process_state state;
  process_operation_t operation;
}process_descriptor_t;

void add_tail(process_queue* p, process_t process);

void append_descriptor(process_descriptor_t** descriptor ,process_descriptor_t unit_descriptor, int *size);

void remove_head(process_queue *p);

void fifo_sched(process_queue* p, process_descriptor_t** descriptor, int *size);
void round_robin_sched(process_queue* p, process_descriptor_t** descriptor, int *size, int quantum);
void priority_sched(process_queue* p,  process_descriptor_t** descriptor, int *size);
void multilevel_rr_sched(process_queue* p, process_descriptor_t** descriptor, int *size);
void multilevel_rr_aging_sched(process_queue* p, process_descriptor_t** descriptor, int *size);

#endif 
