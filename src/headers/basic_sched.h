#ifndef PROCESS_H
#define PROCESS_H

// this defines the states that the process can be in
enum process_state {
  new_p,
  waiting_p,
  running_p,
  terminated_p,
  ready_p,
  suspended_p,
  blocked_p
};

// this enum defines the operations that can be done by the process
typedef enum {
  calc_p,     // logical and arithmetic operations
  IO_p,       // memory, read/write and input/output operations
  none        // the process does nothing
} process_operation_t;

// this new type is defined to describe each sequence of the process
typedef struct operation_t {
  process_operation_t operation_p;
  int duration_op;
  // int critical; // this will be uncommented in case we implement semaphore or mutex
} operation_t;

// descriptor of the process like 2 units CPU, 3 units I/O and 8 units CPU
// which is an array of operations
typedef struct descriptor_op {
  operation_t *operations;  // dynamically allocated array
  int count;
} descriptor_op;

// process structure
typedef struct process_t {
  char *process_name;
  int begining_date;
  int quantum_p;
  descriptor_op descriptor_p;
  int arrival_time_p;
  int priority_p;
} process_t;

// linked list node structure
typedef struct node_t {
  process_t proc;
  struct node_t *next;
} node_t;

// queue structure for process scheduling
typedef struct process_queue {
  node_t *head;
  node_t *tail;
  int size;
} process_queue;

// Function declarations
process_queue fifo_sched(process_queue p);

void round_robin(process_queue* p, int quantum);

void shortest_job_first(process_queue* p);

void priority_sched(process_queue* p, int preemptive);

#endif // PROCESS_H
