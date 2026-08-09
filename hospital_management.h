/**
 * @file hospital_management.h
 * @author Suriya Prakash R
 * @version 2.0.0
 * @brief Hospital record management APIs.
 */

#ifndef BMS_HOSPITAL_MANAGEMENT_H
#define BMS_HOSPITAL_MANAGEMENT_H

#include <stdbool.h>

#include "common.h"
#include "graph_management.h"
#include "hash_table.h"
#include "linked_list.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef BmsStatus_t (*BmsHospitalVisitor_t)(const BmsHospital_t *hospital,
                                           void *context);

typedef struct
{
    BmsLinkedList_t hospitals;
    BmsHashTable_t hospitalIdIndex;
    BmsHospitalGraph_t graph;
    bool initialized;
} BmsHospitalContext_t;

BmsStatus_t HospitalManagementInitialize(BmsHospitalContext_t *context);
BmsStatus_t HospitalManagementLoad(BmsHospitalContext_t *context);
BmsStatus_t HospitalManagementSave(const BmsHospitalContext_t *context);
BmsStatus_t HospitalManagementAdd(BmsHospitalContext_t *context,
                                  const BmsHospital_t *hospital);
BmsStatus_t HospitalManagementSearchById(const BmsHospitalContext_t *context,
                                         BmsHospitalId_t hospitalId,
                                         BmsHospital_t *hospital);
BmsStatus_t HospitalManagementUpdate(BmsHospitalContext_t *context,
                                     const BmsHospital_t *hospital);
BmsStatus_t HospitalManagementDelete(BmsHospitalContext_t *context,
                                     BmsHospitalId_t hospitalId);
BmsStatus_t HospitalManagementTraverse(const BmsHospitalContext_t *context,
                                       BmsHospitalVisitor_t visitor,
                                       void *visitorContext);
BmsStatus_t HospitalManagementAddRoute(BmsHospitalContext_t *context,
                                       BmsHospitalId_t sourceHospitalId,
                                       BmsHospitalId_t destinationHospitalId,
                                       uint32_t distanceKm);
BmsStatus_t HospitalManagementFindNearest(
    const BmsHospitalContext_t *context,
    BmsHospitalId_t sourceHospitalId,
    BmsHospital_t *nearestHospital,
    uint32_t *distanceKm);
BmsStatus_t HospitalManagementGetRouteDistance(
    const BmsHospitalContext_t *context,
    BmsHospitalId_t sourceHospitalId,
    BmsHospitalId_t destinationHospitalId,
    uint32_t *distanceKm);
void HospitalManagementDeinitialize(BmsHospitalContext_t *context);

#ifdef __cplusplus
}
#endif

#endif /* BMS_HOSPITAL_MANAGEMENT_H */
