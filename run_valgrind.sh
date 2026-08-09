#!/bin/sh
set -eu

if ! command -v valgrind >/dev/null 2>&1; then
    echo "valgrind is not installed or not in PATH." >&2
    exit 127
fi

if [ ! -x ./bmsv21 ]; then
    sh build_bms_multithread.sh
fi

valgrind \
    --tool=memcheck \
    --leak-check=full \
    --show-leak-kinds=all \
    --track-origins=yes \
    --errors-for-leak-kinds=definite,indirect \
    --error-exitcode=1 \
    --log-file=valgrind_memcheck_report.txt \
    ./bmsv21
