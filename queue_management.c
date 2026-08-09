/** @file queue_management.c @brief Circular queue implementation. */
#include "queue_management.h"
#include "logging.h"
#include <stdlib.h>
#include <string.h>

BmsStatus_t QueueInitialize(BmsQueue_t *queue, uint32_t capacity, size_t elementSize)
{
    if ((queue == NULL) || (capacity == 0U) || (elementSize == 0U))
    { return BMS_STATUS_INVALID_ARGUMENT; }
    (void)memset(queue, 0, sizeof(*queue));
    queue->storage = (uint8_t *)calloc(capacity, elementSize);
    if (queue->storage == NULL) { return BMS_STATUS_MEMORY_ERROR; }
    queue->capacity = capacity; queue->elementSize = elementSize;
    return BMS_STATUS_OK;
}
BmsStatus_t QueueEnqueue(BmsQueue_t *queue, const void *element)
{
    if ((queue == NULL) || (element == NULL) || (queue->storage == NULL))
    { return BMS_STATUS_INVALID_ARGUMENT; }
    if (QueueIsFull(queue))
    {
        BMS_LOG_WARNING("QUEUE", "Queue full: capacity=%u",
                        (unsigned int)queue->capacity);
        return BMS_STATUS_QUEUE_FULL;
    }
    (void)memcpy(queue->storage + ((size_t)queue->rear * queue->elementSize),
                 element, queue->elementSize);
    queue->rear = (queue->rear + 1U) % queue->capacity; ++queue->count;
    return BMS_STATUS_OK;
}
BmsStatus_t QueueDequeue(BmsQueue_t *queue, void *element)
{
    if ((queue == NULL) || (element == NULL) || (queue->storage == NULL))
    { return BMS_STATUS_INVALID_ARGUMENT; }
    if (QueueIsEmpty(queue))
    {
        BMS_LOG_DEBUG("QUEUE", "Queue empty on dequeue/peek");
        return BMS_STATUS_QUEUE_EMPTY;
    }
    (void)memcpy(element, queue->storage + ((size_t)queue->front * queue->elementSize),
                 queue->elementSize);
    queue->front = (queue->front + 1U) % queue->capacity; --queue->count;
    return BMS_STATUS_OK;
}
BmsStatus_t QueuePeek(const BmsQueue_t *queue, void *element)
{
    if ((queue == NULL) || (element == NULL) || (queue->storage == NULL))
    { return BMS_STATUS_INVALID_ARGUMENT; }
    if (QueueIsEmpty(queue))
    {
        BMS_LOG_DEBUG("QUEUE", "Queue empty on dequeue/peek");
        return BMS_STATUS_QUEUE_EMPTY;
    }
    (void)memcpy(element, queue->storage + ((size_t)queue->front * queue->elementSize),
                 queue->elementSize);
    return BMS_STATUS_OK;
}
bool QueueIsEmpty(const BmsQueue_t *queue) { return (queue == NULL) || (queue->count == 0U); }
bool QueueIsFull(const BmsQueue_t *queue) { return (queue != NULL) && (queue->count == queue->capacity); }
uint32_t QueueGetCount(const BmsQueue_t *queue) { return (queue == NULL) ? 0U : queue->count; }
void QueueClear(BmsQueue_t *queue)
{
    if (queue == NULL) { return; }
    queue->front = 0U; queue->rear = 0U; queue->count = 0U;
    if (queue->storage != NULL) { (void)memset(queue->storage, 0, queue->capacity * queue->elementSize); }
}
void QueueDeinitialize(BmsQueue_t *queue)
{
    if (queue == NULL) { return; }
    free(queue->storage); (void)memset(queue, 0, sizeof(*queue));
}
