#include "test_common.h"
#include "logging.h"

#include <stdio.h>
#include <string.h>

static void test_level_names(void)
{
    CU_ASSERT_STRING_EQUAL(LoggingLevelToString(BMS_LOG_LEVEL_DEBUG), "DEBUG");
    CU_ASSERT_STRING_EQUAL(LoggingLevelToString(BMS_LOG_LEVEL_INFO), "INFO");
    CU_ASSERT_STRING_EQUAL(LoggingLevelToString(BMS_LOG_LEVEL_WARNING), "WARNING");
    CU_ASSERT_STRING_EQUAL(LoggingLevelToString(BMS_LOG_LEVEL_ERROR), "ERROR");
}

static void test_initialize_write_flush(void)
{
    FILE *file;
    char line[512U];
    bool found = false;

    ASSERT_OK(LoggingInitialize());
    ASSERT_OK(LoggingWrite(BMS_LOG_LEVEL_INFO,
                           "TEST",
                           __func__,
                           "CUNIT_LOG_TEST"));
    ASSERT_OK(LoggingFlush());
    LoggingDeinitialize();

    file = fopen(BMS_LOG_FILE_PATH, "r");
    CU_ASSERT_PTR_NOT_NULL(file);
    if (file != NULL)
    {
        while (fgets(line, (int)sizeof(line), file) != NULL)
        {
            if (strstr(line, "CUNIT_LOG_TEST") != NULL)
            {
                found = true;
                break;
            }
        }
        (void)fclose(file);
    }
    CU_ASSERT_TRUE(found);
}

void RegisterLoggingTests(void)
{
    CU_pSuite suite = CU_add_suite("Timestamping and Logging", NULL, NULL);
    CU_add_test(suite, "level names", test_level_names);
    CU_add_test(suite, "initialize write flush", test_initialize_write_flush);
}
