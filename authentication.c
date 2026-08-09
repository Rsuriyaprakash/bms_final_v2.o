/** @file authentication.c @brief Persistent authentication implementation. */
#include "authentication.h"
#include "logging.h"
#include "common_validation.h"
#include "file_names.h"
#include "utility.h"

#include <stdio.h>
#include <string.h>

#define BMS_AUTH_FILE_MAGIC (0x42555352UL)
#define BMS_AUTH_FILE_VERSION (2U)

typedef struct
{
    uint32_t magic;
    uint16_t version;
    uint16_t recordSize;
    uint32_t recordCount;
} BmsAuthenticationFileHeader_t;

static bool MatchId(const void *data, const void *key)
{
    bool result = false;
    if ((data != NULL) && (key != NULL))
    {
        result = (((const BmsUser_t *)data)->userId ==
                  (*(const BmsUserId_t *)key));
    }
    return result;
}

static bool MatchName(const void *data, const void *key)
{
    bool result = false;
    if ((data != NULL) && (key != NULL))
    {
        result = (strcmp(((const BmsUser_t *)data)->username,
                         (const char *)key) == 0);
    }
    return result;
}

static void HashSecret(const char *plainText, char *output, size_t outputSize)
{
    if ((plainText != NULL) && (output != NULL) && (outputSize > 0U))
    {
        const uint32_t checksum =
            UtilityCalculateChecksum(plainText, strlen(plainText));
        (void)snprintf(output, outputSize, "%08lX",
                       (unsigned long)checksum);
    }
}

static BmsStatus_t FindMutableUser(BmsAuthenticationContext_t *context,
                                   BmsUserId_t userId,
                                   BmsUser_t **user)
{
    void *found = NULL;
    BmsStatus_t status = BMS_STATUS_INVALID_ARGUMENT;
    if ((context != NULL) && (user != NULL))
    {
        status = LinkedListFind(&context->users, MatchId, &userId, &found);
        if (status == BMS_STATUS_OK)
        {
            *user = (BmsUser_t *)found;
        }
    }
    return status;
}

BmsStatus_t AuthenticationInitialize(BmsAuthenticationContext_t *context)
{
    BmsStatus_t status = BMS_STATUS_INVALID_ARGUMENT;
    if (context != NULL)
    {
        (void)memset(context, 0, sizeof(*context));
        status = LinkedListInitialize(&context->users, sizeof(BmsUser_t));
        if (status == BMS_STATUS_OK)
        {
            status = HashTableInitialize(&context->usernameIndex,
                                         BMS_HASH_BUCKET_COUNT,
                                         sizeof(BmsUser_t *),
                                         BMS_HASH_KEY_STRING);
        }
        if (status == BMS_STATUS_OK)
        {
            context->initialized = true;
        }
    }
    return status;
}

BmsStatus_t AuthenticationLoad(BmsAuthenticationContext_t *context)
{
    FILE *file = NULL;
    BmsAuthenticationFileHeader_t header = { 0U, 0U, 0U, 0U };
    BmsStatus_t status = BMS_STATUS_INVALID_ARGUMENT;
    if ((context != NULL) && context->initialized)
    {
        file = fopen(BMS_USERS_FILE, "rb");
        if (file == NULL)
        {
            status = BMS_STATUS_FILE_NOT_FOUND;
        }
        else if (fread(&header, sizeof(header), 1U, file) != 1U)
        {
            status = BMS_STATUS_READ_ERROR;
        }
        else if ((header.magic != BMS_AUTH_FILE_MAGIC) ||
                 (header.version != BMS_AUTH_FILE_VERSION) ||
                 (header.recordSize != (uint16_t)sizeof(BmsUser_t)))
        {
            status = BMS_STATUS_FILE_CORRUPT;
        }
        else
        {
            LinkedListClear(&context->users);
            status = BMS_STATUS_OK;
            for (uint32_t index = 0U; (index < header.recordCount) &&
                            (status == BMS_STATUS_OK); ++index)
            {
                BmsUser_t user;
                (void)memset(&user, 0, sizeof(user));
                if (fread(&user, sizeof(user), 1U, file) != 1U)
                {
                    status = BMS_STATUS_READ_ERROR;
                }
                else
                {
                    status = LinkedListInsertBack(&context->users, &user);
                }
            }
        }
        if ((file != NULL) && (fclose(file) != 0) &&
            (status == BMS_STATUS_OK))
        {
            status = BMS_STATUS_FILE_ERROR;
        }
    }
    return status;
}

BmsStatus_t AuthenticationSave(const BmsAuthenticationContext_t *context)
{
    FILE *file = NULL;
    const BmsLinkedListNode_t *node = NULL;
    BmsAuthenticationFileHeader_t header = { 0U, 0U, 0U, 0U };
    BmsStatus_t status = BMS_STATUS_INVALID_ARGUMENT;

    if ((context != NULL) && context->initialized)
    {
        file = fopen(BMS_USERS_FILE, "wb");
        if (file == NULL)
        {
            status = BMS_STATUS_FILE_ERROR;
        }
        else
        {
            header.magic = BMS_AUTH_FILE_MAGIC;
            header.version = BMS_AUTH_FILE_VERSION;
            header.recordSize = (uint16_t)sizeof(BmsUser_t);
            header.recordCount = context->users.count;
            status = BMS_STATUS_OK;
            if (fwrite(&header, sizeof(header), 1U, file) != 1U)
            {
                status = BMS_STATUS_WRITE_ERROR;
            }
            node = context->users.head;
            while ((node != NULL) && (status == BMS_STATUS_OK))
            {
                if ((node->data == NULL) ||
                    (fwrite(node->data, sizeof(BmsUser_t), 1U, file) != 1U))
                {
                    status = BMS_STATUS_WRITE_ERROR;
                }
                node = node->next;
            }
            if ((fclose(file) != 0) && (status == BMS_STATUS_OK))
            {
                status = BMS_STATUS_FILE_ERROR;
            }
        }
    }
    return status;
}

BmsStatus_t AuthenticationRegisterUser(BmsAuthenticationContext_t *context,
                                       const BmsUser_t *user,
                                       const char *plainPassword)
{
    BmsUser_t copy;
    void *found = NULL;
    BmsStatus_t status = BMS_STATUS_INVALID_ARGUMENT;

    if ((context != NULL) && (user != NULL) && (plainPassword != NULL))
    {
        if ((!ValidateUsername(user->username)) ||
            (!ValidatePassword(plainPassword)) ||
            (!ValidateUserId(user->userId)))
        {
            status = BMS_STATUS_PASSWORD_POLICY_FAILED;
        }
        else if ((LinkedListFind(&context->users, MatchName,
                                 user->username, &found) == BMS_STATUS_OK) ||
                 (LinkedListFind(&context->users, MatchId,
                                 &user->userId, &found) == BMS_STATUS_OK))
        {
            status = BMS_STATUS_ALREADY_EXISTS;
        }
        else
        {
            copy = *user;
            HashSecret(plainPassword, copy.passwordHash,
                       sizeof(copy.passwordHash));
            copy.failedLoginAttempts = 0U;
            copy.isLocked = false;
            copy.isActive = (copy.status == BMS_USER_STATUS_ACTIVE);
            if (copy.status == BMS_USER_STATUS_INVALID)
            {
                copy.status = BMS_USER_STATUS_ACTIVE;
                copy.isActive = true;
            }
            status = LinkedListInsertBack(&context->users, &copy);
        }
    }
    if (status == BMS_STATUS_OK)
    {
        BMS_LOG_INFO("AUTH",
                     "User registered: userId=%u role=%u",
                     (unsigned int)user->userId,
                     (unsigned int)user->role);
    }
    else if (status != BMS_STATUS_INVALID_ARGUMENT)
    {
        BMS_LOG_WARNING("AUTH",
                        "User registration failed: status=%u",
                        (unsigned int)status);
    }
    return status;
}

BmsStatus_t AuthenticationLogin(BmsAuthenticationContext_t *context,
                                const char *username,
                                const char *plainPassword,
                                BmsUser_t *authenticatedUser)
{
    void *found = NULL;
    char passwordHash[BMS_MAX_PASSWORD_LENGTH + 1U] = { '\0' };
    BmsUser_t *user = NULL;
    BmsStatus_t status = BMS_STATUS_INVALID_ARGUMENT;

    if ((context != NULL) && (username != NULL) &&
        (plainPassword != NULL) && (authenticatedUser != NULL))
    {
        status = LinkedListFind(&context->users, MatchName, username, &found);
        if (status != BMS_STATUS_OK)
        {
            status = BMS_STATUS_AUTHENTICATION_FAILED;
        }
        else
        {
            user = (BmsUser_t *)found;
            if ((user->status == BMS_USER_STATUS_PENDING) ||
                (user->status == BMS_USER_STATUS_DISABLED) ||
                (!user->isActive))
            {
                status = BMS_STATUS_ACCESS_DENIED;
            }
            else if (user->isLocked ||
                     (user->status == BMS_USER_STATUS_LOCKED))
            {
                status = BMS_STATUS_ACCOUNT_LOCKED;
            }
            else
            {
                HashSecret(plainPassword, passwordHash, sizeof(passwordHash));
                if (strcmp(passwordHash, user->passwordHash) != 0)
                {
                    if (user->failedLoginAttempts < UINT8_MAX)
                    {
                        ++user->failedLoginAttempts;
                    }
                    if (user->failedLoginAttempts >= BMS_MAX_LOGIN_ATTEMPTS)
                    {
                        user->isLocked = true;
                        user->status = BMS_USER_STATUS_LOCKED;
                    }
                    status = BMS_STATUS_AUTHENTICATION_FAILED;
                }
                else
                {
                    user->failedLoginAttempts = 0U;
                    *authenticatedUser = *user;
                    status = BMS_STATUS_OK;
                }
            }
        }
    }
    UtilitySecureZero(passwordHash, sizeof(passwordHash));

    if (status == BMS_STATUS_OK)
    {
        BMS_LOG_INFO("AUTH",
                     "Login successful: userId=%u username=%s",
                     (unsigned int)authenticatedUser->userId,
                     authenticatedUser->username);
    }
    else if ((username != NULL) &&
             ((status == BMS_STATUS_AUTHENTICATION_FAILED) ||
              (status == BMS_STATUS_ACCOUNT_LOCKED) ||
              (status == BMS_STATUS_ACCESS_DENIED)))
    {
        BMS_LOG_WARNING("AUTH",
                        "Login rejected: username=%s status=%u",
                        username,
                        (unsigned int)status);
    }

    return status;
}

BmsStatus_t AuthenticationChangePassword(BmsAuthenticationContext_t *context,
                                         BmsUserId_t userId,
                                         const char *currentPassword,
                                         const char *newPassword)
{
    BmsUser_t *user = NULL;
    char currentHash[BMS_MAX_PASSWORD_LENGTH + 1U] = { '\0' };
    BmsStatus_t status = BMS_STATUS_INVALID_ARGUMENT;
    if ((context != NULL) && (currentPassword != NULL) &&
        (newPassword != NULL))
    {
        if (!ValidatePassword(newPassword))
        {
            status = BMS_STATUS_PASSWORD_POLICY_FAILED;
        }
        else
        {
            status = FindMutableUser(context, userId, &user);
            if (status == BMS_STATUS_OK)
            {
                HashSecret(currentPassword, currentHash, sizeof(currentHash));
                if (strcmp(currentHash, user->passwordHash) != 0)
                {
                    status = BMS_STATUS_AUTHENTICATION_FAILED;
                }
                else
                {
                    HashSecret(newPassword, user->passwordHash,
                               sizeof(user->passwordHash));
                    user->passwordChangeRequired = false;
                }
            }
        }
    }
    UtilitySecureZero(currentHash, sizeof(currentHash));
    if (status == BMS_STATUS_OK)
    {
        BMS_LOG_INFO("AUTH",
                     "Password changed: userId=%u",
                     (unsigned int)userId);
    }
    else if (status != BMS_STATUS_INVALID_ARGUMENT)
    {
        BMS_LOG_WARNING("AUTH",
                        "Password change failed: userId=%u status=%u",
                        (unsigned int)userId,
                        (unsigned int)status);
    }
    return status;
}

BmsStatus_t AuthenticationResetPassword(BmsAuthenticationContext_t *context,
                                        const char *username,
                                        const char *recoveryCode,
                                        const char *newPassword)
{
    void *found = NULL;
    char recoveryHash[BMS_MAX_PASSWORD_LENGTH + 1U] = { '\0' };
    BmsUser_t *user = NULL;
    BmsStatus_t status = BMS_STATUS_INVALID_ARGUMENT;
    if ((context != NULL) && (username != NULL) &&
        (recoveryCode != NULL) && (newPassword != NULL))
    {
        if (!ValidatePassword(newPassword))
        {
            status = BMS_STATUS_PASSWORD_POLICY_FAILED;
        }
        else
        {
            status = LinkedListFind(&context->users, MatchName, username, &found);
            if (status == BMS_STATUS_OK)
            {
                user = (BmsUser_t *)found;
                HashSecret(recoveryCode, recoveryHash, sizeof(recoveryHash));
                if ((user->recoveryCodeHash[0] == '\0') ||
                    (strcmp(recoveryHash, user->recoveryCodeHash) != 0))
                {
                    status = BMS_STATUS_AUTHENTICATION_FAILED;
                }
                else
                {
                    HashSecret(newPassword, user->passwordHash,
                               sizeof(user->passwordHash));
                    user->failedLoginAttempts = 0U;
                    user->isLocked = false;
                    if (user->status == BMS_USER_STATUS_LOCKED)
                    {
                        user->status = BMS_USER_STATUS_ACTIVE;
                        user->isActive = true;
                    }
                    status = BMS_STATUS_OK;
                }
            }
        }
    }
    UtilitySecureZero(recoveryHash, sizeof(recoveryHash));
    return status;
}

BmsStatus_t AuthenticationSetRecoveryCode(BmsAuthenticationContext_t *context,
                                          BmsUserId_t userId,
                                          const char *recoveryCode)
{
    BmsUser_t *user = NULL;
    BmsStatus_t status = BMS_STATUS_INVALID_ARGUMENT;
    if ((context != NULL) && (recoveryCode != NULL) &&
        (strlen(recoveryCode) >= 8U))
    {
        status = FindMutableUser(context, userId, &user);
        if (status == BMS_STATUS_OK)
        {
            HashSecret(recoveryCode, user->recoveryCodeHash,
                       sizeof(user->recoveryCodeHash));
        }
    }
    return status;
}

BmsStatus_t AuthenticationUnlockUser(BmsAuthenticationContext_t *context,
                                     BmsUserId_t userId)
{
    BmsUser_t *user = NULL;
    BmsStatus_t status = FindMutableUser(context, userId, &user);
    if (status == BMS_STATUS_OK)
    {
        user->isLocked = false;
        user->failedLoginAttempts = 0U;
        user->status = BMS_USER_STATUS_ACTIVE;
        user->isActive = true;
    }
    return status;
}

BmsStatus_t AuthenticationSetUserActive(BmsAuthenticationContext_t *context,
                                        BmsUserId_t userId,
                                        bool isActive)
{
    BmsUser_t *user = NULL;
    BmsStatus_t status = FindMutableUser(context, userId, &user);
    if (status == BMS_STATUS_OK)
    {
        user->isActive = isActive;
        user->status = isActive ? BMS_USER_STATUS_ACTIVE :
                                  BMS_USER_STATUS_DISABLED;
    }
    return status;
}

BmsStatus_t AuthenticationSetUserStatus(BmsAuthenticationContext_t *context,
                                        BmsUserId_t userId,
                                        BmsUserStatus_t statusValue)
{
    BmsUser_t *user = NULL;
    BmsStatus_t status = FindMutableUser(context, userId, &user);
    if (status == BMS_STATUS_OK)
    {
        user->status = statusValue;
        user->isActive = (statusValue == BMS_USER_STATUS_ACTIVE);
        user->isLocked = (statusValue == BMS_USER_STATUS_LOCKED);
    }
    return status;
}

BmsStatus_t AuthenticationFindUserById(const BmsAuthenticationContext_t *context,
                                       BmsUserId_t userId,
                                       BmsUser_t *user)
{
    void *found = NULL;
    BmsStatus_t status = BMS_STATUS_INVALID_ARGUMENT;
    if ((context != NULL) && (user != NULL))
    {
        status = LinkedListFind(&context->users, MatchId, &userId, &found);
        if (status == BMS_STATUS_OK)
        {
            *user = *(const BmsUser_t *)found;
        }
    }
    return status;
}

BmsStatus_t AuthenticationFindUserByUsername(const BmsAuthenticationContext_t *context,
                                             const char *username,
                                             BmsUser_t *user)
{
    void *found = NULL;
    BmsStatus_t status = BMS_STATUS_INVALID_ARGUMENT;
    if ((context != NULL) && (username != NULL) && (user != NULL))
    {
        status = LinkedListFind(&context->users, MatchName, username, &found);
        if (status == BMS_STATUS_OK)
        {
            *user = *(const BmsUser_t *)found;
        }
    }
    return status;
}

BmsStatus_t AuthenticationGetNextUserId(const BmsAuthenticationContext_t *context,
                                        BmsUserId_t *userId)
{
    const BmsLinkedListNode_t *node = NULL;
    BmsStatus_t status = BMS_STATUS_INVALID_ARGUMENT;
    if ((context != NULL) && (userId != NULL))
    {
        BmsUserId_t maximum = 1000U;
        node = context->users.head;
        while (node != NULL)
        {
            if ((node->data != NULL) &&
                (((const BmsUser_t *)node->data)->userId > maximum))
            {
                maximum = ((const BmsUser_t *)node->data)->userId;
            }
            node = node->next;
        }
        if (maximum < UINT32_MAX)
        {
            *userId = maximum + 1U;
            status = BMS_STATUS_OK;
        }
        else
        {
            status = BMS_STATUS_INTERNAL_ERROR;
        }
    }
    return status;
}

bool AuthenticationAdministratorExists(const BmsAuthenticationContext_t *context)
{
    const BmsLinkedListNode_t *node = NULL;
    bool exists = false;
    if (context != NULL)
    {
        node = context->users.head;
        while ((node != NULL) && (!exists))
        {
            if ((node->data != NULL) &&
                (((const BmsUser_t *)node->data)->role == BMS_ROLE_ADMIN))
            {
                exists = true;
            }
            node = node->next;
        }
    }
    return exists;
}

bool AuthenticationHasRole(const BmsUser_t *user, BmsUserRole_t requiredRole)
{
    return (user != NULL) && user->isActive &&
           (user->status == BMS_USER_STATUS_ACTIVE) &&
           (user->role == requiredRole);
}


BmsStatus_t AuthenticationSetHospitalId(BmsAuthenticationContext_t *context,
                                        BmsUserId_t userId,
                                        BmsHospitalId_t hospitalId)
{
    const BmsLinkedListNode_t *node = NULL;
    if ((context == NULL) || (!context->initialized) || (hospitalId == 0U))
    {
        return BMS_STATUS_INVALID_ARGUMENT;
    }
    node = context->users.head;
    while (node != NULL)
    {
        BmsUser_t *user = (BmsUser_t *)node->data;
        if ((user != NULL) && (user->userId == userId))
        {
            user->hospitalId = hospitalId;
            return BMS_STATUS_OK;
        }
        node = node->next;
    }
    return BMS_STATUS_NOT_FOUND;
}
void AuthenticationDeinitialize(BmsAuthenticationContext_t *context)
{
    if (context != NULL)
    {
        LinkedListClear(&context->users);
        HashTableDeinitialize(&context->usernameIndex);
        (void)memset(context, 0, sizeof(*context));
    }
}
