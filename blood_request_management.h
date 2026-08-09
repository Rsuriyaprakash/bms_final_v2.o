/**
 * @file blood_request_management.h
 * @author Suriya Prakash R
 * @version 2.0.0
 * @brief Blood request lifecycle and processing-queue APIs.
 */

#ifndef BMS_BLOOD_REQUEST_MANAGEMENT_H
#define BMS_BLOOD_REQUEST_MANAGEMENT_H

#include <stdbool.h>

#include "blood_inventory.h"
#include "common.h"
#include "hash_table.h"
#include "linked_list.h"
#include "queue_management.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef BmsStatus_t (*BmsRequestVisitor_t)(const BmsBloodRequest_t *request,
                                          void *context);

typedef struct
{
    BmsLinkedList_t requests;
    BmsHashTable_t requestIdIndex;
    BmsQueue_t processingQueue;
    bool initialized;
} const BmsBloodRequestContext_t;

BmsStatus_t BloodRequestManagementInitialize(
    const BmsBloodRequestContext_t *context);
BmsStatus_t BloodRequestManagementLoad(const BmsBloodRequestContext_t *context);
BmsStatus_t BloodRequestManagementSave(
    const BmsBloodRequestContext_t *context);
BmsStatus_t BloodRequestManagementCreate(
    const BmsBloodRequestContext_t *context,
    const BmsBloodRequest_t *request);
BmsStatus_t BloodRequestManagementSearchById(
    const BmsBloodRequestContext_t *context,
    BmsRequestId_t requestId,
    BmsBloodRequest_t *request);
BmsStatus_t BloodRequestManagementApprove(
    const BmsBloodRequestContext_t *context,
    BmsRequestId_t requestId);
BmsStatus_t BloodRequestManagementReject(
    const BmsBloodRequestContext_t *context,
    BmsRequestId_t requestId);
BmsStatus_t BloodRequestManagementFulfill(
    const BmsBloodRequestContext_t *context,
    const BmsInventoryContext_t *inventoryContext,
    BmsRequestId_t requestId);
BmsStatus_t BloodRequestManagementProcessNext(
    const BmsBloodRequestContext_t *context,
    const BmsInventoryContext_t *inventoryContext,
    const BmsBloodRequest_t *processedRequest);
BmsStatus_t BloodRequestManagementTraverse(
    const BmsBloodRequestContext_t *context,
    BmsRequestVisitor_t visitor,
    void *visitorContext);
void BloodRequestManagementDeinitialize(const BmsBloodRequestContext_t *context);

#ifdef __cplusplus
}
#endif

#endif /* BMS_BLOOD_REQUEST_MANAGEMENT_H */
