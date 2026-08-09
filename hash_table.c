/** @file hash_table.c @brief Chained hash table implementation. */
#include "hash_table.h"
#include "logging.h"
#include <stdlib.h>
#include <string.h>

static uint32_t HashString(const char *key, uint32_t buckets)
{
    uint32_t hash = 5381U;
    while (*key != '\0') { hash = ((hash << 5U) + hash) ^ (uint8_t)*key; ++key; }
    return hash % buckets;
}
static BmsHashNode_t *NewNode(const BmsHashTable_t *table, const void *value)
{
    BmsHashNode_t *node = (BmsHashNode_t *)calloc(1U, sizeof(*node));
    if (node == NULL) { return NULL; }
    node->value = calloc(1U, table->valueSize);
    if (node->value == NULL) { free(node); return NULL; }
    (void)memcpy(node->value, value, table->valueSize); return node;
}
BmsStatus_t HashTableInitialize(BmsHashTable_t *table, uint32_t bucketCount,
                                size_t valueSize, BmsHashKeyType_t keyType)
{
    if ((table == NULL) || (bucketCount == 0U) || (valueSize == 0U))
    { return BMS_STATUS_INVALID_ARGUMENT; }
    (void)memset(table, 0, sizeof(*table));
    table->buckets = (BmsHashNode_t **)calloc(bucketCount, sizeof(BmsHashNode_t *));
    if (table->buckets == NULL) { BMS_LOG_ERROR("HASH", "Hash-table memory allocation failed");
        return BMS_STATUS_MEMORY_ERROR; }
    table->bucketCount = bucketCount; table->valueSize = valueSize; table->keyType = keyType;
    return BMS_STATUS_OK;
}
BmsStatus_t HashTableInsertUint32(BmsHashTable_t *table, uint32_t key, const void *value)
{
    uint32_t index; BmsHashNode_t *node;
    if ((table == NULL) || (value == NULL) || (table->buckets == NULL) ||
        (table->keyType != BMS_HASH_KEY_UINT32)) { return BMS_STATUS_INVALID_ARGUMENT; }
    if (HashTableSearchUint32(table, key, (void **)&node) == BMS_STATUS_OK)
    { return BMS_STATUS_ALREADY_EXISTS; }
    node = NewNode(table, value); if (node == NULL) { BMS_LOG_ERROR("HASH", "Hash-table memory allocation failed");
        return BMS_STATUS_MEMORY_ERROR; }
    index = key % table->bucketCount; node->numericKey = key;
    node->next = table->buckets[index]; table->buckets[index] = node; ++table->entryCount;
    return BMS_STATUS_OK;
}
BmsStatus_t HashTableSearchUint32(const BmsHashTable_t *table, uint32_t key, void **value)
{
    BmsHashNode_t *node;
    if ((table == NULL) || (value == NULL) || (table->buckets == NULL))
    { return BMS_STATUS_INVALID_ARGUMENT; }
    node = table->buckets[key % table->bucketCount];
    while (node != NULL) { if (node->numericKey == key) { *value = node->value; return BMS_STATUS_OK; } node = node->next; }
    *value = NULL; return BMS_STATUS_NOT_FOUND;
}
BmsStatus_t HashTableDeleteUint32(BmsHashTable_t *table, uint32_t key)
{
    uint32_t index; BmsHashNode_t *node, *prev = NULL;
    if ((table == NULL) || (table->buckets == NULL)) { return BMS_STATUS_INVALID_ARGUMENT; }
    index = key % table->bucketCount; node = table->buckets[index];
    while (node != NULL) { if (node->numericKey == key) { if (prev == NULL) table->buckets[index]=node->next; else prev->next=node->next; free(node->value); free(node); --table->entryCount; return BMS_STATUS_OK; } prev=node; node=node->next; }
    return BMS_STATUS_NOT_FOUND;
}
BmsStatus_t HashTableInsertString(BmsHashTable_t *table, const char *key, const void *value)
{
    uint32_t index; BmsHashNode_t *node; void *existing= NULL;
    if ((table == NULL) || (key == NULL) || (value == NULL) || (table->buckets == NULL) ||
        (table->keyType != BMS_HASH_KEY_STRING)) { return BMS_STATUS_INVALID_ARGUMENT; }
    if (HashTableSearchString(table, key, &existing) == BMS_STATUS_OK) { return BMS_STATUS_ALREADY_EXISTS; }
    node = NewNode(table, value); if (node == NULL){ BMS_LOG_ERROR("HASH", "Hash-table memory allocation failed");return BMS_STATUS_MEMORY_ERROR;}
    node->stringKey = (char *)malloc(strlen(key)+1U);
    if (node->stringKey == NULL) { free(node->value); free(node); BMS_LOG_ERROR("HASH", "Hash-table memory allocation failed");return BMS_STATUS_MEMORY_ERROR; }
    (void)strcpy(node->stringKey, key); index = HashString(key, table->bucketCount);
    node->next=table->buckets[index]; table->buckets[index]=node; ++table->entryCount;
    return BMS_STATUS_OK;
}
BmsStatus_t HashTableSearchString(const BmsHashTable_t *table, const char *key, void **value)
{
    BmsHashNode_t *node; uint32_t index;
    if ((table == NULL) || (key == NULL) || (value == NULL) || (table->buckets == NULL))
    { return BMS_STATUS_INVALID_ARGUMENT; }
    index=HashString(key,table->bucketCount); node=table->buckets[index];
    while(node!=NULL){ if((node->stringKey!=NULL)&&(strcmp(node->stringKey,key)==0)){*value=node->value;return BMS_STATUS_OK;} node=node->next;}
    *value=NULL; return BMS_STATUS_NOT_FOUND;
}
BmsStatus_t HashTableDeleteString(BmsHashTable_t *table, const char *key)
{
    uint32_t index; BmsHashNode_t *node,*prev=NULL;
    if((table==NULL)||(key==NULL)||(table->buckets==NULL)) return BMS_STATUS_INVALID_ARGUMENT;
    index=HashString(key,table->bucketCount); node=table->buckets[index];
    while(node!=NULL){if((node->stringKey!=NULL)&&(strcmp(node->stringKey,key)==0)){if(prev==NULL)table->buckets[index]=node->next;else prev->next=node->next;free(node->stringKey);free(node->value);free(node);--table->entryCount;return BMS_STATUS_OK;}prev=node;node=node->next;}
    return BMS_STATUS_NOT_FOUND;
}
uint32_t HashTableGetCount(const BmsHashTable_t *table){return(table==NULL)?0U:table->entryCount;}
void HashTableClear(BmsHashTable_t *table)
{
    uint32_t i; BmsHashNode_t *node,*next;
    if((table==NULL)||(table->buckets==NULL))return;
    for(i=0U;i<table->bucketCount;++i){node=table->buckets[i];while(node!=NULL){next=node->next;free(node->stringKey);free(node->value);free(node);node=next;}table->buckets[i]=NULL;}table->entryCount=0U;
}
void HashTableDeinitialize(BmsHashTable_t *table){if(table==NULL)return;HashTableClear(table);free(table->buckets);(void)memset(table,0,sizeof(*table));}
