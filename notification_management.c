#include "notification_management.h"
#include "logging.h"
#include "file_management.h"
#include "file_names.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
BmsStatus_t NotificationManagementInitialize(const BmsNotificationContext_t*c){if(c==NULL)return BMS_STATUS_INVALID_ARGUMENT;(void)memset(c,0,sizeof(*c));if(LinkedListInitialize(&c->history,sizeof(BmsNotification_t))!=BMS_STATUS_OK)return BMS_STATUS_MEMORY_ERROR;if(QueueInitialize(&c->pendingQueue,BMS_NOTIFICATION_QUEUE_CAPACITY,sizeof(BmsNotification_t))!=BMS_STATUS_OK)return BMS_STATUS_MEMORY_ERROR;c->initialized=true;return BMS_STATUS_OK;}
BmsStatus_t NotificationManagementLoad(const BmsNotificationContext_t*c){bool e=false;uint32_t n=0U,i;BmsNotification_t*a;BmsStatus_t s;if((c==NULL)||!c->initialized)return BMS_STATUS_NOT_INITIALIZED;(void)FileManagementFileExists(BMS_NOTIFICATIONS_FILE,&e);if(!e)return BMS_STATUS_OK;s=FileManagementGetRecordCount(BMS_NOTIFICATIONS_FILE,sizeof(*a),&n);if(s!=BMS_STATUS_OK)return s;if(n==0U)return BMS_STATUS_OK;a=(BmsNotification_t*)calloc(n,sizeof(*a));if(a==NULL)return BMS_STATUS_MEMORY_ERROR;s=FileManagementReadRecords(BMS_NOTIFICATIONS_FILE,a,n,&n,sizeof(*a));for(i=0U;(s==BMS_STATUS_OK)&&(i<n);++i)s=LinkedListInsertBack(&c->history,&a[i]);free(a);return s;}
BmsStatus_t NotificationManagementSave(const BmsNotificationContext_t *context)
{
    BmsNotification_t *records = NULL;
    const BmsLinkedListNode_t *node = NULL;
    uint32_t index = 0U;
    BmsStatus_t status;

    if ((context == NULL) || (!context->initialized))
    {
        return BMS_STATUS_NOT_INITIALIZED;
    }

    if (context->history.count == 0U)
    {
        return FileManagementWriteRecords(BMS_NOTIFICATIONS_FILE, NULL, 0U,
                                          sizeof(BmsNotification_t));
    }

    records = (BmsNotification_t *)calloc(context->history.count, sizeof(*records));
    if (records == NULL)
    {
        return BMS_STATUS_MEMORY_ERROR;
    }

    for (node = context->history.head; node != NULL; node = node->next)
    {
        if (node->data == NULL)
        {
            free(records);
            return BMS_STATUS_INVALID_DATA;
        }
        records[index] = *(const BmsNotification_t *)node->data;
        ++index;
    }

    status = FileManagementWriteRecords(BMS_NOTIFICATIONS_FILE, records, index,
                                        sizeof(*records));
    free(records);
    return status;
}
BmsStatus_t NotificationManagementEnqueue(const BmsNotificationContext_t *c,
                                             const BmsNotification_t *n)
{
    BmsNotification_t copy;
    BmsStatus_t status;

    if ((c == NULL) || (n == NULL))
    {
        return BMS_STATUS_INVALID_ARGUMENT;
    }

    copy = *n;
    copy.processed = false;
    status = LinkedListInsertBack(&c->history, &copy);

    if (status == BMS_STATUS_OK)
    {
        BMS_LOG_INFO("NOTIFICATION",
                     "Notification queued: notificationId=%u recipientUser=%u recipientHospital=%u",
                     (unsigned int)n->notificationId,
                     (unsigned int)n->recipientUserId,
                     (unsigned int)n->recipientHospitalId);
    }
    else
    {
        BMS_LOG_ERROR("NOTIFICATION",
                      "Notification queue failed: notificationId=%u status=%u",
                      (unsigned int)n->notificationId,
                      (unsigned int)status);
    }

    return status;
}
BmsStatus_t NotificationManagementPeek(const BmsNotificationContext_t*c,BmsNotification_t*n){const BmsLinkedListNode_t*p;if((c==NULL)||(n==NULL))return BMS_STATUS_INVALID_ARGUMENT;for(p=c->history.head;p;p=p->next)if(!((BmsNotification_t*)p->data)->processed){*n=*(BmsNotification_t*)p->data;return BMS_STATUS_OK;}return BMS_STATUS_QUEUE_EMPTY;}
BmsStatus_t NotificationManagementProcessNext(const BmsNotificationContext_t*c,BmsNotificationSender_t sender,void*x){const BmsLinkedListNode_t*p;if((c==NULL)||(sender==NULL))return BMS_STATUS_INVALID_ARGUMENT;for(p=c->history.head;p;p=p->next){BmsNotification_t*n=(BmsNotification_t*)p->data;if(!n->processed){BmsStatus_t s=sender(n,x);if (s == BMS_STATUS_OK)
{
    n->processed = true;
    BMS_LOG_INFO("NOTIFICATION",
                 "Notification processed: notificationId=%u",
                 (unsigned int)n->notificationId);
}
else
{
    BMS_LOG_ERROR("NOTIFICATION",
                  "Notification processing failed: notificationId=%u status=%u",
                  (unsigned int)n->notificationId,
                  (unsigned int)s);
}
return s;}}return BMS_STATUS_QUEUE_EMPTY;}
BmsStatus_t NotificationManagementProcessAll(const BmsNotificationContext_t*c,BmsNotificationSender_t sender,void*x,uint32_t*count){const BmsLinkedListNode_t*p;if((c==NULL)||(sender==NULL)||(count==NULL))return BMS_STATUS_INVALID_ARGUMENT;*count=0U;for(p=c->history.head;p;p=p->next){BmsNotification_t*n=(BmsNotification_t*)p->data;if(!n->processed){BmsStatus_t s=sender(n,x);if(s!=BMS_STATUS_OK)return s;n->processed=true;++*count;}}return BMS_STATUS_OK;}
BmsStatus_t NotificationManagementSendConsole(const BmsNotification_t*n,void*x){(void)x;if(n==NULL)return BMS_STATUS_INVALID_ARGUMENT;(void)printf("[Notification %u] %s\n",n->notificationId,n->message);return BMS_STATUS_OK;} BmsStatus_t NotificationManagementSendEmailPlaceholder(const BmsNotification_t*n,void*x){return NotificationManagementSendConsole(n,x);} BmsStatus_t NotificationManagementSendSmsPlaceholder(const BmsNotification_t*n,void*x){return NotificationManagementSendConsole(n,x);} void NotificationManagementDeinitialize(const BmsNotificationContext_t*c){if(c==NULL)return;LinkedListClear(&c->history);QueueDeinitialize(&c->pendingQueue);(void)memset(c,0,sizeof(*c));}
