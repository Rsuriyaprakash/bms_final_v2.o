/** @file blood_inventory.c @brief Thread-safe blood inventory implementation. */
#include "blood_inventory.h"
#include "logging.h"
#include "file_management.h"
#include "file_names.h"
#include "utility.h"

#include <stdlib.h>
#include <string.h>

static bool Match(const void *data, const void *key)
{
    return ((const BmsBloodInventory_t *)data)->bloodId ==
           *(const BmsBloodId_t *)key;
}

static void Lock(BmsInventoryContext_t *context)
{
    if ((context != NULL) && context->mutexInitialized)
    {
        (void)pthread_mutex_lock(&context->mutex);
    }
}

static void Unlock(BmsInventoryContext_t *context)
{
    if ((context != NULL) && context->mutexInitialized)
    {
        (void)pthread_mutex_unlock(&context->mutex);
    }
}

BmsStatus_t BloodInventoryInitialize(BmsInventoryContext_t *context)
{
    if (context == NULL)
    {
        return BMS_STATUS_INVALID_ARGUMENT;
    }
    (void)memset(context, 0, sizeof(*context));
    if (pthread_mutex_init(&context->mutex, NULL) != 0)
    {
        return BMS_STATUS_INTERNAL_ERROR;
    }
    context->mutexInitialized = true;
    LinkedListInitialize(&context->inventory, sizeof(BmsBloodInventory_t));
    HashTableInitialize(&context->bloodIdIndex, BMS_HASH_BUCKET_COUNT,
                        sizeof(BmsBloodInventory_t *), BMS_HASH_KEY_UINT32);
    context->initialized = true;
    return BMS_STATUS_OK;
}

BmsStatus_t BloodInventoryLoad(BmsInventoryContext_t *context)
{
    bool exists = false;
    uint32_t count = 0U;
    uint32_t index = 0U;
    BmsBloodInventory_t *records = NULL;
    BmsStatus_t status;

    if ((context == NULL) || (!context->initialized))
    {
        return BMS_STATUS_NOT_INITIALIZED;
    }
    (void)FileManagementFileExists(BMS_INVENTORY_FILE, &exists);
    if (!exists)
    {
        return BMS_STATUS_OK;
    }
    status = FileManagementGetRecordCount(BMS_INVENTORY_FILE,
                                          sizeof(BmsBloodInventory_t), &count);
    if ((status != BMS_STATUS_OK) || (count == 0U))
    {
        return status;
    }
    records = (BmsBloodInventory_t *)calloc(count, sizeof(*records));
    if (records == NULL)
    {
        return BMS_STATUS_MEMORY_ERROR;
    }
    status = FileManagementReadRecords(BMS_INVENTORY_FILE, records, count,
                                       &count, sizeof(*records));
    for (index = 0U; (status == BMS_STATUS_OK) && (index < count); ++index)
    {
        status = BloodInventoryAddStock(context, &records[index]);
    }
    free(records);
    return status;
}

BmsStatus_t BloodInventorySave(const BmsInventoryContext_t *context)
{
    BmsInventoryContext_t *mutableContext = (BmsInventoryContext_t *)context;
    BmsBloodInventory_t *records = NULL;
    const BmsLinkedListNode_t *node = NULL;
    uint32_t index = 0U;
    BmsStatus_t status;

    if ((context == NULL) || (!context->initialized))
    {
        return BMS_STATUS_NOT_INITIALIZED;
    }

    Lock(mutableContext);
    if (context->inventory.count == 0U)
    {
        status = FileManagementWriteRecords(BMS_INVENTORY_FILE, NULL, 0U,
                                             sizeof(BmsBloodInventory_t));
        Unlock(mutableContext);
        return status;
    }

    records = (BmsBloodInventory_t *)calloc(context->inventory.count,
                                             sizeof(*records));
    if (records == NULL)
    {
        Unlock(mutableContext);
        return BMS_STATUS_MEMORY_ERROR;
    }

    for (node = context->inventory.head; node != NULL; node = node->next)
    {
        if (node->data == NULL)
        {
            free(records);
            Unlock(mutableContext);
            return BMS_STATUS_INVALID_DATA;
        }
        records[index++] = *(const BmsBloodInventory_t *)node->data;
    }
    status = FileManagementWriteRecords(BMS_INVENTORY_FILE, records, index,
                                         sizeof(*records));
    free(records);
    Unlock(mutableContext);
    return status;
}

BmsStatus_t BloodInventoryAddStock(BmsInventoryContext_t *context,
                                   const BmsBloodInventory_t *record)
{
    void *found = NULL;
    BmsStatus_t status;
    if ((context == NULL) || (record == NULL))
    {
        return BMS_STATUS_INVALID_ARGUMENT;
    }
    Lock(context);
    if (LinkedListFind(&context->inventory, Match, &record->bloodId, &found) ==
        BMS_STATUS_OK)
    {
        status = BMS_STATUS_ALREADY_EXISTS;
    }
    else
    {
        status = LinkedListInsertBack(&context->inventory, record);
    }
    Unlock(context);

    if (status == BMS_STATUS_OK)
    {
        BMS_LOG_INFO("INVENTORY",
                     "Stock added: bloodId=%u group=%s units=%u",
                     (unsigned int)record->bloodId,
                     UtilityBloodGroupToString(record->bloodGroup),
                     (unsigned int)record->units);
    }
    else
    {
        BMS_LOG_WARNING("INVENTORY",
                        "Stock add failed: bloodId=%u status=%u",
                        (unsigned int)record->bloodId,
                        (unsigned int)status);
    }

    return status;
}

BmsStatus_t BloodInventoryUpdateStock(BmsInventoryContext_t *context,
                                      BmsBloodId_t bloodId, uint32_t units)
{
    void *found = NULL;
    BmsStatus_t status;
    if (context == NULL)
    {
        return BMS_STATUS_INVALID_ARGUMENT;
    }
    Lock(context);
    status = LinkedListFind(&context->inventory, Match, &bloodId, &found);
    if (status == BMS_STATUS_OK)
    {
        ((BmsBloodInventory_t *)found)->units = units;
        ((BmsBloodInventory_t *)found)->isAvailable = (units > 0U);
    }
    Unlock(context);

    if (status == BMS_STATUS_OK)
    {
        BMS_LOG_INFO("INVENTORY",
                     "Stock updated: bloodId=%u units=%u",
                     (unsigned int)bloodId,
                     (unsigned int)units);
    }
    else
    {
        BMS_LOG_WARNING("INVENTORY",
                        "Stock update failed: bloodId=%u status=%u",
                        (unsigned int)bloodId,
                        (unsigned int)status);
    }

    return status;
}

BmsStatus_t BloodInventoryRemoveStock(BmsInventoryContext_t *context,
                                      BmsBloodId_t bloodId, uint32_t units)
{
    void *found = NULL;
    BmsStatus_t status;
    if (context == NULL)
    {
        return BMS_STATUS_INVALID_ARGUMENT;
    }
    Lock(context);
    status = LinkedListFind(&context->inventory, Match, &bloodId, &found);
    if (status == BMS_STATUS_OK)
    {
        BmsBloodInventory_t *record = (BmsBloodInventory_t *)found;
        if (record->units < units)
        {
            status = BMS_STATUS_INSUFFICIENT_STOCK;
        }
        else
        {
            record->units -= units;
            record->isAvailable = (record->units > 0U);
        }
    }
    Unlock(context);

    if (status == BMS_STATUS_OK)
    {
        BMS_LOG_INFO("INVENTORY",
                     "Stock consumed: bloodId=%u requestedUnits=%u",
                     (unsigned int)bloodId,
                     (unsigned int)units);
    }
    else if (status == BMS_STATUS_INSUFFICIENT_STOCK)
    {
        BMS_LOG_WARNING("INVENTORY",
                        "Insufficient stock: bloodId=%u requestedUnits=%u",
                        (unsigned int)bloodId,
                        (unsigned int)units);
    }
    else
    {
        BMS_LOG_WARNING("INVENTORY",
                        "Stock removal failed: bloodId=%u status=%u",
                        (unsigned int)bloodId,
                        (unsigned int)status);
    }

    return status;
}

BmsStatus_t BloodInventorySearchById(const BmsInventoryContext_t *context,
                                     BmsBloodId_t bloodId,
                                     BmsBloodInventory_t *record)
{
    BmsInventoryContext_t *mutableContext = (BmsInventoryContext_t *)context;
    void *found = NULL;
    BmsStatus_t status;
    if ((context == NULL) || (record == NULL))
    {
        return BMS_STATUS_INVALID_ARGUMENT;
    }
    Lock(mutableContext);
    status = LinkedListFind(&context->inventory, Match, &bloodId, &found);
    if (status == BMS_STATUS_OK)
    {
        *record = *(const BmsBloodInventory_t *)found;
    }
    Unlock(mutableContext);
    return status;
}

BmsStatus_t BloodInventoryGetAvailableUnits(BmsInventoryContext_t *context,
                                             BmsBloodGroup_t bloodGroup,
                                             uint32_t *availableUnits)
{
    const BmsLinkedListNode_t *node;
    uint32_t total = 0U;
    if ((context == NULL) || (availableUnits == NULL))
    {
        return BMS_STATUS_INVALID_ARGUMENT;
    }
    Lock(context);
    for (node = context->inventory.head; node != NULL; node = node->next)
    {
        const BmsBloodInventory_t *record =
            (const BmsBloodInventory_t *)node->data;
        if ((record->bloodGroup == bloodGroup) && record->isAvailable)
        {
            total += record->units;
        }
    }
    *availableUnits = total;
    Unlock(context);
    return BMS_STATUS_OK;
}

BmsStatus_t BloodInventoryRemoveExpired(BmsInventoryContext_t *context,
                                        const BmsDate_t *currentDate,
                                        uint32_t *removedRecordCount)
{
    const BmsLinkedListNode_t *node;
    if ((context == NULL) || (currentDate == NULL) ||
        (removedRecordCount == NULL))
    {
        return BMS_STATUS_INVALID_ARGUMENT;
    }
    *removedRecordCount = 0U;
    Lock(context);
    for (node = context->inventory.head; node != NULL; node = node->next)
    {
        BmsBloodInventory_t *record = (BmsBloodInventory_t *)node->data;
        if (UtilityCompareDates(&record->expiryDate, currentDate) < 0)
        {
            BMS_LOG_WARNING("INVENTORY",
                            "Expired stock removed: bloodId=%u group=%s",
                            (unsigned int)record->bloodId,
                            UtilityBloodGroupToString(record->bloodGroup));
            record->units = 0U;
            record->isAvailable = false;
            ++(*removedRecordCount);
        }
    }
    Unlock(context);
    return BMS_STATUS_OK;
}

BmsStatus_t BloodInventoryDetectLowStock(BmsInventoryContext_t *context,
                                         uint32_t thresholdUnits,
                                         BmsInventoryVisitor_t visitor,
                                         void *visitorContext)
{
    const BmsLinkedListNode_t *node;
    BmsStatus_t status = BMS_STATUS_OK;
    if ((context == NULL) || (visitor == NULL))
    {
        return BMS_STATUS_INVALID_ARGUMENT;
    }
    Lock(context);
    for (node = context->inventory.head;
         (node != NULL) && (status == BMS_STATUS_OK); node = node->next)
    {
        const BmsBloodInventory_t *record =
            (const BmsBloodInventory_t *)node->data;
        if (record->isAvailable && (record->units <= thresholdUnits))
        {
            BMS_LOG_WARNING("INVENTORY",
                            "Low stock: bloodId=%u group=%s units=%u threshold=%u",
                            (unsigned int)record->bloodId,
                            UtilityBloodGroupToString(record->bloodGroup),
                            (unsigned int)record->units,
                            (unsigned int)thresholdUnits);
            status = visitor(record, visitorContext);
        }
    }
    Unlock(context);
    return status;
}


BmsStatus_t BloodInventoryCountLowStock(BmsInventoryContext_t *context,
                                        uint32_t thresholdUnits,
                                        uint32_t *lowStockCount)
{
    const BmsLinkedListNode_t *node;

    if ((context == NULL) || (lowStockCount == NULL))
    {
        return BMS_STATUS_INVALID_ARGUMENT;
    }

    *lowStockCount = 0U;

    Lock(context);
    for (node = context->inventory.head; node != NULL; node = node->next)
    {
        const BmsBloodInventory_t *record =
            (const BmsBloodInventory_t *)node->data;

        if ((record != NULL) &&
            record->isAvailable &&
            (record->units <= thresholdUnits))
        {
            ++(*lowStockCount);
        }
    }
    Unlock(context);

    return BMS_STATUS_OK;
}

BmsStatus_t BloodInventoryTraverse(BmsInventoryContext_t *context,
                                   BmsInventoryVisitor_t visitor,
                                   void *visitorContext)
{
    const BmsLinkedListNode_t *node;
    BmsStatus_t status = BMS_STATUS_OK;
    if ((context == NULL) || (visitor == NULL))
    {
        return BMS_STATUS_INVALID_ARGUMENT;
    }
    Lock(context);
    for (node = context->inventory.head;
         (node != NULL) && (status == BMS_STATUS_OK); node = node->next)
    {
        status = visitor((const BmsBloodInventory_t *)node->data,
                         visitorContext);
    }
    Unlock(context);
    return status;
}

void BloodInventoryDeinitialize(BmsInventoryContext_t *context)
{
    if (context == NULL)
    {
        return;
    }
    Lock(context);
    LinkedListClear(&context->inventory);
    HashTableDeinitialize(&context->bloodIdIndex);
    Unlock(context);
    if (context->mutexInitialized)
    {
        (void)pthread_mutex_destroy(&context->mutex);
    }
    (void)memset(context, 0, sizeof(*context));
}
