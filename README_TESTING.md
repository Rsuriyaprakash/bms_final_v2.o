# Revised CUnit Test Suite — Blood Bank Management System

This suite contains only tests mapped to APIs and features present in the current console-based C POC.

## Compile

All production `.c/.h` files and these tests should be in the same folder. Exclude the application `main.c` and compile exactly one utility implementation.

```bash
gcc -std=c11 -Wall -Wextra -Wpedantic \
  test_*.c \
  authentication.c blood_inventory.c blood_request_management.c \
  common_error_codes.c common_validation.c config.c donation_management.c \
  donor_management.c emergency_alert_management.c file_management.c \
  file_names.c graph_management.c hash_table.c hospital_management.c \
  linked_list.c notification_management.c queue_management.c \
  report_management.c utility_updated.c \
  -o cunit_revised -lcunit
```

If your active utility file is named `utility.c`, replace `utility_updated.c` with `utility.c`.

## Run and save report

```bash
./cunit_revised
./cunit_revised > cunit_revised_report.txt 2>&1
```

## Scope decisions

Included: validation, authentication, donor CRUD/eligibility, hospital CRUD/routes, inventory, donations, blood requests, notifications, emergency alerts, binary file persistence, reports, linked list, queue, hash table, graph, and utility functions.

Excluded because not implemented in the POC: browser forms, CAPTCHA, OTP, real SMS/email delivery, PDF/Excel/CSV export, file upload/download, MIME scanning, dashboards/charts, audit UI, cloud services, laboratory screening, appointment scheduling, medical certificates, QR/barcodes, and web-session behavior.
