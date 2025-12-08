#!/bin/bash
# Test script for Process Scheduling Simulator
# Tests all scheduling policies with example configurations

echo "========================================="
echo "Process Scheduling Simulator - Test Suite"
echo "========================================="
echo ""

# Check if executable exists
if [ ! -f "./bin/scheduler" ]; then
    echo "Error: Executable not found. Building project..."
    make clean && make
    if [ $? -ne 0 ]; then
        echo "Build failed. Exiting."
        exit 1
    fi
fi

# Test configuration files
TEST_FILES=("examples/simple.txt" "examples/processes.txt")

# Test each scheduling policy
POLICIES=("FIFO" "Round-Robin" "Priority" "Multi-level" "Multi-level Aging")
POLICY_INPUTS=("1" "2\n2" "3" "4" "5")

echo "Testing scheduling policies..."
echo ""

for TEST_FILE in "${TEST_FILES[@]}"; do
    if [ ! -f "$TEST_FILE" ]; then
        echo "Warning: Test file $TEST_FILE not found. Skipping."
        continue
    fi
    
    echo "----------------------------------------"
    echo "Testing with: $TEST_FILE"
    echo "----------------------------------------"
    echo ""
    
    for i in {0..4}; do
        POLICY="${POLICIES[$i]}"
        INPUT="${POLICY_INPUTS[$i]}"
        
        echo "Testing: $POLICY"
        echo "Input: $INPUT"
        
        # Run test (exit after one simulation)
        echo -e "${INPUT}\n\n0" | ./bin/scheduler "$TEST_FILE" > /tmp/test_output_${i}.txt 2>&1
        
        if [ $? -eq 0 ]; then
            # Check if output contains expected keywords
            if grep -q "SIMULATION RESULTS" /tmp/test_output_${i}.txt && \
               grep -q "STATISTICS" /tmp/test_output_${i}.txt; then
                echo "✓ $POLICY: PASSED"
            else
                echo "✗ $POLICY: FAILED (missing expected output)"
                echo "Output preview:"
                head -20 /tmp/test_output_${i}.txt
            fi
        else
            echo "✗ $POLICY: FAILED (execution error)"
        fi
        echo ""
    done
done

echo "========================================="
echo "Test Suite Complete"
echo "========================================="

# Cleanup
rm -f /tmp/test_output_*.txt

echo ""
echo "To run individual tests manually:"
echo "  ./bin/scheduler examples/simple.txt"
echo "  ./bin/scheduler examples/processes.txt"
