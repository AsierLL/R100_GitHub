/*
 * CU_tests_patmon.c
 *
 *  Created on: 25 jun. 2024
 *      Author: asier
 */

#include "CUnit/CU_header.h"

void test_ECG_Offset_Removal(){
    CU_ASSERT_TRUE(CU_ECG_Offset_Removal(INT32_MAX) <=   CU_ERR_MAX_INT32);
    CU_ASSERT_TRUE(CU_ECG_Offset_Removal(INT32_MIN) >=  CU_ERR_MIN_INT32);
}

void test_patMon_Get_Zp_Ohms(){
    patMon_Get_Zp_Ohms();
}

static CU_TestInfo tests_thread_patMon[] = {
    {"test_ECG_Offset_Removal", test_ECG_Offset_Removal},
    //{"test_patMon_Get_Zp_Ohms", test_patMon_Get_Zp_Ohms},

    CU_TEST_INFO_NULL,
};

// Declare the test suite in SuiteInfo
static CU_SuiteInfo suites[] = {
    {"TestSimpleAssert_Test_patMon", NULL, NULL, tests_thread_patMon},
    CU_SUITE_INFO_NULL,
};

void CU_test_patMon(void) {
    // Retrieve a pointer to the current test registry
    assert(NULL != CU_get_registry());
    // Flag for whether a test run is in progress
    assert(!CU_is_test_running());

    // Register the suites in a single CU_SuiteInfo array
    if (CU_register_suites(suites) != CUE_SUCCESS) {
        // Get the error message
        printf("Suite registration failed - %s\n", CU_get_error_msg());
        exit(EXIT_FAILURE);
    }
}
