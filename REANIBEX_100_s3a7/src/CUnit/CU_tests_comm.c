/*
 * CU_tests_comm.c
 *
 *  Created on: 25 sept. 2024
 *      Author: asier
 */

#include "CUnit/CU_header.h"

void test_Wifi_Send_Test(){
    ERROR_ID_e result;

    result = Wifi_Send_Test();

    CU_ASSERT_EQUAL(result, eERR_NONE);
}

void test_Wifi_Send_Episode(){
    ERROR_ID_e result;

    result = Wifi_Send_Episode();

    CU_ASSERT_EQUAL(result, eERR_NONE);
}

void test_Gps_Get_Lat_Long_Data(){
    ERROR_ID_e result;

    result = Gps_Get_Lat_Long_Data();

    CU_ASSERT_EQUAL(result, eERR_NONE);
}

static CU_TestInfo tests_thread_comm[] = {
    {"test_Wifi_Send_Test", test_Wifi_Send_Test},
    {"test_Wifi_Send_Episode", test_Wifi_Send_Episode},
    //{"test_Gps_Get_Lat_Long_Data", test_Gps_Get_Lat_Long_Data},
    CU_TEST_INFO_NULL,
};

// Declare the test suite in SuiteInfo
static CU_SuiteInfo suites[] = {
    {"TestSimpleAssert_Test_comm", NULL, NULL, tests_thread_comm},
    CU_SUITE_INFO_NULL,
};

void CU_test_comm(void) {
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
