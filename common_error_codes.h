/**
 * @file common_error_codes.h
 * @author Suriya Prakash R
 * @version 2.0.0
 * @brief Common status and error codes used by all BMS modules.
 *
 * This module defines a single status enumeration shared by every public API.
 */

#ifndef BMS_COMMON_ERROR_CODES_H
#define BMS_COMMON_ERROR_CODES_H

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Common return status for all BMS APIs.
 */
typedef enum
{
    BMS_STATUS_OK = 0,
    BMS_STATUS_INVALID_ARGUMENT,
    BMS_STATUS_INVALID_DATA,
    BMS_STATUS_NOT_INITIALIZED,
    BMS_STATUS_ALREADY_INITIALIZED,
    BMS_STATUS_NOT_FOUND,
    BMS_STATUS_ALREADY_EXISTS,
    BMS_STATUS_MEMORY_ERROR,
    BMS_STATUS_FILE_ERROR,
    BMS_STATUS_FILE_NOT_FOUND,
    BMS_STATUS_FILE_CORRUPT,
    BMS_STATUS_READ_ERROR,
    BMS_STATUS_WRITE_ERROR,
    BMS_STATUS_AUTHENTICATION_FAILED,
    BMS_STATUS_ACCESS_DENIED,
    BMS_STATUS_ACCOUNT_LOCKED,
    BMS_STATUS_PASSWORD_POLICY_FAILED,
    BMS_STATUS_INSUFFICIENT_STOCK,
    BMS_STATUS_EXPIRED,
    BMS_STATUS_QUEUE_FULL,
    BMS_STATUS_QUEUE_EMPTY,
    BMS_STATUS_HASH_TABLE_FULL,
    BMS_STATUS_GRAPH_FULL,
    BMS_STATUS_ROUTE_NOT_FOUND,
    BMS_STATUS_BUFFER_TOO_SMALL,
    BMS_STATUS_UNSUPPORTED,
    BMS_STATUS_INTERNAL_ERROR
} BmsStatus_t;

#ifdef __cplusplus
}
#endif

#endif /* BMS_COMMON_ERROR_CODES_H */
