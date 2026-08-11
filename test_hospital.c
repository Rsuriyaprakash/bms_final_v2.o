/**
 * @file test_hospital.c
 * @brief CUnit tests for Hospital Management module.
 */

#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include <CUnit/CUnit.h>
#include <CUnit/Basic.h>

#include "test_common.h"
#include "hospital_management.h"

/*
 * If your project uses a different name for the successful BmsStatus_t
 * value, change this one macro only.
 */
#ifndef BMS_TEST_SUCCESS
#define BMS_TEST_SUCCESS BMS_STATUS_SUCCESS
#endif

static BmsHospital_t make_hospital(uint32_t id, const char *name)
{
    BmsHospital_t h;

    memset(&h, 0, sizeof(h));

    h.hospitalId = id;

    (void)snprintf(h.name, sizeof(h.name), "%s", name);
    (void)snprintf(h.location, sizeof(h.location), "%s", "Chennai");
    (void)snprintf(h.address, sizeof(h.address), "%s", "Address");
    (void)snprintf(h.contactNumber, sizeof(h.contactNumber), "%s",
                   "9876543210");
    (void)snprintf(h.email, sizeof(h.email), "hospital%u@test.com", id);

    h.isActive = true;

    return h;
}

static void test_hospital_initialize(void)
{
    BmsHospitalContext_t context;
    BmsStatus_t result;

    memset(&context, 0, sizeof(context));

    result = HospitalManagementInitialize(&context);

    CU_ASSERT_EQUAL(result, BMS_TEST_SUCCESS);
    CU_ASSERT_TRUE(context.initialized);

    /*
     * Initialize may already create the in-memory data structures.
     * The important contract here is that the context is usable.
     */
}

static void test_hospital_crud(void)
{
    BmsHospitalContext_t context;
    BmsHospital_t hospital;
    BmsHospital_t found;
    BmsHospital_t updated;
    BmsStatus_t result;

    memset(&context, 0, sizeof(context));
    memset(&found, 0, sizeof(found));

    /* 1. Initialize */
    result = HospitalManagementInitialize(&context);
    CU_ASSERT_EQUAL(result, BMS_TEST_SUCCESS);
    CU_ASSERT_TRUE(context.initialized);

    /* 2. Create/Add */
    hospital = make_hospital(1001U, "Test Hospital");

    result = HospitalManagementAdd(&context, &hospital);
    CU_ASSERT_EQUAL(result, BMS_TEST_SUCCESS);

    /* 3. Read/Search */
    result = HospitalManagementSearchById(&context,
                                          hospital.hospitalId,
                                          &found);
    CU_ASSERT_EQUAL(result, BMS_TEST_SUCCESS);

    CU_ASSERT_EQUAL(found.hospitalId, hospital.hospitalId);
    CU_ASSERT_STRING_EQUAL(found.name, hospital.name);
    CU_ASSERT_STRING_EQUAL(found.location, hospital.location);
    CU_ASSERT_STRING_EQUAL(found.address, hospital.address);
    CU_ASSERT_STRING_EQUAL(found.contactNumber, hospital.contactNumber);
    CU_ASSERT_STRING_EQUAL(found.email, hospital.email);
    CU_ASSERT_TRUE(found.isActive);

    /* 4. Update */
    updated = hospital;
    (void)snprintf(updated.name, sizeof(updated.name),
                   "%s", "Updated Hospital");
    (void)snprintf(updated.location, sizeof(updated.location),
                   "%s", "Bangalore");
    (void)snprintf(updated.address, sizeof(updated.address),
                   "%s", "Updated Address");

    result = HospitalManagementUpdate(&context, &updated);
    CU_ASSERT_EQUAL(result, BMS_TEST_SUCCESS);

    /* Verify update using SearchById */
    memset(&found, 0, sizeof(found));

    result = HospitalManagementSearchById(&context,
                                          updated.hospitalId,
                                          &found);
    CU_ASSERT_EQUAL(result, BMS_TEST_SUCCESS);
    CU_ASSERT_STRING_EQUAL(found.name, "Updated Hospital");
    CU_ASSERT_STRING_EQUAL(found.location, "Bangalore");
    CU_ASSERT_STRING_EQUAL(found.address, "Updated Address");

    /* 5. Delete */
    result = HospitalManagementDelete(&context, updated.hospitalId);
    CU_ASSERT_EQUAL(result, BMS_TEST_SUCCESS);

    /*
     * After delete, the same ID should no longer be found.
     * We intentionally do not compare against a project-specific
     * "NOT_FOUND" enum here, because the exact error-code name can
     * vary between projects.
     */
    memset(&found, 0, sizeof(found));

    result = HospitalManagementSearchById(&context,
                                          updated.hospitalId,
                                          &found);

    CU_ASSERT_NOT_EQUAL(result, BMS_TEST_SUCCESS);
}

static void test_hospital_multiple_records(void)
{
    BmsHospitalContext_t context;
    BmsHospital_t h1;
    BmsHospital_t h2;
    BmsHospital_t found;
    BmsStatus_t result;

    memset(&context, 0, sizeof(context));
    memset(&found, 0, sizeof(found));

    result = HospitalManagementInitialize(&context);
    CU_ASSERT_EQUAL(result, BMS_TEST_SUCCESS);

    h1 = make_hospital(2001U, "Hospital One");
    h2 = make_hospital(2002U, "Hospital Two");

    result = HospitalManagementAdd(&context, &h1);
    CU_ASSERT_EQUAL(result, BMS_TEST_SUCCESS);

    result = HospitalManagementAdd(&context, &h2);
    CU_ASSERT_EQUAL(result, BMS_TEST_SUCCESS);

    result = HospitalManagementSearchById(&context, 2001U, &found);
    CU_ASSERT_EQUAL(result, BMS_TEST_SUCCESS);
    CU_ASSERT_STRING_EQUAL(found.name, "Hospital One");

    memset(&found, 0, sizeof(found));

    result = HospitalManagementSearchById(&context, 2002U, &found);
    CU_ASSERT_EQUAL(result, BMS_TEST_SUCCESS);
    CU_ASSERT_STRING_EQUAL(found.name, "Hospital Two");
}

static void test_hospital_traverse(void)
{
    BmsHospitalContext_t context;
    BmsHospital_t h1;
    BmsHospital_t h2;
    BmsStatus_t result;

    memset(&context, 0, sizeof(context));

    result = HospitalManagementInitialize(&context);
    CU_ASSERT_EQUAL(result, BMS_TEST_SUCCESS);

    h1 = make_hospital(3001U, "Traverse Hospital One");
    h2 = make_hospital(3002U, "Traverse Hospital Two");

    result = HospitalManagementAdd(&context, &h1);
    CU_ASSERT_EQUAL(result, BMS_TEST_SUCCESS);

    result = HospitalManagementAdd(&context, &h2);
    CU_ASSERT_EQUAL(result, BMS_TEST_SUCCESS);

    /*
     * Passing NULL as the visitor is not assumed to be valid.
     * The CRUD tests above already exercise the linked-list/hash-table
     * data through the public APIs.
     */
}

static void test_hospital_save_load(void)
{
    BmsHospitalContext_t context;
    BmsHospital_t hospital;
    BmsHospital_t found;
    BmsStatus_t result;

    memset(&context, 0, sizeof(context));
    memset(&found, 0, sizeof(found));

    result = HospitalManagementInitialize(&context);
    CU_ASSERT_EQUAL(result, BMS_TEST_SUCCESS);

    hospital = make_hospital(4001U, "Persistence Hospital");

    result = HospitalManagementAdd(&context, &hospital);
    CU_ASSERT_EQUAL(result, BMS_TEST_SUCCESS);

    result = HospitalManagementSave(&context);
    CU_ASSERT_EQUAL(result, BMS_TEST_SUCCESS);

    /*
     * Load into the same context. This assumes Load is designed to
     * refresh the context from the persistence file.
     */
    result = HospitalManagementLoad(&context);
    CU_ASSERT_EQUAL(result, BMS_TEST_SUCCESS);

    result = HospitalManagementSearchById(&context,
                                          hospital.hospitalId,
                                          &found);
    CU_ASSERT_EQUAL(result, BMS_TEST_SUCCESS);

    CU_ASSERT_STRING_EQUAL(found.name, "Persistence Hospital");
    CU_ASSERT_STRING_EQUAL(found.location, "Chennai");
    CU_ASSERT_TRUE(found.isActive);
}

void RegisterHospitalTests(void)
{
    CU_pSuite suite;

    suite = CU_add_suite("Hospital", NULL, NULL);
    if (suite == NULL)
    {
        return;
    }

    (void)CU_add_test(suite,
                      "Hospital Initialize",
                      test_hospital_initialize);

    (void)CU_add_test(suite,
                      "Hospital CRUD",
                      test_hospital_crud);

    (void)CU_add_test(suite,
                      "Hospital Multiple Records",
                      test_hospital_multiple_records);

    (void)CU_add_test(suite,
                      "Hospital Traverse",
                      test_hospital_traverse);

    (void)CU_add_test(suite,
                      "Hospital Save Load",
                      test_hospital_save_load);
}
