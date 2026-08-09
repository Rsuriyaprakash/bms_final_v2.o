# New Applicable Tests Added

The following tests were added because they map directly to features and public APIs present in the current BMS POC:

1. Validation of blank/invalid names and addresses.
2. Email formats: valid, missing `@`, missing domain, and missing local part.
3. Phone validation: valid 10 digits, too short, too long, alphabetic, and special-character input.
4. Password minimum length and uppercase/lowercase/number/special-character policy.
5. Blood-group aliases including `A Rh Positive` and parsing of `AB-`.
6. Boundary tests for donor age 18–65 and minimum weight 45 kg.
7. Positive/non-zero blood-unit validation and leap-date validation.
8. Authentication duplicate username/user, valid/invalid login, role authorization, next-ID generation, password change, recovery reset, and disabled-account denial.
9. Donor duplicate ID, search, update, delete, blood-group filtering, sorting, and eligibility calculation.
10. Hospital duplicate ID, update, route distance, nearest-hospital lookup, and deletion.
11. Inventory duplicate ID, update/remove stock, insufficient stock, expiry removal, low-stock detection, and blood-group availability.
12. Blood-request duplicate prevention, approval, rejection, fulfillment, inventory deduction, and status verification.
13. Donation recording, lookup, and automatic inventory increase.
14. Notification enqueue, peek, single processing, and process-all behavior.
15. Emergency-alert create, dequeue/process, and resolve behavior.
16. Donation-camp add and traverse/view behavior.
17. Binary `.dat`-style persistence: write, read, append, count, visit, existence, delete, and invalid-argument handling.
18. Empty operational summary report generation.
19. Linked-list CRUD, traversal, sorting, count, and clear behavior.
20. Queue FIFO, full, empty, peek, and boundary behavior.
21. Hash-table numeric/string insert, duplicate, search, delete, and count behavior.
22. Hospital graph route, nearest hospital, BFS, DFS, route removal, and hospital removal.
23. Utility parsing, trimming, leap year, days in month, date comparison, checksum consistency, and secure-zero behavior.

Not added: web-only, upload/download, CAPTCHA/OTP, real email/SMS delivery, PDF/Excel/CSV export, QR/barcode, laboratory screening, appointment scheduling, dashboard/chart, browser session, and cloud/database test cases because these are not implemented by the current console POC.
