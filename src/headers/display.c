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
        init_pair(1, COLOR_GREEN, COLOR_BLACK);    // CALC - Green text
        init_pair(2, COLOR_BLUE, COLOR_BLACK);     // I/O - Blue text
        init_pair(3, COLOR_YELLOW, COLOR_BLACK);   // Waiting - Yellow text
        init_pair(4, COLOR_RED, COLOR_BLACK);      // Terminated/Blocked - Red text
        init_pair(5, COLOR_CYAN, COLOR_BLACK);     // Headers - Cyan text
        init_pair(6, COLOR_WHITE, COLOR_BLUE);     // Title - White on blue
        init_pair(7, COLOR_WHITE, COLOR_BLACK);    // Normal text - White
    }
}

void cleanup_ncurses_display() {
    endwin();
}

void display_gantt_chart(process_descriptor_t* descriptor, int size) {
    clear();
    
    // Display title with padding
    attron(COLOR_PAIR(6) | A_BOLD);
    mvprintw(0, 0, "                                                                              ");
    mvprintw(0, 10, "       GANTT CHART - PROCESS TIMELINE");
    attroff(COLOR_PAIR(6) | A_BOLD);
    
    // Legend with better formatting
    attron(COLOR_PAIR(5) | A_BOLD);
    mvprintw(2, 0, "Legend: ");
    attroff(COLOR_PAIR(5) | A_BOLD);
    
    attron(COLOR_PAIR(1) | A_BOLD);
    printw("C");
    attroff(COLOR_PAIR(1) | A_BOLD);
    printw("=CALC   ");
    
    attron(COLOR_PAIR(2) | A_BOLD);
    printw("I");
    attroff(COLOR_PAIR(2) | A_BOLD);
    printw("=I/O   ");
    
    attron(COLOR_PAIR(3) | A_BOLD);
    printw("W");
    attroff(COLOR_PAIR(3) | A_BOLD);
    printw("=Wait   ");
    
    attron(COLOR_PAIR(4) | A_BOLD);
    printw("B");
    attroff(COLOR_PAIR(4) | A_BOLD);
    printw("=Block   ");
    
    attron(COLOR_PAIR(4) | A_BOLD);
    printw("T");
    attroff(COLOR_PAIR(4) | A_BOLD);
    printw("=Term");
    
    if (size == 0) {
        mvprintw(5, 0, "No simulation data available");
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
    
    int start_row = 6;  // Increased from 4 for better spacing
    int row_spacing = 2;
    int label_col = 2;  // Add left margin
    int chart_start_col = 18;  // Increased for longer process names
    int available_width = max_x - chart_start_col - 2;
    
    // Scrolling state
    int scroll_offset = 0;
    int max_scroll = (max_time > available_width) ? (max_time - available_width + 1) : 0;
    int proc_scroll = 0;
    int max_proc_scroll = (num_procs * row_spacing > max_y - 6) ? (num_procs - (max_y - 6) / row_spacing) : 0;
    
    int ch;
    do {
        // Clear display area
        for (int r = start_row - 2; r < max_y - 1; r++) {
            move(r, 0);
            clrtoeol();
        }
        
        // Display column headers with separation line
        attron(COLOR_PAIR(5) | A_BOLD);
        mvprintw(start_row - 2, label_col, "Process");
        mvprintw(start_row - 2, chart_start_col - 1, "|");
        mvprintw(start_row - 2, chart_start_col, "Timeline");
        attroff(COLOR_PAIR(5) | A_BOLD);
        
        // Draw separator line
        mvprintw(start_row - 1, label_col, "---------------");
        mvprintw(start_row - 1, chart_start_col - 1, "+");
        for (int i = 0; i < available_width && (chart_start_col + i) < max_x - 1; i++) {
            mvprintw(start_row - 1, chart_start_col + i, "-");
        }
        
        // Draw time scale on top
        attron(COLOR_PAIR(5));
        for (int t = scroll_offset; t <= max_time && (t - scroll_offset) < available_width; t++) {
            int col = chart_start_col + (t - scroll_offset);
            if (col + 2 < max_x) {
                mvprintw(start_row - 2, col, "%d", t % 10);
            }
        }
        attroff(COLOR_PAIR(5));
        
        // Draw Gantt chart for each process
        int displayed_procs = 0;
        for (int p = proc_scroll; p < num_procs && displayed_procs < (max_y - 8) / row_spacing; p++, displayed_procs++) {
            int row = start_row + displayed_procs * row_spacing;
            
            // Process name with better formatting
            attron(COLOR_PAIR(7) | A_BOLD);
            mvprintw(row, label_col, "%-14s", processes[p]);
            attroff(COLOR_PAIR(7) | A_BOLD);
            
            // Draw border separator
            attron(COLOR_PAIR(5));
            mvprintw(row, chart_start_col - 1, "|");
            attroff(COLOR_PAIR(5));
            
            // Track process states - draw letters for each time unit
            for (int t = scroll_offset; t <= max_time && (t - scroll_offset) < available_width; t++) {
                // Find the descriptor entry for this process at this time
                char display_char = ' ';
                int color_pair = 7;
                
                for (int i = 0; i < size; i++) {
                    if (strcmp(descriptor[i].process_name, processes[p]) == 0 && 
                        descriptor[i].date == t) {
                        
                        // Determine character and color based on state and operation
                        if (descriptor[i].state == running_p) {
                            if (descriptor[i].operation == calc_p) {
                                display_char = 'C';
                                color_pair = 1; // Green for CALC
                            } else if (descriptor[i].operation == IO_p) {
                                display_char = 'I';
                                color_pair = 2; // Blue for I/O
                            } else {
                                display_char = 'C';
                                color_pair = 1;
                            }
                        } else if (descriptor[i].state == waiting_p || descriptor[i].state == ready_p) {
                            display_char = 'W';
                            color_pair = 3; // Yellow for waiting
                        } else if (descriptor[i].state == blocked_p) {
                            display_char = 'B';
                            color_pair = 4; // Red for blocked
                        } else if (descriptor[i].state == terminated_p) {
                            display_char = 'T';
                            color_pair = 4; // Red for terminated
                        }
                        break;
                    }
                }
                
                int col = chart_start_col + (t - scroll_offset);
                if (col < max_x - 1 && display_char != ' ') {
                    attron(COLOR_PAIR(color_pair) | A_BOLD);
                    mvprintw(row, col, "%c", display_char);
                    attroff(COLOR_PAIR(color_pair) | A_BOLD);
                }
            }
        }
        
        // Display scroll instructions with better formatting
        attron(COLOR_PAIR(6));
        mvprintw(max_y - 1, 0, "                                                                              ");
        mvprintw(max_y - 1, 2, "Time: %d-%d | Processes: %d-%d | Arrow keys=scroll | q/Enter/Space=continue", 
                 scroll_offset, 
                 (scroll_offset + available_width - 1 < max_time) ? scroll_offset + available_width - 1 : max_time,
                 proc_scroll + 1, 
                 (proc_scroll + displayed_procs < num_procs) ? proc_scroll + displayed_procs : num_procs);
        attroff(COLOR_PAIR(6));
        
        refresh();
        ch = getch();
        
        // Handle scrolling
        if (ch == KEY_RIGHT && scroll_offset < max_scroll) scroll_offset++;
        else if (ch == KEY_LEFT && scroll_offset > 0) scroll_offset--;
        else if (ch == KEY_DOWN && proc_scroll < max_proc_scroll) proc_scroll++;
        else if (ch == KEY_UP && proc_scroll > 0) proc_scroll--;
        else if (ch == 'q' || ch == 'Q' || ch == '\n' || ch == ' ') break;
        
    } while (1);
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
