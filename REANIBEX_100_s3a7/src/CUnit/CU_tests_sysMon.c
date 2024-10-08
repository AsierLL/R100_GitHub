/*
 * CU_sysMon.c
 *
 *  Created on: 7 mar. 2024
 *      Author: asier
 */

#include "CUnit/CU_header.h"

void test_Check_Battery_Voltage(){
    int my_temperature = 10;
    uint32_t result;

    uint32_t vLimit_safe  = ((VB_25C_SAFE_BATTERY - VB_00C_SAFE_BATTERY) * (uint32_t) my_temperature) / 25;
    vLimit_safe += VB_00C_SAFE_BATTERY;

    // Declaración modificada. Pasa de static a extern.
    result = CU_Check_Battery_Voltage(Defib_Get_Vbatt(), 0,NULL);
    CU_ASSERT_EQUAL(VB_00C_SAFE_BATTERY , result);

    result = CU_Check_Battery_Voltage(Defib_Get_Vbatt(), 25,NULL);
    CU_ASSERT_EQUAL(VB_25C_SAFE_BATTERY , result);

    result = CU_Check_Battery_Voltage(Defib_Get_Vbatt(), 10,NULL);
    CU_ASSERT_EQUAL(vLimit_safe , result);
}

void test_Is_SAT_Error(){
    int i=0;
        while (i<MAX_ERRORS)
        {

            if(R100_Errors_cfg[i].warning_error==1){
            //Se debería comprobar que el valor que le pasamos esta dentro del rango 0-254
                CU_ASSERT_TRUE(CU_Is_SAT_Error(i));
            //Debería de encender los leds correspondientes a ese error
            }
            else{
                CU_ASSERT_FALSE(CU_Is_SAT_Error(i));
            }
            i++;
        }
}

void test_NV_Data_Read(){
    NV_DATA_t CU_test_data;
    NV_DATA_BLOCK_t CU_test_data_block;
    CU_ERRORS result;

    result = NV_Data_Read(NULL, NULL);
    CU_ASSERT_EQUAL(result, CU_ERR_POINTER_IS_NULL);

    result = NV_Data_Read(&CU_test_data, &CU_test_data_block);
    CU_ASSERT_EQUAL(result, CU_SUCCESS);

    //  NV_DATA_t
    CU_ASSERT_TRUE(CU_test_data.test_pending >= 0);
    CU_ASSERT_TRUE(CU_test_data.test_pending <= 1);

    CU_ASSERT_TRUE(CU_test_data.update_flag >= 0);
    CU_ASSERT_TRUE(CU_test_data.update_flag <= 1);

    CU_ASSERT_TRUE(CU_test_data.open_cover_tx_flag >= 0);
    CU_ASSERT_TRUE(CU_test_data.open_cover_tx_flag <= 1);

    CU_ASSERT_TRUE(CU_test_data.open_cover_first >= 0);
    CU_ASSERT_TRUE(CU_test_data.open_cover_first <= 1);

    CU_ASSERT_TRUE(CU_test_data.prev_cover_status >= 0);
    CU_ASSERT_TRUE(CU_test_data.prev_cover_status <= 1);

    CU_ASSERT_TRUE(CU_test_data.status_led_blink >= 0);
    CU_ASSERT_TRUE(CU_test_data.status_led_blink <= 1);

    CU_ASSERT_TRUE(CU_test_data.time_test >= 0);
    CU_ASSERT_TRUE(CU_test_data.time_test <= 23);

    CU_ASSERT_TRUE(CU_test_data.test_id >= 0);
    CU_ASSERT_TRUE(CU_test_data.test_id <= 60);

    CU_ASSERT_TRUE(CU_test_data.error_code >= 0);
    CU_ASSERT_TRUE(CU_test_data.error_code <= MAX_ERRORS-1);

    CU_ASSERT_EQUAL(CU_test_data.must_be_0x55, 0x55);

    /*
     * test_id --> 30, 40, 50, 60 o del 0-29
     */

    //  NFC_SETTINGS_t
    CU_ASSERT_TRUE(CU_test_data.default_settings.aCPR_init <= MAX_RCP_TIME);
    CU_ASSERT_TRUE(CU_test_data.default_settings.aCPR_1 <= MAX_RCP_TIME);
    CU_ASSERT_TRUE(CU_test_data.default_settings.aCPR_2 <= MAX_RCP_TIME);
    CU_ASSERT_TRUE(CU_test_data.default_settings.pCPR_init <= MAX_RCP_TIME);
    CU_ASSERT_TRUE(CU_test_data.default_settings.pCPR_1 <= MAX_RCP_TIME);
    CU_ASSERT_TRUE(CU_test_data.default_settings.pCPR_2 <= MAX_RCP_TIME);

    CU_ASSERT_TRUE(CU_test_data.default_settings.cpr_msg_long <= 1);
    CU_ASSERT_TRUE(CU_test_data.default_settings.analysis_cpr <= 1);
    CU_ASSERT_TRUE(CU_test_data.default_settings.asys_detect <= 1);

    CU_ASSERT_TRUE(CU_test_data.default_settings.asys_time <= MAX_ASYSTOLE_TIME);

    CU_ASSERT_TRUE(CU_test_data.default_settings.metronome_en <= 1);
    CU_ASSERT_TRUE(CU_test_data.default_settings.metronome_ratio_a <= 2);
    CU_ASSERT_TRUE(CU_test_data.default_settings.metronome_ratio_p <= 2);
    CU_ASSERT_TRUE(CU_test_data.default_settings.metronome_rate <= 2);
    CU_ASSERT_TRUE(CU_test_data.default_settings.feedback_en <= 1);

    CU_ASSERT_TRUE(CU_test_data.default_settings.consec_shocks <= MAX_CONSEC_SHOCKS);
    CU_ASSERT_TRUE(CU_test_data.default_settings.metronome_en <= 1);
    CU_ASSERT_TRUE(CU_test_data.default_settings.metronome_en <= 1);

    CU_ASSERT_TRUE(CU_test_data.default_settings.energy_shock1_a >= 150);
    //CU_ASSERT_TRUE(CU_test_data.default_settings.energy_shock1_a == 175);
    CU_ASSERT_TRUE(CU_test_data.default_settings.energy_shock1_a <= 200);

    CU_ASSERT_TRUE(CU_test_data.default_settings.energy_shock2_a >= 150);
    //CU_ASSERT_TRUE(CU_test_data.default_settings.energy_shock2_a == 175);
    CU_ASSERT_TRUE(CU_test_data.default_settings.energy_shock2_a <= 200);

    CU_ASSERT_TRUE(CU_test_data.default_settings.energy_shock3_a >= 150);
    //CU_ASSERT_TRUE(CU_test_data.default_settings.energy_shock3_a == 175);
    CU_ASSERT_TRUE(CU_test_data.default_settings.energy_shock3_a <= 200);

    CU_ASSERT_TRUE(CU_test_data.default_settings.energy_shock1_p >= 50);
    //CU_ASSERT_TRUE(CU_test_data.default_settings.energy_shock1_p == 65);
    //CU_ASSERT_TRUE(CU_test_data.default_settings.energy_shock1_p == 75);
    //CU_ASSERT_TRUE(CU_test_data.default_settings.energy_shock1_p == 90);
    CU_ASSERT_TRUE(CU_test_data.default_settings.energy_shock1_p <= 100);

    CU_ASSERT_TRUE(CU_test_data.default_settings.energy_shock2_p >= 50);
    //CU_ASSERT_TRUE(CU_test_data.default_settings.energy_shock2_p == 65);
    //CU_ASSERT_TRUE(CU_test_data.default_settings.energy_shock2_p == 75);
    //CU_ASSERT_TRUE(CU_test_data.default_settings.energy_shock2_p == 90);
    CU_ASSERT_TRUE(CU_test_data.default_settings.energy_shock2_p <= 100);

    CU_ASSERT_TRUE(CU_test_data.default_settings.energy_shock3_p >= 50);
    //CU_ASSERT_TRUE(CU_test_data.default_settings.energy_shock3_p == 65);
    //CU_ASSERT_TRUE(CU_test_data.default_settings.energy_shock3_p == 75);
    //CU_ASSERT_TRUE(CU_test_data.default_settings.energy_shock3_p == 90);
    CU_ASSERT_TRUE(CU_test_data.default_settings.energy_shock3_p <= 100);

    CU_ASSERT_TRUE(CU_test_data.default_settings.patient_type <= 1);
    CU_ASSERT_TRUE(CU_test_data.default_settings.mains_filter <= 2);
    CU_ASSERT_TRUE(CU_test_data.default_settings.language <= MAX_LANGUAGES);
    CU_ASSERT_TRUE(CU_test_data.default_settings.audio_volume <= MAX_AUDIO_VOLUME);
    CU_ASSERT_TRUE(CU_test_data.default_settings.audio_volume != 0);

    CU_ASSERT_TRUE(CU_test_data.default_settings.utc_time >= UTC_MIN);
    CU_ASSERT_TRUE(CU_test_data.default_settings.utc_time <= UTC_MAX);
    CU_ASSERT_TRUE(CU_test_data.default_settings.demo_mode <= 1);
    CU_ASSERT_TRUE(CU_test_data.default_settings.patient_alert <= 1);
    CU_ASSERT_TRUE(CU_test_data.default_settings.movement_alert <= 1);

    CU_ASSERT_TRUE(CU_test_data.default_settings.transmis_mode <= 0b11111111);
    CU_ASSERT_TRUE(CU_test_data.default_settings.warning_alert <= 1);
}

void test_Electrodes_Get_Signatures(){
    int result = 0;
    uint8_t CU_pon_date = 0;
    char str[15];

    // electrodes_presence_flag == 0
    setElectrodes_presence_flag(false);

        // electrodes_zp_segment = eZP_SEGMENT_OPEN_CIRC
        setElectrodes_zp_segment(eZP_SEGMENT_OPEN_CIRC);
        result = Electrodes_Get_Signature();
        CU_ASSERT_EQUAL(result, eEL_SIGN_NONE);

        //electrodes_zp_segment != eZP_SEGMENT_OPEN_CIRC
        setElectrodes_zp_segment(eZP_SEGMENT_SHORT_CIRC);
        result = Electrodes_Get_Signature();
        CU_ASSERT_EQUAL(result, eEL_SIGN_UNKNOWN);

    // electrodes_presence_flag == 1
       setElectrodes_presence_flag(true);

        // Mode Demo = TRUE
        strcpy(str, "DEMO");
        setMode_Demo(str);

        strcpy(str, "TRAINER");
        setElectrodes_data_Name(str);

        result = Electrodes_Get_Signature();
        CU_ASSERT_EQUAL(result, eEL_SIGN_DEMO);

        strcpy(str, "TEST");
        setElectrodes_data_Name(str);
        result = Electrodes_Get_Signature();
        CU_ASSERT_EQUAL(result, eEL_SIGN_MUST_DEMO);

        //Mode Demo = FALSE
        setMode_Demo(str);
        strcpy(str, "TRAINER");
        setElectrodes_data_Name(str);

        result = Electrodes_Get_Signature();
        CU_ASSERT_EQUAL(result, eEL_SIGN_MUST_DEMO);

        // electrodes_data.info.expiration_date >= pon_date
        strcpy(str, "TEST");
        setMode_Demo(str);
        setElectrodes_data_Name(str);

        setPon_Date(2);
        setElectrodes_data_ExpDate(2);

        result = Electrodes_Get_Signature();
        CU_ASSERT_EQUAL(result, eEL_SIGN_BEXEN);

        // electrodes_data.info.expiration_date
        setElectrodes_data_ExpDate(1);
        result = Electrodes_Get_Signature();
        CU_ASSERT_EQUAL(result, eEL_SIGN_BEXEN_EXPIRED);

        // else
        setElectrodes_data_ExpDate(0);
        result = Electrodes_Get_Signature();
        CU_ASSERT_EQUAL(result, eEL_SIGN_UNKNOWN);
}

void test_bsp_clock_init_Reconfigure(){
    CU_ERRORS err;
    err = bsp_clock_init_Reconfigure();

    CU_ASSERT_EQUAL(err, CU_SUCCESS);

    if(err == CU_ERR_CLOCK_START || err == CU_ERR_SYSTEM_CLOCK_SET){
        BSP_CFG_HANDLE_UNRECOVERABLE_ERROR(0);
    }
}


void test_Comm_Uart_Set_Baud_Rate(){
    int32_t CU_baud_rate, baud_rate = 9600;
    ERROR_ID_e err;

    Comm_Uart_Init();

    err = Comm_Uart_Set_Baud_Rate(baud_rate);
    CU_baud_rate = commUART.p_cfg->baud_rate;

    CU_ASSERT_EQUAL(CU_baud_rate, baud_rate);
    CU_ASSERT_EQUAL(err, eERR_NONE);

    baud_rate = -1;
    err = Comm_Uart_Set_Baud_Rate(baud_rate);
    CU_ASSERT_EQUAL(err, eERR_COMM_UART_BAUD_RATE);
}

void test_Execute_Calibration(){
    CU_Execute_Calibration();
}

void test_Settings_Open_From_uSD(){
    Settings_Open_From_uSD();
}

void test_Load_FW_File(){
    CU_Load_FW_File();
    // TRUE es que hay actualización
    // FALSE es que no hay actualización
}

void test_Battery_Get_Charge(){
    uint16_t result;
    char str[15];

    /*********************
     ****   ERRORES    ***
     *********************/
    // (battery_info.must_be_0xAA != 0xAA) || (battery_statistics.must_be_0x55 != 0x55)
    set_battery_info_integrity(1);
    set_battery_statistics_integrity(1);

    result = Battery_Get_Charge();
    CU_ASSERT_EQUAL(result, 0);

    set_battery_info_integrity(0xAA);
    set_battery_statistics_integrity(0x55);

    // nominal_capacity == 0
    set_nominal_capacity(0);

    result = Battery_Get_Charge();
    CU_ASSERT_EQUAL(result, 0);

    set_nominal_capacity(1500);

    // my_date == 132382977
    set_battery_RTC_date(20,1,1);

    result = Battery_Get_Charge();
    CU_ASSERT_EQUAL(result, 0);

    set_battery_RTC_date(19,6,16);

    /*********************
     ****   ERRORES    ***
     *********************/

    /*
     *
     * Comprobar en "diff_time_days (my_date, battery_info.manufacture_date)"
     *
     *  Si se da la situacion donde el valor de manufacture_date es mayor, se devuelve 0.
     *  No se deberia acabar la funcion ya que se han leido mal los datos
     *
     */

    //  PRUEBA 1 - The remaining capacity is bigger than 100% return 1
    result = Battery_Get_Charge(); // ndays es muy alto por lo que devuelve 1
    CU_ASSERT_EQUAL(result, 1);

    //  PRUEBA 2 - The remaining capacity is less than 100% then calculate the remaining capacity
    set_battery_manufacture_date(24,4,16);
    set_battery_RTC_date(24,6,30);

    set_daily_test(3);

    set_runTime(5);

    set_charges(1);

    set_open_cover(2);

    result = Battery_Get_Charge();
    CU_ASSERT_EQUAL(result, 68);

}

void test_NFC_Write_Device_Info(){
    bool_t CU_flag_write;
    uint64_t available_bytes;
    uint32_t err_open;

    NFC_DEVICE_INFO_t   mi_nfc_device_info, original_nfc_device_info;
    BATTERY_INFO_t      mi_battery_info;
    ELECTRODES_DATA_t    mi_electrodes_data;

    Battery_Get_Info (&mi_battery_info);

    memcpy(mi_nfc_device_info.battery_sn, mi_battery_info.sn, RSC_NAME_SZ);
    mi_nfc_device_info.battery_manufacture_date = mi_battery_info.manufacture_date;
    mi_nfc_device_info.battery_expiration_date  = mi_battery_info.expiration_date;
    mi_nfc_device_info.battery_cap = (uint16_t) mi_battery_info.nominal_capacity;
    mi_nfc_device_info.battery_rem = (uint16_t) Battery_Get_Charge();

    Electrodes_Get_Data (&mi_electrodes_data);

    memcpy (&mi_nfc_device_info.electrodes_sn1, &(((uint8_t *) &mi_electrodes_data.sn) [4]), 4);
    memcpy (&mi_nfc_device_info.electrodes_sn2, &(((uint8_t *) &mi_electrodes_data.sn) [0]), 4);
    mi_nfc_device_info.electrodes_manufacture_date = mi_electrodes_data.info.manufacture_date;
    mi_nfc_device_info.electrodes_expiration_date  = mi_electrodes_data.info.expiration_date;
    mi_nfc_device_info.electrodes_event_date       = mi_electrodes_data.event.date;
    mi_nfc_device_info.electrodes_event_time       = mi_electrodes_data.event.time;
    mi_nfc_device_info.electrodes_event_id         = mi_electrodes_data.event.id;

    mi_nfc_device_info.app_sw_version = APP_REV_CODE;
    mi_nfc_device_info.error_code = Get_NV_Data()->error_code;

    /* SWAP */

    NFC_Write_Device_Info(CU_flag_write);
    original_nfc_device_info = *(Get_NFC_Device_Info());

    /* EMPEZAR A COMPARAR*/
    mi_nfc_device_info.internal_memory_cap = 7372;
    mi_nfc_device_info.internal_memory_rem = 7372;

    fx_media_extended_space_available(&sd_fx_media, &available_bytes);
    mi_nfc_device_info.internal_memory_rem = (uint16_t) (available_bytes / (1024 * 1024));

    /*
     * Crear MI estructura NFC_DEVICE_INFO_t
     *
     * Llamar a las funciones Battery_Get_Info (&my_battery_info) y Electrodes_Get_Data (&my_electrodes_data)
     * para llenar MI estructura
     *
     * Hacer SWAP de los todos los campos
     *
     * Llamar a la funcion original
     *
     * Comparar la estructura original con MI estructura
     *
     *      [igual se puede usar NFC_READ para obtener la estructura original]
     */
}

void test_NV_Data_Write(){
    NV_DATA_t CU_test_data;
    NV_DATA_BLOCK_t CU_test_data_block;
    CU_ERRORS CU_err;

    CU_err = NV_Data_Read(&CU_test_data, &CU_test_data_block);

    CU_ASSERT_EQUAL(CU_err, CU_SUCCESS);

    CU_err = NV_Data_Write(&CU_test_data, &CU_test_data_block);
    CU_ASSERT_EQUAL(CU_err, CU_ERR_NO_CHANGE_REGISTERED);

   CU_test_data.error_code = eERR_NONE;
   CU_test_data_block.zp_adcs_short = 10;

    CU_err = NV_Data_Write(&CU_test_data, &CU_test_data_block);
    CU_ASSERT_EQUAL(CU_err, CU_ERR_CHANGES_DONE);
}

void test_NFC_Write_Device_ID(){
    bool_t CU_flag_write;
    bool_t CU_flag_cal_write;

    NFC_Write_Device_ID(CU_flag_write, CU_flag_cal_write);
}

void test_NFC_Write_Settings(){
    NFC_SETTINGS_t CU_nfc_settings, CU_original_nfc_settings;

    CU_NFC_Read ((uint8_t *) &CU_nfc_settings, sizeof (NFC_SETTINGS_t), NFC_DEVICE_SETTINGS_BLOCK);

    NFC_Write_Settings();

    CU_original_nfc_settings = *(CU_Get_NFC_Settings());

    CU_Settings_Swap(&CU_original_nfc_settings);

    //CU_NFC_Read ((uint8_t *) &CU_original_nfc_settings, sizeof (NFC_SETTINGS_t), NFC_DEVICE_SETTINGS_BLOCK);

    //CU_Settings_Swap(&CU_original_nfc_settings);

    CU_ASSERT_EQUAL(memcmp(&CU_nfc_settings, &CU_original_nfc_settings, sizeof(NFC_SETTINGS_t)), 0);

    // volver a hacer el swap al reves y mirar que vuelve a tener el valor del principio
    /*
     * leer los datos antes, pasar la funcion, pasar la funcion swap y comprobar que tiene el valor del principio
     */
}


static CU_TestInfo tests_thread_sysMon[] = {
    /*{"test_Check_Battery_Voltage", test_Check_Battery_Voltage},
    {"test_Is_SAT_Error", test_Is_SAT_Error},
    {"test_NV_Data_Read", test_NV_Data_Read},
    {"test_Electrodes_Get_Signatures", test_Electrodes_Get_Signatures},
    {"test_bsp_clock_init_Reconfigure", test_bsp_clock_init_Reconfigure},
    {"test_Comm_Uart_Set_Baud_Rate", test_Comm_Uart_Set_Baud_Rate},*/

    //ERROR         {"test_Execute_Calibration", test_Execute_Calibration},
    //ERROR         {"test_Settings_Open_From_uSD", test_Settings_Open_From_uSD},

    //NO COMPROBAR  {"test_Load_FW_File", test_Load_FW_File},

    {"test_Battery_Get_Charge", test_Battery_Get_Charge},

    //{"test_NFC_Write_Device_Info", test_NFC_Write_Device_Info},

    //{"test_NV_Data_Write", test_NV_Data_Write},

    //{"test_NFC_Write_Device_ID", test_NFC_Write_Device_ID},// ¿QUE COMPROBAR?

    //{"test_NFC_Write_Settings", test_NFC_Write_Settings},

    CU_TEST_INFO_NULL,
};

// Declare the test suite in SuiteInfo
static CU_SuiteInfo suites[] = {
    {"TestSimpleAssert_Test_SysMon", NULL, NULL, tests_thread_sysMon},
    CU_SUITE_INFO_NULL,
};

void CU_test_sysMon(void) {
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

