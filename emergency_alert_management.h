/**
 * @file emergency_alert_management.h
 * @author Suriya Prakash R
 * @version 2.0.0
 * @brief Emergency alert creation, graph broadcast, and resolution APIs.
 */

#ifndef BMS_EMERGENCY_ALERT_MANAGEMENT_H
#define BMS_EMERGENCY_ALERT_MANAGEMENT_H

#include <stdbool.h>

#include "common.h"
#include "graph_management.h"
#include "linked_list.h"
#include "notification_management.h"
#include "queue_management.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct
{
    BmsLinkedList_t alertHistory;
    BmsQueue_t alertQueue;
    bool initialized;
} BmsEmergencyAlertContext_t;

BmsStatus_t EmergencyAlertManagementInitialize(
    BmsEmergencyAlertContext_t *context);
BmsStatus_t EmergencyAlertManagementLoad(
    BmsEmergencyAlertContext_t *context);
BmsStatus_t EmergencyAlertManagementSave(
    const BmsEmergencyAlertContext_t *context);
BmsStatus_t EmergencyAlertManagementCreate(
    BmsEmergencyAlertContext_t *context,
    const BmsEmergencyAlert_t *alert);
BmsStatus_t EmergencyAlertManagementBroadcast(
    BmsEmergencyAlertContext_t *context,
    const BmsHospitalGraph_t *graph,
    BmsNotificationContext_t *notificationContext,
    BmsAlertId_t alertId,
    uint32_t maximumDistanceKm);
BmsStatus_t EmergencyAlertManagementProcessNext(
    BmsEmergencyAlertContext_t *context,
    BmsEmergencyAlert_t *alert);
BmsStatus_t EmergencyAlertManagementResolve(
    BmsEmergencyAlertContext_t *context,
    BmsAlertId_t alertId);
void EmergencyAlertManagementDeinitialize(
    BmsEmergencyAlertContext_t *context);

#ifdef __cplusplus
}
#endif

#endif /* BMS_EMERGENCY_ALERT_MANAGEMENT_H */
