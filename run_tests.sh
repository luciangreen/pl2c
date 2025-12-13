#!/bin/bash
# run_tests.sh - Run all tests for pl2c

set -e

echo "========================================"
echo "PL2C Test Suite"
echo "========================================"
echo ""

# Test 1: Compile and run unification tests
echo "Test 1: Unification Algorithm"
echo "------------------------------"
gcc -o tests/test_unification tests/test_unification.c -std=c99 -Wall
./tests/test_unification
echo ""

# Test 2: Convert simple example
echo "Test 2: Simple Prolog to C Conversion"
echo "--------------------------------------"
./pl2c.sh examples/simple.pl simple_test
echo ""

# Test 3: Convert nondeterministic example
echo "Test 3: Nondeterministic Predicates"
echo "------------------------------------"
./pl2c.sh examples/nondeterministic.pl nondet_test
echo ""

echo "========================================"
echo "All Tests Passed!"
echo "========================================"
