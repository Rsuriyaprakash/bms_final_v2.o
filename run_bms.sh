gcc -std=c11 -Wall -Wextra -Wpedantic -pthread \
$(find . -maxdepth 1 -name '*.c' \
! -name 'test_*.c' \
! -name 'test_runner.c') \
-o bmsv22
