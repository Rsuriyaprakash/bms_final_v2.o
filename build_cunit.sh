#!/bin/sh
set -eu

UTILITY_SOURCE="utility_updated.c"
if [ ! -f "$UTILITY_SOURCE" ]; then
    UTILITY_SOURCE="utility.c"
fi

gcc ${CFLAGS:-} -std=c11 -Wall -Wextra -Wpedantic \
    test_*.c \
    authentication.c blood_inventory.c blood_request_management.c \
    common_error_codes.c common_validation.c config.c donation_camp_management.c \
    donation_management.c donor_management.c emergency_alert_management.c \
    file_management.c file_names.c graph_management.c hash_table.c \
    hospital_management.c linked_list.c notification_management.c \
    queue_management.c report_management.c logging.c "$UTILITY_SOURCE" \
    -pthread ${LDFLAGS:-} -o cunit_revised -lcunit

echo "Built: ./cunit_revised"
