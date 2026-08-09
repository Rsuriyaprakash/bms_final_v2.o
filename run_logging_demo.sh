#!/bin/sh
set -eu

gcc -std=c11 -Wall -Wextra -Wpedantic -Wconversion -Wshadow \
    -pthread -I. logging.c tools/logging_smoke.c -o logging_smoke

./logging_smoke
rm -f logging_smoke

echo "Generated log: logs/bms.log"
tail -n 10 logs/bms.log
