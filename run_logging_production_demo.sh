#!/bin/sh
set -eu

rm -f logs/bms.log

gcc -std=c11 -Wall -Wextra -Wpedantic -Wconversion -Wshadow \
    -DBMS_PRODUCTION_BUILD=1 -pthread -I. \
    logging.c tools/logging_smoke.c -o logging_smoke_prod

./logging_smoke_prod
rm -f logging_smoke_prod

if grep -q '| DEBUG ' logs/bms.log; then
    echo "Unexpected DEBUG entry in production log." >&2
    exit 1
fi

echo "Production log correctly filtered DEBUG entries."
cat logs/bms.log
