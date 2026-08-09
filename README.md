# Blood Bank Management System v2

Complete C11 BMS source package with persistence, data structures, CUnit tests, multithreading, timestamping/logging, static-analysis scripts, optimization comparison, coverage support, and GDB review material.

## Build

```bash
sh build_bms_multithread.sh
./bmsv21
```

or:

```bash
make debug
```

Production build (DEBUG log lines filtered):

```bash
make release
./bmsv21_release
```

## CUnit

```bash
sh build_cunit.sh
./cunit_revised
```

## Analysis

```bash
sh run_cppcheck.sh
sh run_valgrind.sh
sh run_helgrind.sh
sh run_optimization_compare.sh
sh run_coverage.sh
```

See `docs/EVALUATION_CHECKLIST.md`.

## Timestamping/logging

Runtime logs are written to `logs/bms.log`.
The implementation supports timestamping, four levels, rotation, debug-vs-production filtering, and mutex-protected writes.

## Important

Tool availability differs by machine. The package includes scripts, but a tool should only be claimed as passed after it has actually been run on the evaluation machine.
