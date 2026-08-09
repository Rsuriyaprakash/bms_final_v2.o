#include "donation_camp_management.h"
#include "logging.h"
#include "file_management.h"
#include "file_names.h"
#include <stdlib.h>
#include <string.h>
BmsStatus_t DonationCampManagementInitialize(BmsDonationCampContext_t*c){if(c==NULL)return BMS_STATUS_INVALID_ARGUMENT;(void)memset(c,0,sizeof(*c));if(LinkedListInitialize(&c->camps,sizeof(BmsDonationCamp_t))!=BMS_STATUS_OK)return BMS_STATUS_MEMORY_ERROR;c->initialized=true;return BMS_STATUS_OK;}
BmsStatus_t DonationCampManagementLoad(BmsDonationCampContext_t*c){bool e=false;uint32_t n=0U,i;BmsDonationCamp_t*a;BmsStatus_t s;if((c==NULL)||!c->initialized)return BMS_STATUS_NOT_INITIALIZED;(void)FileManagementFileExists(BMS_DONATION_CAMPS_FILE,&e);if(!e)return BMS_STATUS_OK;s=FileManagementGetRecordCount(BMS_DONATION_CAMPS_FILE,sizeof(*a),&n);if(s!=BMS_STATUS_OK)return s;if(n==0U)return BMS_STATUS_OK;a=(BmsDonationCamp_t*)calloc(n,sizeof(*a));if(a==NULL)return BMS_STATUS_MEMORY_ERROR;s=FileManagementReadRecords(BMS_DONATION_CAMPS_FILE,a,n,&n,sizeof(*a));for(i=0U;(s==BMS_STATUS_OK)&&(i<n);++i)s=LinkedListInsertBack(&c->camps,&a[i]);free(a);return s;}
BmsStatus_t DonationCampManagementSave(const BmsDonationCampContext_t *context)
{
    BmsDonationCamp_t *records = NULL;
    const BmsLinkedListNode_t *node = NULL;
    uint32_t index = 0U;
    BmsStatus_t status;

    if ((context == NULL) || (!context->initialized))
    {
        return BMS_STATUS_NOT_INITIALIZED;
    }

    if (context->camps.count == 0U)
    {
        return FileManagementWriteRecords(BMS_DONATION_CAMPS_FILE, NULL, 0U,
                                          sizeof(BmsDonationCamp_t));
    }

    records = (BmsDonationCamp_t *)calloc(context->camps.count, sizeof(*records));
    if (records == NULL)
    {
        return BMS_STATUS_MEMORY_ERROR;
    }

    for (node = context->camps.head; node != NULL; node = node->next)
    {
        if (node->data == NULL)
        {
            free(records);
            return BMS_STATUS_INVALID_DATA;
        }
        records[index] = *(const BmsDonationCamp_t *)node->data;
        ++index;
    }

    status = FileManagementWriteRecords(BMS_DONATION_CAMPS_FILE, records, index,
                                        sizeof(*records));
    free(records);
    return status;
}
BmsStatus_t DonationCampManagementAdd(BmsDonationCampContext_t *c,
                                        const BmsDonationCamp_t *camp)
{
    BmsStatus_t status;

    if ((c == NULL) || (camp == NULL))
    {
        return BMS_STATUS_INVALID_ARGUMENT;
    }

    status = LinkedListInsertBack(&c->camps, camp);
    if (status == BMS_STATUS_OK)
    {
        BMS_LOG_INFO("CAMP",
                     "Donation camp added: campId=%u location=%s",
                     (unsigned int)camp->campId,
                     camp->venue,
                     camp->city);
    }
    else
    {
        BMS_LOG_ERROR("CAMP",
                      "Donation camp add failed: campId=%u status=%u",
                      (unsigned int)camp->campId,
                      (unsigned int)status);
    }

    return status;
} BmsStatus_t DonationCampManagementTraverse(const BmsDonationCampContext_t*c,BmsCampVisitor_t v,void*x){const BmsLinkedListNode_t*n;if((c==NULL)||(v==NULL))return BMS_STATUS_INVALID_ARGUMENT;for(n=c->camps.head;n;n=n->next){BmsStatus_t s=v((const BmsDonationCamp_t*)n->data,x);if(s!=BMS_STATUS_OK)return s;}return BMS_STATUS_OK;} void DonationCampManagementDeinitialize(BmsDonationCampContext_t*c){if(c){LinkedListClear(&c->camps);(void)memset(c,0,sizeof(*c));}}
