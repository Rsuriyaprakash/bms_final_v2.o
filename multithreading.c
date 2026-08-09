/**
 * @file multithreading.c
 * @brief POSIX-thread based background inventory monitoring.
 */
#include "multithreading.h"
#include "logging.h"

#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

static bool GetCurrentDate(BmsDate_t *date)
{
    time_t now;
    struct tm localTime;

    if (date == NULL)
    {
        return false;
    }

    now = time(NULL);
    if ((now == (time_t)-1) || (localtime_r(&now, &localTime) == NULL))
    {
        return false;
    }

    date->year = (uint16_t)(localTime.tm_year + 1900);
    date->month = (uint8_t)(localTime.tm_mon + 1);
    date->day = (uint8_t)localTime.tm_mday;
    return true;
}

static void RunInventoryCheck(BmsThreadController_t *controller)
{
    BmsDate_t currentDate = { 0U, 0U, 0U };
    uint32_t expiredCount = 0U;
    uint32_t lowStockCount = 0U;

    if ((controller == NULL) || (controller->inventory == NULL))
    {
        BMS_LOG_ERROR("THREAD", "Inventory monitor received invalid context");
        return;
    }

    if (!GetCurrentDate(&currentDate))
    {
        BMS_LOG_ERROR("THREAD", "Inventory monitor could not obtain current date");
        return;
    }

    if (BloodInventoryRemoveExpired(controller->inventory,
                                    &currentDate,
                                    &expiredCount) == BMS_STATUS_OK)
    {
        controller->lastExpiredCount = expiredCount;
        if (expiredCount > 0U)
        {
            (void)BloodInventorySave(controller->inventory);
        }
    }

    if (BloodInventoryCountLowStock(controller->inventory,
                                    controller->lowStockThreshold,
                                    &lowStockCount) == BMS_STATUS_OK)
    {
        char message[128];

        controller->lastLowStockCount = lowStockCount;

        (void)snprintf(message,
                       sizeof(message),
                       "Inventory check complete: expired=%u, low-stock=%u",
                       (unsigned int)controller->lastExpiredCount,
                       (unsigned int)controller->lastLowStockCount);
        (void)LoggingWrite(BMS_LOG_LEVEL_INFO,
                           "MULTITHREAD",
                           "RunInventoryCheck",
                           message);
    }
}

static void *InventoryMonitorMain(void *argument)
{
    BmsThreadController_t *controller = (BmsThreadController_t *)argument;

    if (controller == NULL)
    {
        return NULL;
    }

    RunInventoryCheck(controller);

    (void)pthread_mutex_lock(&controller->controlMutex);
    while (!controller->stopRequested)
    {
        struct timespec wakeTime;
        int waitResult;

        if (clock_gettime(CLOCK_REALTIME, &wakeTime) != 0)
        {
            break;
        }
        wakeTime.tv_sec += (time_t)controller->intervalSeconds;

        waitResult = pthread_cond_timedwait(&controller->stopCondition,
                                            &controller->controlMutex,
                                            &wakeTime);
        if (controller->stopRequested)
        {
            break;
        }
        if ((waitResult == ETIMEDOUT) || (waitResult == 0))
        {
            (void)pthread_mutex_unlock(&controller->controlMutex);
            RunInventoryCheck(controller);
            (void)pthread_mutex_lock(&controller->controlMutex);
        }
    }
    (void)pthread_mutex_unlock(&controller->controlMutex);
    return NULL;
}

BmsStatus_t BmsThreadControllerStart(BmsThreadController_t *controller,
                                     BmsInventoryContext_t *inventory,
                                     uint32_t intervalSeconds,
                                     uint32_t lowStockThreshold)
{
    int result;

    if ((controller == NULL) || (inventory == NULL) ||
        (intervalSeconds == 0U))
    {
        return BMS_STATUS_INVALID_ARGUMENT;
    }

    (void)memset(controller, 0, sizeof(*controller));
    controller->inventory = inventory;
    controller->intervalSeconds = intervalSeconds;
    controller->lowStockThreshold = lowStockThreshold;

    if (pthread_mutex_init(&controller->controlMutex, NULL) != 0)
    {
        return BMS_STATUS_INTERNAL_ERROR;
    }
    if (pthread_cond_init(&controller->stopCondition, NULL) != 0)
    {
        (void)pthread_mutex_destroy(&controller->controlMutex);
        return BMS_STATUS_INTERNAL_ERROR;
    }

    result = pthread_create(&controller->thread,
                            NULL,
                            InventoryMonitorMain,
                            controller);
    if (result != 0)
    {
        (void)pthread_cond_destroy(&controller->stopCondition);
        (void)pthread_mutex_destroy(&controller->controlMutex);
        return BMS_STATUS_INTERNAL_ERROR;
    }

    controller->started = true;
    (void)LoggingWrite(BMS_LOG_LEVEL_INFO,
                       "MULTITHREAD",
                       "BmsThreadControllerStart",
                       "Background inventory monitor started");
    return BMS_STATUS_OK;
}

void BmsThreadControllerStop(BmsThreadController_t *controller)
{
    if ((controller != NULL) && controller->started)
    {
        (void)pthread_mutex_lock(&controller->controlMutex);
        controller->stopRequested = true;
        (void)pthread_cond_signal(&controller->stopCondition);
        (void)pthread_mutex_unlock(&controller->controlMutex);
        (void)pthread_join(controller->thread, NULL);
        (void)pthread_cond_destroy(&controller->stopCondition);
        (void)pthread_mutex_destroy(&controller->controlMutex);
        controller->started = false;
        (void)LoggingWrite(BMS_LOG_LEVEL_INFO,
                           "MULTITHREAD",
                           "BmsThreadControllerStop",
                           "Background inventory monitor stopped");
    }
}
