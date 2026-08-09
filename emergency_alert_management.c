/** @file emergency_alert_management.c @brief Emergency alert implementation. */
#include "common_validation.h"
#include "logging.h"
#include "emergency_alert_management.h"
#include "utility.h"
#include "file_management.h"
#include "file_names.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
static bool Match(const void*d,const void*k){return((const BmsEmergencyAlert_t*)d)->alertId==*(const BmsAlertId_t*)k;}
typedef struct{BmsNotificationContext_t*n;const BmsEmergencyAlert_t*a;}BroadcastCtx;
static BmsStatus_t Notify(const BmsHospital_t*h,void*x){BroadcastCtx*c=(BroadcastCtx*)x;BmsNotification_t n;(void)memset(&n,0,sizeof(n));n.notificationId=c->a->alertId+h->hospitalId;n.recipientHospitalId=h->hospitalId;n.channel=BMS_NOTIFICATION_CHANNEL_BROADCAST;n.priority=BMS_PRIORITY_EMERGENCY;(void)snprintf(n.message,sizeof(n.message),"Emergency need: %u units of %s",c->a->requiredUnits,UtilityBloodGroupToString(c->a->bloodGroup));return NotificationManagementEnqueue(c->n,&n);}
BmsStatus_t EmergencyAlertManagementInitialize(BmsEmergencyAlertContext_t*c){if(c==NULL)return BMS_STATUS_INVALID_ARGUMENT;(void)memset(c,0,sizeof(*c));LinkedListInitialize(&c->alertHistory,sizeof(BmsEmergencyAlert_t));QueueInitialize(&c->alertQueue,BMS_ALERT_QUEUE_CAPACITY,sizeof(BmsEmergencyAlert_t));c->initialized=true;return BMS_STATUS_OK;}
BmsStatus_t EmergencyAlertManagementLoad(BmsEmergencyAlertContext_t*c){bool e=false;uint32_t n=0U,i;BmsEmergencyAlert_t*a;BmsStatus_t s;if((c==NULL)||!c->initialized)return BMS_STATUS_NOT_INITIALIZED;(void)FileManagementFileExists(BMS_EMERGENCY_ALERTS_FILE,&e);if(!e)return BMS_STATUS_OK;s=FileManagementGetRecordCount(BMS_EMERGENCY_ALERTS_FILE,sizeof(*a),&n);if(s!=BMS_STATUS_OK)return s;if(n==0U)return BMS_STATUS_OK;a=(BmsEmergencyAlert_t*)calloc(n,sizeof(*a));if(a==NULL)return BMS_STATUS_MEMORY_ERROR;s=FileManagementReadRecords(BMS_EMERGENCY_ALERTS_FILE,a,n,&n,sizeof(*a));for(i=0U;(s==BMS_STATUS_OK)&&(i<n);++i)s=LinkedListInsertBack(&c->alertHistory,&a[i]);free(a);return s;}
BmsStatus_t EmergencyAlertManagementSave(const BmsEmergencyAlertContext_t *context)
{
    BmsEmergencyAlert_t *records = NULL;
    const BmsLinkedListNode_t *node = NULL;
    uint32_t index = 0U;
    BmsStatus_t status;

    if ((context == NULL) || (!context->initialized))
    {
        return BMS_STATUS_NOT_INITIALIZED;
    }

    if (context->alertHistory.count == 0U)
    {
        return FileManagementWriteRecords(BMS_EMERGENCY_ALERTS_FILE, NULL, 0U,
                                          sizeof(BmsEmergencyAlert_t));
    }

    records = (BmsEmergencyAlert_t *)calloc(context->alertHistory.count, sizeof(*records));
    if (records == NULL)
    {
        return BMS_STATUS_MEMORY_ERROR;
    }

    for (node = context->alertHistory.head; node != NULL; node = node->next)
    {
        if (node->data == NULL)
        {
            free(records);
            return BMS_STATUS_INVALID_DATA;
        }
        records[index] = *(const BmsEmergencyAlert_t *)node->data;
        ++index;
    }

    status = FileManagementWriteRecords(BMS_EMERGENCY_ALERTS_FILE, records, index,
                                        sizeof(*records));
    free(records);
    return status;
}
BmsStatus_t EmergencyAlertManagementCreate(BmsEmergencyAlertContext_t *c,
                                              const BmsEmergencyAlert_t *a)
{
    BmsStatus_t status;

    if ((c == NULL) || (a == NULL))
    {
        return BMS_STATUS_INVALID_ARGUMENT;
    }

    status = LinkedListInsertBack(&c->alertHistory, a);
    if (status != BMS_STATUS_OK)
    {
        BMS_LOG_ERROR("EMERGENCY",
                      "Alert history insertion failed: alertId=%u status=%u",
                      (unsigned int)a->alertId,
                      (unsigned int)status);
        return BMS_STATUS_MEMORY_ERROR;
    }

    status = QueueEnqueue(&c->alertQueue, a);
    if (status == BMS_STATUS_OK)
    {
        BMS_LOG_INFO("EMERGENCY",
                     "Emergency alert created: alertId=%u group=%s units=%u sourceHospital=%u",
                     (unsigned int)a->alertId,
                     UtilityBloodGroupToString(a->bloodGroup),
                     (unsigned int)a->requiredUnits,
                     (unsigned int)a->sourceHospitalId);
    }
    else
    {
        BMS_LOG_ERROR("EMERGENCY",
                      "Alert queue failed: alertId=%u status=%u",
                      (unsigned int)a->alertId,
                      (unsigned int)status);
    }

    return status;
}
BmsStatus_t EmergencyAlertManagementBroadcast(BmsEmergencyAlertContext_t*c,const BmsHospitalGraph_t*g,BmsNotificationContext_t*n,BmsAlertId_t id,uint32_t max){void*f=NULL;BroadcastCtx x;if((c==NULL)||(g==NULL)||(n==NULL))return BMS_STATUS_INVALID_ARGUMENT;if(LinkedListFind(&c->alertHistory,Match,&id,&f)!=BMS_STATUS_OK)return BMS_STATUS_NOT_FOUND;x.n=n;x.a=(const BmsEmergencyAlert_t*)f;{
    BmsStatus_t status = GraphBroadcastEmergencyAlert(g,
                                                      x.a->sourceHospitalId,
                                                      max,
                                                      Notify,
                                                      &x);
    if (status == BMS_STATUS_OK)
    {
        BMS_LOG_INFO("EMERGENCY",
                     "Emergency alert broadcast: alertId=%u sourceHospital=%u maxDistance=%u",
                     (unsigned int)id,
                     (unsigned int)x.a->sourceHospitalId,
                     (unsigned int)max);
    }
    else
    {
        BMS_LOG_ERROR("EMERGENCY",
                      "Emergency broadcast failed: alertId=%u status=%u",
                      (unsigned int)id,
                      (unsigned int)status);
    }
    return status;
}}
BmsStatus_t EmergencyAlertManagementProcessNext(BmsEmergencyAlertContext_t*c,BmsEmergencyAlert_t*a){return((c==NULL)||(a==NULL))?BMS_STATUS_INVALID_ARGUMENT:QueueDequeue(&c->alertQueue,a);}
BmsStatus_t EmergencyAlertManagementResolve(BmsEmergencyAlertContext_t*c,BmsAlertId_t id){void*f=NULL;BmsStatus_t s;if(c==NULL)return BMS_STATUS_INVALID_ARGUMENT;s=LinkedListFind(&c->alertHistory,Match,&id,&f);if (s == BMS_STATUS_OK)
{
    ((BmsEmergencyAlert_t*)f)->resolved = true;
    BMS_LOG_INFO("EMERGENCY",
                 "Emergency alert resolved: alertId=%u",
                 (unsigned int)id);
}
return s;}
void EmergencyAlertManagementDeinitialize(BmsEmergencyAlertContext_t*c){if(c==NULL)return;LinkedListClear(&c->alertHistory);QueueDeinitialize(&c->alertQueue);(void)memset(c,0,sizeof(*c));}
