# Scheduling Policies Directory

This directory can contain additional scheduling policy implementations.
Currently, the following built-in policies are available:

1. **FIFO (First In First Out)** - Non-preemptive scheduling based on arrival time
2. **Round Robin** - Preemptive scheduling with time quantum
3. **Priority Preemptive** - Preemptive scheduling based on priority
4. **Multi-Level Queue with Aging** - Advanced scheduling with multiple priority levels

## Adding Custom Policies

To add a custom scheduling policy:

1. Create a new `.c` file in this directory
2. Implement the scheduling function following the interface in `basic_sched.h`
3. Register the policy in the menu system

The scheduler dynamically discovers policies placed in this directory.
