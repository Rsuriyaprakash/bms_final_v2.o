#!/bin/sh

set -eu

OUT_DIR="analysis/optimization"
REPORT="$OUT_DIR/optimization_report.txt"

rm -rf "$OUT_DIR"
mkdir -p "$OUT_DIR"

# Production source files only.
# Exclude all CUnit test files and the test runner.
SOURCES=$(find . -maxdepth 1 -name '*.c' \
    ! -name 'test_*.c' \
    ! -name 'test_runner.c')

echo "BMS Optimization Comparison" > "$REPORT"
echo "===========================" >> "$REPORT"
echo "" >> "$REPORT"

for LEVEL in 1 2 3
do
    LEVEL_DIR="$OUT_DIR/O$LEVEL"
    mkdir -p "$LEVEL_DIR"

    echo "Building O$LEVEL..."

    TOTAL_ASM=0

    for SRC in $SOURCES
    do
        BASE=$(basename "$SRC" .c)
        ASM="$LEVEL_DIR/$BASE.s"

        gcc -std=c11 -O"$LEVEL" -Wall -Wextra -Wpedantic \
            -pthread -S "$SRC" -o "$ASM"

        SIZE=$(wc -c < "$ASM")
        TOTAL_ASM=$((TOTAL_ASM + SIZE))
    done

    echo "O$LEVEL total assembly bytes: $TOTAL_ASM"

    {
        echo "O$LEVEL"
        echo "---------------------------"
        echo "Total assembly bytes: $TOTAL_ASM"
        echo ""
    } >> "$REPORT"
done

echo "===========================" >> "$REPORT"
echo "Optimization comparison completed." >> "$REPORT"

echo ""
echo "Assembly files: $OUT_DIR/O1, $OUT_DIR/O2, $OUT_DIR/O3"
echo "Report: $REPORT"
