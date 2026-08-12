
#include "test_common.h"
#include "report_management.h"
#include <stdio.h>
#include <string.h>

static void test_empty_summary(void)
{
    BmsDonorContext_t d; BmsHospitalContext_t h; BmsInventoryContext_t i;
    BmsBloodRequestContext_t r; BmsDonationContext_t dn;
    BmsEmergencyAlertContext_t a; BmsSummaryReport_t s;

    ASSERT_OK(DonorManagementInitialize(&d));
    ASSERT_OK(HospitalManagementInitialize(&h));
    ASSERT_OK(BloodInventoryInitialize(&i));
    ASSERT_OK(BloodRequestManagementInitialize(&r));
    ASSERT_OK(DonationManagementInitialize(&dn));
    ASSERT_OK(EmergencyAlertManagementInitialize(&a));
    ASSERT_OK(ReportManagementBuildSummary(&d,&h,&i,&r,&dn,&a,&s));

    CU_ASSERT_EQUAL(s.totalDonors,0U);
    CU_ASSERT_EQUAL(s.totalHospitals,0U);
    CU_ASSERT_EQUAL(s.totalInventoryUnits,0U);
    CU_ASSERT_EQUAL(s.pendingRequests,0U);
    CU_ASSERT_EQUAL(s.totalDonations,0U);
    ASSERT_OK(ReportManagementPrintSummary(&s));

    EmergencyAlertManagementDeinitialize(&a);
    DonationManagementDeinitialize(&dn);
    BloodRequestManagementDeinitialize(&r);
    BloodInventoryDeinitialize(&i);
    HospitalManagementDeinitialize(&h);
    DonorManagementDeinitialize(&d);
}

static void test_individual_reports(void)
{
    BmsDonorContext_t d; BmsHospitalContext_t h; BmsInventoryContext_t i;
    BmsBloodRequestContext_t r; BmsDonationContext_t dn;
    BmsEmergencyAlertContext_t a;

    ASSERT_OK(DonorManagementInitialize(&d));
    ASSERT_OK(HospitalManagementInitialize(&h));
    ASSERT_OK(BloodInventoryInitialize(&i));
    ASSERT_OK(BloodRequestManagementInitialize(&r));
    ASSERT_OK(DonationManagementInitialize(&dn));
    ASSERT_OK(EmergencyAlertManagementInitialize(&a));

    ASSERT_OK(ReportManagementGenerateInventoryReport(&i, "test_inventory_report.txt"));
    ASSERT_OK(ReportManagementGenerateDonorReport(&d, "test_donor_report.txt"));
    ASSERT_OK(ReportManagementGenerateHospitalReport(&h, "test_hospital_report.txt"));
    ASSERT_OK(ReportManagementGenerateBloodRequestReport(&r, "test_request_report.txt"));
    ASSERT_OK(ReportManagementGenerateDonationReport(&dn, "test_donation_report.txt"));
    ASSERT_OK(ReportManagementGenerateEmergencyAlertReport(&a, "test_alert_report.txt"));

    remove("test_inventory_report.txt");
    remove("test_donor_report.txt");
    remove("test_hospital_report.txt");
    remove("test_request_report.txt");
    remove("test_donation_report.txt");
    remove("test_alert_report.txt");

    EmergencyAlertManagementDeinitialize(&a);
    DonationManagementDeinitialize(&dn);
    BloodRequestManagementDeinitialize(&r);
    BloodInventoryDeinitialize(&i);
    HospitalManagementDeinitialize(&h);
    DonorManagementDeinitialize(&d);
}

static void test_report_invalid_arguments(void)
{
    BmsDonorContext_t d;
    ASSERT_OK(DonorManagementInitialize(&d));

    CU_ASSERT_EQUAL(
        ReportManagementGenerateDonorReport(NULL, "invalid.txt"),
        BMS_STATUS_INVALID_ARGUMENT);

    CU_ASSERT_EQUAL(
        ReportManagementGenerateDonorReport(&d, NULL),
        BMS_STATUS_INVALID_ARGUMENT);

    CU_ASSERT_EQUAL(
        ReportManagementGenerateDonorReport(
            &d, "/definitely_missing_directory/report.txt"),
        BMS_STATUS_FILE_ERROR);

    DonorManagementDeinitialize(&d);
}

void RegisterReportTests(void)
{
    CU_pSuite s = CU_add_suite("Reports", NULL, NULL);
    CU_add_test(s, "empty operational summary", test_empty_summary);
    CU_add_test(s, "individual reports", test_individual_reports);
    CU_add_test(s, "invalid arguments and file path", test_report_invalid_arguments);
}
