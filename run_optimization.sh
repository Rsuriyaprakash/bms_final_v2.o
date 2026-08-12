#!/bin/sh

set -eu

OUT_DIR="analysis/optimization"
REPORT="$OUT_DIR/optimization_report.txt"

# Change this to the C file containing main()
MAIN_SOURCE="bms.c"

rm -rf "$OUT_DIR"
mkdir -p "$OUT_DIR"

# Get BMS source files, excluding CUnit tests
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

    EXECUTABLE="$LEVEL_DIR/bms_O$LEVEL"

    echo ""
    echo "================================="
    echo "Building O$LEVEL..."
    echo "================================="

    # -------------------------------
    # 1. Generate assembly files
    # -------------------------------
    TOTAL_ASM=0

    for SRC in $SOURCES
    do
        BASE=$(basename "$SRC" .c)
        ASM="$LEVEL_DIR/$BASE.s"

        gcc -std=c11 -O"$LEVEL" \
            -Wall -Wextra -Wpedantic \
            -pthread -S "$SRC" -o "$ASM"

        SIZE=$(wc -c < "$ASM")
        TOTAL_ASM=$((TOTAL_ASM + SIZE))
    done

    # -------------------------------
    # 2. Build executable
    # -------------------------------
    echo "Creating executable..."

    gcc -std=c11 -O"$LEVEL" \
        -Wall -Wextra -Wpedantic \
        -pthread $SOURCES \
        -o "$EXECUTABLE"

    # -------------------------------
    # 3. Measure executable size
    # -------------------------------
    EXEC_SIZE=$(wc -c < "$EXECUTABLE")

    # -------------------------------
    # 4. Measure execution time
    # -------------------------------
    TIME_FILE="$LEVEL_DIR/execution_time.txt"

    /usr/bin/time -f "%e" \
        "$EXECUTABLE" \
        > /dev/null 2> "$TIME_FILE"

    EXEC_TIME=$(cat "$TIME_FILE")

    # -------------------------------
    # 5. Display results
    # -------------------------------
    echo ""
    echo "O$LEVEL Results"
    echo "-----------------------------"
    echo "Assembly size      : $TOTAL_ASM bytes"
    echo "Executable size    : $EXEC_SIZE bytes"
    echo "Execution time     : $EXEC_TIME seconds"

    # -------------------------------
    # 6. Save results
    # -------------------------------
    {
        echo "O$LEVEL Results"
        echo "-----------------------------"
        echo "Total assembly size : $TOTAL_ASM bytes"
        echo "Executable size     : $EXEC_SIZE bytes"
        echo "Execution time      : $EXEC_TIME seconds"
        echo ""
    } >> "$REPORT"

done

echo "=============================" >> "$REPORT"
echo "Optimization comparison completed." >> "$REPORT"

echo ""
echo "================================="
echo "Optimization comparison complete"
echo "================================="
echo "Report: $REPORT"
echo "Assembly: $OUT_DIR/O1, O2, O3"
