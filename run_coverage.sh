#!/bin/sh

set -eu

COVERAGE_DIR="analysis/coverage"
REPORT="coverage_summary.txt"
EXECUTABLE="cunit_coverage"

echo "======================================"
echo "       BMS GCOV CODE COVERAGE"
echo "======================================"

# Check required tools
if ! command -v gcc >/dev/null 2>&1; then
    echo "ERROR: gcc is not installed or not in PATH."
    exit 1
fi

if ! command -v gcov >/dev/null 2>&1; then
    echo "ERROR: gcov is not installed or not in PATH."
    exit 1
fi

# Clean old coverage data
rm -rf "$COVERAGE_DIR"
mkdir -p "$COVERAGE_DIR"

rm -f ./*.gcda
rm -f ./*.gcno
rm -f ./*.gcov
rm -f "$EXECUTABLE"

# Source files
SOURCES="
authentication.c
blood_inventory.c
blood_request_management.c
common_error_codes.c
common_validation.c
config.c
donation_camp_management.c
donation_management.c
donor_management.c
emergency_alert_management.c
file_management.c
file_names.c
hash_table.c
hospital_management.c
linked_list.c
notification_management.c
queue_management.c
report_management.c
logging.c
utility.c
"

echo ""
echo "Building with GCOV instrumentation..."

gcc -std=c11 -O0 -g \
    --coverage \
    -Wall -Wextra -Wpedantic \
    -pthread \
    test.c \
    $SOURCES \
    -lcunit \
    -o "$EXECUTABLE"

echo "Build successful."

echo ""
echo "Running CUnit tests..."

"./$EXECUTABLE" | tee "$COVERAGE_DIR/cunit_test_output.txt"

echo ""
echo "Generating GCOV report..."

: > "$REPORT"

echo "========================================" >> "$REPORT"
echo "       BMS CODE COVERAGE SUMMARY" >> "$REPORT"
echo "========================================" >> "$REPORT"
echo "" >> "$REPORT"

for SRC in $SOURCES
do
    if [ -f "$SRC" ]; then
        gcov -b -c "$SRC" >> "$REPORT" 2>&1 || true
    fi
done

# Move generated gcov files
for FILE in ./*.gcov
do
    if [ -f "$FILE" ]; then
        mv "$FILE" "$COVERAGE_DIR/"
    fi
done

echo "" >> "$REPORT"
echo "Coverage files: $COVERAGE_DIR" >> "$REPORT"

echo ""
echo "========================================"
echo "Coverage summary saved to:"
echo "$REPORT"
echo "========================================"

echo ""
echo "Generated GCOV files:"
ls -1 "$COVERAGE_DIR"/*.gcov 2>/dev/null || true

echo ""
echo "Coverage summary:"
grep -E "File '|Lines executed:|Branches executed:|Taken at least once:|Calls executed:" \
    "$REPORT" || true
