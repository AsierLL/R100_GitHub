/*
 * CU_test_core.c
 *
 *  Created on: 26 jun. 2024
 *      Author: asier
 */

#include "CUnit/CU_header.h"

void test_Power_Off(){
    CU_ERRORS CU_err;

    // PowerOff_Write_Batt_Elec
        // Comprobar el valor que devuelve, dentro de la funcion "Power_Off"

    // Check_Device_Led_Status
        // Se comprueba el valor que devuelve, dentro de la funcion "Power_Off"

    //NFC_Write_Device_Info
        // Cambiar lo que devuelve la funcion, tiene su propio test

    // Defib_Charge
        // Se comprueba el valor que devuelve, dentro de la funcion "Power_Off"
        //test_Defib_Charge();

    // R100_Program_Autotest
    test_Program_Autotest();

    // R100_PowerOff
    test_R100_PowerOff();

    CU_err = CU_Power_Off();
    CU_ASSERT_EQUAL(CU_err, SSP_SUCCESS);
}

void test_R100_PowerOff(){
    NV_DATA_t CU_nv_data;
    ssp_err_t CU_ssp_err;
    ERROR_ID_e CU_err;
    uint32_t CU_pon_date;
    BATTERY_STATISTICS_t CU_battery_statistics;

    // Is_Mode_Trainer == TRUE
    CU_setMode_trainer(TRUE);
    CU_nv_data = CU_getNV_Data();
    R100_PowerOff();
    CU_ASSERT_EQUAL(CU_nv_data.status_led_blink, 0);

    // Is_Mode_Trainer == FALSE
    CU_setMode_trainer(FALSE);
    R100_PowerOff();

    // Battery_Write_Statistics
    CU_ssp_err = Battery_Write_Statistics(&CU_battery_statistics);
    CU_ASSERT_EQUAL(CU_err, SSP_SUCCESS);

    // Check_Electrodes_Type_RTC --> Forzar para obtener diferentes valores

        // electrodes_presence_flag --> eERR_ELECTRODE_NOT_CONNECTED
        CU_setElectrodes_presence_flag(false);
        CU_err = Check_Electrodes_Type_RTC();
        CU_ASSERT_EQUAL_FATAL(CU_err, eERR_ELECTRODE_NOT_CONNECTED);

        // Is_Mode_Trainer --> eERR_ELECTRODE_EXPIRED
        CU_setMode_trainer(FALSE);
        CU_pon_date = CU_getPon_date();
        CU_setElectrodes_data(CU_pon_date-1, NULL, CU_ELECTRODES_DATA_EXPIRATION_DATE);
        CU_err = Check_Electrodes_Type_RTC();
        CU_ASSERT_EQUAL(CU_err, eERR_ELECTRODE_EXPIRED);

        // electrodes_data.event.id != 0 --> eERR_ELECTRODE_USED
        CU_setElectrodes_data(CU_pon_date+1, NULL, CU_ELECTRODES_DATA_EXPIRATION_DATE);
        CU_setElectrodes_data(1, NULL, CU_ELECTRODES_DATA_EVENT_ID);
        CU_err = Check_Electrodes_Type_RTC();
        CU_ASSERT_EQUAL(CU_err, eERR_ELECTRODE_USED);
}

void test_Defib_charge(){
    DEFIB_STATE_e   CU_defib_state;
    int result;

    result = Defib_Charge(0, 0);
    CU_ASSERT_EQUAL(result, CU_SUCCESS);

    CU_setDefib_state(eDEFIB_STATE_IN_ERROR);
    result = Defib_Charge(0, 0);
    CU_ASSERT_EQUAL(result, CU_ERR_eDEFIB_STATE_IN_ERROR);

    CU_setDefib_state(eDEFIB_STATE_OUT_OF_SERVICE);
    result = Defib_Charge(0, 0);
    CU_ASSERT_EQUAL(result, CU_ERR_eDEFIB_STATE_OUT_OF_SERVICE);
}

void test_Program_Autotest(){
    uint8_t CU_time_test;

    // Test es a la misma hora
    CU_setBatteryRTC_hour(18);
    CU_setNV_Data_time_test(18);

    CU_time_test = CU_getNV_Data_time_test();

    R100_Program_Autotest();
    CU_ASSERT_EQUAL(CU_time_test, 20);

    // Test es 1h despues
    CU_setBatteryRTC_hour(17);
    CU_setNV_Data_time_test(18);

    CU_time_test = CU_getNV_Data_time_test();

    R100_Program_Autotest();
    CU_ASSERT_EQUAL(CU_time_test, 19);

    // Test es a cualquier otra hora
    CU_setBatteryRTC_hour(17);
    CU_setNV_Data_time_test(20);

    CU_time_test = CU_getNV_Data_time_test();

    R100_Program_Autotest();
    CU_ASSERT_EQUAL(CU_time_test, 20);
}

static CU_TestInfo tests_thread_core[] = {
    {"test_Power_Off", test_Power_Off},

    CU_TEST_INFO_NULL,
};

// Declare the test suite in SuiteInfo
static CU_SuiteInfo suites[] = {
    {"TestSimpleAssert_Test_core", NULL, NULL, tests_thread_core},
    CU_SUITE_INFO_NULL,
};

void CU_test_core(void) {
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

