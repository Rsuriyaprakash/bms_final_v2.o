/**
 * @file utility.c
 * @author Suriya Prakash R
 * @version 2.0.0
 * @brief Safe general-purpose utility implementation.
 */
#include "utility.h"

#include <ctype.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

BmsStatus_t UtilitySafeStringCopy(char *destination, size_t destinationSize,
                                  const char *source)
{
    size_t length;
    if ((destination == NULL) || (source == NULL) || (destinationSize == 0U))
    {
        return BMS_STATUS_INVALID_ARGUMENT;
    }
    length = strlen(source);
    if (length >= destinationSize)
    {
        destination[0] = '\0';
        return BMS_STATUS_BUFFER_TOO_SMALL;
    }
    (void)memcpy(destination, source, length + 1U);
    return BMS_STATUS_OK;
}

BmsStatus_t UtilityReadLine(char *buffer, size_t bufferSize)
{
    size_t length;
    if ((buffer == NULL) || (bufferSize < 2U))
    {
        return BMS_STATUS_INVALID_ARGUMENT;
    }
    if (fgets(buffer, (int)bufferSize, stdin) == NULL)
    {
        return BMS_STATUS_READ_ERROR;
    }
    length = strlen(buffer);
    if ((length > 0U) && (buffer[length - 1U] == '\n'))
    {
        buffer[length - 1U] = '\0';
    }
    else
    {
        UtilityClearInputBuffer();
    }
    return BMS_STATUS_OK;
}

BmsStatus_t UtilityParseUint32(const char *text, uint32_t *value)
{
    char *endPointer = NULL;
    unsigned long parsed;
    if ((text == NULL) || (value == NULL) || (*text == '\0'))
    {
        return BMS_STATUS_INVALID_ARGUMENT;
    }
    errno = 0;
    parsed = strtoul(text, &endPointer, 10);
    if ((errno != 0) || (endPointer == text) || (*endPointer != '\0') ||
        (parsed > UINT32_MAX))
    {
        return BMS_STATUS_INVALID_DATA;
    }
    *value = (uint32_t)parsed;
    return BMS_STATUS_OK;
}

BmsStatus_t UtilityReadUint32(const char *prompt, uint32_t *value)
{
    char buffer[32];
    if (value == NULL)
    {
        return BMS_STATUS_INVALID_ARGUMENT;
    }
    if (prompt != NULL)
    {
        (void)printf("%s", prompt);
    }
    if (UtilityReadLine(buffer, sizeof(buffer)) != BMS_STATUS_OK)
    {
        return BMS_STATUS_READ_ERROR;
    }
    UtilityTrimWhitespace(buffer);
    return UtilityParseUint32(buffer, value);
}

void UtilityTrimWhitespace(char *text)
{
    char *start;
    char *end;
    size_t length;
    if (text == NULL)
    {
        return;
    }
    start = text;
    while ((*start != '\0') && (isspace((unsigned char)*start) != 0))
    {
        ++start;
    }
    if (start != text)
    {
        (void)memmove(text, start, strlen(start) + 1U);
    }
    length = strlen(text);
    if (length == 0U)
    {
        return;
    }
    end = text + length - 1U;
    while ((end >= text) && (isspace((unsigned char)*end) != 0))
    {
        *end = '\0';
        if (end == text) { break; }
        --end;
    }
}

void UtilityClearInputBuffer(void)
{
    int character;
    do
    {
        character = getchar();
    } while ((character != '\n') && (character != EOF));
}

void UtilitySecureZero(void *buffer, size_t bufferSize)
{
    volatile unsigned char *pointer = (volatile unsigned char *)buffer;
    if (pointer == NULL) { return; }
    while (bufferSize > 0U)
    {
        *pointer = 0U;
        ++pointer;
        --bufferSize;
    }
}

uint32_t UtilityCalculateChecksum(const void *data, size_t size)
{
    const uint8_t *bytes = (const uint8_t *)data;
    uint32_t hash = 2166136261UL;
    size_t index;
    if (bytes == NULL) { return 0U; }
    for (index = 0U; index < size; ++index)
    {
        hash ^= bytes[index];
        hash *= 16777619UL;
    }
    return hash;
}

bool UtilityIsLeapYear(uint16_t year)
{
    return (((year % 4U) == 0U) && (((year % 100U) != 0U) || ((year % 400U) == 0U)));
}

uint8_t UtilityDaysInMonth(uint16_t year, uint8_t month)
{
    static const uint8_t days[12] =
        {31U,28U,31U,30U,31U,30U,31U,31U,30U,31U,30U,31U};
    if ((month == 0U) || (month > 12U)) { return 0U; }
    if ((month == 2U) && UtilityIsLeapYear(year)) { return 29U; }
    return days[month - 1U];
}

int32_t UtilityCompareDates(const BmsDate_t *left, const BmsDate_t *right)
{
    if ((left == NULL) || (right == NULL)) { return 0; }
    if (left->year != right->year) { return (left->year < right->year) ? -1 : 1; }
    if (left->month != right->month) { return (left->month < right->month) ? -1 : 1; }
    if (left->day != right->day) { return (left->day < right->day) ? -1 : 1; }
    return 0;
}

const char *UtilityStatusToString(BmsStatus_t status)
{
    static const char *const names[] = {
        "OK","Invalid argument","Invalid data","Not initialized","Already initialized",
        "Not found","Already exists","Memory error","File error","File not found",
        "File corrupt","Read error","Write error","Authentication failed","Access denied",
        "Account locked","Password policy failed","Insufficient stock","Expired",
        "Queue full","Queue empty","Hash table full","Graph full","Route not found",
        "Buffer too small","Unsupported","Internal error"
    };
    uint32_t index = (uint32_t)status;
    return (index < (sizeof(names) / sizeof(names[0]))) ? names[index] : "Unknown";
}

const char *UtilityBloodGroupToString(BmsBloodGroup_t bloodGroup)
{
    static const char *const names[] = {"INVALID","A+","A-","B+","B-","AB+","AB-","O+","O-"};
    uint32_t index = (uint32_t)bloodGroup;
    return (index < (sizeof(names) / sizeof(names[0]))) ? names[index] : "INVALID";
}

BmsStatus_t UtilityStringToBloodGroup(const char *text, BmsBloodGroup_t *bloodGroup)
{
    BmsBloodGroup_t group;
    if ((text == NULL) || (bloodGroup == NULL)) { return BMS_STATUS_INVALID_ARGUMENT; }
    for (group = BMS_BLOOD_GROUP_A_POSITIVE; group < BMS_BLOOD_GROUP_COUNT;
         group = (BmsBloodGroup_t)((uint32_t)group + 1U))
    {
        if (strcmp(text, UtilityBloodGroupToString(group)) == 0)
        {
            *bloodGroup = group;
            return BMS_STATUS_OK;
        }
    }
    return BMS_STATUS_INVALID_DATA;
}
