/**
 * @file logging.h
 * @brief Thread-safe timestamped logging for the Blood Bank Management System.
 */

#ifndef BMS_LOGGING_H
#define BMS_LOGGING_H

#include "common.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum
{
    BMS_LOG_LEVEL_DEBUG = 0,
    BMS_LOG_LEVEL_INFO,
    BMS_LOG_LEVEL_WARNING,
    BMS_LOG_LEVEL_ERROR,
    BMS_LOG_LEVEL_COUNT
} BmsLogLevel_t;

/**
 * @brief Initialize the logging subsystem and open the configured log file.
 */
BmsStatus_t LoggingInitialize(void);

/**
 * @brief Write a preformatted message to the BMS log.
 */
BmsStatus_t LoggingWrite(BmsLogLevel_t level,
                         const char *moduleName,
                         const char *functionName,
                         const char *message);

/**
 * @brief printf-style logging helper.
 *
 * Example:
 * LoggingWriteFormat(BMS_LOG_LEVEL_INFO,
 *                    "INVENTORY",
 *                    __func__,
 *                    "Added blood: group=%s units=%u",
 *                    BloodGroupToString(group),
 *                    (unsigned int)units);
 */
BmsStatus_t LoggingWriteFormat(BmsLogLevel_t level,
                               const char *moduleName,
                               const char *functionName,
                               const char *format,
                               ...);

/**
 * @brief Flush buffered log data to disk.
 */
BmsStatus_t LoggingFlush(void);

const char *LoggingLevelToString(BmsLogLevel_t level);

void LoggingDeinitialize(void);

/*
 * Convenience macros.
 *
 * These macros automatically record the current function name and make
 * instrumentation throughout the BMS much shorter and less error-prone.
 *
 * Example:
 * BMS_LOG_INFO("INVENTORY", "Added A+ inventory, units=%u", units);
 */
#define BMS_LOG_DEBUG(moduleName, ...) \
    ((void)LoggingWriteFormat(BMS_LOG_LEVEL_DEBUG, \
                              (moduleName), \
                              __func__, \
                              __VA_ARGS__))

#define BMS_LOG_INFO(moduleName, ...) \
    ((void)LoggingWriteFormat(BMS_LOG_LEVEL_INFO, \
                              (moduleName), \
                              __func__, \
                              __VA_ARGS__))

#define BMS_LOG_WARNING(moduleName, ...) \
    ((void)LoggingWriteFormat(BMS_LOG_LEVEL_WARNING, \
                              (moduleName), \
                              __func__, \
                              __VA_ARGS__))

#define BMS_LOG_ERROR(moduleName, ...) \
    ((void)LoggingWriteFormat(BMS_LOG_LEVEL_ERROR, \
                              (moduleName), \
                              __func__, \
                              __VA_ARGS__))

#ifdef __cplusplus
}
#endif

#endif /* BMS_LOGGING_H */
