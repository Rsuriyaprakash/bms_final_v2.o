#include "test_common.h"
#include "queue_management.h"
static void test_fifo_bounds(void){BmsQueue_t q;int a=1,b=2,c=0;ASSERT_OK(QueueInitialize(&q,2U,sizeof(int)));CU_ASSERT_TRUE(QueueIsEmpty(&q));ASSERT_OK(QueueEnqueue(&q,&a));ASSERT_OK(QueueEnqueue(&q,&b));CU_ASSERT_TRUE(QueueIsFull(&q));ASSERT_STATUS(BMS_STATUS_QUEUE_FULL,QueueEnqueue(&q,&a));ASSERT_OK(QueuePeek(&q,&c));CU_ASSERT_EQUAL(c,1);ASSERT_OK(QueueDequeue(&q,&c));CU_ASSERT_EQUAL(c,1);ASSERT_OK(QueueDequeue(&q,&c));CU_ASSERT_EQUAL(c,2);ASSERT_STATUS(BMS_STATUS_QUEUE_EMPTY,QueueDequeue(&q,&c));QueueClear(&q);QueueDeinitialize(&q);}
void RegisterQueueTests(void){CU_pSuite s=CU_add_suite("Queue",NULL,NULL);CU_add_test(s,"FIFO and boundaries",test_fifo_bounds);}
