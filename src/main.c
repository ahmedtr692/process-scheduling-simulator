#include "headers/basic_sched.h"
#include "headers/config_parser.h"
#include "headers/ncurses_display.h"
#include <string.h>

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

    if (parse_config_file(argv[1], &pqueue) <= 0) {
        fprintf(stderr, "Error: Failed to load processes from configuration file\n");
        return 1;
    }

    // Initialize ncurses
    init_ncurses_display();

    int choice;
    int running = 1;

    while (running) {
        choice = show_menu();

        if (choice == 0) {
            running = 0;
            continue;
        }

        // Check if selected algorithm is available
        available_algorithms_t avail = check_available_algorithms();
        int algorithm_unavailable = 0;
        
        switch (choice) {
            case 1:
                if (!avail.fifo_available) algorithm_unavailable = 1;
                break;
            case 2:
                if (!avail.round_robin_available) algorithm_unavailable = 1;
                break;
            case 3:
                if (!avail.priority_available) algorithm_unavailable = 1;
                break;
            case 4:
                if (!avail.multilevel_available) algorithm_unavailable = 1;
                break;
            case 5:
                if (!avail.multilevel_aging_available) algorithm_unavailable = 1;
                break;
            default:
                algorithm_unavailable = 1;
                break;
        }
        
        if (algorithm_unavailable) {
            clear();
            attron(COLOR_PAIR(3) | A_BOLD);  // Yellow color
            mvprintw(10, 10, "Selected algorithm is not available.");
            mvprintw(11, 10, "Press any key to continue...");
            attroff(COLOR_PAIR(3) | A_BOLD);
            refresh();
            getch();
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

        switch (choice) {
            case 1:
                fifo_sched(&sim_queue, &descriptor, &desc_size);
                break;
            case 2: {
                int quantum = get_quantum();
                round_robin_sched(&sim_queue, &descriptor, &desc_size, quantum);
                break;
            }
            case 3:
                priority_sched(&sim_queue, &descriptor, &desc_size);
                break;
            case 4:
                multilevel_rr_sched(&sim_queue, &descriptor, &desc_size);
                break;
            case 5:
                multilevel_rr_aging_sched(&sim_queue, &descriptor, &desc_size);
                break;
            default:
                while (sim_queue.size > 0) {
                    free(sim_queue.head->proc.process_name);
                    free(sim_queue.head->proc.descriptor_p);
                    remove_head(&sim_queue);
                }
                continue;
        }

        if (descriptor != NULL) {
            display_gantt_chart(descriptor, desc_size);
            display_simulation_results(descriptor, desc_size);
            display_statistics(descriptor, desc_size);
            free(descriptor);
        }

        // Clean up simulation queue
        while (sim_queue.size > 0) {
            free(sim_queue.head->proc.process_name);
            free(sim_queue.head->proc.descriptor_p);
            remove_head(&sim_queue);
        }
    }

    // Clean up ncurses
    cleanup_ncurses_display();

    // Clean up original queue
    while (pqueue.size > 0) {
        free(pqueue.head->proc.process_name);
        free(pqueue.head->proc.descriptor_p);
        remove_head(&pqueue);
    }

    return 0;
}
