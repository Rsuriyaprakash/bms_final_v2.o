#!/bin/sh
set -eu

cppcheck --enable=warning,style,performance,portability \
    --std=c11 \
    --suppress=missingIncludeSystem \
    --error-exitcode=1 \
    $(find . -maxdepth 1 -name '*.c' \
        ! -name 'test_*.c' \
        ! -name 'test_runner.c') \
    2>&1 | tee cppcheck_report.txt
