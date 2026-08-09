#!/bin/sh
set -eu

OUT_DIR="analysis/optimization"
SOURCES=$(find . -maxdepth 1 -name '*.c' \
    ! -name 'test_*.c' \
    ! -name 'test_runner.c')

rm -rf "$OUT_DIR"
mkdir -p "$OUT_DIR"

REPORT="$OUT_DIR/assembly_sizes.txt"
: > "$REPORT"

for LEVEL in 1 2 3
do
    LEVEL_DIR="$OUT_DIR/O$LEVEL"
    mkdir -p "$LEVEL_DIR"
    TOTAL=0

    for SRC in $SOURCES
    do
        BASE=$(basename "$SRC" .c)
        ASM="$LEVEL_DIR/$BASE.s"
        gcc -std=c11 -O"$LEVEL" -Wall -Wextra -Wpedantic \
            -pthread -S "$SRC" -o "$ASM"
        SIZE=$(wc -c < "$ASM")
        TOTAL=$((TOTAL + SIZE))
    done

    echo "O$LEVEL total assembly bytes: $TOTAL" | tee -a "$REPORT"
done

echo "Assembly files: $OUT_DIR/O1, O2, O3"
echo "Size report: $REPORT"
