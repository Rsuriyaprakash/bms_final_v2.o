/**
 * @file queue_management.h
 * @author Suriya Prakash R
 * @version 2.0.0
 * @brief Generic fixed-capacity circular queue abstraction.
 */

#ifndef BMS_QUEUE_MANAGEMENT_H
#define BMS_QUEUE_MANAGEMENT_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "common_error_codes.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct
{
    uint8_t *storage;
    size_t elementSize;
    uint32_t capacity;
    uint32_t front;
    uint32_t rear;
    uint32_t count;
} BmsQueue_t;

BmsStatus_t QueueInitialize(BmsQueue_t *queue,
                            uint32_t capacity,
                            size_t elementSize);
BmsStatus_t QueueEnqueue(BmsQueue_t *queue, const void *element);
BmsStatus_t QueueDequeue(BmsQueue_t *queue, void *element);
BmsStatus_t QueuePeek(const BmsQueue_t *queue, void *element);
bool QueueIsEmpty(const BmsQueue_t *queue);
bool QueueIsFull(const BmsQueue_t *queue);
uint32_t QueueGetCount(const BmsQueue_t *queue);
void QueueClear(BmsQueue_t *queue);
void QueueDeinitialize(BmsQueue_t *queue);

#ifdef __cplusplus
}
#endif

#endif /* BMS_QUEUE_MANAGEMENT_H */
