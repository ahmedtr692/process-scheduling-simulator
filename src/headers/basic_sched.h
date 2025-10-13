#ifndef PROCESS
#define PROCESS


typedef struct{
  char * process_name ;
  int date;
  int duration;
  int priority;
 }process;

void fifo_sched(process* p);

void round_robin(process* p, itn quantum);

void shortest_job_first(process* p);

void priority(process* p, int preemptive);

#endif // !DEBUGdef 
