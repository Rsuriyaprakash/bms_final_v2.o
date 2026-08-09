/**
 * @file file_management.h
 * @author Suriya Prakash R
 * @version 2.0.0
 * @brief Generic binary file persistence services.
 */

#ifndef BMS_FILE_MANAGEMENT_H
#define BMS_FILE_MANAGEMENT_H

#include <stddef.h>
#include <stdint.h>

#include "common.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef BmsStatus_t (*BmsFileRecordVisitor_t)(const void *record,
                                             uint32_t index,
                                             void *context);

BmsStatus_t FileManagementInitialize(void);
BmsStatus_t FileManagementEnsureDataDirectory(void);

BmsStatus_t FileManagementWriteRecords(const char *filePath,
                                       const void *records,
                                       uint32_t recordCount,
                                       size_t recordSize);

BmsStatus_t FileManagementReadRecords(const char *filePath,
                                      void *records,
                                      uint32_t capacity,
                                      uint32_t *recordCount,
                                      size_t recordSize);

BmsStatus_t FileManagementAppendRecord(const char *filePath,
                                       const void *record,
                                       size_t recordSize);

BmsStatus_t FileManagementVisitRecords(const char *filePath,
                                       size_t recordSize,
                                       BmsFileRecordVisitor_t visitor,
                                       void *context);

BmsStatus_t FileManagementGetRecordCount(const char *filePath,
                                         size_t recordSize,
                                         uint32_t *recordCount);

BmsStatus_t FileManagementDeleteFile(const char *filePath);
BmsStatus_t FileManagementFileExists(const char *filePath, bool *exists);

#ifdef __cplusplus
}
#endif

#endif /* BMS_FILE_MANAGEMENT_H */
