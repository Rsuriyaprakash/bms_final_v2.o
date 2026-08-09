/**
 * @file authentication.h
 * @brief Authentication, account security, recovery and role authorization APIs.
 */
#ifndef BMS_AUTHENTICATION_H
#define BMS_AUTHENTICATION_H

#include <stdbool.h>
#include <stdint.h>
#include "common.h"
#include "hash_table.h"
#include "linked_list.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct
{
    BmsLinkedList_t users;
    BmsHashTable_t usernameIndex;
    bool initialized;
} BmsAuthenticationContext_t;

BmsStatus_t AuthenticationInitialize(BmsAuthenticationContext_t *context);
BmsStatus_t AuthenticationLoad(BmsAuthenticationContext_t *context);
BmsStatus_t AuthenticationSave(const BmsAuthenticationContext_t *context);
BmsStatus_t AuthenticationRegisterUser(BmsAuthenticationContext_t *context,
                                       const BmsUser_t *user,
                                       const char *plainPassword);
BmsStatus_t AuthenticationLogin(BmsAuthenticationContext_t *context,
                                const char *username,
                                const char *plainPassword,
                                BmsUser_t *authenticatedUser);
BmsStatus_t AuthenticationChangePassword(BmsAuthenticationContext_t *context,
                                         BmsUserId_t userId,
                                         const char *currentPassword,
                                         const char *newPassword);
BmsStatus_t AuthenticationResetPassword(BmsAuthenticationContext_t *context,
                                        const char *username,
                                        const char *recoveryCode,
                                        const char *newPassword);
BmsStatus_t AuthenticationSetRecoveryCode(BmsAuthenticationContext_t *context,
                                          BmsUserId_t userId,
                                          const char *recoveryCode);
BmsStatus_t AuthenticationUnlockUser(BmsAuthenticationContext_t *context,
                                     BmsUserId_t userId);
BmsStatus_t AuthenticationSetUserActive(BmsAuthenticationContext_t *context,
                                        BmsUserId_t userId,
                                        bool isActive);
BmsStatus_t AuthenticationSetUserStatus(BmsAuthenticationContext_t *context,
                                        BmsUserId_t userId,
                                        BmsUserStatus_t status);
BmsStatus_t AuthenticationSetHospitalId(BmsAuthenticationContext_t *context,
                                        BmsUserId_t userId,
                                        BmsHospitalId_t hospitalId);
BmsStatus_t AuthenticationFindUserById(const BmsAuthenticationContext_t *context,
                                       BmsUserId_t userId,
                                       BmsUser_t *user);
BmsStatus_t AuthenticationFindUserByUsername(const BmsAuthenticationContext_t *context,
                                             const char *username,
                                             BmsUser_t *user);
BmsStatus_t AuthenticationGetNextUserId(const BmsAuthenticationContext_t *context,
                                        BmsUserId_t *userId);
bool AuthenticationAdministratorExists(const BmsAuthenticationContext_t *context);
bool AuthenticationHasRole(const BmsUser_t *user, BmsUserRole_t requiredRole);
void AuthenticationDeinitialize(BmsAuthenticationContext_t *context);

#ifdef __cplusplus
}
#endif
#endif /* BMS_AUTHENTICATION_H */
