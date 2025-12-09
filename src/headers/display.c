#include "display.h"
#include <string.h>

static const char* state_to_string(process_state state) {
    switch (state) {
        case waiting_p: return "WAITING";
        case running_p: return "RUNNING";
        case terminated_p: return "TERMINATED";
        case ready_p: return "READY";
        case blocked_p: return "BLOCKED";
        default: return "UNKNOWN";
    }
}

static const char* operation_to_string(process_operation_t op) {
    switch (op) {
        case calc_p: return "CALC";
        case IO_p: return "I/O";
        case none: return "NONE";
        default: return "UNKNOWN";
    }
}

void print_simulation_results(process_descriptor_t* descriptor, int size) {
    printf("\n");
    printf("========================================\n");
    printf("   SIMULATION RESULTS\n");
    printf("========================================\n\n");
    
    printf("%-15s %-10s %-15s %-10s\n", "PROCESS", "TIME", "STATE", "OPERATION");
    printf("%-15s %-10s %-15s %-10s\n", "-------", "----", "-----", "---------");
    
    for (int i = 0; i < size; i++) {
        printf("%-15s %-10d %-15s %-10s\n",
               descriptor[i].process_name,
               descriptor[i].date,
               state_to_string(descriptor[i].state),
               operation_to_string(descriptor[i].operation));
    }
    printf("\n");
}

void print_statistics(process_descriptor_t* descriptor, int size) {
    if (size == 0) return;
    
    printf("========================================\n");
    printf("   STATISTICS\n");
    printf("========================================\n\n");
    
    // Calculate per-process statistics
    typedef struct {
        char name[64];
        int start_time;
        int end_time;
        int total_wait;
        int total_run;
    } proc_stats_t;
    
    proc_stats_t stats[100];
    int num_procs = 0;
    
    for (int i = 0; i < size; i++) {
        int found = -1;
        for (int j = 0; j < num_procs; j++) {
            if (strcmp(stats[j].name, descriptor[i].process_name) == 0) {
                found = j;
                break;
            }
        }
        
        if (found == -1) {
            found = num_procs++;
            strncpy(stats[found].name, descriptor[i].process_name, 63);
            stats[found].name[63] = '\0';
            stats[found].start_time = descriptor[i].date;
            stats[found].end_time = descriptor[i].date;
            stats[found].total_wait = 0;
            stats[found].total_run = 0;
        }
        
        if (descriptor[i].date > stats[found].end_time) {
            stats[found].end_time = descriptor[i].date;
        }
        
        if (descriptor[i].state == running_p) {
            stats[found].total_run++;
        } else if (descriptor[i].state == waiting_p) {
            stats[found].total_wait++;
        }
    }
    
    printf("%-15s %-12s %-12s %-12s %-15s\n", 
           "PROCESS", "START", "END", "TURNAROUND", "WAITING");
    printf("%-15s %-12s %-12s %-12s %-15s\n",
           "-------", "-----", "---", "----------", "-------");
    
    for (int i = 0; i < num_procs; i++) {
        int turnaround = stats[i].end_time - stats[i].start_time;
        printf("%-15s %-12d %-12d %-12d %-15d\n",
               stats[i].name,
               stats[i].start_time,
               stats[i].end_time,
               turnaround,
               stats[i].total_wait);
    }
    printf("\n");
}
