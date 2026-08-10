CC ?= gcc

TARGET          := bmsv22
RELEASE_TARGET  := bmsv22_release
TEST_TARGET     := cunit_revised

CPPFLAGS :=
CFLAGS_COMMON := -std=c11 -Wall -Wextra -Wpedantic -Wconversion -Wshadow -pthread
CFLAGS_DEBUG  := -O0 -g3
CFLAGS_RELEASE:= -O2 -DNDEBUG -DBMS_PRODUCTION_BUILD=1
LDLIBS_COMMON := -pthread
LDLIBS_TEST   := -lcunit -pthread

APP_SOURCES := \
	api_reference.c \
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
	logging.c \
	main.c \
	multithreading.c \
	notification_management.c \
	queue_management.c \
	report_management.c \
	utility.c

APP_OBJECTS := $(APP_SOURCES:.c=.o)

TEST_SOURCES := \
	test_runner.c \
	test_authentication.c \
	test_donation.c \
	test_donation_camp.c \
	test_donor.c \
	test_emergency_alert.c \
	test_file_management.c \
	test_hash_table.c \
	test_hospital.c \
	test_inventory.c \
	test_linked_list.c \
	test_logging.c \
	test_notification.c \
	test_queue.c \
	test_report.c \
	test_request.c \
	test_utility.c \
	test_validation.c

TEST_SUPPORT_SOURCES := $(filter-out main.c,$(APP_SOURCES))

.PHONY: all debug release test run cppcheck valgrind helgrind optimization coverage clean help

all: debug

help:
	@echo "BMS Makefile targets:"
	@echo "  make / make debug  - build debug executable: $(TARGET)"
	@echo "  make run           - build and run the BMS"
	@echo "  make release       - build optimized production executable"
	@echo "  make test          - build and run CUnit tests"
	@echo "  make cppcheck      - run Cppcheck script"
	@echo "  make valgrind      - run Valgrind Memcheck"
	@echo "  make helgrind      - run Helgrind"
	@echo "  make optimization  - run O1/O2/O3 assembly comparison"
	@echo "  make coverage      - run coverage script"
	@echo "  make clean         - remove generated files"

debug:
	$(CC) $(CPPFLAGS) $(CFLAGS_COMMON) $(CFLAGS_DEBUG) $(APP_SOURCES) \
		-o $(TARGET) $(LDLIBS_COMMON)
	@echo "Built: ./$(TARGET)"

run: debug
	./$(TARGET)

release:
	$(CC) $(CPPFLAGS) $(CFLAGS_COMMON) $(CFLAGS_RELEASE) $(APP_SOURCES) \
		-o $(RELEASE_TARGET) $(LDLIBS_COMMON)
	@echo "Built: ./$(RELEASE_TARGET)"

test:
	$(CC) $(CPPFLAGS) -std=c11 -Wall -Wextra -Wpedantic -g \
		$(TEST_SOURCES) $(TEST_SUPPORT_SOURCES) \
		-o $(TEST_TARGET) $(LDLIBS_TEST)
	./$(TEST_TARGET)

cppcheck:
	@if [ -f ./run_cppcheck.sh ]; then \
		sh ./run_cppcheck.sh; \
	else \
		cppcheck --enable=warning,style,performance,portability \
			--std=c11 --suppress=missingIncludeSystem \
			$(APP_SOURCES) 2>&1 | tee cppcheck_report.txt; \
	fi

valgrind: debug
	@if [ -f ./run_valgrind.sh ]; then \
		sh ./run_valgrind.sh; \
	else \
		valgrind --leak-check=full --show-leak-kinds=all ./$(TARGET); \
	fi

helgrind: debug
	@if [ -f ./run_helgrind.sh ]; then \
		sh ./run_helgrind.sh; \
	else \
		valgrind --tool=helgrind ./$(TARGET); \
	fi

optimization:
	@if [ -f ./run_optimization_compare.sh ]; then \
		sh ./run_optimization_compare.sh; \
	else \
		@echo "run_optimization_compare.sh not found"; \
		exit 1; \
	fi

coverage:
	@if [ -f ./run_coverage.sh ]; then \
		sh ./run_coverage.sh; \
	else \
		@echo "run_coverage.sh not found"; \
		exit 1; \
	fi

clean:
	rm -f $(TARGET) $(RELEASE_TARGET) $(TEST_TARGET)
	rm -f *.o *.gcda *.gcno *.gcov
	rm -f cppcheck_report.txt cppcheck_inconclusive_report.txt
	rm -f valgrind_report.txt valgrind_memcheck_report.txt helgrind_report.txt
	rm -f coverage_summary.txt
	rm -rf analysis/optimization
	@echo "Clean complete."
