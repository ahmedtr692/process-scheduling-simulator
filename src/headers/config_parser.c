#include "config_parser.h"
#include <string.h>
#include <ctype.h>

#define MAX_LINE 1024
#define MAX_OPS 100

static char* trim(char* str) {
    while (isspace(*str)) str++;
    if (*str == 0) return str;
    char* end = str + strlen(str) - 1;
    while (end > str && isspace(*end)) end--;
    end[1] = '\0';
    return str;
}

int parse_config_file(const char* filename, process_queue* pqueue) {
    FILE* fp = fopen(filename, "r");
    if (!fp) {
        fprintf(stderr, "Error: Cannot open configuration file '%s'\n", filename);
        return -1;
    }

    char line[MAX_LINE];
    int line_num = 0;
    int process_count = 0;

    while (fgets(line, sizeof(line), fp)) {
        line_num++;
        char* trimmed = trim(line);
        
        // Skip empty lines and comments
        if (trimmed[0] == '\0' || trimmed[0] == '#') {
            continue;
        }

        // Parse process definition
        // Format: name arrival_time priority operation1_type:duration operation2_type:duration ...
        // Example: P1 0 5 calc:10 io:5 calc:3
        
        char name[64];
        int arrival_time, priority;
        
        char* token = strtok(trimmed, " \t");
        if (!token) continue;
        strncpy(name, token, sizeof(name) - 1);
        name[sizeof(name) - 1] = '\0';
        
        token = strtok(NULL, " \t");
        if (!token) {
            fprintf(stderr, "Warning: Line %d - Missing arrival time\n", line_num);
            continue;
        }
        arrival_time = atoi(token);
        
        token = strtok(NULL, " \t");
        if (!token) {
            fprintf(stderr, "Warning: Line %d - Missing priority\n", line_num);
            continue;
        }
        priority = atoi(token);
        
        // Parse operations
        operation_t ops[MAX_OPS];
        int op_count = 0;
        
        while ((token = strtok(NULL, " \t")) != NULL && op_count < MAX_OPS) {
            char op_type[32];
            int duration;
            
            if (sscanf(token, "%[^:]:%d", op_type, &duration) == 2) {
                if (strcmp(op_type, "calc") == 0) {
                    ops[op_count].operation_p = calc_p;
                } else if (strcmp(op_type, "io") == 0) {
                    ops[op_count].operation_p = IO_p;
                } else {
                    ops[op_count].operation_p = none;
                }
                ops[op_count].duration_op = duration;
                op_count++;
            }
        }
        
        if (op_count == 0) {
            fprintf(stderr, "Warning: Line %d - No operations defined for process %s\n", line_num, name);
            continue;
        }
        
        // Create process
        process_t proc;
        proc.process_name = malloc(strlen(name) + 1);
        strcpy(proc.process_name, name);
        proc.arrival_time_p = arrival_time;
        proc.begining_date = arrival_time;
        proc.priority_p = priority;
        proc.operations_count = op_count;
        proc.descriptor_p = malloc(op_count * sizeof(operation_t));
        memcpy(proc.descriptor_p, ops, op_count * sizeof(operation_t));
        
        add_tail(pqueue, proc);
        process_count++;
    }

    fclose(fp);
    printf("Loaded %d processes from configuration file\n", process_count);
    return process_count;
}
