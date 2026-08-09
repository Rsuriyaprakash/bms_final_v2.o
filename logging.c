/**
 * @file logging.c
 * @brief Thread-safe timestamped logging with rotation and build-level filtering.
 */

#define _POSIX_C_SOURCE 200809L

#include "logging.h"
#include "config.h"

#include <errno.h>
#include <pthread.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

#ifndef BMS_LOG_DIRECTORY
#define BMS_LOG_DIRECTORY "logs"
#endif
#ifndef BMS_LOG_FILE_PATH
#define BMS_LOG_FILE_PATH "logs/bms.log"
#endif
#ifndef BMS_LOG_BACKUP_1_PATH
#define BMS_LOG_BACKUP_1_PATH "logs/bms.log.1"
#endif
#ifndef BMS_LOG_BACKUP_2_PATH
#define BMS_LOG_BACKUP_2_PATH "logs/bms.log.2"
#endif
#ifndef BMS_LOG_BACKUP_3_PATH
#define BMS_LOG_BACKUP_3_PATH "logs/bms.log.3"
#endif
#ifndef BMS_LOG_TIMESTAMP_FORMAT
#define BMS_LOG_TIMESTAMP_FORMAT "%Y-%m-%d %H:%M:%S"
#endif
#ifndef BMS_LOG_MAX_FILE_SIZE_BYTES
#define BMS_LOG_MAX_FILE_SIZE_BYTES (1048576UL)
#endif
#ifndef BMS_LOG_MIN_LEVEL_VALUE
#ifdef NDEBUG
#define BMS_LOG_MIN_LEVEL_VALUE ((uint32_t)BMS_LOG_LEVEL_INFO)
#else
#define BMS_LOG_MIN_LEVEL_VALUE ((uint32_t)BMS_LOG_LEVEL_DEBUG)
#endif
#endif


#if defined(_WIN32)
#include <direct.h>
#else
#include <sys/stat.h>
#include <sys/types.h>
#endif

#define BMS_LOG_TIMESTAMP_BUFFER_SIZE (32U)
#define BMS_LOG_MESSAGE_BUFFER_SIZE   (512U)

static FILE *g_logFile = NULL;
static bool g_loggingInitialized = false;
static pthread_mutex_t g_logMutex = PTHREAD_MUTEX_INITIALIZER;

static int CreateLogDirectory(void)
{
#if defined(_WIN32)
    return _mkdir(BMS_LOG_DIRECTORY);
#else
    return mkdir(BMS_LOG_DIRECTORY, (mode_t)0755);
#endif
}

static bool LogFileExists(const char *path)
{
    bool exists = false;

    if (path != NULL)
    {
        FILE *file = fopen(path, "rb");
        if (file != NULL)
        {
            exists = true;
            (void)fclose(file);
        }
    }

    return exists;
}

static void RotateLogFilesUnlocked(void)
{
    (void)remove(BMS_LOG_BACKUP_3_PATH);

    if (LogFileExists(BMS_LOG_BACKUP_2_PATH))
    {
        (void)rename(BMS_LOG_BACKUP_2_PATH, BMS_LOG_BACKUP_3_PATH);
    }

    if (LogFileExists(BMS_LOG_BACKUP_1_PATH))
    {
        (void)rename(BMS_LOG_BACKUP_1_PATH, BMS_LOG_BACKUP_2_PATH);
    }

    if (LogFileExists(BMS_LOG_FILE_PATH))
    {
        (void)rename(BMS_LOG_FILE_PATH, BMS_LOG_BACKUP_1_PATH);
    }
}

static BmsStatus_t GetTimestamp(char *buffer, size_t bufferSize)
{
    time_t currentTime;
    struct tm timeInfo;
    size_t written;

    if ((buffer == NULL) || (bufferSize == 0U))
    {
        return BMS_STATUS_INVALID_ARGUMENT;
    }

    currentTime = time(NULL);
    if (currentTime == (time_t)-1)
    {
        return BMS_STATUS_INTERNAL_ERROR;
    }

    if (localtime_r(&currentTime, &timeInfo) == NULL)
    {
        return BMS_STATUS_INTERNAL_ERROR;
    }

    written = strftime(buffer,
                       bufferSize,
                       BMS_LOG_TIMESTAMP_FORMAT,
                       &timeInfo);
    if (written == 0U)
    {
        return BMS_STATUS_BUFFER_TOO_SMALL;
    }

    return BMS_STATUS_OK;
}

const char *LoggingLevelToString(BmsLogLevel_t level)
{
    const char *text;

    switch (level)
    {
        case BMS_LOG_LEVEL_DEBUG:
            text = "DEBUG";
            break;

        case BMS_LOG_LEVEL_INFO:
            text = "INFO";
            break;

        case BMS_LOG_LEVEL_WARNING:
            text = "WARNING";
            break;

        case BMS_LOG_LEVEL_ERROR:
            text = "ERROR";
            break;

        default:
            text = "UNKNOWN";
            break;
    }

    return text;
}

static BmsStatus_t GetLogFileSize(unsigned long *fileSize)
{
    struct stat fileInfo;

    if (fileSize == NULL)
    {
        return BMS_STATUS_INVALID_ARGUMENT;
    }

    *fileSize = 0UL;

    if (stat(BMS_LOG_FILE_PATH, &fileInfo) != 0)
    {
        if (errno == ENOENT)
        {
            return BMS_STATUS_OK;
        }

        return BMS_STATUS_FILE_ERROR;
    }

    *fileSize = (unsigned long)fileInfo.st_size;

    return BMS_STATUS_OK;
}

static BmsStatus_t OpenLogFileUnlocked(void)
{
    unsigned long fileSize = 0UL;
    BmsStatus_t status;

    status = GetLogFileSize(&fileSize);

    if (status != BMS_STATUS_OK)
    {
        return status;
    }

    if (fileSize >= BMS_LOG_MAX_FILE_SIZE_BYTES)
    {
        RotateLogFilesUnlocked();
    }

    g_logFile = fopen(BMS_LOG_FILE_PATH, "a");

    if (g_logFile == NULL)
    {
        return BMS_STATUS_FILE_ERROR;
    }

    return BMS_STATUS_OK;
}


static BmsStatus_t EnsureRotationUnlocked(void)
{
    unsigned long fileSize = 0UL;
    BmsStatus_t status;

    if (g_logFile == NULL)
    {
        return BMS_STATUS_NOT_INITIALIZED;
    }

    if (fflush(g_logFile) != 0)
    {
        return BMS_STATUS_WRITE_ERROR;
    }

    status = GetLogFileSize(&fileSize);

    if (status != BMS_STATUS_OK)
    {
        return status;
    }

    if (fileSize >= BMS_LOG_MAX_FILE_SIZE_BYTES)
    {
        (void)fclose(g_logFile);
        g_logFile = NULL;

        RotateLogFilesUnlocked();

        g_logFile = fopen(BMS_LOG_FILE_PATH, "a");

        if (g_logFile == NULL)
        {
            return BMS_STATUS_FILE_ERROR;
        }
    }

    return BMS_STATUS_OK;
}

static BmsStatus_t WriteLogUnlocked(BmsLogLevel_t level,
                                    const char *moduleName,
                                    const char *functionName,
                                    const char *message)
{
    char timestamp[BMS_LOG_TIMESTAMP_BUFFER_SIZE];
    BmsStatus_t status;
    int writeResult;

    status = EnsureRotationUnlocked();
    if (status != BMS_STATUS_OK)
    {
        return status;
    }

    status = GetTimestamp(timestamp, sizeof(timestamp));
    if (status != BMS_STATUS_OK)
    {
        return status;
    }

    writeResult = fprintf(g_logFile,
                          "%s | %-7s | %-12s | %-32s | %s\n",
                          timestamp,
                          LoggingLevelToString(level),
                          moduleName,
                          functionName,
                          message);
    if (writeResult < 0)
    {
        return BMS_STATUS_WRITE_ERROR;
    }

    if (fflush(g_logFile) != 0)
    {
        return BMS_STATUS_WRITE_ERROR;
    }

    return BMS_STATUS_OK;
}

BmsStatus_t LoggingInitialize(void)
{
    BmsStatus_t status = BMS_STATUS_OK;

    (void)pthread_mutex_lock(&g_logMutex);

    if (!g_loggingInitialized)
    {
        int directoryResult = CreateLogDirectory();
        if ((directoryResult != 0) && (errno != EEXIST))
        {
            status = BMS_STATUS_FILE_ERROR;
        }

        if (status == BMS_STATUS_OK)
        {
            status = OpenLogFileUnlocked();
        }

        if (status == BMS_STATUS_OK)
        {
            g_loggingInitialized = true;

            (void)WriteLogUnlocked(BMS_LOG_LEVEL_INFO,
                                   "SYSTEM",
                                   "LoggingInitialize",
                                   "Logging subsystem initialized");
        }
    }

    (void)pthread_mutex_unlock(&g_logMutex);

    return status;
}

BmsStatus_t LoggingWrite(BmsLogLevel_t level,
                         const char *moduleName,
                         const char *functionName,
                         const char *message)
{
    BmsStatus_t status;

    if ((moduleName == NULL) ||
        (functionName == NULL) ||
        (message == NULL) ||
        (level < BMS_LOG_LEVEL_DEBUG) ||
        (level >= BMS_LOG_LEVEL_COUNT))
    {
        return BMS_STATUS_INVALID_ARGUMENT;
    }

    /*
     * Build-level filtering:
     * development can include DEBUG, while production can configure
     * BMS_LOG_MIN_LEVEL_VALUE to INFO/WARNING/ERROR.
     */

    if (level<(BmsLogLevel_t)BMS_LOG_MIN_LEVEL_VALUE){
	        return BMS_STATUS_OK;
    }

    (void)pthread_mutex_lock(&g_logMutex);

    if (!g_loggingInitialized)
    {
        status = BMS_STATUS_NOT_INITIALIZED;
    }
    else
    {
        status = WriteLogUnlocked(level,
                                  moduleName,
                                  functionName,
                                  message);
    }

    (void)pthread_mutex_unlock(&g_logMutex);

    return status;
}

BmsStatus_t LoggingWriteFormat(BmsLogLevel_t level,
                               const char *moduleName,
                               const char *functionName,
                               const char *format,
                               ...)
{
    char message[BMS_LOG_MESSAGE_BUFFER_SIZE];
    va_list args;
    int result;

    if ((moduleName == NULL) ||
        (functionName == NULL) ||
        (format == NULL))
    {
        return BMS_STATUS_INVALID_ARGUMENT;
    }

    va_start(args, format);
    result = vsnprintf(message, sizeof(message), format, args);
    va_end(args);

    if (result < 0)
    {
        return BMS_STATUS_INTERNAL_ERROR;
    }

    /*
     * vsnprintf always terminates the buffer when its size is non-zero.
     * If the message is longer than the buffer, log the truncated message
     * instead of failing the actual BMS operation.
     */
    return LoggingWrite(level,
                        moduleName,
                        functionName,
                        message);
}

BmsStatus_t LoggingFlush(void)
{
    BmsStatus_t status = BMS_STATUS_OK;

    (void)pthread_mutex_lock(&g_logMutex);

    if (!g_loggingInitialized || (g_logFile == NULL))
    {
        status = BMS_STATUS_NOT_INITIALIZED;
    }
    else if (fflush(g_logFile) != 0)
    {
        status = BMS_STATUS_WRITE_ERROR;
    }
    else
    {
        /* No action required. */
    }

    (void)pthread_mutex_unlock(&g_logMutex);

    return status;
}

void LoggingDeinitialize(void)
{
    (void)pthread_mutex_lock(&g_logMutex);

    if (g_loggingInitialized)
    {
        (void)WriteLogUnlocked(BMS_LOG_LEVEL_INFO,
                               "SYSTEM",
                               "LoggingDeinitialize",
                               "Logging subsystem stopped");

        if (g_logFile != NULL)
        {
            (void)fclose(g_logFile);
            g_logFile = NULL;
        }

        g_loggingInitialized = false;
    }

    (void)pthread_mutex_unlock(&g_logMutex);
}
