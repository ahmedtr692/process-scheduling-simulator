# Process Scheduling Simulator

A multi-task process scheduling simulator for Linux systems, implementing various CPU scheduling algorithms.

## Table of Contents
- [Overview](#overview)
- [Features](#features)
- [Requirements](#requirements)
- [Installation](#installation)
- [Usage](#usage)
- [Configuration File Format](#configuration-file-format)
- [Scheduling Policies](#scheduling-policies)
- [Building from Source](#building-from-source)
- [Examples](#examples)
- [License](#license)

## Overview

This project simulates process scheduling using different scheduling algorithms. It provides a textual console interface to select and run simulations with various scheduling policies.

## Features

### Minimum Functionalities
- ✅ **FIFO (First In First Out)** - Non-preemptive scheduling
- ✅ **Round-Robin** - Time-sliced preemptive scheduling with configurable quantum
- ✅ **Priority Preemptive** - Processes scheduled based on priority
- ✅ Textual console output with simulation results
- ✅ Configuration file support for process definitions
- ✅ Dynamic menu-based policy selection
- ✅ Comprehensive Makefile for building and installation

### Advanced Functionalities
- ✅ **Multi-level Queue** with static priority
- ✅ **Multi-level Queue with Aging** - Dynamic priority adjustment to prevent starvation
- ✅ Detailed statistics (turnaround time, waiting time)

## Requirements

- **Operating System**: Linux (Ubuntu, Debian, Fedora, etc.)
- **Compiler**: GCC (GNU Compiler Collection) version 4.8 or later
- **Standard**: C99
- **Build System**: GNU Make

## Installation

### Option 1: System-wide Installation (requires sudo)

```bash
# Clone the repository
git clone https://github.com/ahmedtr692/process-scheduling-simulator.git
cd process-scheduling-simulator

# Build the project
make

# Install system-wide (requires administrator privileges)
sudo make install
```

The executable will be installed to `/usr/local/bin/scheduler`.

### Option 2: User Installation (no sudo required)

```bash
# Clone the repository
git clone https://github.com/ahmedtr692/process-scheduling-simulator.git
cd process-scheduling-simulator

# Build the project
make

# Install to user's local directory
make install PREFIX=$HOME/.local

# Add to PATH (add this to your ~/.bashrc or ~/.bash_profile)
export PATH="$HOME/.local/bin:$PATH"
```

### Option 3: Run without Installation

```bash
# Clone the repository
git clone https://github.com/ahmedtr692/process-scheduling-simulator.git
cd process-scheduling-simulator

# Build the project
make

# Run directly from the bin directory
./bin/scheduler examples/processes.txt
```

## Usage

### Basic Usage

```bash
scheduler <config_file>
```

**Example:**
```bash
scheduler examples/processes.txt
```

### Interactive Menu

After loading the configuration file, you'll see an interactive menu:

```
=========================================
  PROCESS SCHEDULING SIMULATOR
=========================================
Select a scheduling policy:

  1. FIFO (First In First Out)
  2. Round-Robin
  3. Priority Preemptive
  4. Multi-level Queue (Static Priority)
  5. Multi-level Queue with Aging
  0. Exit
=========================================
Enter your choice:
```

### Running Without Arguments

If you run the program without arguments, it will display usage information:

```bash
scheduler
```

Output:
```
Usage: scheduler <config_file>

Example:
  scheduler processes.txt

Configuration file format:
  # Comments start with #
  ProcessName ArrivalTime Priority Operation1:Duration Operation2:Duration ...

Example:
  P1 0 5 calc:10 io:5 calc:3
  P2 1 3 calc:8 io:4
```

## Configuration File Format

The configuration file defines processes and their operations. Each line represents one process.

### Format Specification

```
ProcessName ArrivalTime Priority Operation1:Duration Operation2:Duration ...
```

- **ProcessName**: Unique identifier for the process (no spaces)
- **ArrivalTime**: Time when the process arrives (integer >= 0)
- **Priority**: Process priority (higher number = higher priority)
- **Operations**: Sequence of operations in format `type:duration`

### Operation Types

- `calc` - CPU computation operation
- `io` - I/O operation
- `none` - No operation (idle)

### Comments and Blank Lines

- Lines starting with `#` are treated as comments
- Blank lines are ignored

### Example Configuration File

```
# Example process configuration
# Format: ProcessName ArrivalTime Priority Operation1:Duration Operation2:Duration

# High priority process arriving at time 0
P1 0 5 calc:10 io:5 calc:3

# Medium priority process arriving at time 1
P2 1 3 calc:8 io:4

# High priority process arriving at time 2
P3 2 7 calc:6 io:3 calc:2

# Low priority process arriving at time 3
P4 3 2 io:4 calc:5

# High priority process arriving at time 5
P5 5 6 calc:12
```

## Scheduling Policies

### 1. FIFO (First In First Out)

- **Type**: Non-preemptive
- **Description**: Processes are executed in the order they arrive
- **Characteristics**: Simple but can lead to convoy effect

### 2. Round-Robin

- **Type**: Preemptive
- **Description**: Each process gets a fixed time quantum
- **Configuration**: You'll be prompted to enter the time quantum
- **Characteristics**: Fair allocation, good for time-sharing systems

### 3. Priority Preemptive

- **Type**: Preemptive
- **Description**: Higher priority processes execute first
- **Characteristics**: Can lead to starvation of low-priority processes

### 4. Multi-level Queue (Static Priority)

- **Type**: Preemptive
- **Description**: Multiple queues with different priorities, round-robin within same priority
- **Characteristics**: Combines priority and round-robin scheduling

### 5. Multi-level Queue with Aging

- **Type**: Preemptive with dynamic priority
- **Description**: Like multi-level queue but increases priority of waiting processes
- **Aging Threshold**: Default is 5 time units
- **Characteristics**: Prevents starvation by gradually increasing priority

## Building from Source

### Makefile Targets

- `make` or `make all` - Build the project
- `make clean` - Remove build artifacts
- `make rebuild` - Clean and rebuild
- `make install` - Install the executable (may require sudo)
- `make install PREFIX=<path>` - Install to custom location
- `make uninstall` - Remove installed files
- `make run` - Build and run with example configuration
- `make help` - Display help information

### Build Process

```bash
# Clean previous builds
make clean

# Build the project
make

# The executable will be in bin/scheduler
```

### Project Structure

```
process-scheduling-simulator/
├── src/
│   ├── main.c                          # Main program
│   └── headers/
│       ├── basic_sched.h               # Core data structures
│       ├── basic.c                     # Queue operations
│       ├── fifo.c                      # FIFO algorithm
│       ├── round_robin.c               # Round-Robin algorithm
│       ├── priority_preemptive.c       # Priority algorithm
│       ├── multilevel.c                # Multi-level queue
│       ├── multilevel_aging.c          # Multi-level with aging
│       ├── config_parser.h/.c          # Configuration parser
│       └── display.h/.c                # Output display
├── examples/
│   ├── processes.txt                   # Example configuration
│   └── simple.txt                      # Simple test case
├── Makefile                            # Build system
├── LICENSE                             # GPL-3.0 License
└── README.md                           # This file
```

## Examples

### Example 1: Running FIFO Scheduling

```bash
./bin/scheduler examples/simple.txt
```

Select option 1 (FIFO), and you'll see:

```
========================================
   SIMULATION RESULTS
========================================

PROCESS         TIME       STATE           OPERATION 
-------         ----       -----           --------- 
ProcessA        0          RUNNING         CALC      
ProcessB        0          WAITING         NONE      
ProcessC        0          WAITING         NONE      
...
```

### Example 2: Running Round-Robin with Quantum=2

```bash
./bin/scheduler examples/processes.txt
```

Select option 2 (Round-Robin), enter quantum 2, and observe the time-sliced execution.

### Example 3: Testing Priority Scheduling

Create a file `priority_test.txt`:

```
# Priority test
HighPrio 0 10 calc:5
MediumPrio 0 5 calc:5
LowPrio 0 1 calc:5
```

Run:
```bash
./bin/scheduler priority_test.txt
```

Select option 3 to see priority-based execution.

## Output Format

The simulator produces two types of output:

### 1. Simulation Results
Shows each time unit with process states and operations:
- **PROCESS**: Process name
- **TIME**: Current time unit
- **STATE**: RUNNING, WAITING, READY, TERMINATED, BLOCKED
- **OPERATION**: CALC, I/O, or NONE

### 2. Statistics
Summary of process execution:
- **START**: First execution time
- **END**: Completion time
- **TURNAROUND**: Total time from arrival to completion
- **WAITING**: Total waiting time

## Troubleshooting

### Build Errors

If you encounter build errors:

```bash
# Make sure GCC is installed
gcc --version

# Install GCC if needed (Ubuntu/Debian)
sudo apt-get update
sudo apt-get install build-essential

# Install GCC (Fedora/RHEL)
sudo dnf install gcc make
```

### Permission Errors During Installation

If you get permission errors:

```bash
# Option 1: Use sudo
sudo make install

# Option 2: Install to user directory
make install PREFIX=$HOME/.local
```

### Configuration File Errors

If your configuration file isn't loading:

- Check file format (space-separated values)
- Ensure no invalid characters
- Verify operation format: `type:duration`
- Check for proper line endings (LF, not CRLF)

## License

This project is licensed under the **GNU General Public License v3.0 (GPL-3.0)**.

See the [LICENSE](LICENSE) file for details.

### Why GPL-3.0?

We chose GPL-3.0 because:
- **Copyleft Protection**: Ensures derivative works remain free and open source
- **Community Contribution**: Encourages sharing improvements with the community
- **Patent Protection**: Includes explicit patent grant and retaliation clauses
- **Compatibility**: Wide adoption in academic and open-source projects
- **Strong Freedom Guarantee**: Users can run, study, modify, and distribute

## Contributing

Contributions are welcome! Please feel free to submit issues and pull requests.

## Authors

Developed as part of Operating Systems course project (October - December 2025).

## Acknowledgments

- Course: Operating Systems (Systèmes d'exploitation)
- Development Methodology: SCRUM
- Team Size: 5-7 members per group
