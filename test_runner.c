#include "test_common.h"
void RegisterValidationTests(void); void RegisterAuthenticationTests(void);
void RegisterDonorTests(void); void RegisterInventoryTests(void);
void RegisterRequestTests(void); void RegisterHospitalTests(void);
void RegisterDonationTests(void); void RegisterNotificationTests(void);
void RegisterEmergencyAlertTests(void); void RegisterLinkedListTests(void);
void RegisterQueueTests(void); void RegisterHashTableTests(void);
void RegisterGraphTests(void); void RegisterUtilityTests(void);
void RegisterFileManagementTests(void); void RegisterReportTests(void);
void RegisterDonationCampTests(void); void RegisterLoggingTests(void);
int main(void){
 if(CU_initialize_registry()!=CUE_SUCCESS) return (int)CU_get_error();
 RegisterValidationTests(); RegisterAuthenticationTests(); RegisterDonorTests();
 RegisterInventoryTests(); RegisterRequestTests(); RegisterHospitalTests();
 RegisterDonationTests(); RegisterNotificationTests(); RegisterEmergencyAlertTests();
 RegisterLinkedListTests(); RegisterQueueTests(); RegisterHashTableTests();
 RegisterGraphTests(); RegisterUtilityTests(); RegisterFileManagementTests(); RegisterReportTests(); RegisterDonationCampTests(); RegisterLoggingTests();
 CU_basic_set_mode(CU_BRM_VERBOSE); CU_basic_run_tests();
 unsigned int failed=CU_get_number_of_failures(); CU_cleanup_registry(); return failed==0U?0:1;
}
