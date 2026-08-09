#!/bin/sh
set -eu

SOURCES=$(find . -maxdepth 1 -name '*.c' \
    ! -name 'main.c' \
    ! -name 'test_*.c' \
    ! -name 'test_runner.c')

gcc -std=c11 -Wall -Wextra -Wpedantic -Wconversion -Wshadow \
    -pthread -I. standalone_tests/test_requested_changes.c \
    $SOURCES -o requested_changes_test

./requested_changes_test
rm -f requested_changes_test
