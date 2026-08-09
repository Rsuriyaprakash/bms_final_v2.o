/**
 * @file multithreading.h
 * @brief Background inventory monitoring thread for the BMS.
 */
#ifndef BMS_MULTITHREADING_H
#define BMS_MULTITHREADING_H

#include <stdbool.h>
#include <stdint.h>
#include <pthread.h>

#include "blood_inventory.h"
#include "common.h"

typedef struct
{
    pthread_t thread;
    pthread_mutex_t controlMutex;
    pthread_cond_t stopCondition;
    BmsInventoryContext_t *inventory;
    uint32_t intervalSeconds;
    uint32_t lowStockThreshold;
    uint32_t lastExpiredCount;
    uint32_t lastLowStockCount;
    bool started;
    bool stopRequested;
} BmsThreadController_t;

BmsStatus_t BmsThreadControllerStart(BmsThreadController_t *controller,
                                     BmsInventoryContext_t *inventory,
                                     uint32_t intervalSeconds,
                                     uint32_t lowStockThreshold);
void BmsThreadControllerStop(BmsThreadController_t *controller);

#endif
