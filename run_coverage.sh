#!/usr/bin/env bash
set -euo pipefail

if ! command -v gcov >/dev/null 2>&1; then
    echo "gcov is not installed or not in PATH." >&2
    exit 127
fi

rm -f ./*.gcda ./*.gcno ./*.gcov cunit_revised

CFLAGS="--coverage -O0 -g" LDFLAGS="--coverage" sh build_cunit.sh
./cunit_revised | tee cunit_coverage_run.txt

gcov -b -c \
    authentication.c blood_inventory.c blood_request_management.c \
    common_validation.c donation_management.c donor_management.c \
    emergency_alert_management.c file_management.c graph_management.c \
    hash_table.c hospital_management.c linked_list.c logging.c \
    notification_management.c queue_management.c report_management.c \
    utility.c \
    > coverage_summary.txt

echo "Coverage summary saved to coverage_summary.txt"
