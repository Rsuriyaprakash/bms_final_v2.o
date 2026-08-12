
#include "test_common.h"
#include "emergency_alert_management.h"

static void test_create_process_resolve(void)
{
    BmsEmergencyAlertContext_t c;
    BmsEmergencyAlert_t a,out;

    memset(&a,0,sizeof a);
    a.alertId=1; a.sourceHospitalId=1;
    a.bloodGroup=BMS_BLOOD_GROUP_O_NEGATIVE;
    a.requiredUnits=3; a.priority=BMS_PRIORITY_EMERGENCY;
    snprintf(a.message,sizeof(a.message),"Urgent O- required");

    ASSERT_OK(EmergencyAlertManagementInitialize(&c));
    ASSERT_OK(EmergencyAlertManagementCreate(&c,&a));
    ASSERT_OK(EmergencyAlertManagementProcessNext(&c,&out));
    CU_ASSERT_EQUAL(out.alertId,1U);
    ASSERT_OK(EmergencyAlertManagementResolve(&c,1));
    EmergencyAlertManagementDeinitialize(&c);
}

static void test_empty_and_missing_alerts(void)
{
    BmsEmergencyAlertContext_t c;
    BmsEmergencyAlert_t out;

    ASSERT_OK(EmergencyAlertManagementInitialize(&c));

    CU_ASSERT_NOT_EQUAL(
        EmergencyAlertManagementProcessNext(&c,&out),
        BMS_STATUS_OK);

    CU_ASSERT_NOT_EQUAL(
        EmergencyAlertManagementResolve(&c,999U),
        BMS_STATUS_OK);

    EmergencyAlertManagementDeinitialize(&c);
}

void RegisterEmergencyAlertTests(void)
{
    CU_pSuite s=CU_add_suite("Emergency Alert",NULL,NULL);
    CU_add_test(s,"create process resolve",test_create_process_resolve);
    CU_add_test(s,"empty and missing cases",test_empty_and_missing_alerts);
}
