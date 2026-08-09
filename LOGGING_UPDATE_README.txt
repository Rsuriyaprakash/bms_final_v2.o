BMS logging instrumentation update

Business-event logging added to:
- authentication.c: registration, login success/failure, password change
- blood_inventory.c: add/update/remove stock, insufficient stock, expiry, low stock
- blood_request_management.c: create, status changes, processing, fulfillment, insufficient inventory
- donation_management.c: donation recording and inventory creation failures
- hospital_management.c: add/update/delete hospital and add route
- notification_management.c: enqueue and processing success/failure
- emergency_alert_management.c: create, broadcast and resolve
- donation_camp_management.c: camp creation
- graph_management.c: graph hospital/route changes
- file_management.c: persistence read/write/open events
- report_management.c: report generation
- multithreading.c: monitor errors plus existing monitor-cycle logging
- queue_management.c: full/empty conditions
- linked_list.c: allocation failures
- hash_table.c: allocation failures
- main.c: existing application lifecycle logging preserved

Intentionally unchanged:
- common_validation.c: invalid user input is expected behavior and logging every rejection would flood logs.
- config.c: no runtime business action.
- file_names.c: constants only.

Also included:
- logging.c / logging.h with formatted BMS_LOG_DEBUG/INFO/WARNING/ERROR macros
- blood_inventory.h and multithreading.h needed by the updated low-stock monitor API

Security:
- Passwords, password hashes, and recovery secrets are never logged.

Usage:
1. Back up your current project.
2. Replace matching files with the files in this package.
3. Rebuild the complete BMS.
4. Run the BMS and inspect logs/bms.log.
