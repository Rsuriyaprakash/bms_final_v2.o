#!/bin/sh

set -eu

OUT_DIR="analysis/optimization"
REPORT="$OUT_DIR/optimization_report.txt"

rm -rf "$OUT_DIR"
mkdir -p "$OUT_DIR"

SOURCES=$(find . -maxdepth 1 -name '*.c' \
    ! -name 'test_*.c' \
    ! -name 'test_runner.c' \
    ! -name 'main.c')

echo "BMS Optimization Comparison" > "$REPORT"
echo "===========================" >> "$REPORT"
echo "" >> "$REPORT"

for LEVEL in 1 2 3
do
    LEVEL_DIR="$OUT_DIR/O$LEVEL"
    mkdir -p "$LEVEL_DIR"

    echo "Building O$LEVEL..."

    # Generate assembly files and calculate total assembly size
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

    # Build the complete CUnit test executable
    gcc -std=c11 -O"$LEVEL" -Wall -Wextra -Wpedantic \
        -pthread \
        test.c \
        authentication.c \
        blood_inventory.c \
        blood_request_management.c \
        common_error_codes.c \
        common_validation.c \
        config.c \
        donation_camp_management.c \
        donation_management.c \
        donor_management.c \
        emergency_alert_management.c \
        file_management.c \
        file_names.c \
        hash_table.c \
        hospital_management.c \
        linked_list.c \
        notification_management.c \
        queue_management.c \
        report_management.c \
        logging.c \
        utility.c \
        -lcunit \
        -o "$LEVEL_DIR/cunit_O$LEVEL"

    # Execute the SAME CUnit workload for every optimization level
    TIME_FILE="$LEVEL_DIR/execution_time.txt"

    /usr/bin/time -f "%e" \
        "$LEVEL_DIR/cunit_O$LEVEL" \
        > "$LEVEL_DIR/test_output.txt" 2>&1 \
        2> "$TIME_FILE"

    EXEC_TIME=$(cat "$TIME_FILE")

    {
        echo "O$LEVEL Results"
        echo "---------------"
        echo "Total assembly bytes : $TOTAL_ASM"
        echo "Execution time       : ${EXEC_TIME} seconds"
        echo ""
    } | tee -a "$REPORT"

done

echo "==================================" | tee -a "$REPORT"
echo "Optimization comparison complete" | tee -a "$REPORT"
echo "==================================" | tee -a "$REPORT"

echo ""
echo "Report: $REPORT"
echo "Assembly files: $OUT_DIR/O1, O2, O3"
