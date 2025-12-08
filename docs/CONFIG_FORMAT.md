# Configuration File Format Guide

This document provides detailed information about the configuration file format used by the Process Scheduling Simulator.

## Overview

The configuration file defines a set of processes to be scheduled. Each process has:
- A unique name
- An arrival time
- A priority level
- A sequence of operations to execute

## File Format Syntax

### Basic Syntax

```
ProcessName ArrivalTime Priority Operation1:Duration Operation2:Duration ...
```

### Components

1. **ProcessName** (String, no spaces)
   - Unique identifier for the process
   - Can contain letters, numbers, underscores
   - Examples: `P1`, `Process_A`, `WebServer`

2. **ArrivalTime** (Integer >= 0)
   - Time unit when the process arrives in the system
   - Must be a non-negative integer
   - Examples: `0`, `5`, `100`

3. **Priority** (Integer)
   - Static priority of the process
   - Higher numbers indicate higher priority
   - Typically ranges from 1-10, but any integer is valid
   - Examples: `1` (low), `5` (medium), `10` (high)

4. **Operations** (Type:Duration pairs)
   - Sequence of operations the process must execute
   - Each operation has a type and duration
   - Format: `type:duration`
   - Multiple operations separated by spaces

### Operation Types

- **calc** - CPU computation (process uses CPU)
- **io** - I/O operation (process waits for I/O)
- **none** - No operation (idle/waiting)

### Operation Duration

- Positive integer representing time units
- Examples: `5`, `10`, `100`

## Comments and Blank Lines

### Comments

- Lines starting with `#` are comments
- Comments can explain process purposes or configurations
- Ignored by the parser

Example:
```
# This is a comment
# Define high-priority processes
```

### Blank Lines

- Empty lines are ignored
- Use blank lines to improve readability

## Complete Example

```conf
# Process Configuration File Example
# Format: ProcessName ArrivalTime Priority Operation1:Duration Operation2:Duration

# Web server - high priority, mixed workload
WebServer 0 9 calc:10 io:5 calc:8 io:3

# Database - medium priority, I/O heavy
Database 1 6 io:7 calc:5 io:4

# Background task - low priority
BackgroundTask 2 2 calc:20

# API service - high priority
APIService 3 8 calc:6 io:2 calc:4

# Batch job - low priority, CPU intensive
BatchJob 5 3 calc:50
```

## Validation Rules

The parser performs the following validations:

1. **Line Format**: Each non-comment, non-blank line must have at least:
   - Process name
   - Arrival time
   - Priority
   - At least one operation

2. **Data Types**:
   - Arrival time must be parseable as integer
   - Priority must be parseable as integer
   - Duration must be parseable as integer

3. **Operation Format**:
   - Must follow `type:duration` pattern
   - Type must be `calc`, `io`, or will default to `none`
   - Duration must be positive integer

## Error Handling

### Missing Fields

If a line is missing required fields, the parser will:
- Print a warning message with line number
- Skip that line
- Continue parsing remaining lines

Example warning:
```
Warning: Line 5 - Missing priority
```

### Invalid Format

If an operation doesn't match the `type:duration` format:
- That operation is skipped
- Warning is displayed
- Other valid operations on the same line are processed

### File Not Found

If the configuration file doesn't exist:
```
Error: Cannot open configuration file 'filename.txt'
```

## Best Practices

### 1. Organize by Priority

Group processes by priority for easier reading:

```conf
# High Priority Processes
HighPrio1 0 10 calc:5
HighPrio2 1 9 calc:8

# Medium Priority Processes
MedPrio1 2 5 calc:10
MedPrio2 3 5 calc:12

# Low Priority Processes
LowPrio1 4 1 calc:20
```

### 2. Use Descriptive Names

Choose process names that indicate their purpose:

```conf
# Good names
WebServer 0 9 calc:10
DatabaseQuery 1 7 io:5 calc:3
BackgroundSync 2 2 calc:15

# Less descriptive
P1 0 9 calc:10
P2 1 7 io:5 calc:3
P3 2 2 calc:15
```

### 3. Document Complex Scenarios

Add comments explaining non-obvious configurations:

```conf
# Simulate burst arrival - multiple processes at time 0
Process1 0 5 calc:10
Process2 0 5 calc:8
Process3 0 5 calc:12

# Delayed high-priority process to test preemption
UrgentTask 10 10 calc:5
```

### 4. Vary Arrival Times

For realistic simulations, vary arrival times:

```conf
EarlyBird 0 5 calc:10
OnTime 5 5 calc:10
Fashionably Late 10 5 calc:10
```

## Example Scenarios

### Scenario 1: Testing FIFO

All processes same priority, different arrival times:

```conf
P1 0 5 calc:10
P2 2 5 calc:8
P3 4 5 calc:6
```

### Scenario 2: Testing Priority

Same arrival, different priorities:

```conf
LowPrio 0 1 calc:10
MedPrio 0 5 calc:10
HighPrio 0 10 calc:10
```

### Scenario 3: Testing Round-Robin Fairness

Multiple processes with same priority:

```conf
Task1 0 5 calc:15
Task2 0 5 calc:15
Task3 0 5 calc:15
```

### Scenario 4: Testing Aging

Low priority arriving early, high priority arriving late:

```conf
OldTimer 0 1 calc:30
NewComer 10 10 calc:5
```

### Scenario 5: Mixed Workloads

Combination of CPU and I/O operations:

```conf
CPUBound 0 5 calc:20
IOBound 0 5 io:10 calc:5 io:8
Mixed 0 5 calc:5 io:5 calc:5 io:5
```

## Advanced Usage

### Large-Scale Testing

For stress testing, create files with many processes:

```bash
# Generate 100 processes programmatically
for i in {1..100}; do
    echo "P$i $((i % 10)) $((i % 10 + 1)) calc:$((i % 20 + 5))"
done > large_test.txt
```

### Benchmarking Different Algorithms

Create specific scenarios to compare algorithms:

```conf
# convoy_effect.txt - Shows FIFO weakness
LongJob 0 5 calc:100
ShortJob1 1 5 calc:2
ShortJob2 2 5 calc:2
ShortJob3 3 5 calc:2

# starvation.txt - Shows priority weakness
HighPrio1 0 10 calc:5
HighPrio2 5 10 calc:5
HighPrio3 10 10 calc:5
LowPrio 0 1 calc:5    # May starve
```

## Troubleshooting

### Problem: Process Not Executing

**Possible Causes**:
1. Syntax error in configuration file
2. No operations defined
3. Invalid operation format

**Solution**: Check parser output for warnings

### Problem: Unexpected Execution Order

**Possible Causes**:
1. Arrival times not as expected
2. Priority values reversed
3. Scheduling algorithm behavior

**Solution**: 
- Verify arrival times and priorities
- Understand the selected scheduling algorithm's behavior
- Check documentation for algorithm specifics

### Problem: Simulation Takes Too Long

**Possible Causes**:
1. Very long operation durations
2. Too many processes
3. Complex operation sequences

**Solution**:
- Reduce operation durations for testing
- Test with smaller process sets first
- Use simpler configurations for initial validation

## File Extension

While any extension works, common conventions:
- `.txt` - Plain text (most common)
- `.conf` - Configuration file
- `.cfg` - Configuration file
- No extension - Also acceptable

## Character Encoding

- Use UTF-8 or ASCII encoding
- Avoid special characters in process names
- Use standard newline characters (LF or CRLF)

## Summary

The configuration file format is simple yet flexible:
- One process per line
- Space-separated values
- Comments with `#`
- Operation format: `type:duration`

This design makes it easy to:
- Create test cases manually
- Generate configurations programmatically
- Understand configurations at a glance
- Debug scheduling issues

For more examples, see the `examples/` directory in the project.
