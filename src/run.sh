#!/usr/bin/bash

# Process Scheduling Simulator Runner Script
# Builds and runs the simulator with default configuration

echo "Building Process Scheduling Simulator..."
make clean && make

if [ $? -ne 0 ]; then
    echo "Build failed. Please check the errors above."
    exit 1
fi

echo ""
echo "Build successful!"
echo ""

# Check if a config file was provided as argument
if [ $# -eq 0 ]; then
    # Use default example
    CONFIG="examples/processes.txt"
    echo "No configuration file specified. Using default: $CONFIG"
else
    CONFIG="$1"
    echo "Using configuration file: $CONFIG"
fi

echo ""
echo "Starting simulator..."
echo ""

./bin/scheduler "$CONFIG"
