/** @file donor_management.c @brief Donor management implementation. */
#include "donor_management.h"
#include "common_validation.h"
#include "utility.h"
#include "logging.h"

#include "file_management.h"
#include "file_names.h"
#include <stdlib.h>
#include <string.h>

static bool Match(const void*d,const void*k){return((const BmsDonor_t*)d)->donorId==*(const BmsDonorId_t*)k;}
static int32_t CompareName(const void*a,const void*b){return strcmp(((const BmsDonor_t*)a)->name,((const BmsDonor_t*)b)->name);}
typedef struct { BmsDonorVisitor_t visitor; void *context; } DonorVisitAdapter_t;
static BmsStatus_t CallVisitor(void *data, void *context)
{
    DonorVisitAdapter_t *adapter = (DonorVisitAdapter_t *)context;
    return adapter->visitor((const BmsDonor_t *)data, adapter->context);
}
BmsStatus_t DonorManagementInitialize(BmsDonorContext_t*c){if(c==NULL)return BMS_STATUS_INVALID_ARGUMENT;(void)memset(c,0,sizeof(*c));if(LinkedListInitialize(&c->donors,sizeof(BmsDonor_t))!=BMS_STATUS_OK)return BMS_STATUS_MEMORY_ERROR;if(HashTableInitialize(&c->donorIdIndex,BMS_HASH_BUCKET_COUNT,sizeof(BmsDonor_t*),BMS_HASH_KEY_UINT32)!=BMS_STATUS_OK)return BMS_STATUS_MEMORY_ERROR;c->initialized=true;return BMS_STATUS_OK;}
BmsStatus_t DonorManagementLoad(BmsDonorContext_t *c)
{
    bool exists = false;
    uint32_t count = 0U;
    uint32_t i;
    BmsDonor_t *records;
    BmsStatus_t status;

    if ((c == NULL) || (!c->initialized))
    {
        return BMS_STATUS_NOT_INITIALIZED;
    }

    FileManagementFileExists(BMS_DONORS_FILE, &exists);
    if (!exists)
    {
        BMS_LOG_DEBUG("DONOR", "Donor file does not exist; starting with empty donor list");
        return BMS_STATUS_OK;
    }

    status = FileManagementGetRecordCount(BMS_DONORS_FILE,
                                          sizeof(BmsDonor_t),
                                          &count);
    if (status != BMS_STATUS_OK)
    {
        BMS_LOG_ERROR("DONOR",
                      "Failed to read donor record count: status=%u",
                      (unsigned int)status);
        return status;
    }

    if (count == 0U)
    {
        BMS_LOG_DEBUG("DONOR", "Donor file contains zero records");
        return BMS_STATUS_OK;
    }

    records = (BmsDonor_t *)calloc(count, sizeof(*records));
    if (records == NULL)
    {
        BMS_LOG_ERROR("DONOR",
                      "Memory allocation failed while loading %u donor records",
                      (unsigned int)count);
        return BMS_STATUS_MEMORY_ERROR;
    }

    status = FileManagementReadRecords(BMS_DONORS_FILE,
                                       records,
                                       count,
                                       &count,
                                       sizeof(*records));

    for (i = 0U; (status == BMS_STATUS_OK) && (i < count); ++i)
    {
        status = DonorManagementAdd(c, &records[i]);
    }

    free(records);

    if (status == BMS_STATUS_OK)
    {
        BMS_LOG_INFO("DONOR",
                     "Loaded donor records: count=%u",
                     (unsigned int)count);
    }
    else
    {
        BMS_LOG_ERROR("DONOR",
                      "Donor load failed: status=%u",
                      (unsigned int)status);
    }

    return status;
}
BmsStatus_t DonorManagementSave(const BmsDonorContext_t *c)
{
    BmsDonor_t *records;
    const BmsLinkedListNode_t *node;
    uint32_t index = 0U;
    BmsStatus_t status;

    if ((c == NULL) || (!c->initialized))
    {
        return BMS_STATUS_NOT_INITIALIZED;
    }

    if (c->donors.count == 0U)
    {
        status = FileManagementWriteRecords(BMS_DONORS_FILE,
                                            NULL,
                                            0U,
                                            sizeof(BmsDonor_t));
        if (status == BMS_STATUS_OK)
        {
            BMS_LOG_DEBUG("DONOR", "Saved empty donor dataset");
        }
        else
        {
            BMS_LOG_ERROR("DONOR",
                          "Failed to save empty donor dataset: status=%u",
                          (unsigned int)status);
        }
        return status;
    }

    records = (BmsDonor_t *)calloc(c->donors.count, sizeof(*records));
    if (records == NULL)
    {
        BMS_LOG_ERROR("DONOR",
                      "Memory allocation failed while saving %u donors",
                      (unsigned int)c->donors.count);
        return BMS_STATUS_MEMORY_ERROR;
    }

    for (node = c->donors.head; node != NULL; node = node->next)
    {
        records[index] = *(BmsDonor_t *)node->data;
        ++index;
    }

    status = FileManagementWriteRecords(BMS_DONORS_FILE,
                                        records,
                                        index,
                                        sizeof(*records));
    free(records);

    if (status == BMS_STATUS_OK)
    {
        BMS_LOG_INFO("DONOR",
                     "Saved donor records: count=%u",
                     (unsigned int)index);
    }
    else
    {
        BMS_LOG_ERROR("DONOR",
                      "Donor save failed: status=%u",
                      (unsigned int)status);
    }

    return status;
}
BmsStatus_t DonorManagementAdd(BmsDonorContext_t *c,
                                           const BmsDonor_t *d)
{
    void *found;
    BmsStatus_t status;

    if ((c == NULL) || (d == NULL))
    {
        return BMS_STATUS_INVALID_ARGUMENT;
    }

    if ((!ValidateDonorId(d->donorId)) ||
        (!ValidateName(d->name)) ||
        (!ValidateBloodGroupValue(d->bloodGroup)))
    {
        BMS_LOG_WARNING("DONOR",
                        "Invalid donor data rejected: donorId=%u",
                        (unsigned int)d->donorId);
        return BMS_STATUS_INVALID_DATA;
    }

    if (LinkedListFind(&c->donors,
                       Match,
                       &d->donorId,
                       &found) == BMS_STATUS_OK)
    {
        BMS_LOG_WARNING("DONOR",
                        "Duplicate donor rejected: donorId=%u",
                        (unsigned int)d->donorId);
        return BMS_STATUS_ALREADY_EXISTS;
    }

    status = LinkedListInsertBack(&c->donors, d);

    if (status == BMS_STATUS_OK)
    {
        BMS_LOG_INFO("DONOR",
                     "Donor profile created: donorId=%u group=%s",
                     (unsigned int)d->donorId,
                     UtilityBloodGroupToString(d->bloodGroup));
    }
    else
    {
        BMS_LOG_ERROR("DONOR",
                      "Donor profile creation failed: donorId=%u status=%u",
                      (unsigned int)d->donorId,
                      (unsigned int)status);
    }

    return status;
}
BmsStatus_t DonorManagementSearchById(const BmsDonorContext_t*c,BmsDonorId_t id,BmsDonor_t*d){void*f=NULL;BmsStatus_t s;if((c==NULL)||(d==NULL))return BMS_STATUS_INVALID_ARGUMENT;s=LinkedListFind(&c->donors,Match,&id,&f);if(s==BMS_STATUS_OK)*d=*(BmsDonor_t*)f;return s;}
BmsStatus_t DonorManagementUpdate(BmsDonorContext_t *c,
                                              const BmsDonor_t *d)
{
    BmsDonor_t existing;
    BmsStatus_t status;

    if ((c == NULL) || (d == NULL))
    {
        return BMS_STATUS_INVALID_ARGUMENT;
    }

    status = DonorManagementSearchById(c, d->donorId, &existing);
    if (status != BMS_STATUS_OK)
    {
        BMS_LOG_WARNING("DONOR",
                        "Donor update failed; donor not found: donorId=%u",
                        (unsigned int)d->donorId);
        return status;
    }

    if (existing.bloodGroup != d->bloodGroup)
    {
        BMS_LOG_WARNING("DONOR",
                        "Rejected blood-group change: donorId=%u existing=%s requested=%s",
                        (unsigned int)d->donorId,
                        UtilityBloodGroupToString(existing.bloodGroup),
                        UtilityBloodGroupToString(d->bloodGroup));
        return BMS_STATUS_INVALID_DATA;
    }

    status = LinkedListUpdate(&c->donors,
                              Match,
                              &d->donorId,
                              d);

    if (status == BMS_STATUS_OK)
    {
        BMS_LOG_INFO("DONOR",
                     "Donor profile updated: donorId=%u",
                     (unsigned int)d->donorId);
    }
    else
    {
        BMS_LOG_ERROR("DONOR",
                      "Donor profile update failed: donorId=%u status=%u",
                      (unsigned int)d->donorId,
                      (unsigned int)status);
    }

    return status;
}
BmsStatus_t DonorManagementDelete(BmsDonorContext_t *c,
                                              BmsDonorId_t id)
{
    BmsStatus_t status;

    if (c == NULL)
    {
        return BMS_STATUS_INVALID_ARGUMENT;
    }

    status = LinkedListDelete(&c->donors, Match, &id);

    if (status == BMS_STATUS_OK)
    {
        BMS_LOG_INFO("DONOR",
                     "Donor profile deleted: donorId=%u",
                     (unsigned int)id);
    }
    else
    {
        BMS_LOG_WARNING("DONOR",
                        "Donor delete failed: donorId=%u status=%u",
                        (unsigned int)id,
                        (unsigned int)status);
    }

    return status;
}
BmsStatus_t DonorManagementCheckEligibility(BmsDonor_t *d,
                                                        const BmsDate_t *now)
{
    if ((d == NULL) || (now == NULL))
    {
        return BMS_STATUS_INVALID_ARGUMENT;
    }

    d->isEligible =
        ValidateAge(d->age) &&
        ValidateWeight(d->weightKg) &&
        (UtilityCompareDates(&d->lastDonationDate, now) <= 0);

    if (d->isEligible)
    {
        BMS_LOG_DEBUG("DONOR",
                      "Donor eligibility passed: donorId=%u",
                      (unsigned int)d->donorId);
    }
    else
    {
        BMS_LOG_WARNING("DONOR",
                        "Donor not eligible: donorId=%u age=%u weight=%u",
                        (unsigned int)d->donorId,
                        (unsigned int)d->age,
                        (unsigned int)d->weightKg);
    }

    return BMS_STATUS_OK;
}
BmsStatus_t DonorManagementFindByBloodGroup(const BmsDonorContext_t*c,BmsBloodGroup_t g,BmsDonorVisitor_t v,void*ctx){
const BmsLinkedListNode_t*n;
if((c==NULL)||(v==NULL))return BMS_STATUS_INVALID_ARGUMENT;
for(n=c->donors.head;n!=NULL;n=n->next)if(((BmsDonor_t*)n->data)->bloodGroup==g){BmsStatus_t s=v((const BmsDonor_t*)n->data,ctx);if(s!=BMS_STATUS_OK)return s;}return BMS_STATUS_OK;}
BmsStatus_t DonorManagementTraverse(BmsDonorContext_t *c, BmsDonorVisitor_t v, void *ctx)
{
    DonorVisitAdapter_t adapter;
    if ((c == NULL) || (v == NULL)) { return BMS_STATUS_INVALID_ARGUMENT; }
    adapter.visitor = v;
    adapter.context = ctx;
    return LinkedListTraverse(&c->donors, CallVisitor, &adapter);
}
BmsStatus_t DonorManagementSortByName(BmsDonorContext_t*c){return(c==NULL)?BMS_STATUS_INVALID_ARGUMENT:LinkedListSort(&c->donors,CompareName);}
void DonorManagementDeinitialize(BmsDonorContext_t*c){if(c==NULL)return;LinkedListClear(&c->donors);HashTableDeinitialize(&c->donorIdIndex);(void)memset(c,0,sizeof(*c));}
