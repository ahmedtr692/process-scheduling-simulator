#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include "headers/basic_sched.h"

#define POLICIES_DIR "policies"
#define MAX_POLICIES 10

typedef enum {
    POLICY_FIFO = 1,
    POLICY_ROUND_ROBIN,
    POLICY_PRIORITY_PREEMPTIVE,
    POLICY_MULTILEVEL
} scheduling_policy_t;

typedef struct {
    char name[64];
    scheduling_policy_t type;
} policy_entry_t;

/* Built-in policies */
static policy_entry_t builtin_policies[] = {
    {"FIFO (First In First Out)", POLICY_FIFO},
    {"Round Robin", POLICY_ROUND_ROBIN},
    {"Priority Preemptive", POLICY_PRIORITY_PREEMPTIVE},
    {"Multi-Level Queue with Aging", POLICY_MULTILEVEL}
};
static int num_builtin_policies = 4;

void print_usage(const char *prog_name) {
    printf("Process Scheduling Simulator\n");
    printf("============================\n\n");
    printf("Usage: %s <config_file>\n\n", prog_name);
    printf("Config file format:\n");
    printf("  # Comment lines start with # or ;\n");
    printf("  # Empty lines are allowed\n");
    printf("  # Process format: name arrival_time burst_time priority [quantum]\n");
    printf("  # Example:\n");
    printf("  P1 0 10 2\n");
    printf("  P2 1 5 1\n");
    printf("  P3 2 8 3 4\n\n");
    printf("Options:\n");
    printf("  -h, --help    Show this help message\n");
}

void print_menu(void) {
    printf("\n=========================================\n");
    printf("   Process Scheduling Simulator Menu\n");
    printf("=========================================\n\n");
    printf("Select a scheduling policy:\n\n");
    
    for (int i = 0; i < num_builtin_policies; i++) {
        printf("  %d. %s\n", i + 1, builtin_policies[i].name);
    }
    
    printf("\n  0. Exit\n");
    printf("\n=========================================\n");
    printf("Enter your choice (default=1 for FIFO): ");
}

void print_process_list(process_queue *q) {
    if (q == NULL || q->head == NULL) {
        printf("No processes loaded.\n");
        return;
    }
    
    printf("\n============ Loaded Processes ============\n");
    printf("%-10s %-10s %-10s %-10s %-10s\n",
           "Name", "Arrival", "Burst", "Priority", "Quantum");
    printf("-------------------------------------------\n");
    
    node_t *current = q->head;
    while (current != NULL) {
        printf("%-10s %-10d %-10d %-10d %-10d\n",
               current->proc.process_name,
               current->proc.arrival_time_p,
               current->proc.burst_time,
               current->proc.priority_p,
               current->proc.quantum_p);
        current = current->next;
    }
    printf("===========================================\n");
}

int get_user_quantum(void) {
    int quantum;
    printf("Enter time quantum for Round Robin (default=4): ");
    
    char input[32];
    if (fgets(input, sizeof(input), stdin) == NULL || input[0] == '\n') {
        return 4;  /* Default quantum */
    }
    
    if (sscanf(input, "%d", &quantum) != 1 || quantum <= 0) {
        printf("Invalid quantum, using default value 4\n");
        return 4;
    }
    
    return quantum;
}

int get_multilevel_params(int *num_levels, int *aging_threshold) {
    char input[32];
    
    printf("Enter number of priority levels (default=3): ");
    if (fgets(input, sizeof(input), stdin) == NULL || input[0] == '\n') {
        *num_levels = 3;
    } else if (sscanf(input, "%d", num_levels) != 1 || *num_levels <= 0) {
        printf("Invalid value, using default 3 levels\n");
        *num_levels = 3;
    }
    
    printf("Enter aging threshold (0 to disable, default=10): ");
    if (fgets(input, sizeof(input), stdin) == NULL || input[0] == '\n') {
        *aging_threshold = 10;
    } else if (sscanf(input, "%d", aging_threshold) != 1 || *aging_threshold < 0) {
        printf("Invalid value, using default threshold 10\n");
        *aging_threshold = 10;
    }
    
    return 0;
}

void run_simulation(process_queue *q, scheduling_policy_t policy) {
    simulation_result_t *result = NULL;
    int quantum, num_levels, aging_threshold;
    
    /* Create a copy of the queue for simulation */
    process_queue *work_q = copy_queue(q);
    if (work_q == NULL) {
        printf("Error: Could not create working queue\n");
        return;
    }
    
    printf("\n");
    
    switch (policy) {
        case POLICY_FIFO:
            printf("Running FIFO Scheduling...\n");
            result = run_fifo(work_q);
            break;
            
        case POLICY_ROUND_ROBIN:
            quantum = get_user_quantum();
            printf("Running Round Robin Scheduling (quantum=%d)...\n", quantum);
            result = run_round_robin(work_q, quantum);
            break;
            
        case POLICY_PRIORITY_PREEMPTIVE:
            printf("Running Priority Preemptive Scheduling...\n");
            result = run_priority_preemptive(work_q);
            break;
            
        case POLICY_MULTILEVEL:
            get_multilevel_params(&num_levels, &aging_threshold);
            printf("Running Multi-Level Queue Scheduling (levels=%d, aging=%d)...\n",
                   num_levels, aging_threshold);
            result = run_multilevel_queue(work_q, num_levels, aging_threshold);
            break;
            
        default:
            printf("Unknown policy selected\n");
            free_queue(work_q);
            return;
    }
    
    if (result == NULL) {
        printf("Error: Simulation failed\n");
        free_queue(work_q);
        return;
    }
    
    /* Display results */
    print_simulation_result(result);
    print_gantt_chart(result);
    print_process_stats(q, result);
    
    /* Cleanup */
    free_simulation_result(result);
    free_queue(work_q);
}

int main(int argc, char *argv[]) {
    /* Check command line arguments */
    if (argc < 2) {
        print_usage(argv[0]);
        return 1;
    }
    
    if (strcmp(argv[1], "-h") == 0 || strcmp(argv[1], "--help") == 0) {
        print_usage(argv[0]);
        return 0;
    }
    
    /* Parse configuration file */
    const char *config_file = argv[1];
    process_queue *processes = parse_config_file(config_file);
    
    if (processes == NULL) {
        fprintf(stderr, "Error: Failed to load processes from '%s'\n", config_file);
        return 1;
    }
    
    if (is_queue_empty(processes)) {
        fprintf(stderr, "Error: No valid processes found in '%s'\n", config_file);
        free_queue(processes);
        return 1;
    }
    
    /* Display loaded processes */
    print_process_list(processes);
    
    /* Main menu loop */
    int running = 1;
    while (running) {
        print_menu();
        
        char input[32];
        if (fgets(input, sizeof(input), stdin) == NULL) {
            break;
        }
        
        int choice = 1;  /* Default to FIFO */
        if (input[0] != '\n') {
            sscanf(input, "%d", &choice);
        }
        
        if (choice == 0) {
            printf("\nExiting. Goodbye!\n");
            running = 0;
        } else if (choice >= 1 && choice <= num_builtin_policies) {
            run_simulation(processes, builtin_policies[choice - 1].type);
            
            printf("\nPress Enter to continue...");
            fgets(input, sizeof(input), stdin);
        } else {
            printf("Invalid choice. Please try again.\n");
        }
    }
    
    /* Cleanup */
    free_queue(processes);
    
    return 0;
}
