#ifndef CONFIG_PARSER_H
#define CONFIG_PARSER_H

#include "basic_sched.h"

int parse_config_file(const char* filename, process_queue* pqueue);

#endif
