/**
 * @file common.h
 * @author Suriya Prakash R
 * @version 2.0.0
 * @brief Shared types, constants, and model definitions for BMS Version 2.
 */
//header guard
#ifndef BMS_COMMON_H
#define BMS_COMMON_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "config.h"
#include "common_error_codes.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef uint32_t BmsUserId_t;
typedef uint32_t BmsDonorId_t;
typedef uint32_t BmsHospitalId_t;
typedef uint32_t BmsBloodId_t;
typedef uint32_t BmsRequestId_t;
typedef uint32_t BmsDonationId_t;
typedef uint32_t BmsNotificationId_t;
typedef uint32_t BmsAlertId_t;
typedef uint32_t BmsCampId_t;

/**
 * @brief Supported application roles.
 */

/** @brief User-account approval and security state. */
typedef enum
{
    BMS_USER_STATUS_INVALID = 0,
    BMS_USER_STATUS_PENDING,
    BMS_USER_STATUS_ACTIVE,
    BMS_USER_STATUS_LOCKED,
    BMS_USER_STATUS_DISABLED
} BmsUserStatus_t;

typedef enum
{
    BMS_ROLE_INVALID = 0,
    BMS_ROLE_ADMIN = 1,
    BMS_ROLE_HOSPITAL_STAFF = 2,
    BMS_ROLE_BLOOD_BANK_STAFF = 3,
    BMS_ROLE_DONOR = 4
} BmsUserRole_t;

/**
 * @brief Supported blood groups.
 */
typedef enum
{
    BMS_BLOOD_GROUP_INVALID = 0,
    BMS_BLOOD_GROUP_A_POSITIVE,
    BMS_BLOOD_GROUP_A_NEGATIVE,
    BMS_BLOOD_GROUP_B_POSITIVE,
    BMS_BLOOD_GROUP_B_NEGATIVE,
    BMS_BLOOD_GROUP_AB_POSITIVE,
    BMS_BLOOD_GROUP_AB_NEGATIVE,
    BMS_BLOOD_GROUP_O_POSITIVE,
    BMS_BLOOD_GROUP_O_NEGATIVE,
    BMS_BLOOD_GROUP_A1_POSITIVE,
    BMS_BLOOD_GROUP_COUNT
} BmsBloodGroup_t;

/**
 * @brief Blood request processing state.
 */
typedef enum
{
    BMS_REQUEST_STATUS_INVALID = 0,
    BMS_REQUEST_STATUS_PENDING,
    BMS_REQUEST_STATUS_APPROVED,
    BMS_REQUEST_STATUS_REJECTED,
    BMS_REQUEST_STATUS_PROCESSING,
    BMS_REQUEST_STATUS_FULFILLED,
    BMS_REQUEST_STATUS_CANCELLED
} BmsRequestStatus_t;

/**
 * @brief Notification delivery channel.
 */
typedef enum
{
    BMS_NOTIFICATION_CHANNEL_CONSOLE = 0,
    BMS_NOTIFICATION_CHANNEL_EMAIL,
    BMS_NOTIFICATION_CHANNEL_SMS,
    BMS_NOTIFICATION_CHANNEL_BROADCAST
} BmsNotificationChannel_t;

/**
 * @brief Notification priority.
 */
typedef enum
{
    BMS_PRIORITY_LOW = 0,
    BMS_PRIORITY_NORMAL,
    BMS_PRIORITY_HIGH,
    BMS_PRIORITY_EMERGENCY
} BmsPriority_t;

/**
 * @brief ISO-like date record.
 */
typedef struct
{
    uint16_t year;
    uint8_t month;
    uint8_t day;
} BmsDate_t;

/**
 * @brief User account record.
 */
typedef struct
{
    BmsUserId_t userId;
    char username[BMS_MAX_USERNAME_LENGTH + 1U];
    char passwordHash[BMS_MAX_PASSWORD_LENGTH + 1U];
    char recoveryCodeHash[BMS_MAX_PASSWORD_LENGTH + 1U];
    BmsUserRole_t role;
    BmsUserStatus_t status;
    BmsDonorId_t linkedDonorId;
    BmsHospitalId_t hospitalId;
    uint8_t failedLoginAttempts;
    bool passwordChangeRequired;
    bool isLocked;
    bool isActive;
} BmsUser_t;

/**
 * @brief Donor record.
 */
typedef struct
{
    BmsDonorId_t donorId;
    char name[BMS_MAX_NAME_LENGTH + 1U];
    uint8_t age;
    uint16_t weightKg;
    BmsBloodGroup_t bloodGroup;
    char phone[BMS_MAX_PHONE_LENGTH + 1U];
    char email[BMS_MAX_EMAIL_LENGTH + 1U];
    char address[BMS_MAX_ADDRESS_LENGTH + 1U];
    BmsDate_t lastDonationDate;
    bool isEligible;
    bool isActive;
} BmsDonor_t;

/**
 * @brief Hospital record.
 */
typedef struct
{
    BmsHospitalId_t hospitalId;
    char name[BMS_MAX_NAME_LENGTH + 1U];
    char location[BMS_MAX_LOCATION_LENGTH + 1U];
    char address[BMS_MAX_ADDRESS_LENGTH + 1U];
    char contactNumber[BMS_MAX_PHONE_LENGTH + 1U];
    char email[BMS_MAX_EMAIL_LENGTH + 1U];
    bool isActive;
} BmsHospital_t;

/**
 * @brief Blood inventory record.
 */
typedef struct
{
    BmsBloodId_t bloodId;
    BmsBloodGroup_t bloodGroup;
    uint32_t units;
    BmsDate_t collectionDate;
    BmsDate_t expiryDate;
    bool isAvailable;
} BmsBloodInventory_t;

/**
 * @brief Blood donation record.
 */
typedef struct
{
    BmsDonationId_t donationId;
    BmsDonorId_t donorId;
    BmsBloodGroup_t bloodGroup;
    uint32_t units;
    BmsDate_t donationDate;
    BmsHospitalId_t collectionHospitalId;
} BmsDonation_t;

/**
 * @brief Blood request record.
 */
typedef struct
{
    BmsRequestId_t requestId;
    BmsHospitalId_t requesterHospitalId;
    BmsUserId_t requesterId;
    BmsBloodGroup_t bloodGroup;
    uint32_t requestedUnits;
    uint32_t fulfilledUnits;
    BmsPriority_t priority;
    BmsRequestStatus_t status;
    BmsDate_t requestDate;
    BmsDate_t requiredByDate;
} BmsBloodRequest_t;

/**
 * @brief Notification record.
 */
typedef struct
{
    BmsNotificationId_t notificationId;
    BmsUserId_t recipientUserId;
    BmsHospitalId_t recipientHospitalId;
    BmsNotificationChannel_t channel;
    BmsPriority_t priority;
    char message[BMS_MAX_MESSAGE_LENGTH + 1U];
    bool processed;
} BmsNotification_t;

/**
 * @brief Emergency alert record.
 */
typedef struct
{
    BmsAlertId_t alertId;
    BmsRequestId_t requestId; /* optional; 0 means independent emergency */
    BmsHospitalId_t sourceHospitalId;
    BmsUserId_t createdByUserId;
    BmsBloodGroup_t bloodGroup;
    uint32_t requiredUnits;
    BmsPriority_t priority;
    char message[BMS_MAX_MESSAGE_LENGTH + 1U];
    bool resolved;
} BmsEmergencyAlert_t;

typedef struct
{
    BmsCampId_t campId;
    char name[BMS_MAX_NAME_LENGTH + 1U];
    BmsDate_t date;
    char time[16U];
    char venue[BMS_MAX_ADDRESS_LENGTH + 1U];
    char city[BMS_MAX_LOCATION_LENGTH + 1U];
    char organizer[BMS_MAX_NAME_LENGTH + 1U];
    char contactNumber[BMS_MAX_PHONE_LENGTH + 1U];
    char email[BMS_MAX_EMAIL_LENGTH + 1U];
    bool isActive;
} BmsDonationCamp_t;

/**
 * @brief Persistent file header.
 */
typedef struct
{
    uint32_t magic;
    uint16_t formatVersion;
    uint16_t recordSize;
    uint32_t recordCount;
    uint32_t checksum;
} BmsFileHeader_t;

#ifdef __cplusplus
}
#endif

#endif /* BMS_COMMON_H */
