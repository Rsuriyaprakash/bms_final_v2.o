#include "logging.h"

int main(void)
{
    BmsStatus_t status = LoggingInitialize();

    if (status == BMS_STATUS_OK)
    {
        (void)LoggingWrite(BMS_LOG_LEVEL_DEBUG, "SMOKE", __func__, "Debug logging enabled in development build");
        (void)LoggingWrite(BMS_LOG_LEVEL_INFO, "SMOKE", __func__, "Information event");
        (void)LoggingWrite(BMS_LOG_LEVEL_WARNING, "SMOKE", __func__, "Warning event");
        (void)LoggingWrite(BMS_LOG_LEVEL_ERROR, "SMOKE", __func__, "Error-tracing sample");
        LoggingDeinitialize();
        return 0;
    }

    return 1;
}
