/*
 * CU_test_defibrillator.c
 *
 *  Created on: 15 may. 2024
 *      Author: asier
 */

/*
 * CU_tests_audio.c
 *
 *  Created on: 2 may. 2024
 *      Author: asier
 */

#include "CUnit/CU_header.h"

void test_Set_Ovch_Alarm(){
   CU_ERRORS err;
   err = CU_Set_Ovch_Alarm(2000);             //MAS DEL MAXIMO (1800)
   CU_ASSERT_EQUAL(err , SSP_SUCCESS);
}

void test_Defib_Shock(){
    CU_ERRORS err;

    //  defib_state == eDEFIB_STATE_IN_ERROR
    CU_set_defib_state(eDEFIB_STATE_IN_ERROR);

    err = Defib_Shock(100);
    CU_ASSERT_EQUAL(err , CU_ERR_eDEFIB_STATE_IN_ERROR);

    //  defib_state == eDEFIB_STATE_OUT_OF_SERVICE
    CU_set_defib_state(eDEFIB_STATE_OUT_OF_SERVICE);

    err = Defib_Shock(100);
    CU_ASSERT_EQUAL(err , CU_ERR_eDEFIB_STATE_OUT_OF_SERVICE);

    err = Defib_Shock(100); // La funcion recibe parametros de ente 15 - 300
    CU_ASSERT_EQUAL(err , SSP_SUCCESS);
}

void test_Defib_Start_Discharge(){
    CU_ERRORS err;
    err = CU_Defib_Start_Discharge(0);
    CU_ASSERT_EQUAL(err , SSP_SUCCESS);
}

void test_GPT_Shock_Callback(){

    /*  TEST VIEJO  */
    CU_ERRORS err;
    int result;

    // FASE 1
    result = Defib_Shock(100);
    CU_ASSERT_EQUAL(result, CU_SUCCESS)

    // FASE 2
    err = GPT_Shock_Callback(100);
    CU_ASSERT_EQUAL(err , SSP_SUCCESS);

    // FASE 3
    err = GPT_Shock_Callback(100);
    CU_ASSERT_EQUAL(err , SSP_SUCCESS);
}

void test_Scan_Analog(){
    ssp_err_t err;

    err = CU_Scan_Analog();
    CU_ASSERT_EQUAL(err , SSP_SUCCESS);
}

static CU_TestInfo tests_thread_defibrillator[] = {
    {"test_Set_Ovch_Alarm", test_Set_Ovch_Alarm},
    {"test_Defib_Shock", test_Defib_Shock},
    {"test_Defib_Start_Discharge", test_Defib_Start_Discharge},
    {"test_GPT_Shock_Callback", test_GPT_Shock_Callback},
    {"test_Scan_Analog", test_Scan_Analog},

    CU_TEST_INFO_NULL,
};

// Declare the test suite in SuiteInfo
static CU_SuiteInfo suites[] = {
    {"TestSimpleAssert_Test_Defibrillator", NULL, NULL, tests_thread_defibrillator},
    CU_SUITE_INFO_NULL,
};

void CU_test_defibrillator(void) {
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

