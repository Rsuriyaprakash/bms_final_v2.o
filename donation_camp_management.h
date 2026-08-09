#ifndef BMS_DONATION_CAMP_MANAGEMENT_H
#define BMS_DONATION_CAMP_MANAGEMENT_H
#include "common.h"
#include "linked_list.h"
typedef BmsStatus_t (*BmsCampVisitor_t)(const BmsDonationCamp_t*,void*);
typedef struct{BmsLinkedList_t camps;bool initialized;}BmsDonationCampContext_t;
BmsStatus_t DonationCampManagementInitialize(BmsDonationCampContext_t*); BmsStatus_t DonationCampManagementLoad(BmsDonationCampContext_t*); BmsStatus_t DonationCampManagementSave(const BmsDonationCampContext_t*); BmsStatus_t DonationCampManagementAdd(BmsDonationCampContext_t*,const BmsDonationCamp_t*); BmsStatus_t DonationCampManagementTraverse(BmsDonationCampContext_t*,BmsCampVisitor_t,void*); void DonationCampManagementDeinitialize(BmsDonationCampContext_t*);
#endif
