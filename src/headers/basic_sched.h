#ifndef PROCESS_H
#define PROCESS_H

enum process_state{
  new_p,
  waiting_p,
  running_p,
  terminated_p,
  ready_p
};

typedef struct process{
  char * process_name ; 
  int date ; 
  int duration ;
  int priority ;
  struct process* next ;

  enum process_state state;
  int rest ;
  int begining_date ;
  int ending_date;
  
}process;

typedef struct ProcessQueue {
  process* header;
  process* next; 
}ProcessQueue;



ProcessQueue fifo_sched(process* p); 

void round_robin(process* p, int quantum);

void shortest_job_first(process* p);

void priority_sched(process* p, int preemptive);

#endif // !DEBUGdef 
