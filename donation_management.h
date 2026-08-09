/**
 * @file donation_management.h
 * @author Suriya Prakash R
 * @version 2.0.0
 * @brief Blood donation recording and history APIs.
 */

#ifndef BMS_DONATION_MANAGEMENT_H
#define BMS_DONATION_MANAGEMENT_H

#include <stdbool.h>

#include "blood_inventory.h"
#include "common.h"
#include "donor_management.h"
#include "hash_table.h"
#include "linked_list.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef BmsStatus_t (*BmsDonationVisitor_t)(const BmsDonation_t *donation,
                                           void *context);

typedef struct
{
    BmsLinkedList_t donations;
    BmsHashTable_t donationIdIndex;
    bool initialized;
} BmsDonationContext_t;

BmsStatus_t DonationManagementInitialize(BmsDonationContext_t *context);
BmsStatus_t DonationManagementLoad(BmsDonationContext_t *context);
BmsStatus_t DonationManagementSave(const BmsDonationContext_t *context);
BmsStatus_t DonationManagementRecord(
    BmsDonationContext_t *context,
    BmsDonorContext_t *donorContext,
    BmsInventoryContext_t *inventoryContext,
    const BmsDonation_t *donation);
BmsStatus_t DonationManagementSearchById(
    const BmsDonationContext_t *context,
    BmsDonationId_t donationId,
    BmsDonation_t *donation);
BmsStatus_t DonationManagementFindByDonor(
    BmsDonationContext_t *context,
    BmsDonorId_t donorId,
    BmsDonationVisitor_t visitor,
    void *visitorContext);
BmsStatus_t DonationManagementTraverse(BmsDonationContext_t *context,
                                       BmsDonationVisitor_t visitor,
                                       void *visitorContext);
void DonationManagementDeinitialize(BmsDonationContext_t *context);

#ifdef __cplusplus
}
#endif

#endif /* BMS_DONATION_MANAGEMENT_H */
