#include "test_common.h"
#include "utility.h"
static void test_parse_trim(void){uint32_t v=0;char text[32]="  12345  ";ASSERT_OK(UtilityParseUint32("12345",&v));CU_ASSERT_EQUAL(v,12345U);CU_ASSERT_NOT_EQUAL(UtilityParseUint32("12A",&v),BMS_STATUS_OK);UtilityTrimWhitespace(text);CU_ASSERT_STRING_EQUAL(text,"12345");}
static void test_dates(void){BmsDate_t a={2024U,2U,29U},b={2025U,1U,1U};CU_ASSERT_TRUE(UtilityIsLeapYear(2024U));CU_ASSERT_FALSE(UtilityIsLeapYear(2023U));CU_ASSERT_EQUAL(UtilityDaysInMonth(2024U,2U),29U);CU_ASSERT(UtilityCompareDates(&a,&b)<0);}
static void test_checksum_zero(void){char x[8]="secret";uint32_t c1=UtilityCalculateChecksum(x,strlen(x));uint32_t c2=UtilityCalculateChecksum(x,strlen(x));CU_ASSERT_EQUAL(c1,c2);UtilitySecureZero(x,sizeof x);for(size_t i=0;i<sizeof x;i++)CU_ASSERT_EQUAL(x[i],0);}
void RegisterUtilityTests(void){CU_pSuite s=CU_add_suite("Utility",NULL,NULL);CU_add_test(s,"parse and trim",test_parse_trim);CU_add_test(s,"date helpers",test_dates);CU_add_test(s,"checksum and secure zero",test_checksum_zero);}
