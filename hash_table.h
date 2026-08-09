/**
 * @file hash_table.h
 * @author Suriya Prakash R
 * @version 2.0.0
 * @brief Generic chained hash table for fast ID and username lookup.
 */

#ifndef BMS_HASH_TABLE_H
#define BMS_HASH_TABLE_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "common_error_codes.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum
{
    BMS_HASH_KEY_UINT32 = 0,
    BMS_HASH_KEY_STRING
} BmsHashKeyType_t;

typedef struct BmsHashNode
{
    uint32_t numericKey;
    char *stringKey;
    void *value;
    struct BmsHashNode *next;
} BmsHashNode_t;

typedef struct
{
    BmsHashNode_t **buckets;
    uint32_t bucketCount;
    uint32_t entryCount;
    size_t valueSize;
    BmsHashKeyType_t keyType;
} BmsHashTable_t;

BmsStatus_t HashTableInitialize(BmsHashTable_t *table,
                                uint32_t bucketCount,
                                size_t valueSize,
                                BmsHashKeyType_t keyType);

BmsStatus_t HashTableInsertUint32(BmsHashTable_t *table,
                                  uint32_t key,
                                  const void *value);
BmsStatus_t HashTableSearchUint32(const BmsHashTable_t *table,
                                  uint32_t key,
                                  void **value);
BmsStatus_t HashTableDeleteUint32(BmsHashTable_t *table, uint32_t key);

BmsStatus_t HashTableInsertString(BmsHashTable_t *table,
                                  const char *key,
                                  const void *value);
BmsStatus_t HashTableSearchString(const BmsHashTable_t *table,
                                  const char *key,
                                  void **value);
BmsStatus_t HashTableDeleteString(BmsHashTable_t *table, const char *key);

uint32_t HashTableGetCount(const BmsHashTable_t *table);
void HashTableClear(BmsHashTable_t *table);
void HashTableDeinitialize(BmsHashTable_t *table);

#ifdef __cplusplus
}
#endif

#endif /* BMS_HASH_TABLE_H */
