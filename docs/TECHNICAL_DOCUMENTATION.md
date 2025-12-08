# Process Scheduling Simulator - Technical Documentation

**Project**: Process Scheduling Simulator  
**Course**: Operating Systems (Systèmes d'exploitation)  
**Period**: October - December 2025  
**License**: GNU GPL v3.0

---

## Table of Contents

1. [Introduction](#1-introduction)
2. [Data Structures](#2-data-structures)
3. [Algorithms](#3-algorithms)
4. [Implementation Details](#4-implementation-details)
5. [Development Process](#5-development-process)

---

## 1. Introduction

This document describes the data structures and algorithms used in the Process Scheduling Simulator. The simulator implements various CPU scheduling algorithms to demonstrate different approaches to process management in operating systems.

### 1.1 Project Objectives

- Simulate multi-task process scheduling on Linux systems
- Implement multiple scheduling policies (FIFO, Round-Robin, Priority, Multi-level)
- Provide textual output of simulation results
- Support flexible configuration through external files

### 1.2 Architecture Overview

The system follows a modular architecture:

```
┌─────────────────┐
│   main.c        │  ← Entry point, menu system
└────────┬────────┘
         │
    ┌────▼────────────────────────────┐
    │  Configuration Parser           │
    │  (config_parser.c)              │
    └────┬────────────────────────────┘
         │
    ┌────▼────────────────────────────┐
    │  Core Data Structures           │
    │  (basic_sched.h, basic.c)       │
    └────┬────────────────────────────┘
         │
    ┌────▼────────────────────────────┐
    │  Scheduling Algorithms          │
    │  - FIFO                         │
    │  - Round-Robin                  │
    │  - Priority Preemptive          │
    │  - Multi-level                  │
    │  - Multi-level with Aging       │
    └────┬────────────────────────────┘
         │
    ┌────▼────────────────────────────┐
    │  Display & Statistics           │
    │  (display.c)                    │
    └─────────────────────────────────┘
```

---

## 2. Data Structures

### 2.1 Process State Enumeration

```c
typedef enum process_state {
    waiting_p,      // Process is waiting for CPU
    running_p,      // Process is executing
    terminated_p,   // Process has completed
    ready_p,        // Process is ready to execute
    blocked_p       // Process is blocked (I/O wait)
} process_state;
```

**Purpose**: Represents the current state of a process in the system.

**States**:
- `waiting_p`: Process is in ready queue but not running
- `running_p`: Process is currently executing on CPU
- `terminated_p`: Process has finished all operations
- `ready_p`: Process is ready but not yet scheduled
- `blocked_p`: Process is waiting for I/O completion

### 2.2 Operation Type Enumeration

```c
typedef enum {
    calc_p,   // CPU computation
    IO_p,     // I/O operation
    none      // No operation
} process_operation_t;
```

**Purpose**: Defines the type of operation a process is performing.

### 2.3 Operation Structure

```c
typedef struct operation_t {
    process_operation_t operation_p;  // Type of operation
    int duration_op;                   // Duration in time units
} operation_t;
```

**Purpose**: Represents a single operation with its type and duration.

**Fields**:
- `operation_p`: Type of operation (CPU, I/O, or none)
- `duration_op`: How many time units the operation takes

### 2.4 Process Structure

```c
typedef struct process_t {
    char *process_name;              // Unique process identifier
    int begining_date;               // When process starts
    operation_t *descriptor_p;       // Array of operations
    int arrival_time_p;              // When process arrives
    int operations_count;            // Number of operations
    int priority_p;                  // Process priority
} process_t;
```

**Purpose**: Complete description of a process.

**Fields**:
- `process_name`: String identifier for the process
- `begining_date`: Initial start time
- `descriptor_p`: Dynamic array of operations to execute
- `arrival_time_p`: Time when process arrives in system
- `operations_count`: Total number of operations
- `priority_p`: Static priority (higher = more important)

### 2.5 Queue Node Structure

```c
typedef struct node_t {
    process_t proc;       // Process data
    struct node_t *next;  // Pointer to next node
} node_t;
```

**Purpose**: Linked list node for process queue.

**Design Choice**: Single-linked list for simplicity and O(1) insertion at tail.

### 2.6 Process Queue Structure

```c
typedef struct process_queue {
    node_t *head;   // First node in queue
    node_t *tail;   // Last node in queue
    int size;       // Number of processes
} process_queue;
```

**Purpose**: FIFO queue for managing processes.

**Operations**:
- `add_tail()`: O(1) insertion at end
- `remove_head()`: O(1) removal from front
- Maintains both head and tail pointers for efficiency

### 2.7 Process Descriptor Structure

```c
typedef struct process_descriptor_t {
    char* process_name;              // Process identifier
    int date;                        // Time unit
    process_state state;             // Current state
    process_operation_t operation;   // Current operation
} process_descriptor_t;
```

**Purpose**: Snapshot of process state at a specific time.

**Usage**: Array of descriptors creates timeline of simulation.

---

## 3. Algorithms

### 3.1 FIFO (First In First Out)

**File**: `src/headers/fifo.c`

**Algorithm**:
```
1. Sort processes by arrival time
2. For each process in sorted order:
   a. Wait until arrival time
   b. Execute all operations sequentially
   c. Mark other ready processes as waiting
   d. Mark process as terminated when done
3. Output complete execution timeline
```

**Complexity**:
- Time: O(n²) for bubble sort + O(m) for execution (m = total operations)
- Space: O(n) for queue copy

**Characteristics**:
- Non-preemptive
- Simple implementation
- Can cause convoy effect (short processes wait for long ones)

**Code Structure**:
```c
void fifo_sched(process_queue *p, process_descriptor_t **descriptor, int* size) {
    // 1. Create working copy of queue
    // 2. Sort by arrival time (bubble sort)
    // 3. Execute each process completely
    // 4. Track states of all processes at each time unit
    // 5. Clean up
}
```

### 3.2 Round-Robin

**File**: `src/headers/round_robin.c`

**Algorithm**:
```
1. Initialize quantum (time slice)
2. While processes remain:
   a. Get process from front of queue
   b. Execute for min(quantum, remaining_time)
   c. If work remains, add to back of queue
   d. If completed, mark as terminated
   e. Update time and states
3. Output execution timeline
```

**Complexity**:
- Time: O(m/q) iterations × O(n) state updates = O(mn/q)
  where m = total operations, q = quantum, n = processes
- Space: O(n) for queue

**Characteristics**:
- Preemptive
- Fair time allocation
- Response time depends on quantum size
- Small quantum → high context switch overhead
- Large quantum → approaches FIFO

**Parameters**:
- `quantum`: Time slice per process (user configurable)

### 3.3 Priority Preemptive

**File**: `src/headers/priority_preemptive.c`

**Algorithm**:
```
1. For each time unit:
   a. Find ready process with highest priority
   b. If multiple have same priority, use arrival time
   c. Execute selected process for 1 time unit
   d. Mark others as waiting
   e. Check if current operation completed
2. Continue until all processes terminated
```

**Complexity**:
- Time: O(T × n) where T = total execution time, n = processes
- Space: O(n) for process tracking arrays

**Characteristics**:
- Preemptive (highest priority always runs)
- Can cause starvation of low-priority processes
- Priority is static (doesn't change)

**Priority Comparison**:
1. Higher priority number = higher priority
2. Same priority → earlier arrival time
3. Same arrival → process order in input

### 3.4 Multi-level Queue (Static Priority)

**File**: `src/headers/multilevel.c`

**Algorithm**:
```
1. For each time unit:
   a. Find highest priority level with ready processes
   b. Among that level, use round-robin selection
   c. Execute selected process for 1 time unit
   d. Track last-served index per priority level
2. Continue until all processes complete
```

**Complexity**:
- Time: O(T × n) for scheduling decisions
- Space: O(P) for round-robin indices (P = max priority levels)

**Characteristics**:
- Combines priority scheduling with fairness
- Round-robin within same priority level
- No starvation within same priority
- Lower priorities can still starve

**Data Structure**:
```c
int *rr_index;  // Last served index for each priority level
```

### 3.5 Multi-level Queue with Aging

**File**: `src/headers/multilevel_aging.c`

**Algorithm**:
```
1. Initialize wait_time[i] = 0 for all processes
2. For each time unit:
   a. Update wait times for all ready processes
   b. If wait_time[i] >= THRESHOLD:
      - Increase priority[i]
      - Reset wait_time[i] = 0
   c. Select highest priority ready process (round-robin within level)
   d. Execute selected process for 1 time unit
   e. Reset wait_time for executed process
3. Continue until all complete
```

**Complexity**:
- Time: O(T × n) for scheduling + O(T × n) for aging = O(T × n)
- Space: O(n) for wait times + O(P) for RR indices

**Characteristics**:
- Prevents starvation through aging
- Dynamic priority adjustment
- Fairness improves over time
- Aging threshold balances responsiveness vs. overhead

**Aging Parameters**:
```c
#define AGING_THRESHOLD 5  // Time units before priority boost
```

**Aging Logic**:
- Every AGING_THRESHOLD time units, priority increases by 1
- Only ready (not blocked) processes age
- Running process doesn't age
- Prevents indefinite waiting

---

## 4. Implementation Details

### 4.1 Configuration File Parsing

**File**: `src/headers/config_parser.c`

**Function**: `parse_config_file()`

**Algorithm**:
```
1. Open configuration file
2. For each line:
   a. Trim whitespace
   b. Skip if empty or comment (#)
   c. Parse: name arrival_time priority operations...
   d. For each operation:
      - Parse type:duration format
      - Create operation_t structure
   e. Create process_t and add to queue
3. Return number of processes loaded
```

**Error Handling**:
- File not found → return -1
- Invalid format → skip line with warning
- Missing fields → skip line with warning

**Format Support**:
- Comments: Lines starting with `#`
- Blank lines: Ignored
- Operations: `type:duration` format
- Flexible whitespace

### 4.2 Display and Statistics

**File**: `src/headers/display.c`

**Functions**:
1. `print_simulation_results()`: Shows timeline
2. `print_statistics()`: Calculates metrics

**Statistics Calculated**:

1. **Start Time**: First execution time for each process
2. **End Time**: Last state change for each process
3. **Turnaround Time**: End - Start
4. **Waiting Time**: Count of waiting_p states

**Implementation**:
```c
// Per-process tracking
typedef struct {
    char name[64];
    int start_time;
    int end_time;
    int total_wait;
    int total_run;
} proc_stats_t;
```

### 4.3 Queue Operations

**File**: `src/headers/basic.c`

#### add_tail()

```c
void add_tail(process_queue* p, process_t process)
```

**Algorithm**:
```
1. Allocate new node
2. Copy process data
3. Set next = NULL
4. If queue empty:
   - Set head = tail = new node
5. Else:
   - Link tail->next = new node
   - Update tail = new node
6. Increment size
```

**Time Complexity**: O(1)

#### remove_head()

```c
void remove_head(process_queue *p)
```

**Algorithm**:
```
1. If queue empty, return
2. Save current head
3. Move head to head->next
4. Decrement size
5. If size becomes 0, set tail = NULL
6. Free old head
```

**Time Complexity**: O(1)

#### append_descriptor()

```c
void append_descriptor(process_descriptor_t **descriptor, 
                       process_descriptor_t unit_descriptor, 
                       int *size)
```

**Algorithm**:
```
1. Reallocate descriptor array (size + 1)
2. Copy unit_descriptor to new slot
3. Increment size
```

**Time Complexity**: O(n) due to realloc, but amortized O(1)

**Note**: Could be optimized with capacity-based growth

### 4.4 Memory Management

**Allocation Points**:
1. Process names: `malloc()` in config parser
2. Operation arrays: `malloc()` per process
3. Queue nodes: `malloc()` per process
4. Descriptor array: `realloc()` during simulation

**Deallocation**:
1. After simulation: Free descriptor array
2. After completion: Free all queue nodes
3. Process cleanup: Free names and operation arrays

**Memory Leaks Prevention**:
- Each `malloc()` has corresponding `free()`
- Simulation creates copy of queue to preserve original
- Cleanup after each simulation run

---

## 5. Development Process

### 5.1 SCRUM Methodology

This project was developed using SCRUM agile methodology with the following structure:

#### Team Composition
- Team Size: 5-7 members
- Roles: Product Owner, Scrum Master, Development Team

#### Sprint Structure
- Sprint Duration: 2 weeks
- Total Sprints: ~4-5 sprints (October - December 2025)

#### Sprint Planning

**Sprint 1**: Foundation
- Set up repository
- Design data structures
- Implement basic queue operations
- Initial FIFO algorithm

**Sprint 2**: Core Algorithms
- Implement Round-Robin
- Implement Priority Preemptive
- Add configuration file parsing
- Unit testing

**Sprint 3**: Advanced Features
- Multi-level queue scheduling
- Aging mechanism
- Enhanced statistics
- Integration testing

**Sprint 4**: Documentation & Polish
- User documentation (README)
- Technical documentation
- Example configurations
- Final testing and bug fixes

#### Development Practices

**Version Control**:
- Git with feature branches
- Pull request reviews
- Semantic commit messages

**Code Quality**:
- C99 standard compliance
- Compiler warnings enabled (-Wall -Wextra)
- Memory leak checking
- Code reviews

**Testing Strategy**:
- Unit tests for queue operations
- Integration tests for each scheduler
- End-to-end tests with example processes
- Edge case validation

### 5.2 Design Decisions

#### Why Linked Lists for Queues?

**Advantages**:
- O(1) insertion and deletion at ends
- Dynamic size (no fixed capacity)
- Simple implementation
- Matches queue semantics

**Alternatives Considered**:
- Array-based: Would require reallocation
- Circular buffer: More complex for variable-size processes

#### Why Dynamic Descriptor Array?

**Rationale**:
- Unknown total output size beforehand
- Realloc allows growth as needed
- Simpler than linked list for sequential output

**Trade-off**:
- O(n) realloc cost vs. O(1) list append
- Sequential memory access better for display
- Acceptable for simulation (not real-time)

#### Why Separate Scheduling Functions?

**Modularity Benefits**:
- Each policy in separate file
- Easy to add new policies
- Testing in isolation
- Clear separation of concerns

**Consistent Interface**:
```c
void policy_sched(process_queue* p, 
                  process_descriptor_t** descriptor, 
                  int *size);
```

### 5.3 Build System Design

**Makefile Structure**:
- Modular compilation (separate .o files)
- Dependency tracking
- Clean separation of source/build/bin
- Installation flexibility (user/system)

**Installation Modes**:
1. System-wide: `/usr/local/bin` (requires sudo)
2. User local: `$HOME/.local/bin` (no sudo)
3. No install: Run from `bin/` directory

### 5.4 Future Enhancements

Potential improvements for future versions:

1. **Graphical Interface**:
   - Gantt chart visualization
   - Real-time simulation display
   - Interactive process creation

2. **Additional Algorithms**:
   - Shortest Job First (SJF)
   - Shortest Remaining Time First (SRTF)
   - Fair-share scheduling
   - Real-time scheduling (EDF, RMS)

3. **Performance Optimization**:
   - Priority queue for priority scheduling
   - Capacity-based array growth
   - Reduced memory allocations

4. **Enhanced Features**:
   - Process creation/fork simulation
   - Inter-process communication
   - Resource contention modeling
   - Multi-core simulation

5. **Analysis Tools**:
   - Average waiting time calculation
   - CPU utilization metrics
   - Throughput measurement
   - Export to CSV/JSON

---

## Conclusion

This technical documentation describes the complete architecture, data structures, and algorithms of the Process Scheduling Simulator. The implementation demonstrates fundamental operating system concepts through practical simulation of CPU scheduling policies.

The modular design allows easy extension with new scheduling algorithms, while the SCRUM development process ensured iterative improvement and team collaboration.

---

**Document Version**: 1.0  
**Last Updated**: December 2025  
**Authors**: Process Scheduling Simulator Development Team
