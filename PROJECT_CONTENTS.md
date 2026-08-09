# Complete Project Contents

## Core application
All BMS source/header modules are in the project root, including authentication, donor,
inventory, request, donation, hospital, emergency alert, notification, persistence,
linked list, queue, hash table, graph, reports, validation, utility and multithreading.

## Timestamping and logging
- `logging.c`
- `logging.h`
- `docs/TIMESTAMPING_AND_LOGGING.md`
- `logs/LOG_SAMPLE.txt`
- `run_logging_demo.sh`
- `run_logging_production_demo.sh`

## Evaluation support
- Product backlog/user stories/sprint backlog
- POC review deck and presentation guide
- Valgrind Memcheck script
- O1/O2/O3 assembly-size comparison script and generated assembly evidence
- GCC warning/MISRA-oriented build configuration
- Cppcheck scripts, including optional MISRA addon runner
- CUnit test suite and logging tests
- High-level/low-level design and UML documentation
- Coverage script
- Helgrind script
- Makefile
- GDB fault-injection/debugging guide

See `docs/EVALUATION_CHECKLIST.md` for the phase-by-phase mapping.
