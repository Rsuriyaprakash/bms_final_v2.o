
#include "test_common.h"
#include "notification_management.h"

static BmsStatus_t sender(const BmsNotification_t*n,void*c)
{
    CU_ASSERT_PTR_NOT_NULL(n);
    (*(uint32_t*)c)++;
    return BMS_STATUS_OK;
}

static void test_queue_process(void)
{
    BmsNotificationContext_t c;
    BmsNotification_t n,out;
    uint32_t count=0,processed=0;

    memset(&n,0,sizeof n);
    n.notificationId=1;
    n.channel=BMS_NOTIFICATION_CHANNEL_CONSOLE;
    n.priority=BMS_PRIORITY_HIGH;
    snprintf(n.message,sizeof(n.message),"Emergency alert");

    ASSERT_OK(NotificationManagementInitialize(&c));
    ASSERT_OK(NotificationManagementEnqueue(&c,&n));
    ASSERT_OK(NotificationManagementPeek(&c,&out));
    CU_ASSERT_EQUAL(out.notificationId,1U);
    ASSERT_OK(NotificationManagementProcessNext(&c,sender,&count));
    CU_ASSERT_EQUAL(count,1U);

    ASSERT_OK(NotificationManagementEnqueue(&c,&n));
    ASSERT_OK(NotificationManagementProcessAll(&c,sender,&count,&processed));
    CU_ASSERT_EQUAL(processed,1U);
    NotificationManagementDeinitialize(&c);
}

static void test_empty_queue_and_multiple_messages(void)
{
    BmsNotificationContext_t c;
    BmsNotification_t n,out;
    uint32_t count=0,processed=0;

    memset(&n,0,sizeof n);
    n.notificationId=10;
    n.channel=BMS_NOTIFICATION_CHANNEL_CONSOLE;
    n.priority=BMS_PRIORITY_NORMAL;
    snprintf(n.message,sizeof(n.message),"A");

    ASSERT_OK(NotificationManagementInitialize(&c));

    CU_ASSERT_NOT_EQUAL(NotificationManagementPeek(&c,&out),BMS_STATUS_OK);
    CU_ASSERT_NOT_EQUAL(NotificationManagementProcessNext(&c,sender,&count),BMS_STATUS_OK);

    n.notificationId=11;
    snprintf(n.message,sizeof(n.message),"B");
    ASSERT_OK(NotificationManagementEnqueue(&c,&n));
    n.notificationId=12;
    snprintf(n.message,sizeof(n.message),"C");
    ASSERT_OK(NotificationManagementEnqueue(&c,&n));

    ASSERT_OK(NotificationManagementProcessAll(&c,sender,&count,&processed));
    CU_ASSERT_EQUAL(processed,2U);
    CU_ASSERT_EQUAL(count,2U);

    NotificationManagementDeinitialize(&c);
}

void RegisterNotificationTests(void)
{
    CU_pSuite s=CU_add_suite("Notification",NULL,NULL);
    CU_add_test(s,"enqueue peek process",test_queue_process);
    CU_add_test(s,"empty queue and multiple messages",test_empty_queue_and_multiple_messages);
}
