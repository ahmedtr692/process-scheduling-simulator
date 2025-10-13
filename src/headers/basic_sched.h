#ifndef PROCESS_H
#define PROCESS_H


typedef struct process{
  char * process_name ; 
  int date ; 
  int duration ;
  int priority ;
  struct process* next ;
 }process;

typedef struct FifoQueue {
  process* header;
  process* tail;
}fifo_queue;



void fifo_sched(process* p); 

void round_robin(process* p, int quantum);

void shortest_job_first(process* p);

void priority(process* p, int preemptive);

#endif // !DEBUGdef 
