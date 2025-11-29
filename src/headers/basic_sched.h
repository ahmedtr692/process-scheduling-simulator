#ifndef BASIC_SCHED_H
#define BASIC_SCHED_H
#include <stdlib.h>
#include <stdio.h>

typedef enum process_state {
  new_p,
  waiting_p,
  running_p,
  terminated_p,
  ready_p,
  suspended_p,
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

typedef struct descriptor_op {
  operation_t *operations;
  int count;
} descriptor_op;

typedef struct process_t {
  char *process_name;
  int begining_date;
  int quantum_p;
  descriptor_op descriptor_p;
  int arrival_time_p;
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
  int date ;
  process_state state;
  process_operation_t operation;
}process_descriptor_t;

void fifo_sched(process_queue* p, process_descriptor_t* descriptor[], int mode);




void priority_sched(process_queue* p, int preemptive, process_descriptor_t* descriptor[]);

#endif
