/*
 * CU_tests_audio.c
 *
 *  Created on: 2 may. 2024
 *      Author: asier
 */

#include "CUnit/CU_header.h"

void test_Audio_Message(){
    CU_ERRORS result;
    bool_t flag_error_in_audio;
    uint8_t audio_playing_msg_id;

    // Cambiar flag_error_in_audio
    flag_error_in_audio = 1;
    setFlag_error_in_audio(flag_error_in_audio);
    result = Audio_Message(eAUDIO_CMD_CONCAT, eAUDIO_MODE_TEST, TRUE);
    CU_ASSERT_EQUAL(CU_ERR_FLAG_ERROR_IN_AUDIO , result);
    flag_error_in_audio = 0;
    setFlag_error_in_audio(flag_error_in_audio);

    // Forzar msg_id ERROR
    result = Audio_Message(eAUDIO_CMD_CONCAT, eAUDIO_MAX_ID, TRUE);
    CU_ASSERT_EQUAL(CU_ERR_AUDIO_MAX_ID , result);

    // Forzar pAed_Settings->aed_cpr_msg_long
    pAed_Settings = Get_AED_Settings();
    pAed_Settings->aed_cpr_msg_long = 0;
    result = Audio_Message(eAUDIO_CMD_CONCAT, eAUDIO_KEEP_CALM, TRUE);
    CU_ASSERT_EQUAL(CU_ERR_IGNORED_MESSAGES , result);

    pAed_Settings->aed_cpr_msg_long = 1;

    // Audio Test
    //CU_ASSERT_EQUAL(eERR_AUDIO_QUEUE_FULL , Audio_Message(eAUDIO_CMD_PLAY, msg_id, TRUE));

    result = Audio_Message(eAUDIO_CMD_CONCAT, eAUDIO_MODE_TEST, TRUE);
    CU_ASSERT_EQUAL(eERR_AUDIO_QUEUE_FULL , result);

    // Check Electrode Type
    result = Audio_Message(eAUDIO_CMD_CONCAT, eAUDIO_REPLACE_ELECTRODES, TRUE);
    CU_ASSERT_EQUAL(eERR_AUDIO_QUEUE_FULL , result);

    //Load_Audio_Resources();
    result = Audio_Message(eAUDIO_CMD_CONCAT, eAUDIO_REPLACE_ELECTRODES, TRUE);
    CU_ASSERT_EQUAL(eERR_AUDIO_QUEUE_FULL , result);

    // Execute Calibration / Execute_Test_Montador / Execute_Test
    result = Audio_Message(eAUDIO_CMD_PLAY, eAUDIO_CALL_SAT, TRUE);
    CU_ASSERT_EQUAL(eERR_NONE , result);

    result = Audio_Message(eAUDIO_CMD_PLAY, eAUDIO_DEVICE_READY, TRUE);
    CU_ASSERT_EQUAL(eERR_NONE , result);

    result = Audio_Message(eAUDIO_CMD_PLAY, eAUDIO_CONNECT_ELECTRODES_DEVICE, TRUE);
    CU_ASSERT_EQUAL(eERR_NONE , result);

    // GPS / WIFI / SIGFOX / WIRELESS
    result = Audio_Message(eAUDIO_CMD_CONCAT, eAUDIO_TRANSMISSION_ONGOING, TRUE);
    CU_ASSERT_EQUAL(eERR_NONE , result);

    result = Audio_Message(eAUDIO_CMD_CONCAT, eAUDIO_TRANSMISSION_DONE, TRUE);
    CU_ASSERT_EQUAL(eERR_NONE , result);

    // BATTERY / DEFIB
    result = Audio_Message(eAUDIO_CMD_CONCAT, eAUDIO_REPLACE_BATTERY, TRUE);
    CU_ASSERT_EQUAL(eERR_NONE , result);

    result = Audio_Message(eAUDIO_CMD_CONCAT, eAUDIO_LOW_BATTERY, TRUE);
    CU_ASSERT_EQUAL(eERR_AUDIO_QUEUE_FULL , result);

    // Execute_Test_Electrodes / Execute_Test_Patmon
    result = Audio_Message(eAUDIO_CMD_CONCAT, eAUDIO_DISCONNECT_PATIENT, TRUE);
    CU_ASSERT_EQUAL(eERR_AUDIO_QUEUE_FULL , result);

    result = Audio_Message(eAUDIO_CMD_CONCAT, eAUDIO_CONNECT_ELECTRODES_DEVICE, TRUE);
    CU_ASSERT_EQUAL(eERR_AUDIO_QUEUE_FULL , result);

    // Execute Test Montador
    result = Audio_Message(eAUDIO_CMD_CONCAT, eAUDIO_DEVICE_READY, TRUE);
    CU_ASSERT_EQUAL(eERR_AUDIO_QUEUE_FULL , result);

    // Execute_Test
    result = Audio_Message(eAUDIO_CMD_CONCAT, eAUDIO_TRAINER_MODE, TRUE);
    CU_ASSERT_EQUAL(eERR_AUDIO_QUEUE_FULL , result);

    // Load_FW_File / Load_Gold_File
    result = Audio_Message(eAUDIO_CMD_PLAY, eAUDIO_UPGRADING, TRUE);
    CU_ASSERT_EQUAL(eERR_AUDIO_QUEUE_FULL , result);

    // Power_Off
    result = Audio_Message(eAUDIO_CMD_PLAY, eAUDIO_TRAINER_MODE, TRUE);
    CU_ASSERT_EQUAL(eERR_AUDIO_QUEUE_FULL , result);

    result = Audio_Message(eAUDIO_CMD_PLAY, eAUDIO_DISCONNECT_PATIENT, TRUE);
    CU_ASSERT_EQUAL(eERR_AUDIO_QUEUE_FULL , result);

    result = Audio_Message(eAUDIO_CMD_STOP, eAUDIO_ASYSTOLE, TRUE);
    CU_ASSERT_EQUAL(eERR_NONE , result);

    // Check Cover
    result = Audio_Message(eAUDIO_CMD_PLAY, eAUDIO_OPEN_COVER, TRUE);
    CU_ASSERT_EQUAL(eERR_NONE , result);

    // Check SAT
    result = Audio_Message(eAUDIO_CMD_PLAY, eAUDIO_REPLACE_BATTERY, TRUE);
    CU_ASSERT_EQUAL(eERR_NONE , result);

    // CHeck Warning
    result = Audio_Message(eAUDIO_CMD_PLAY, eAUDIO_TONE, TRUE);
    CU_ASSERT_EQUAL(eERR_NONE , result);

    // R100_StartupAsiert
    result = Audio_Message(eAUDIO_CMD_PLAY, eAUDIO_UPGRADE_ERROR, TRUE);
    CU_ASSERT_EQUAL(eERR_NONE , result);

}

void test_Audio_Init(){
    ssp_err_t err;
    dac_audio.p_api->close (dac_audio.p_ctrl);
    timer_audio.p_api->close (timer_audio.p_ctrl);
    err = Audio_Init();
    CU_ASSERT_EQUAL(err, SSP_SUCCESS);
}

void test_Audio_Play_Start(){
    uint16_t arg1, arg2;
    ssp_err_t result;

    result = Audio_Play_Start(&arg1, &arg2);
    CU_ASSERT_EQUAL(result, SSP_SUCCESS);
}

void test_Load_Audio_Resources(){
    uint8_t CU_audio_num;
    ERROR_ID_e result;

    result = Load_Audio_Resources(eAUDIO_MAX_ID);
    CU_ASSERT_EQUAL(result, eERR_NONE);

    result = Load_Audio_Resources(eAUDIO_CPR_BREATH);
    CU_ASSERT_EQUAL(result, eERR_AUDIO_MAX);
}

static CU_TestInfo tests_thread_audio[] = {
    {"test_Audio_Message", test_Audio_Message},
    {"test_Audio_Init", test_Audio_Init},
    {"test_Audio_Play_Start", test_Audio_Play_Start},
    //  CORREGIR    {"test_Load_Audio_Resources", test_Load_Audio_Resources},

    CU_TEST_INFO_NULL,
};

// Declare the test suite in SuiteInfo
static CU_SuiteInfo suites[] = {
    {"TestSimpleAssert_Test_Audio", NULL, NULL, tests_thread_audio},
    CU_SUITE_INFO_NULL,
};

void CU_test_audio(void) {
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
