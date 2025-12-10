#include "ncurses_display.h"
#include <string.h>
#include <stdlib.h>

// Color pairs
#define COLOR_CALC 1    // Green for CALC
#define COLOR_IO 2      // Blue for I/O
#define COLOR_WAIT 3    // Yellow for WAITING
#define COLOR_TERM 4    // Red for TERMINATED
#define COLOR_HEADER 5  // Cyan for headers
#define COLOR_TITLE 6   // White on Blue for title

void init_ncurses_display() {
    initscr();
    cbreak();
    noecho();
    keypad(stdscr, TRUE);
    curs_set(0);
    
    if (has_colors()) {
        start_color();
        init_pair(COLOR_CALC, COLOR_GREEN, COLOR_BLACK);
        init_pair(COLOR_IO, COLOR_BLUE, COLOR_BLACK);
        init_pair(COLOR_WAIT, COLOR_YELLOW, COLOR_BLACK);
        init_pair(COLOR_TERM, COLOR_RED, COLOR_BLACK);
        init_pair(COLOR_HEADER, COLOR_CYAN, COLOR_BLACK);
        init_pair(COLOR_TITLE, COLOR_WHITE, COLOR_BLUE);
    }
}

void cleanup_ncurses_display() {
    endwin();
}

int show_menu() {
    clear();
    int width;
    getmaxyx(stdscr, width, width);  // Just get width, ignore height
    
    // Title
    attron(COLOR_PAIR(COLOR_TITLE) | A_BOLD);
    mvprintw(2, (width - 40) / 2, "                                        ");
    mvprintw(3, (width - 40) / 2, "  PROCESS SCHEDULING SIMULATOR          ");
    mvprintw(4, (width - 40) / 2, "                                        ");
    attroff(COLOR_PAIR(COLOR_TITLE) | A_BOLD);
    
    // Menu options
    attron(COLOR_PAIR(COLOR_HEADER) | A_BOLD);
    mvprintw(7, (width - 50) / 2, "Select a scheduling policy:");
    attroff(COLOR_PAIR(COLOR_HEADER) | A_BOLD);
    
    mvprintw(9, (width - 50) / 2,  "  1. FIFO (First In First Out)");
    mvprintw(10, (width - 50) / 2, "  2. Round-Robin");
    mvprintw(11, (width - 50) / 2, "  3. Priority Preemptive");
    mvprintw(12, (width - 50) / 2, "  4. Multi-level Queue (Static Priority)");
    mvprintw(13, (width - 50) / 2, "  5. Multi-level Queue with Aging");
    mvprintw(14, (width - 50) / 2, "  0. Exit");
    
    attron(COLOR_PAIR(COLOR_HEADER));
    mvprintw(16, (width - 50) / 2, "Enter your choice: ");
    attroff(COLOR_PAIR(COLOR_HEADER));
    
    refresh();
    
    echo();
    curs_set(1);
    int choice;
    scanw("%d", &choice);
    curs_set(0);
    noecho();
    
    return choice;
}

int get_quantum() {
    int height, width;
    getmaxyx(stdscr, height, width);
    
    attron(COLOR_PAIR(COLOR_HEADER));
    mvprintw(height / 2, (width - 40) / 2, "Enter time quantum: ");
    attroff(COLOR_PAIR(COLOR_HEADER));
    
    refresh();
    
    echo();
    curs_set(1);
    int quantum;
    scanw("%d", &quantum);
    curs_set(0);
    noecho();
    
    if (quantum <= 0) quantum = 2;
    return quantum;
}

/*
static const char* state_to_char(process_state state) {
    switch (state) {
        case waiting_p: return "W";
        case running_p: return "R";
        case terminated_p: return "T";
        case ready_p: return "R";
        case blocked_p: return "B";
        default: return "?";
    }
}

static const char* operation_to_char(process_operation_t op) {
    switch (op) {
        case calc_p: return "C";
        case IO_p: return "I";
        case none: return "-";
        default: return "?";
    }
}
*/

static int get_color_for_operation(process_operation_t op, process_state state) {
    if (state == terminated_p) return COLOR_TERM;
    if (state == waiting_p) return COLOR_WAIT;
    if (op == calc_p) return COLOR_CALC;
    if (op == IO_p) return COLOR_IO;
    return COLOR_WAIT;
}

void display_gantt_chart(process_descriptor_t* descriptor, int size) {
    if (size == 0) return;
    
    clear();
    
    // Find unique processes and max time
    char proc_names[100][64];
    int proc_count = 0;
    int max_time = 0;
    
    for (int i = 0; i < size; i++) {
        int found = 0;
        for (int j = 0; j < proc_count; j++) {
            if (strcmp(proc_names[j], descriptor[i].process_name) == 0) {
                found = 1;
                break;
            }
        }
        if (!found) {
            strncpy(proc_names[proc_count], descriptor[i].process_name, 63);
            proc_names[proc_count][63] = '\0';
            proc_count++;
        }
        if (descriptor[i].date > max_time) max_time = descriptor[i].date;
    }
    
    int height, width;
    getmaxyx(stdscr, height, width);
    
    // Title
    attron(COLOR_PAIR(COLOR_TITLE) | A_BOLD);
    mvprintw(0, (width - 40) / 2, "        GANTT CHART - SCHEDULING        ");
    attroff(COLOR_PAIR(COLOR_TITLE) | A_BOLD);
    
    // Legend
    attron(COLOR_PAIR(COLOR_HEADER) | A_BOLD);
    mvprintw(2, 2, "Legend: ");
    attroff(COLOR_PAIR(COLOR_HEADER) | A_BOLD);
    
    attron(COLOR_PAIR(COLOR_CALC));
    printw("C=CALC  ");
    attroff(COLOR_PAIR(COLOR_CALC));
    
    attron(COLOR_PAIR(COLOR_IO));
    printw("I=I/O  ");
    attroff(COLOR_PAIR(COLOR_IO));
    
    attron(COLOR_PAIR(COLOR_WAIT));
    printw("W=Wait  ");
    attroff(COLOR_PAIR(COLOR_WAIT));
    
    attron(COLOR_PAIR(COLOR_TERM));
    printw("T=Term");
    attroff(COLOR_PAIR(COLOR_TERM));
    
    // Scrolling variables
    int scroll_x = 0;
    int scroll_y = 0;
    int view_width = width - 20;
    int view_height = height - 10;
    
    int running = 1;
    while (running) {
        // Clear display area
        for (int i = 4; i < height - 2; i++) {
            move(i, 0);
            clrtoeol();
        }
        
        // Display time header
        mvprintw(4, 2, "Process        |");
        for (int t = scroll_x; t < scroll_x + view_width && t <= max_time; t++) {
            printw("%d", t % 10);
        }
        
        mvprintw(5, 2, "---------------|");
        for (int t = scroll_x; t < scroll_x + view_width && t <= max_time; t++) {
            printw("-");
        }
        
        // Display each process timeline
        for (int p = scroll_y; p < scroll_y + view_height && p < proc_count; p++) {
            mvprintw(6 + (p - scroll_y) * 2, 2, "%-14s |", proc_names[p]);
            
            // Build timeline for this process
            char timeline[10000];
            for (int t = 0; t <= max_time; t++) {
                timeline[t] = ' ';
            }
            
            for (int i = 0; i < size; i++) {
                if (strcmp(descriptor[i].process_name, proc_names[p]) == 0) {
                    int t = descriptor[i].date;
                    if (t >= 0 && t <= max_time) {
                        if (descriptor[i].state == running_p) {
                            timeline[t] = (descriptor[i].operation == calc_p) ? 'C' : 'I';
                        } else if (descriptor[i].state == waiting_p) {
                            timeline[t] = 'W';
                        } else if (descriptor[i].state == terminated_p) {
                            timeline[t] = 'T';
                        }
                    }
                }
            }
            
            // Display visible portion with colors
            for (int t = scroll_x; t < scroll_x + view_width && t <= max_time; t++) {
                char ch = timeline[t];
                int color = COLOR_WAIT;
                
                // Find the descriptor for this time to get proper color
                for (int i = 0; i < size; i++) {
                    if (strcmp(descriptor[i].process_name, proc_names[p]) == 0 && descriptor[i].date == t) {
                        color = get_color_for_operation(descriptor[i].operation, descriptor[i].state);
                        break;
                    }
                }
                
                if (ch != ' ') {
                    attron(COLOR_PAIR(color));
                    addch(ch);
                    attroff(COLOR_PAIR(color));
                } else {
                    addch(' ');
                }
            }
        }
        
        // Status bar
        attron(COLOR_PAIR(COLOR_HEADER));
        mvprintw(height - 2, 0, " Time: %d-%d | Process: %d-%d | Arrow keys=scroll | q/Enter/Space=continue ",
                 scroll_x, scroll_x + view_width, scroll_y + 1, 
                 (scroll_y + view_height < proc_count) ? scroll_y + view_height : proc_count);
        attroff(COLOR_PAIR(COLOR_HEADER));
        
        refresh();
        
        // Handle input
        int ch = getch();
        switch (ch) {
            case KEY_LEFT:
                if (scroll_x > 0) scroll_x--;
                break;
            case KEY_RIGHT:
                if (scroll_x < max_time - view_width) scroll_x++;
                break;
            case KEY_UP:
                if (scroll_y > 0) scroll_y--;
                break;
            case KEY_DOWN:
                if (scroll_y < proc_count - view_height) scroll_y++;
                break;
            case 'q':
            case 'Q':
            case '\n':
            case ' ':
                running = 0;
                break;
        }
    }
}

void display_ready_queue(process_queue* queue __attribute__((unused)), int current_time __attribute__((unused))) {
    // This would be called during simulation to show current queue state
    // For now, this is a placeholder - in a full implementation, this would
    // update a window showing the current ready queue
}

void display_simulation_results(process_descriptor_t* descriptor, int size) {
    clear();
    
    int height, width;
    getmaxyx(stdscr, height, width);
    
    // Title
    attron(COLOR_PAIR(COLOR_TITLE) | A_BOLD);
    mvprintw(0, (width - 40) / 2, "          SIMULATION RESULTS            ");
    attroff(COLOR_PAIR(COLOR_TITLE) | A_BOLD);
    
    // Headers
    attron(COLOR_PAIR(COLOR_HEADER) | A_BOLD);
    mvprintw(2, 2, "%-15s %-10s %-15s %-10s", "PROCESS", "TIME", "STATE", "OPERATION");
    mvprintw(3, 2, "%-15s %-10s %-15s %-10s", "-------", "----", "-----", "---------");
    attroff(COLOR_PAIR(COLOR_HEADER) | A_BOLD);
    
    // Scrolling variables
    int scroll = 0;
    int view_height = height - 7;
    
    int running = 1;
    while (running) {
        // Clear display area
        for (int i = 4; i < height - 2; i++) {
            move(i, 0);
            clrtoeol();
        }
        
        // Display visible portion
        for (int i = scroll; i < scroll + view_height && i < size; i++) {
            const char* state_str;
            switch (descriptor[i].state) {
                case waiting_p: state_str = "WAITING"; break;
                case running_p: state_str = "RUNNING"; break;
                case terminated_p: state_str = "TERMINATED"; break;
                case ready_p: state_str = "READY"; break;
                case blocked_p: state_str = "BLOCKED"; break;
                default: state_str = "UNKNOWN"; break;
            }
            
            const char* op_str;
            switch (descriptor[i].operation) {
                case calc_p: op_str = "CALC"; break;
                case IO_p: op_str = "I/O"; break;
                case none: op_str = "NONE"; break;
                default: op_str = "UNKNOWN"; break;
            }
            
            int color = get_color_for_operation(descriptor[i].operation, descriptor[i].state);
            attron(COLOR_PAIR(color));
            mvprintw(4 + (i - scroll), 2, "%-15s %-10d %-15s %-10s",
                     descriptor[i].process_name,
                     descriptor[i].date,
                     state_str,
                     op_str);
            attroff(COLOR_PAIR(color));
        }
        
        // Status bar
        attron(COLOR_PAIR(COLOR_HEADER));
        mvprintw(height - 2, 0, " Showing: %d-%d of %d | Arrow keys=scroll | q/Enter/Space=continue ",
                 scroll + 1, (scroll + view_height < size) ? scroll + view_height : size, size);
        attroff(COLOR_PAIR(COLOR_HEADER));
        
        refresh();
        
        // Handle input
        int ch = getch();
        switch (ch) {
            case KEY_UP:
                if (scroll > 0) scroll--;
                break;
            case KEY_DOWN:
                if (scroll < size - view_height) scroll++;
                break;
            case 'q':
            case 'Q':
            case '\n':
            case ' ':
                running = 0;
                break;
        }
    }
}

void display_statistics(process_descriptor_t* descriptor, int size) {
    if (size == 0) return;
    
    clear();
    
    int height, width;
    getmaxyx(stdscr, height, width);
    
    // Title
    attron(COLOR_PAIR(COLOR_TITLE) | A_BOLD);
    mvprintw(0, (width - 40) / 2, "              STATISTICS                ");
    attroff(COLOR_PAIR(COLOR_TITLE) | A_BOLD);
    
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
    
    // Headers
    attron(COLOR_PAIR(COLOR_HEADER) | A_BOLD);
    mvprintw(2, 2, "%-15s %-12s %-12s %-12s %-15s", 
             "PROCESS", "START", "END", "TURNAROUND", "WAITING");
    mvprintw(3, 2, "%-15s %-12s %-12s %-12s %-15s",
             "-------", "-----", "---", "----------", "-------");
    attroff(COLOR_PAIR(COLOR_HEADER) | A_BOLD);
    
    // Display statistics
    for (int i = 0; i < num_procs; i++) {
        int turnaround = stats[i].end_time - stats[i].start_time;
        mvprintw(4 + i, 2, "%-15s %-12d %-12d %-12d %-15d",
                 stats[i].name,
                 stats[i].start_time,
                 stats[i].end_time,
                 turnaround,
                 stats[i].total_wait);
    }
    
    // Status bar
    attron(COLOR_PAIR(COLOR_HEADER));
    mvprintw(height - 2, 0, " Press any key to continue... ");
    attroff(COLOR_PAIR(COLOR_HEADER));
    
    refresh();
    getch();
}
