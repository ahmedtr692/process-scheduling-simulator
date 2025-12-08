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
        init_pair(1, COLOR_BLACK, COLOR_GREEN);   // CALC (running CPU) - Green background
        init_pair(2, COLOR_BLACK, COLOR_BLUE);    // I/O operations - Blue background
        init_pair(3, COLOR_BLACK, COLOR_YELLOW);  // Waiting - Yellow background
        init_pair(4, COLOR_BLACK, COLOR_RED);     // Blocked/Terminated - Red background
        init_pair(5, COLOR_CYAN, COLOR_BLACK);    // Headers - Cyan text
        init_pair(6, COLOR_WHITE, COLOR_BLUE);    // Title - White on blue
        init_pair(7, COLOR_GREEN, COLOR_BLACK);   // Text labels - Green text
    }
}

void cleanup_ncurses_display() {
    endwin();
}

void display_gantt_chart(process_descriptor_t* descriptor, int size) {
    clear();
    
    // Display title
    attron(COLOR_PAIR(6) | A_BOLD);
    mvprintw(0, 0, "                  GANTT CHART - PROCESS TIMELINE                  ");
    attroff(COLOR_PAIR(6) | A_BOLD);
    
    // Legend
    attron(COLOR_PAIR(5));
    mvprintw(1, 0, "Legend: ");
    attroff(COLOR_PAIR(5));
    
    attron(COLOR_PAIR(1));
    printw(" CALC ");
    attroff(COLOR_PAIR(1));
    printw(" ");
    
    attron(COLOR_PAIR(2));
    printw(" I/O ");
    attroff(COLOR_PAIR(2));
    printw(" ");
    
    attron(COLOR_PAIR(3));
    printw(" WAIT ");
    attroff(COLOR_PAIR(3));
    printw(" ");
    
    attron(COLOR_PAIR(4));
    printw(" BLOCK ");
    attroff(COLOR_PAIR(4));
    
    if (size == 0) {
        mvprintw(3, 0, "No simulation data available");
        refresh();
        getch();
        return;
    }
    
    // Find all unique processes
    char processes[100][64];
    int num_procs = 0;
    
    for (int i = 0; i < size; i++) {
        int found = 0;
        for (int j = 0; j < num_procs; j++) {
            if (strcmp(processes[j], descriptor[i].process_name) == 0) {
                found = 1;
                break;
            }
        }
        if (!found && num_procs < 100) {
            strncpy(processes[num_procs], descriptor[i].process_name, 63);
            processes[num_procs][63] = '\0';
            num_procs++;
        }
    }
    
    // Find max time
    int max_time = 0;
    for (int i = 0; i < size; i++) {
        if (descriptor[i].date > max_time) max_time = descriptor[i].date;
    }
    
    int max_y, max_x;
    getmaxyx(stdscr, max_y, max_x);
    
    int start_row = 3;
    int chart_width = max_x - 20;
    int time_scale = (max_time > chart_width) ? (max_time / chart_width + 1) : 1;
    
    // Display timeline header
    attron(COLOR_PAIR(5));
    mvprintw(start_row, 0, "Process");
    mvprintw(start_row, 15, "Time -->");
    attroff(COLOR_PAIR(5));
    
    // Draw Gantt chart for each process
    for (int p = 0; p < num_procs && (start_row + p + 1) < max_y - 2; p++) {
        int row = start_row + p + 1;
        
        // Process name
        attron(COLOR_PAIR(7));
        mvprintw(row, 0, "%-12s", processes[p]);
        attroff(COLOR_PAIR(7));
        
        // Draw timeline
        mvprintw(row, 15, "|");
        
        // Track what we've drawn at each position
        for (int i = 0; i < size; i++) {
            if (strcmp(descriptor[i].process_name, processes[p]) != 0) continue;
            
            int time_pos = descriptor[i].date / time_scale;
            if (time_pos >= chart_width - 1) continue;
            
            int color_pair = 0;
            
            // Determine color based on state and operation
            if (descriptor[i].state == running_p) {
                if (descriptor[i].operation == calc_p) {
                    color_pair = 1; // Green for CALC
                } else if (descriptor[i].operation == IO_p) {
                    color_pair = 2; // Blue for I/O
                } else {
                    color_pair = 1; // Default green for running
                }
            } else if (descriptor[i].state == waiting_p || descriptor[i].state == ready_p) {
                color_pair = 3; // Yellow for waiting
            } else if (descriptor[i].state == blocked_p || descriptor[i].state == terminated_p) {
                color_pair = 4; // Red for blocked/terminated
            }
            
            if (color_pair > 0) {
                attron(COLOR_PAIR(color_pair) | A_BOLD);
                mvprintw(row, 16 + time_pos, "█");
                attroff(COLOR_PAIR(color_pair) | A_BOLD);
            }
        }
    }
    
    // Time markers
    if (start_row + num_procs + 2 < max_y - 1) {
        int marker_row = start_row + num_procs + 2;
        attron(COLOR_PAIR(5));
        mvprintw(marker_row, 15, "|");
        
        // Add time markers every 10 units (scaled)
        for (int t = 0; t <= max_time; t += 10) {
            int pos = t / time_scale;
            if (pos < chart_width - 5) {
                mvprintw(marker_row, 16 + pos, "%d", t);
            }
        }
        attroff(COLOR_PAIR(5));
    }
    
    mvprintw(max_y - 1, 0, "Scale: 1 char = %d time units. Press any key to continue...", time_scale);
    
    refresh();
    getch();
}

void display_simulation_results(process_descriptor_t* descriptor, int size) {
    // First show the Gantt chart
    display_gantt_chart(descriptor, size);
    
    // Then show detailed results
    clear();
    
    // Display title
    attron(COLOR_PAIR(6) | A_BOLD);
    mvprintw(0, 0, "                    SIMULATION RESULTS                    ");
    attroff(COLOR_PAIR(6) | A_BOLD);
    
    // Display headers
    attron(COLOR_PAIR(5) | A_BOLD);
    mvprintw(2, 0, "%-15s %-10s %-15s %-10s", "PROCESS", "TIME", "STATE", "OPERATION");
    mvprintw(3, 0, "%-15s %-10s %-15s %-10s", "-------", "----", "-----", "---------");
    attroff(COLOR_PAIR(5) | A_BOLD);
    
    // Display results with scrolling
    int max_y, max_x;
    getmaxyx(stdscr, max_y, max_x);
    (void)max_x; // Suppress unused warning
    
    int visible_lines = max_y - 6;
    
    for (int i = 0; i < size && i < visible_lines; i++) {
        int color_pair = 0;
        if (descriptor[i].state == running_p) {
            if (descriptor[i].operation == calc_p) color_pair = 1;
            else if (descriptor[i].operation == IO_p) color_pair = 2;
            else color_pair = 1;
        }
        else if (descriptor[i].state == waiting_p) color_pair = 3;
        else if (descriptor[i].state == terminated_p) color_pair = 4;
        
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
    attron(COLOR_PAIR(6) | A_BOLD);
    mvprintw(0, 0, "                       STATISTICS                         ");
    attroff(COLOR_PAIR(6) | A_BOLD);
    
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
    attron(COLOR_PAIR(5) | A_BOLD);
    mvprintw(2, 0, "%-15s %-12s %-12s %-12s %-15s", 
             "PROCESS", "START", "END", "TURNAROUND", "WAITING");
    mvprintw(3, 0, "%-15s %-12s %-12s %-12s %-15s",
             "-------", "-----", "---", "----------", "-------");
    attroff(COLOR_PAIR(5) | A_BOLD);
    
    // Display statistics
    for (int i = 0; i < num_procs; i++) {
        int turnaround = stats[i].end_time - stats[i].start_time;
        
        attron(COLOR_PAIR(7));
        mvprintw(4 + i, 0, "%-15s %-12d %-12d %-12d %-15d",
                 stats[i].name,
                 stats[i].start_time,
                 stats[i].end_time,
                 turnaround,
                 stats[i].total_wait);
        attroff(COLOR_PAIR(7));
    }
    
    int max_y, max_x;
    getmaxyx(stdscr, max_y, max_x);
    (void)max_x; // Suppress unused warning
    mvprintw(max_y - 1, 0, "Press any key to continue...");
    
    refresh();
    getch();
}
