#!/bin/sh

set -eu

LOG_FILE="logs/bms.log"
EXECUTABLE="logging_smoke_prod"

echo "Building production logging demo..."

rm -f "$LOG_FILE"
rm -f "$EXECUTABLE"

gcc -std=c11 -Wall -Wextra -Wpedantic -Wconversion -Wshadow \
    -DBMS_PRODUCTION_BUILD=1 \
    -pthread -I. \
    logging.c logging_smoke.c \
    -o "$EXECUTABLE"

echo "Running production logging demo..."

./"$EXECUTABLE"

if [ ! -f "$LOG_FILE" ]; then
    echo "ERROR: Log file was not created."
    rm -f "$EXECUTABLE"
    exit 1
fi

echo "Checking production log for DEBUG entries..."

if grep -q '| DEBUG |' "$LOG_FILE"; then
    echo "ERROR: Unexpected DEBUG entry in production log."
    cat "$LOG_FILE"
    rm -f "$EXECUTABLE"
    exit 1
else
    echo "Production log correctly filtered DEBUG entries."
fi

echo
echo "Production log:"
cat "$LOG_FILE"

rm -f "$EXECUTABLE"

echo
echo "Production logging demo completed successfully."
