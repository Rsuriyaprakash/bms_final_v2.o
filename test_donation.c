
#include "test_common.h"
#include "donation_management.h"

static void test_record_search_history(void)
{
    BmsDonationContext_t dc; BmsDonorContext_t donors; BmsInventoryContext_t inv;
    BmsDonation_t d={1,1,BMS_BLOOD_GROUP_O_POSITIVE,1,{2026,8,4},0},out;
    uint32_t units=0;

    ASSERT_OK(DonationManagementInitialize(&dc));
    ASSERT_OK(DonorManagementInitialize(&donors));
    ASSERT_OK(BloodInventoryInitialize(&inv));
    ASSERT_OK(DonationManagementRecord(&dc,&donors,&inv,&d));
    ASSERT_OK(DonationManagementSearchById(&dc,1,&out));
    CU_ASSERT_EQUAL(out.donorId,1U);
    ASSERT_OK(BloodInventoryGetAvailableUnits(&inv,BMS_BLOOD_GROUP_O_POSITIVE,&units));
    CU_ASSERT_EQUAL(units,1U);

    BloodInventoryDeinitialize(&inv);
    DonorManagementDeinitialize(&donors);
    DonationManagementDeinitialize(&dc);
}

static void test_donation_negative_cases(void)
{
    BmsDonationContext_t dc; BmsDonorContext_t donors; BmsInventoryContext_t inv;
    BmsDonation_t d={99,999,BMS_BLOOD_GROUP_A_POSITIVE,1,{2026,8,4},0},out;

    ASSERT_OK(DonationManagementInitialize(&dc));
    ASSERT_OK(DonorManagementInitialize(&donors));
    ASSERT_OK(BloodInventoryInitialize(&inv));

    CU_ASSERT_NOT_EQUAL(DonationManagementSearchById(&dc,999U,&out),BMS_STATUS_OK);
    CU_ASSERT_NOT_EQUAL(DonationManagementRecord(&dc,&donors,&inv,&d),BMS_STATUS_OK);

    BloodInventoryDeinitialize(&inv);
    DonorManagementDeinitialize(&donors);
    DonationManagementDeinitialize(&dc);
}

void RegisterDonationTests(void)
{
    CU_pSuite s=CU_add_suite("Donation",NULL,NULL);
    CU_add_test(s,"record search inventory update",test_record_search_history);
    CU_add_test(s,"negative cases",test_donation_negative_cases);
}
