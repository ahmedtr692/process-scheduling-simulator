#include "display.h"
#include <ncurses.h>
#include <string.h>
#include <unistd.h>

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

void init_ncurses_display() {
    initscr();
    cbreak();
    noecho();
    keypad(stdscr, TRUE);
    curs_set(0);
    
    if (has_colors()) {
        start_color();
        init_pair(1, COLOR_GREEN, COLOR_BLACK);   // Running
        init_pair(2, COLOR_YELLOW, COLOR_BLACK);  // Waiting
        init_pair(3, COLOR_RED, COLOR_BLACK);     // Terminated
        init_pair(4, COLOR_CYAN, COLOR_BLACK);    // Headers
        init_pair(5, COLOR_WHITE, COLOR_BLUE);    // Title
    }
}

void cleanup_ncurses_display() {
    endwin();
}

void display_simulation_results(process_descriptor_t* descriptor, int size) {
    clear();
    
    // Display title
    attron(COLOR_PAIR(5) | A_BOLD);
    mvprintw(0, 0, "                    SIMULATION RESULTS                    ");
    attroff(COLOR_PAIR(5) | A_BOLD);
    
    // Display headers
    attron(COLOR_PAIR(4) | A_BOLD);
    mvprintw(2, 0, "%-15s %-10s %-15s %-10s", "PROCESS", "TIME", "STATE", "OPERATION");
    mvprintw(3, 0, "%-15s %-10s %-15s %-10s", "-------", "----", "-----", "---------");
    attroff(COLOR_PAIR(4) | A_BOLD);
    
    // Display results with scrolling
    int max_y, max_x;
    getmaxyx(stdscr, max_y, max_x);
    (void)max_x; // Suppress unused warning
    
    int visible_lines = max_y - 6;
    
    for (int i = 0; i < size && i < visible_lines; i++) {
        int color_pair = 0;
        if (descriptor[i].state == running_p) color_pair = 1;
        else if (descriptor[i].state == waiting_p) color_pair = 2;
        else if (descriptor[i].state == terminated_p) color_pair = 3;
        
        if (color_pair > 0) attron(COLOR_PAIR(color_pair));
        
        mvprintw(4 + i, 0, "%-15s %-10d %-15s %-10s",
                 descriptor[i].process_name,
                 descriptor[i].date,
                 state_to_string(descriptor[i].state),
                 operation_to_string(descriptor[i].operation));
        
        if (color_pair > 0) attroff(COLOR_PAIR(color_pair));
    }
    
    // Navigation info
    if (size > visible_lines) {
        mvprintw(max_y - 1, 0, "Showing %d/%d entries. Press any key to continue...", 
                 visible_lines, size);
    } else {
        mvprintw(max_y - 1, 0, "Press any key to continue...");
    }
    
    refresh();
    getch();
}

void display_statistics(process_descriptor_t* descriptor, int size) {
    if (size == 0) return;
    
    clear();
    
    // Display title
    attron(COLOR_PAIR(5) | A_BOLD);
    mvprintw(0, 0, "                       STATISTICS                         ");
    attroff(COLOR_PAIR(5) | A_BOLD);
    
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
    
    // Display headers
    attron(COLOR_PAIR(4) | A_BOLD);
    mvprintw(2, 0, "%-15s %-12s %-12s %-12s %-15s", 
             "PROCESS", "START", "END", "TURNAROUND", "WAITING");
    mvprintw(3, 0, "%-15s %-12s %-12s %-12s %-15s",
             "-------", "-----", "---", "----------", "-------");
    attroff(COLOR_PAIR(4) | A_BOLD);
    
    // Display statistics
    for (int i = 0; i < num_procs; i++) {
        int turnaround = stats[i].end_time - stats[i].start_time;
        
        attron(COLOR_PAIR(1));
        mvprintw(4 + i, 0, "%-15s %-12d %-12d %-12d %-15d",
                 stats[i].name,
                 stats[i].start_time,
                 stats[i].end_time,
                 turnaround,
                 stats[i].total_wait);
        attroff(COLOR_PAIR(1));
    }
    
    int max_y, max_x;
    getmaxyx(stdscr, max_y, max_x);
    (void)max_x; // Suppress unused warning
    mvprintw(max_y - 1, 0, "Press any key to continue...");
    
    refresh();
    getch();
}
