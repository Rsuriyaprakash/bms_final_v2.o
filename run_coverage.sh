#!/bin/sh
set -eu

# Start overall timer (nanoseconds)
SCRIPT_START=$(date +%s%N)

OUT_DIR="analysis/optimization"

SOURCES=$(find . -maxdepth 1 -name '*.c' \
    ! -name 'test_*.c' \
    ! -name 'test_runner.c')

rm -rf "$OUT_DIR"
mkdir -p "$OUT_DIR"

REPORT="$OUT_DIR/assembly_sizes.txt"

echo "Optimization Analysis Report" > "$REPORT"
echo "============================" >> "$REPORT"
echo "" >> "$REPORT"

for LEVEL in 1 2 3
do
    LEVEL_DIR="$OUT_DIR/O$LEVEL"
    mkdir -p "$LEVEL_DIR"

    TOTAL=0

    # Timer for this optimization level (nanoseconds)
    LEVEL_START=$(date +%s%N)

    for SRC in $SOURCES
    do
        BASE=$(basename "$SRC" .c)
        ASM="$LEVEL_DIR/$BASE.s"

        gcc -std=c11 -O$LEVEL -Wall -Wextra -Wpedantic \
            -pthread -S "$SRC" -o "$ASM"

        SIZE=$(wc -c < "$ASM")
        TOTAL=$((TOTAL + SIZE))
    done

    LEVEL_END=$(date +%s%N)
    LEVEL_TIME_MS=$(( (LEVEL_END - LEVEL_START) / 1000000 ))

    {
        echo "O$LEVEL Results"
        echo "---------------"
        echo "Total assembly bytes : $TOTAL"
        echo "Compilation time     : ${LEVEL_TIME_MS} ms"
        echo ""
    } | tee -a "$REPORT"
done

# End overall timer
SCRIPT_END=$(date +%s%N)
TOTAL_TIME_MS=$(( (SCRIPT_END - SCRIPT_START) / 1000000 ))

{
    echo "================================="
    echo "Total script execution time: ${TOTAL_TIME_MS} ms"
    echo "================================="
} | tee -a "$REPORT"

echo "Assembly files: $OUT_DIR/O1, O2, O3"
echo "Size report: $REPORT"
echo "Total execution time: ${TOTAL_TIME_MS} ms"
