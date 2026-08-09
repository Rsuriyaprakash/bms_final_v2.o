#ifndef BMS_TEST_COMMON_H
#define BMS_TEST_COMMON_H
#include <CUnit/Basic.h>
#include <CUnit/CUnit.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "common.h"
#define ASSERT_STATUS(expected, expression) CU_ASSERT_EQUAL((expected), (expression))
#define ASSERT_OK(expression) ASSERT_STATUS(BMS_STATUS_OK, (expression))
#endif
