#include "headers/basic_sched.h"
#include "headers/config_parser.h"
#include "headers/display.h"
#include <string.h>

void print_menu() {
    printf("\n");
    printf("=========================================\n");
    printf("  PROCESS SCHEDULING SIMULATOR\n");
    printf("=========================================\n");
    printf("Select a scheduling policy:\n\n");
    printf("  1. FIFO (First In First Out)\n");
    printf("  2. Round-Robin\n");
    printf("  3. Priority Preemptive\n");
    printf("  4. Multi-level Queue (Static Priority)\n");
    printf("  5. Multi-level Queue with Aging\n");
    printf("  0. Exit\n");
    printf("=========================================\n");
    printf("Enter your choice: ");
}

int main(int argc, char** argv) {
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <config_file>\n", argv[0]);
        fprintf(stderr, "\nExample:\n");
        fprintf(stderr, "  %s processes.txt\n\n", argv[0]);
        fprintf(stderr, "Configuration file format:\n");
        fprintf(stderr, "  # Comments start with #\n");
        fprintf(stderr, "  ProcessName ArrivalTime Priority Operation1:Duration Operation2:Duration ...\n");
        fprintf(stderr, "\nExample:\n");
        fprintf(stderr, "  P1 0 5 calc:10 io:5 calc:3\n");
        fprintf(stderr, "  P2 1 3 calc:8 io:4\n\n");
        return 1;
    }

    process_queue pqueue;
    pqueue.head = NULL;
    pqueue.tail = NULL;
    pqueue.size = 0;

    printf("\n");
    printf("=========================================\n");
    printf("  PROCESS SCHEDULING SIMULATOR\n");
    printf("=========================================\n\n");

    if (parse_config_file(argv[1], &pqueue) <= 0) {
        fprintf(stderr, "Error: Failed to load processes from configuration file\n");
        return 1;
    }

    int choice;
    int running = 1;

    while (running) {
        print_menu();
        
        if (scanf("%d", &choice) != 1) {
            while (getchar() != '\n');
            printf("Invalid input. Please enter a number.\n");
            continue;
        }

        if (choice == 0) {
            printf("\nExiting simulator. Goodbye!\n\n");
            running = 0;
            continue;
        }

        process_descriptor_t* descriptor = NULL;
        int desc_size = 0;

        // Create a copy of the process queue for simulation
        process_queue sim_queue;
        sim_queue.head = NULL;
        sim_queue.tail = NULL;
        sim_queue.size = 0;

        for (node_t* cur = pqueue.head; cur != NULL; cur = cur->next) {
            process_t proc_copy;
            proc_copy.process_name = malloc(strlen(cur->proc.process_name) + 1);
            strcpy(proc_copy.process_name, cur->proc.process_name);
            proc_copy.arrival_time_p = cur->proc.arrival_time_p;
            proc_copy.begining_date = cur->proc.begining_date;
            proc_copy.priority_p = cur->proc.priority_p;
            proc_copy.operations_count = cur->proc.operations_count;
            proc_copy.descriptor_p = malloc(cur->proc.operations_count * sizeof(operation_t));
            memcpy(proc_copy.descriptor_p, cur->proc.descriptor_p, 
                   cur->proc.operations_count * sizeof(operation_t));
            add_tail(&sim_queue, proc_copy);
        }

        printf("\nRunning simulation...\n");

        switch (choice) {
            case 1:
                printf("Policy: FIFO (First In First Out)\n");
                fifo_sched(&sim_queue, &descriptor, &desc_size);
                break;
            case 2: {
                int quantum;
                printf("Enter time quantum for Round-Robin: ");
                if (scanf("%d", &quantum) != 1 || quantum <= 0) {
                    printf("Invalid quantum. Using default: 2\n");
                    quantum = 2;
                }
                printf("Policy: Round-Robin (Quantum = %d)\n", quantum);
                round_robin_sched(&sim_queue, &descriptor, &desc_size, quantum);
                break;
            }
            case 3:
                printf("Policy: Priority Preemptive\n");
                priority_sched(&sim_queue, &descriptor, &desc_size);
                break;
            case 4:
                printf("Policy: Multi-level Queue (Static Priority)\n");
                multilevel_rr_sched(&sim_queue, &descriptor, &desc_size);
                break;
            case 5:
                printf("Policy: Multi-level Queue with Aging\n");
                multilevel_rr_aging_sched(&sim_queue, &descriptor, &desc_size);
                break;
            default:
                printf("Invalid choice. Please try again.\n");
                while (sim_queue.size > 0) {
                    free(sim_queue.head->proc.process_name);
                    free(sim_queue.head->proc.descriptor_p);
                    remove_head(&sim_queue);
                }
                continue;
        }

        if (descriptor != NULL) {
            print_simulation_results(descriptor, desc_size);
            print_statistics(descriptor, desc_size);
            free(descriptor);
        }

        // Clean up simulation queue
        while (sim_queue.size > 0) {
            free(sim_queue.head->proc.process_name);
            free(sim_queue.head->proc.descriptor_p);
            remove_head(&sim_queue);
        }

        printf("\nPress Enter to continue...");
        while (getchar() != '\n');
        getchar();
    }

    // Clean up original queue
    while (pqueue.size > 0) {
        free(pqueue.head->proc.process_name);
        free(pqueue.head->proc.descriptor_p);
        remove_head(&pqueue);
    }

    return 0;
}
