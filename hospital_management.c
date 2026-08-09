/** @file hospital_management.c @brief Hospital management implementation. */
#include "hospital_management.h"
#include "logging.h"
#include "file_management.h"
#include "file_names.h"
#include <string.h>
#include <stdlib.h>
static bool Match(const void*d,const void*k){return((const BmsHospital_t*)d)->hospitalId==*(const BmsHospitalId_t*)k;}
BmsStatus_t HospitalManagementInitialize(BmsHospitalContext_t*c){if(c==NULL)return BMS_STATUS_INVALID_ARGUMENT;(void)memset(c,0,sizeof(*c));LinkedListInitialize(&c->hospitals,sizeof(BmsHospital_t));HashTableInitialize(&c->hospitalIdIndex,BMS_HASH_BUCKET_COUNT,sizeof(BmsHospital_t*),BMS_HASH_KEY_UINT32);GraphInitialize(&c->graph);c->initialized=true;return BMS_STATUS_OK;}
BmsStatus_t HospitalManagementLoad(BmsHospitalContext_t*c){bool exists=false;uint32_t count=0U,i=0U;BmsHospital_t*a=NULL;BmsStatus_t st;if((c==NULL)||!c->initialized)return BMS_STATUS_NOT_INITIALIZED;(void)FileManagementFileExists(BMS_HOSPITALS_FILE,&exists);if(!exists)return BMS_STATUS_OK;st=FileManagementGetRecordCount(BMS_HOSPITALS_FILE,sizeof(BmsHospital_t),&count);if(st!=BMS_STATUS_OK)return st;if(count==0U)return BMS_STATUS_OK;a=(BmsHospital_t*)calloc(count,sizeof(*a));if(a==NULL)return BMS_STATUS_MEMORY_ERROR;st=FileManagementReadRecords(BMS_HOSPITALS_FILE,a,count,&count,sizeof(*a));for(i=0U;(st==BMS_STATUS_OK)&&(i<count);++i)st=HospitalManagementAdd(c,&a[i]);free(a);return st;}
BmsStatus_t HospitalManagementSave(const BmsHospitalContext_t *context)
{
    BmsHospital_t *records = NULL;
    const BmsLinkedListNode_t *node = NULL;
    uint32_t index = 0U;
    BmsStatus_t status;

    if ((context == NULL) || (!context->initialized))
    {
        return BMS_STATUS_NOT_INITIALIZED;
    }

    if (context->hospitals.count == 0U)
    {
        return FileManagementWriteRecords(BMS_HOSPITALS_FILE, NULL, 0U,
                                          sizeof(BmsHospital_t));
    }

    records = (BmsHospital_t *)calloc(context->hospitals.count, sizeof(*records));
    if (records == NULL)
    {
        return BMS_STATUS_MEMORY_ERROR;
    }

    for (node = context->hospitals.head; node != NULL; node = node->next)
    {
        if (node->data == NULL)
        {
            free(records);
            return BMS_STATUS_INVALID_DATA;
        }
        records[index] = *(const BmsHospital_t *)node->data;
        ++index;
    }

    status = FileManagementWriteRecords(BMS_HOSPITALS_FILE, records, index,
                                        sizeof(*records));
    free(records);
    return status;
}
BmsStatus_t HospitalManagementAdd(BmsHospitalContext_t *c,
                                    const BmsHospital_t *h)
{
    void *found = NULL;
    BmsStatus_t status;

    if ((c == NULL) || (h == NULL))
    {
        return BMS_STATUS_INVALID_ARGUMENT;
    }

    if (LinkedListFind(&c->hospitals, Match, &h->hospitalId, &found) == BMS_STATUS_OK)
    {
        BMS_LOG_WARNING("HOSPITAL",
                        "Duplicate hospital rejected: hospitalId=%u",
                        (unsigned int)h->hospitalId);
        return BMS_STATUS_ALREADY_EXISTS;
    }

    status = GraphAddHospital(&c->graph, h);
    if (status != BMS_STATUS_OK)
    {
        BMS_LOG_ERROR("HOSPITAL",
                      "Graph add failed: hospitalId=%u status=%u",
                      (unsigned int)h->hospitalId,
                      (unsigned int)status);
        return BMS_STATUS_GRAPH_FULL;
    }

    status = LinkedListInsertBack(&c->hospitals, h);
    if (status == BMS_STATUS_OK)
    {
        BMS_LOG_INFO("HOSPITAL",
                     "Hospital added: hospitalId=%u name=%s",
                     (unsigned int)h->hospitalId,
                     h->name);
    }

    return status;
}
BmsStatus_t HospitalManagementSearchById(const BmsHospitalContext_t*c,BmsHospitalId_t id,BmsHospital_t*h){void*f=NULL;BmsStatus_t s;if((c==NULL)||(h==NULL))return BMS_STATUS_INVALID_ARGUMENT;s=LinkedListFind(&c->hospitals,Match,&id,&f);if(s==BMS_STATUS_OK)*h=*(BmsHospital_t*)f;return s;}
BmsStatus_t HospitalManagementUpdate(BmsHospitalContext_t *c,
                                       const BmsHospital_t *h)
{
    BmsStatus_t status;

    if ((c == NULL) || (h == NULL))
    {
        return BMS_STATUS_INVALID_ARGUMENT;
    }

    status = LinkedListUpdate(&c->hospitals, Match, &h->hospitalId, h);
    if (status == BMS_STATUS_OK)
    {
        BMS_LOG_INFO("HOSPITAL",
                     "Hospital updated: hospitalId=%u",
                     (unsigned int)h->hospitalId);
    }
    return status;
}
BmsStatus_t HospitalManagementDelete(BmsHospitalContext_t *c,
                                       BmsHospitalId_t id)
{
    BmsStatus_t status;

    if (c == NULL)
    {
        return BMS_STATUS_INVALID_ARGUMENT;
    }

    (void)GraphRemoveHospital(&c->graph, id);
    status = LinkedListDelete(&c->hospitals, Match, &id);
    if (status == BMS_STATUS_OK)
    {
        BMS_LOG_INFO("HOSPITAL",
                     "Hospital deleted: hospitalId=%u",
                     (unsigned int)id);
    }
    return status;
}
BmsStatus_t HospitalManagementTraverse(const BmsHospitalContext_t*c,BmsHospitalVisitor_t v,void*ctx){const BmsLinkedListNode_t*n;if((c==NULL)||(v==NULL))return BMS_STATUS_INVALID_ARGUMENT;for(n=c->hospitals.head;n;n=n->next){BmsStatus_t s=v((const BmsHospital_t*)n->data,ctx);if(s!=BMS_STATUS_OK)return s;}return BMS_STATUS_OK;}
BmsStatus_t HospitalManagementAddRoute(BmsHospitalContext_t *c,
                                         BmsHospitalId_t a,
                                         BmsHospitalId_t b,
                                         uint32_t distance)
{
    BmsStatus_t status;

    if (c == NULL)
    {
        return BMS_STATUS_INVALID_ARGUMENT;
    }

    status = GraphAddRoute(&c->graph, a, b, distance);
    if (status == BMS_STATUS_OK)
    {
        BMS_LOG_INFO("HOSPITAL",
                     "Route added: from=%u to=%u distance=%u km",
                     (unsigned int)a,
                     (unsigned int)b,
                     (unsigned int)distance);
    }
    return status;
}
BmsStatus_t HospitalManagementFindNearest(const BmsHospitalContext_t*c,BmsHospitalId_t s,BmsHospital_t*h,uint32_t*d){BmsHospitalId_t id;BmsStatus_t st;if((c==NULL)||(h==NULL)||(d==NULL))return BMS_STATUS_INVALID_ARGUMENT;st=GraphFindNearestHospital(&c->graph,s,&id,d);return(st==BMS_STATUS_OK)?HospitalManagementSearchById(c,id,h):st;}
BmsStatus_t HospitalManagementGetRouteDistance(const BmsHospitalContext_t*c,BmsHospitalId_t a,BmsHospitalId_t b,uint32_t*d){return(c==NULL)?BMS_STATUS_INVALID_ARGUMENT:GraphGetRouteDistance(&c->graph,a,b,d);}
void HospitalManagementDeinitialize(BmsHospitalContext_t*c){if(c==NULL)return;LinkedListClear(&c->hospitals);HashTableDeinitialize(&c->hospitalIdIndex);GraphClear(&c->graph);(void)memset(c,0,sizeof(*c));}
