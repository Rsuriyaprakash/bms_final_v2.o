#!/bin/sh
set -eu

SOURCES=$(find . -maxdepth 1 -name '*.c' \
    ! -name 'test_*.c' \
    ! -name 'test_runner.c')

set +e
gcc -std=c11 -Wall -Wextra -Wpedantic -Wconversion -Wshadow \
    -fanalyzer -pthread \
    $SOURCES \
    -o bmsv21_analyzed \
    > gcc_analyzer_report.txt 2>&1
RESULT=$?
set -e

cat gcc_analyzer_report.txt
exit "$RESULT"
