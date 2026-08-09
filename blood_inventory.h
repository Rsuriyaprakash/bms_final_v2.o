/**
 * @file blood_inventory.h
 * @author Suriya Prakash R
 * @version 2.0.0
 * @brief Blood stock, expiry, availability, and low-stock management APIs.
 */

#ifndef BMS_BLOOD_INVENTORY_H
#define BMS_BLOOD_INVENTORY_H

#include <stdbool.h>
#include <pthread.h>

#include "common.h"
#include "hash_table.h"
#include "linked_list.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef BmsStatus_t (*BmsInventoryVisitor_t)(
    const BmsBloodInventory_t *record,
    void *context);

typedef struct
{
    BmsLinkedList_t inventory;
    BmsHashTable_t bloodIdIndex;
    pthread_mutex_t mutex;
    bool mutexInitialized;
    bool initialized;
} BmsInventoryContext_t;

BmsStatus_t BloodInventoryInitialize(BmsInventoryContext_t *context);
BmsStatus_t BloodInventoryLoad(BmsInventoryContext_t *context);
BmsStatus_t BloodInventorySave(const BmsInventoryContext_t *context);
BmsStatus_t BloodInventoryAddStock(BmsInventoryContext_t *context,
                                   const BmsBloodInventory_t *record);
BmsStatus_t BloodInventoryUpdateStock(BmsInventoryContext_t *context,
                                      BmsBloodId_t bloodId,
                                      uint32_t units);
BmsStatus_t BloodInventoryRemoveStock(BmsInventoryContext_t *context,
                                      BmsBloodId_t bloodId,
                                      uint32_t units);
BmsStatus_t BloodInventorySearchById(const BmsInventoryContext_t *context,
                                     BmsBloodId_t bloodId,
                                     BmsBloodInventory_t *record);
BmsStatus_t BloodInventoryGetAvailableUnits(
    BmsInventoryContext_t *context,
    BmsBloodGroup_t bloodGroup,
    uint32_t *availableUnits);
BmsStatus_t BloodInventoryRemoveExpired(BmsInventoryContext_t *context,
                                        const BmsDate_t *currentDate,
                                        uint32_t *removedRecordCount);
BmsStatus_t BloodInventoryDetectLowStock(
    BmsInventoryContext_t *context,
    uint32_t thresholdUnits,
    BmsInventoryVisitor_t visitor,
    void *visitorContext);
BmsStatus_t BloodInventoryCountLowStock(
    BmsInventoryContext_t *context,
    uint32_t thresholdUnits,
    uint32_t *lowStockCount);
BmsStatus_t BloodInventoryTraverse(BmsInventoryContext_t *context,
                                   BmsInventoryVisitor_t visitor,
                                   void *visitorContext);
void BloodInventoryDeinitialize(BmsInventoryContext_t *context);

#ifdef __cplusplus
}
#endif

#endif /* BMS_BLOOD_INVENTORY_H */
