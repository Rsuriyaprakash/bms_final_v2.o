#!/bin/sh
set -eu

REPORT_FILE="${1:-cppcheck_inconclusive_report.txt}"

if ! command -v cppcheck >/dev/null 2>&1; then
    echo "cppcheck is not installed or not in PATH." >&2
    exit 127
fi

SOURCES=$(find . -maxdepth 1 -name '*.c' \
    ! -name 'test_*.c' \
    ! -name 'test_runner.c')

cppcheck \
    --enable=warning,style,performance,portability \
    --std=c11 \
    --inconclusive \
    --suppress=missingIncludeSystem \
    $SOURCES \
    > "$REPORT_FILE" 2>&1

cat "$REPORT_FILE"
