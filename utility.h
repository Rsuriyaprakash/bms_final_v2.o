/**
 * @file utility.h
 * @author Suriya Prakash R
 * @version 2.0.0
 * @brief General-purpose safe utility functions.
 */

#ifndef BMS_UTILITY_H
#define BMS_UTILITY_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "common.h"

#ifdef __cplusplus
extern "C" {
#endif

BmsStatus_t UtilitySafeStringCopy(char *destination,
                                  size_t destinationSize,
                                  const char *source);

BmsStatus_t UtilityReadLine(char *buffer, size_t bufferSize);
BmsStatus_t UtilityReadUint32(const char *prompt, uint32_t *value);
BmsStatus_t UtilityParseUint32(const char *text, uint32_t *value);

void UtilityTrimWhitespace(char *text);
void UtilityClearInputBuffer(void);
void UtilitySecureZero(void *buffer, size_t bufferSize);

uint32_t UtilityCalculateChecksum(const void *data, size_t size);
bool UtilityIsLeapYear(uint16_t year);
uint8_t UtilityDaysInMonth(uint16_t year, uint8_t month);
int32_t UtilityCompareDates(const BmsDate_t *left, const BmsDate_t *right);

const char *UtilityStatusToString(BmsStatus_t status);
const char *UtilityBloodGroupToString(BmsBloodGroup_t bloodGroup);
BmsStatus_t UtilityStringToBloodGroup(const char *text,
                                      BmsBloodGroup_t *bloodGroup);

#ifdef __cplusplus
}
#endif

#endif /* BMS_UTILITY_H */
