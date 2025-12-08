#ifndef DISPLAY_H
#define DISPLAY_H

#include "basic_sched.h"

void init_ncurses_display();
void cleanup_ncurses_display();
void display_simulation_results(process_descriptor_t* descriptor, int size);
void display_statistics(process_descriptor_t* descriptor, int size);

#endif
