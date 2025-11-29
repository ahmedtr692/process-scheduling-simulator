#!/usr/bin/bash

# Process Scheduling Simulator - Run Script
# Build and run the scheduler with the sample configuration

# Build the project
echo "Building the scheduler..."
make clean && make

if [ $? -ne 0 ]; then
    echo "Build failed!"
    exit 1
fi

# Run with sample config
echo ""
echo "Running scheduler with sample_config.txt..."
echo ""

./bin/scheduler sample_config.txt