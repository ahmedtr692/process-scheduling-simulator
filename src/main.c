#include "headers/basic_sched.h"
#include "headers/config_parser.h"
#include "headers/display.h"
#include <ncurses.h>
#include <string.h>

int show_menu() {
    clear();
    
    // Title
    attron(COLOR_PAIR(5) | A_BOLD);
    mvprintw(0, 0, "           PROCESS SCHEDULING SIMULATOR           ");
    attroff(COLOR_PAIR(5) | A_BOLD);
    
    attron(COLOR_PAIR(4));
    mvprintw(2, 0, "Select a scheduling policy:");
    attroff(COLOR_PAIR(4));
    
    mvprintw(4, 2, "1. FIFO (First In First Out)");
    mvprintw(5, 2, "2. Round-Robin");
    mvprintw(6, 2, "3. Priority Preemptive");
    mvprintw(7, 2, "4. Multi-level Queue (Static Priority)");
    mvprintw(8, 2, "5. Multi-level Queue with Aging");
    mvprintw(9, 2, "0. Exit");
    
    mvprintw(11, 0, "Enter your choice: ");
    refresh();
    
    echo();
    curs_set(1);
    int choice;
    scanw("%d", &choice);
    noecho();
    curs_set(0);
    
    return choice;
}

int get_quantum() {
    clear();
    attron(COLOR_PAIR(4) | A_BOLD);
    mvprintw(0, 0, "Enter time quantum for Round-Robin: ");
    attroff(COLOR_PAIR(4) | A_BOLD);
    refresh();
    
    echo();
    curs_set(1);
    int quantum;
    scanw("%d", &quantum);
    noecho();
    curs_set(0);
    
    if (quantum <= 0) quantum = 2;
    return quantum;
}

void show_policy_message(const char* policy_name) {
    clear();
    attron(COLOR_PAIR(1) | A_BOLD);
    mvprintw(0, 0, "Running simulation: %s", policy_name);
    attroff(COLOR_PAIR(1) | A_BOLD);
    mvprintw(2, 0, "Please wait...");
    refresh();
    napms(500);
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

    if (parse_config_file(argv[1], &pqueue) <= 0) {
        fprintf(stderr, "Error: Failed to load processes from configuration file\n");
        return 1;
    }

    // Initialize ncurses
    init_ncurses_display();

    int running = 1;

    while (running) {
        int choice = show_menu();

        if (choice == 0) {
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

        switch (choice) {
            case 1:
                show_policy_message("FIFO (First In First Out)");
                fifo_sched(&sim_queue, &descriptor, &desc_size);
                break;
            case 2: {
                int quantum = get_quantum();
                char msg[100];
                snprintf(msg, sizeof(msg), "Round-Robin (Quantum = %d)", quantum);
                show_policy_message(msg);
                round_robin_sched(&sim_queue, &descriptor, &desc_size, quantum);
                break;
            }
            case 3:
                show_policy_message("Priority Preemptive");
                priority_sched(&sim_queue, &descriptor, &desc_size);
                break;
            case 4:
                show_policy_message("Multi-level Queue (Static Priority)");
                multilevel_rr_sched(&sim_queue, &descriptor, &desc_size);
                break;
            case 5:
                show_policy_message("Multi-level Queue with Aging");
                multilevel_rr_aging_sched(&sim_queue, &descriptor, &desc_size);
                break;
            default:
                clear();
                attron(COLOR_PAIR(3));
                mvprintw(0, 0, "Invalid choice. Press any key to continue...");
                attroff(COLOR_PAIR(3));
                refresh();
                getch();
                while (sim_queue.size > 0) {
                    free(sim_queue.head->proc.process_name);
                    free(sim_queue.head->proc.descriptor_p);
                    remove_head(&sim_queue);
                }
                continue;
        }

        if (descriptor != NULL) {
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

    // Clean up original queue
    while (pqueue.size > 0) {
        free(pqueue.head->proc.process_name);
        free(pqueue.head->proc.descriptor_p);
        remove_head(&pqueue);
    }

    // Cleanup ncurses
    cleanup_ncurses_display();

    return 0;
}
