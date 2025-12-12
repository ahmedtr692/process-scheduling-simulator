#ifndef NCURSES_DISPLAY_H
#define NCURSES_DISPLAY_H

#include <ncurses.h>
#include "basic_sched.h"

// Initialize ncurses display
void init_ncurses_display();

// Cleanup ncurses display
void cleanup_ncurses_display();

// Display Gantt chart with scrolling support
void display_gantt_chart(process_descriptor_t* descriptor, int size);

// Display ready queue during execution
void display_ready_queue(process_queue* queue, int current_time);

// Display simulation results with scrolling
void display_simulation_results(process_descriptor_t* descriptor, int size);

// Display statistics
void display_statistics(process_descriptor_t* descriptor, int size);

// Show menu and get user choice
int show_menu();

// Get quantum for Round-Robin
int get_quantum();

#endif
