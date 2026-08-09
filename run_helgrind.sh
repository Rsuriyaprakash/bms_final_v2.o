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
    --tool=helgrind \
    --history-level=full \
    --error-exitcode=1 \
    --log-file=helgrind_report.txt \
    ./bmsv21
