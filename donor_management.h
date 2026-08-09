/**
 * @file donor_management.h
 * @author Suriya Prakash R
 * @version 2.0.0
 * @brief Donor registration, lookup, update, deletion, and eligibility APIs.
 */

#ifndef BMS_DONOR_MANAGEMENT_H
#define BMS_DONOR_MANAGEMENT_H

#include <stdbool.h>

#include "common.h"
#include "hash_table.h"
#include "linked_list.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef BmsStatus_t (*BmsDonorVisitor_t)(const BmsDonor_t *donor,
                                        void *context);

typedef struct
{
    BmsLinkedList_t donors;
    BmsHashTable_t donorIdIndex;
    bool initialized;
} BmsDonorContext_t;

BmsStatus_t DonorManagementInitialize(BmsDonorContext_t *context);
BmsStatus_t DonorManagementLoad(BmsDonorContext_t *context);
BmsStatus_t DonorManagementSave(const BmsDonorContext_t *context);
BmsStatus_t DonorManagementAdd(BmsDonorContext_t *context,
                               const BmsDonor_t *donor);
BmsStatus_t DonorManagementSearchById(const BmsDonorContext_t *context,
                                      BmsDonorId_t donorId,
                                      BmsDonor_t *donor);
BmsStatus_t DonorManagementUpdate(BmsDonorContext_t *context,
                                  const BmsDonor_t *donor);
BmsStatus_t DonorManagementDelete(BmsDonorContext_t *context,
                                  BmsDonorId_t donorId);
BmsStatus_t DonorManagementCheckEligibility(BmsDonor_t *donor,
                                            const BmsDate_t *currentDate);
BmsStatus_t DonorManagementFindByBloodGroup(
    const BmsDonorContext_t *context,
    BmsBloodGroup_t bloodGroup,
    BmsDonorVisitor_t visitor,
    void *visitorContext);
BmsStatus_t DonorManagementTraverse(BmsDonorContext_t *context,
                                    BmsDonorVisitor_t visitor,
                                    void *visitorContext);
BmsStatus_t DonorManagementSortByName(BmsDonorContext_t *context);
void DonorManagementDeinitialize(BmsDonorContext_t *context);

#ifdef __cplusplus
}
#endif

#endif /* BMS_DONOR_MANAGEMENT_H */
