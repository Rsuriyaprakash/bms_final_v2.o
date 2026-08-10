/**
 * @file notification_management.h
 * @author Suriya Prakash R
 * @version 2.0.0
 * @brief Queued console, email, SMS, and broadcast notification APIs.
 */

#ifndef BMS_NOTIFICATION_MANAGEMENT_H
#define BMS_NOTIFICATION_MANAGEMENT_H

#include <stdbool.h>

#include "common.h"
#include "linked_list.h"
#include "queue_management.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef BmsStatus_t (*BmsNotificationSender_t)(
    const BmsNotification_t *notification,
    void *context);

typedef struct
{
    BmsLinkedList_t history;
    BmsQueue_t pendingQueue;
    bool initialized;
} BmsNotificationContext_t;

BmsStatus_t NotificationManagementInitialize(
    BmsNotificationContext_t *context);
BmsStatus_t NotificationManagementLoad(BmsNotificationContext_t *context);
BmsStatus_t NotificationManagementSave(
    const BmsNotificationContext_t *context);
BmsStatus_t NotificationManagementEnqueue(
    BmsNotificationContext_t *context,
    const BmsNotification_t *notification);
BmsStatus_t NotificationManagementPeek(
    const BmsNotificationContext_t *context,
    BmsNotification_t *notification);
BmsStatus_t NotificationManagementProcessNext(
    BmsNotificationContext_t *context,
    BmsNotificationSender_t sender,
    void *senderContext);
BmsStatus_t NotificationManagementProcessAll(
    BmsNotificationContext_t *context,
    BmsNotificationSender_t sender,
    void *senderContext,
    uint32_t *processedCount);
BmsStatus_t NotificationManagementSendConsole(
    const BmsNotification_t *notification,
    void *context);
BmsStatus_t NotificationManagementSendEmailPlaceholder(
    const BmsNotification_t *notification,
    void *context);
BmsStatus_t NotificationManagementSendSmsPlaceholder(
    const BmsNotification_t *notification,
    void *context);
void NotificationManagementDeinitialize(BmsNotificationContext_t *context);

#ifdef __cplusplus
}
#endif

#endif /* BMS_NOTIFICATION_MANAGEMENT_H */
