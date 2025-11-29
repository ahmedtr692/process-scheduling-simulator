#ifndef BASIC_SCHED_H
#define BASIC_SCHED_H
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define MAX_PROCESSES 100
#define MAX_NAME_LEN 64
#define MAX_LINE_LEN 256
#define MAX_OPERATIONS 10
#define MAX_DESCRIPTOR_ENTRIES 1000

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
  int remaining_time;
  int burst_time;
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
  int date;
  char *process_name;
  process_state state;
  process_operation_t operation;
  int duration;
} process_descriptor_t;

typedef struct simulation_result_t {
  process_descriptor_t **entries;
  int count;
  int capacity;
} simulation_result_t;

/* Queue operations */
process_queue* create_queue(void);
void enqueue(process_queue *q, process_t proc);
process_t* dequeue(process_queue *q);
int is_queue_empty(process_queue *q);
void free_queue(process_queue *q);
process_queue* copy_queue(process_queue *q);

/* Process operations */
process_t* create_process(const char *name, int arrival, int priority, int burst);
void free_process(process_t *p);
int get_total_burst_time(process_t *p);

/* Configuration file parsing */
process_queue* parse_config_file(const char *filename);

/* Simulation result operations */
simulation_result_t* create_simulation_result(void);
void add_result_entry(simulation_result_t *result, int date, const char *name, 
                      process_state state, process_operation_t op, int duration);
void free_simulation_result(simulation_result_t *result);
void print_simulation_result(simulation_result_t *result);

/* Scheduling algorithms */
void fifo_sched(process_queue* p, process_descriptor_t* descriptor[], int mode);
simulation_result_t* run_fifo(process_queue *q);
simulation_result_t* run_round_robin(process_queue *q, int quantum);
void priority_sched(process_queue* p, int preemptive, process_descriptor_t* descriptor[]);
simulation_result_t* run_priority_preemptive(process_queue *q);
simulation_result_t* run_multilevel_queue(process_queue *q, int num_levels, int aging_threshold);

/* Display functions */
void print_gantt_chart(simulation_result_t *result);
void print_process_stats(process_queue *original, simulation_result_t *result);

#endif
