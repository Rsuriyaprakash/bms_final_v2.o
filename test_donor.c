
#include "test_common.h"
#include "donor_management.h"

static BmsDonor_t donor(uint32_t id,const char*n,BmsBloodGroup_t g)
{
    BmsDonor_t d;
    memset(&d,0,sizeof d);
    d.donorId=id;
    snprintf(d.name,sizeof(d.name),"%s",n);
    d.age=25; d.weightKg=60; d.bloodGroup=g;
    snprintf(d.phone,sizeof(d.phone),"9876543210");
    snprintf(d.email,sizeof(d.email),"d%u@test.com",id);
    snprintf(d.address,sizeof(d.address),"Chennai");
    d.isActive=true; d.isEligible=true;
    return d;
}

static BmsStatus_t countv(const BmsDonor_t*d,void*c)
{
    (void)d; (*(uint32_t*)c)++; return BMS_STATUS_OK;
}

static void test_crud_search(void)
{
    BmsDonorContext_t c;
    BmsDonor_t a=donor(1,"Rahul",BMS_BLOOD_GROUP_A_POSITIVE),out;

    ASSERT_OK(DonorManagementInitialize(&c));
    ASSERT_OK(DonorManagementAdd(&c,&a));
    ASSERT_STATUS(BMS_STATUS_ALREADY_EXISTS,DonorManagementAdd(&c,&a));
    ASSERT_OK(DonorManagementSearchById(&c,1,&out));
    CU_ASSERT_STRING_EQUAL(out.name,"Rahul");

    snprintf(a.address,sizeof a,"Updated Address");
    ASSERT_OK(DonorManagementUpdate(&c,&a));
    ASSERT_OK(DonorManagementSearchById(&c,1,&out));
    CU_ASSERT_STRING_EQUAL(out.address,"Updated Address");

    ASSERT_OK(DonorManagementDelete(&c,1));
    ASSERT_STATUS(BMS_STATUS_NOT_FOUND,DonorManagementSearchById(&c,1,&out));
    DonorManagementDeinitialize(&c);
}

static void test_filter_sort_eligibility(void)
{
    BmsDonorContext_t c;
    BmsDonor_t a=donor(1,"Zara",BMS_BLOOD_GROUP_O_POSITIVE);
    BmsDonor_t b=donor(2,"Arun",BMS_BLOOD_GROUP_A_POSITIVE);
    BmsDate_t now={2026,8,4};
    uint32_t count=0;

    ASSERT_OK(DonorManagementInitialize(&c));
    ASSERT_OK(DonorManagementAdd(&c,&a));
    ASSERT_OK(DonorManagementAdd(&c,&b));
    ASSERT_OK(DonorManagementFindByBloodGroup(&c,BMS_BLOOD_GROUP_A_POSITIVE,countv,&count));
    CU_ASSERT_EQUAL(count,1U);
    ASSERT_OK(DonorManagementSortByName(&c));

    b.age=17;
    ASSERT_OK(DonorManagementCheckEligibility(&b,&now));
    CU_ASSERT_FALSE(b.isEligible);

    DonorManagementDeinitialize(&c);
}

static void test_donor_missing_records(void)
{
    BmsDonorContext_t c;
    BmsDonor_t out=donor(999,"Missing",BMS_BLOOD_GROUP_O_POSITIVE);

    ASSERT_OK(DonorManagementInitialize(&c));
    CU_ASSERT_NOT_EQUAL(DonorManagementSearchById(&c,999U,&out),BMS_STATUS_OK);
    CU_ASSERT_NOT_EQUAL(DonorManagementUpdate(&c,&out),BMS_STATUS_OK);
    CU_ASSERT_NOT_EQUAL(DonorManagementDelete(&c,999U),BMS_STATUS_OK);
    DonorManagementDeinitialize(&c);
}

void RegisterDonorTests(void)
{
    CU_pSuite s=CU_add_suite("Donor",NULL,NULL);
    CU_add_test(s,"CRUD duplicate search",test_crud_search);
    CU_add_test(s,"filter sort eligibility",test_filter_sort_eligibility);
    CU_add_test(s,"missing record cases",test_donor_missing_records);
}
