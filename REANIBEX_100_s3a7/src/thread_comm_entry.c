/******************************************************************************
 * Name      : S3A7_REANIBEX_100                                              *
 * Copyright : Osatu S. Coop                                                  *
 * Compiler  : MinGW32                                                        *
 * Target    : Reanibex Series                                                *
 ******************************************************************************/

/*!
 * @file        thread_comm_entry.c
 * @brief
 *
 * @version     v1
 * @date        03/05/2019
 * @author      jarribas
 * @warning     author   date    version     .- Edition
 * @bug
 *
 */

/******************************************************************************
 ** Includes
 */
#include "thread_comm.h"
#include "HAL/thread_defibrillator_hal.h"
#include "thread_core_entry.h"
#include "thread_comm_entry.h"
#include "thread_drd_entry.h"
#include "thread_acc_entry.h"
#include "sysMon_Battery.h"
#include "Comm.h"
#include "Keypad.h"
#include "event_ids.h"
#include "Trace.h"
#include "sysMon_RF_Comms.h"

/******************************************************************************
 ** Macros
 */

/******************************************************************************
 ** Defines
 */
#define THREAD_COMM_EVENT_TRACE TRACE_NEWLINE       ///< Enable/disable traces for Thread comm events

#define ASCII_TO_DEC(x)                 (uint8_t) ((x >= '0')? (x-'0'):0)

/******************************************************************************
 ** Typedefs
 */

/******************************************************************************
 ** Constants
 */

/******************************************************************************
 ** Externals
 */
extern SIGFOX_TEST_RESULT_t  sigfox_test_result;                ///< Test result structure
extern SIGFOX_FUNC_ALERT_t   sigfox_func_alert;                 ///< Functional alert structure
extern SIGFOX_GPS_POSITION_t sigfox_gps_position;               ///< GPS position structure
extern SIGFOX_ELEC_BATT_EXPIRATION_t   sigfox_expiration;       ///< Battery and electrodes expiration dates

extern WIFI_TEST_RESULT_t  wifi_test_result;        ///< Test result structure
extern WIFI_FUNC_ALERT_t   wifi_func_alert;         ///< Functional alert structure
extern WIFI_FUNC_ALERT_t   wifi_func_alert_aux;     ///< Auxiliar functional alert structure
extern WIFI_GPS_POSITION_t wifi_gps_position;       ///< Functional alert structure

extern gps_config_t          R100_gps;
extern uint16_t              FSM_shock_energy;           ///< Energy to apply in the shock

/******************************************************************************
 ** Globals
 */
EV_COMM_e                    tx_event;
ERROR_ID_e                   err;
ERROR_ID_e                   sigfox_error;
static bool_t                uplink_test = FALSE;     ///< Performing uplink test
static DEVICE_INFO_t         my_Device_Info;          ///< Pointer to the device info

static bool_t                wifi_initialized           = FALSE;
static bool_t                wifi_init_finished         = FALSE;
static bool_t                wifi_host_alive_flag       = FALSE;
static bool_t                wifi_host_alive_finished   = FALSE;
static bool_t                wifi_test_frame_sended     = FALSE;
static bool_t                wifi_alert_frame_sended    = FALSE;
static bool_t                wifi_position_frame_sended = FALSE;
static bool_t                wifi_test_sended           = FALSE;
static bool_t                wifi_episode_sended        = FALSE;
static bool_t                wifi_firmware_received     = FALSE;
static bool_t                wifi_config_received       = FALSE;
static bool_t                wifi_hw_test_finished      = FALSE;
static ERROR_ID_e            wifi_hw_test_error         = eERR_NONE;

static bool_t                is_wifi_free               = TRUE;

static bool_t                sigfox_initialized         = FALSE;
static bool_t                sigfox_first_time          = FALSE;
static bool_t                sigfox_alert_frame_sended  = FALSE;
static bool_t                sigfox_test_sended         = FALSE;
static bool_t                sigfox_exp_sended          = FALSE;
static bool_t                sigfox_gps_sended          = FALSE;
static bool_t                sigfox_hw_test_finished    = FALSE;
static ERROR_ID_e            sigfox_hw_test_error       = eERR_NONE;

static bool_t                gps_hw_test_finished       = FALSE;
static ERROR_ID_e            gps_hw_test_error          = eERR_NONE;
static ERROR_ID_e            gps_cmd_send               = eERR_COMM_GPS;

static bool_t                acc_hw_test_finished       = FALSE;
static ERROR_ID_e            acc_hw_test_error          = eERR_NONE;

bool_t switch_alert = FALSE;

/******************************************************************************
 ** Locals
 */

/******************************************************************************
 ** Prototypes
 */
static bool_t Process_Comm_All_Modules (EV_COMM_e ev);
static bool_t Process_Comm_Sigfox      (EV_COMM_e ev);
static bool_t Process_Comm_Wifi        (EV_COMM_e ev);
static bool_t Process_Comm_GPS         (EV_COMM_e ev);
static bool_t Process_Comm_ACC         (EV_COMM_e ev);

/******************************************************************************
 ** Name:    Comm_Initialize
 *****************************************************************************/
/**
 ** @brief   Thread COMM entry function
 ** @param   void
 **
 ******************************************************************************/
void thread_comm_entry(void)
{
    EV_COMM_e ev = EV_COMM_MAX;

    Comm_Modules_Initialize();

    // get the device info
    memcpy(&my_Device_Info, Get_Device_Info(), sizeof(DEVICE_INFO_t));
/*
    while (1)
    {
        ev = EV_COMM_MAX;
        err = eERR_NONE;

        (void) tx_queue_receive(&queue_comm, &ev, TX_WAIT_FOREVER);
        //ASSERT_DEBUG(err == (uint8_t)OS_ERR_NONE);
        // Check the received command
        //ASSERT_DEBUG(event_id > (uint32_t) 0);
        //ASSERT_DEBUG(event_id < (uint32_t) eEV_SET_MAX);

        // Process the event
        if (Process_Comm_All_Modules (ev))       // Process ALL MODULES COMM related events
        {
            Trace_Arg (THREAD_COMM_EVENT_TRACE, "  Process_Comm_All = %2d", (uint32_t) ev);
        }
        else if (Process_Comm_Sigfox (ev))    // Process SIGFOX COMM related events
        {
            Trace_Arg (THREAD_COMM_EVENT_TRACE, "  Process_Comm_Sigfox = %2d", (uint32_t) ev);
        }
        else if (Process_Comm_Wifi (ev))      // Process WIFI COMM related events
        {
            Trace_Arg (THREAD_COMM_EVENT_TRACE, "  Process_Comm_Wifi = %2d", (uint32_t) ev);
        }
        else if (Process_Comm_GPS (ev))       // Process GPS COMM settings related events
        {
            Trace_Arg (THREAD_COMM_EVENT_TRACE, "  Process_Comm_GPS = %2d", (uint32_t) ev);
        }
        else if (Process_Comm_ACC (ev))       // Process GPS COMM settings related events
        {
            Trace_Arg (THREAD_COMM_EVENT_TRACE, "  Process_Comm_ACC = %2d", (uint32_t) ev);
        }
        else
        {
            Trace (THREAD_COMM_EVENT_TRACE, "  Wrong Event!!!");
        }

        (void) tx_thread_sleep (OSTIME_50MSEC);
    }
*/
}

/******************************************************************************
** Name:    Send_Comm_Queue
*****************************************************************************/
/**
** @brief   Process events related to All COMM modules
**
** @param   ev event identifier
******************************************************************************/
static void Send_Comm_Queue(EV_COMM_e ev)
{
    //if (Is_Battery_Mode_Demo()) return;
    //Trace_Arg (TRACE_NEWLINE, "  *****************QUEUE AVAILABLE = %5d", (uint32_t) (queue_comm.tx_queue_available_storage));
    if (queue_comm.tx_queue_available_storage == 0) return;
    (void) tx_queue_send(&queue_comm, &ev, TX_WAIT_FOREVER);
}

/******************************************************************************
** Name:    Process_Comm_All_Modules
*****************************************************************************/
/**
** @brief   Process events related to All COMM modules
**
** @param   ev event identifier
**
** @return  if the event has been processed or not
**
******************************************************************************/
static bool_t Process_Comm_All_Modules(EV_COMM_e ev)
{
    bool_t event_processed = TRUE; // event processed variable, initialize as processed

    // Process the event
    switch (ev)
    {
        case eEV_COMM_MODULES_INITIALIZE:

            err = Comm_GPS_Initialize();
            if(err != eERR_NONE)
            {
                Set_NV_Data_Error_IF_NOT_SAT_Comms(err);
            }
            else
            {
                Comm_Modules_Print_Configuration(eMOD_GPS);
            }

            err = Comm_Wifi_Initialize();
            if(err != eERR_NONE)
            {
                Set_NV_Data_Error_IF_NOT_SAT_Comms(err);
            }
            else
            {
                Comm_Modules_Print_Configuration(eMOD_WIFI);
            }

        break;

        default:
            event_processed = FALSE;
        break;
    }

    // return event processed or not!!!
    return event_processed;
}

/******************************************************************************
** Name:    Process_Comm_Sigfox
*****************************************************************************/
/**
** @brief   Process events related to SIGFOX COMM module
**
** @param   ev event identifier
**
** @return  if the event has been processed or not
**
******************************************************************************/
static bool_t Process_Comm_Sigfox(EV_COMM_e ev)
{
    bool_t event_processed = TRUE; // event processed variable, initialize as processed

    // Process the event
    switch (ev)
    {
        case eEV_COMM_SIGFOX_INITIALIZE:

            sigfox_initialized = FALSE;
            err = Comm_Sigfox_Initialize();
            sigfox_error = err;
            if(err != eERR_NONE)
            {
                Set_NV_Data_Error_IF_NOT_SAT_Comms(err);
                sigfox_initialized = FALSE;
            }
            else
            {
                if(sigfox_first_time == FALSE)
                {
                    Comm_Modules_Print_Configuration(eMOD_SIGFOX);
                    sigfox_first_time = TRUE;
                }
                sigfox_initialized = TRUE;
            }

        break;

        case eEV_COMM_SIGFOX_UPLINK_TEST:

            if(uplink_test == TRUE) // If performing test, cancel it
            {
                err = Comm_Sigfox_Stop_Uplink_Test();
                sigfox_error = err;
                if(err != eERR_NONE)
                {
                    Set_NV_Data_Error_IF_NOT_SAT_Comms(err);
                    break;
                }
                uplink_test = FALSE;
                Led_Off(LED_SHOCK);
            }
            else  // Start Uplink Test
            {
                err = Comm_Sigfox_Start_Uplink_Test();
                sigfox_error = err;
                if(err != eERR_NONE)
                {
                    Set_NV_Data_Error_IF_NOT_SAT_Comms(err);
                    break;
                }
                uplink_test = TRUE;
                Led_Blink(LED_SHOCK);
            }

        break;

        case eEV_COMM_SIGFOX_DOWNLINK_TEST:

            err = Comm_Sigfox_Start_Downlink_Test();
            sigfox_error = err;
            if(err != eERR_NONE)
            {
                Set_NV_Data_Error_IF_NOT_SAT_Comms(err);
            }

        break;

        case eEV_COMM_SIGFOX_SEND_TEST:

            sigfox_test_sended = FALSE;
            err = Comm_Sigfox_Send_Test();
            sigfox_error = err;
            if(err != eERR_NONE)
            {
                Set_NV_Data_Error_IF_NOT_SAT_Comms(err);
            }
            sigfox_test_sended = TRUE;

        break;

        case eEV_COMM_SIGFOX_SEND_ALERT:

            sigfox_alert_frame_sended = FALSE;
            err = Comm_Sigfox_Send_Alert();
            sigfox_error = err;
            if(err != eERR_NONE)
            {
                Set_NV_Data_Error_IF_NOT_SAT_Comms(err);
            }
            sigfox_alert_frame_sended = TRUE;

        break;

        case eEV_COMM_SIGFOX_SEND_EXP:

            sigfox_exp_sended = FALSE;
            err = Comm_Sigfox_Send_Exp();
            sigfox_error = err;
            if(err != eERR_NONE)
            {
                Set_NV_Data_Error_IF_NOT_SAT_Comms(err);
            }
            sigfox_exp_sended = TRUE;

        break;

        case eEV_COMM_SIGFOX_SEND_POSITION:

            sigfox_gps_sended = FALSE;
            err = Comm_Sigfox_Send_Position();
            sigfox_error = err;
            if(err != eERR_NONE)
            {
                Set_NV_Data_Error_IF_NOT_SAT_Comms(err);
            }
            sigfox_gps_sended = TRUE;

        break;

        case eEV_COMM_SIGFOX_EXECUTE_TEST:

            sigfox_hw_test_finished = FALSE;
            sigfox_hw_test_error = Comm_Sigfox_Execute_Test();
            sigfox_error = sigfox_hw_test_error;
            if(sigfox_hw_test_error != eERR_NONE)
            {
                Set_NV_Data_Error_IF_NOT_SAT_Comms(sigfox_hw_test_error);
            }
            sigfox_hw_test_finished = TRUE;
        break;

        default:
            event_processed = FALSE;
        break;

    }

    // return event processed or not!!!
    return event_processed;
}

/******************************************************************************
** Name:    Process_Comm_Wifi
*****************************************************************************/
/**
** @brief   Process events related to WIFI COMM module
**
** @param   ev event identifier
**
** @return  if the event has been processed or not
**
******************************************************************************/
static bool_t Process_Comm_Wifi(EV_COMM_e ev)
{
    bool_t event_processed = TRUE; // event processed variable, initialize as processed

    if(ev == eEV_COMM_WIFI_INITIALIZE)
    {
        if(Comm_Is_Wifi_Initialized() == TRUE && Comm_Get_Selected_Uart() == eMOD_WIFI)
        {
            return event_processed;
        }
        if (Comm_Is_Wifi_Initialized() == TRUE && Comm_Get_Selected_Uart() != eMOD_WIFI)
        {
            Comm_Wifi_Initialize_Fast();
            return event_processed;
        }
    }

    // Process the event
    switch (ev)
    {
        case eEV_COMM_WIFI_INITIALIZE:
            is_wifi_free = FALSE;
            wifi_initialized = FALSE;
            err = Comm_Wifi_Initialize();
            is_wifi_free = TRUE;
            if(err != eERR_NONE)
            {
                // Turn off Wifi Radio (just in case)
                err = Comm_Wifi_Radio_Off();

                err = Comm_Wifi_Initialize();
                if(err != eERR_NONE)
                {
                    Set_NV_Data_Error_IF_NOT_SAT_Comms(err);
                    wifi_initialized = FALSE;
                    wifi_init_finished = TRUE;
                }
                else
                {
                    wifi_initialized = TRUE;
                    Comm_Modules_Print_Configuration(eMOD_WIFI);
                }
            }
            else
            {
                wifi_initialized = TRUE;
                Comm_Modules_Print_Configuration(eMOD_WIFI);
            }

        break;

        case eEV_COMM_WIFI_IS_SERVER_ALIVE:

            is_wifi_free = FALSE;
            wifi_host_alive_finished = FALSE;
            err = Comm_Wifi_Is_Host_Server_Alive();
            is_wifi_free = TRUE;
            if(err != eERR_NONE)
            {
                Set_NV_Data_Error_IF_NOT_SAT_Comms(err);
            }
            else
            {
                wifi_host_alive_flag = TRUE;
                sigfox_exp_sended = TRUE;
            }
            wifi_host_alive_finished = TRUE;

        break;

        case eEV_COMM_WIFI_SEND_FRAME_TEST:

            is_wifi_free = FALSE;
            wifi_test_frame_sended = FALSE;
            err = Comm_Wifi_Send_Frame_Test();
            is_wifi_free = TRUE;
            if(err != eERR_NONE)
            {
                Set_NV_Data_Error_IF_NOT_SAT_Comms(err);
                sigfox_initialized = FALSE;
                if(Is_Wifi_TX_Enabled() == WIFI_PRIOR && Is_Sigfox_TX_Enabled() == WIFI_PRIOR)
                {
                    Send_Sigfox_Test(&R100_test_result);
                }
            }
            else sigfox_exp_sended = TRUE;
            wifi_test_frame_sended = TRUE;

        break;

        case eEV_COMM_WIFI_SEND_FRAME_ALERT:

            is_wifi_free = FALSE;
            wifi_alert_frame_sended = FALSE;
            err = Comm_Wifi_Send_Frame_Alert();
            is_wifi_free = TRUE;
            if(err != eERR_NONE)
            {
                Set_NV_Data_Error_IF_NOT_SAT_Comms(err);
                if(Is_Wifi_TX_Enabled() == WIFI_PRIOR && Is_Sigfox_TX_Enabled() == WIFI_PRIOR)
                {
                    sigfox_func_alert.info_type = wifi_func_alert.info_type;
                    Send_Sigfox_Alert(sigfox_func_alert.info_type);
                }
            }
            else sigfox_alert_frame_sended = TRUE;
            wifi_alert_frame_sended = TRUE;
            switch_alert = FALSE;

        break;

        case eEV_COMM_WIFI_SEND_FRAME_ALERT_AUX:

            is_wifi_free = FALSE;
            wifi_alert_frame_sended = FALSE;
            err = Comm_Wifi_Send_Frame_Alert_Aux();
            is_wifi_free = TRUE;
            if(err != eERR_NONE)
            {
                Set_NV_Data_Error_IF_NOT_SAT_Comms(err);
                if(Is_Wifi_TX_Enabled() == WIFI_PRIOR && Is_Sigfox_TX_Enabled() == WIFI_PRIOR)
                {
                    sigfox_func_alert.info_type = wifi_func_alert.info_type;
                    Send_Sigfox_Alert(sigfox_func_alert.info_type);
                }
            }
            else sigfox_alert_frame_sended = TRUE;
            wifi_alert_frame_sended = TRUE;
            switch_alert = FALSE;

        break;

        case eEV_COMM_WIFI_SEND_FRAME_GPS:

            is_wifi_free = FALSE;
            wifi_position_frame_sended = FALSE;
            err = Comm_Wifi_Send_Frame_Gps();
            is_wifi_free = TRUE;
            if(err != eERR_NONE)
            {
                Set_NV_Data_Error_IF_NOT_SAT_Comms(err);
                if(Is_Wifi_TX_Enabled() == WIFI_PRIOR && Is_Sigfox_TX_Enabled() == WIFI_PRIOR)
                {
                    sigfox_func_alert.info_type = wifi_func_alert.info_type;
                    Send_Sigfox_Position(sigfox_func_alert.info_type);
                }
            }
            else sigfox_gps_sended = TRUE;
            wifi_position_frame_sended = TRUE;

        break;

        case eEV_COMM_WIFI_SEND_TEST:

            is_wifi_free = FALSE;
            wifi_test_sended = FALSE;
            err = Comm_Wifi_Send_Last_Test();
            is_wifi_free = TRUE;
            if(err != eERR_NONE)
            {
                Set_NV_Data_Error_IF_NOT_SAT_Comms(err);
            }
            wifi_test_sended = TRUE;
        break;

        case eEV_COMM_WIFI_SEND_EPISODE:

            is_wifi_free = FALSE;
            wifi_episode_sended = FALSE;
            err = Comm_Wifi_Send_Last_Episode();
            is_wifi_free = TRUE;
            if(err != eERR_NONE)
            {
                Set_NV_Data_Error_IF_NOT_SAT_Comms(err);
            }
            wifi_episode_sended = TRUE;

        break;

        case eEV_COMM_WIFI_GET_UPGRADE_FILE:

            is_wifi_free = FALSE;
            wifi_firmware_received = FALSE;
            err = Comm_Wifi_Get_Upgrade_Firmware();
            is_wifi_free = TRUE;
            if(err != eERR_NONE)
            {
                Set_NV_Data_Error_IF_NOT_SAT_Comms(err);
            }
            wifi_firmware_received = TRUE;
        break;

        case eEV_COMM_WIFI_GET_CONFIG_FILE:

            is_wifi_free = FALSE;
            wifi_config_received = FALSE;
            err = Comm_Wifi_Get_Configuration_File();
            is_wifi_free = TRUE;
            if(err != eERR_NONE)
            {
                Set_NV_Data_Error_IF_NOT_SAT_Comms(err);
            }
            wifi_config_received = TRUE;
        break;

        case eEV_COMM_WIFI_EXECUTE_TEST:

            is_wifi_free = FALSE;
            wifi_hw_test_finished = FALSE;
            wifi_hw_test_error = Comm_Wifi_Execute_Test();
            is_wifi_free = TRUE;
            if(wifi_hw_test_error != eERR_NONE)
            {
                Set_NV_Data_Error_IF_NOT_SAT_Comms(wifi_hw_test_error);
            }
            wifi_hw_test_finished = TRUE;

        break;

        default:
            event_processed = FALSE;
        break;
    }

    // return event processed or not!!!
    return event_processed;
}

/******************************************************************************
** Name:    Process_Comm_GPS
*****************************************************************************/
/**
** @brief   Process events related to GPS COMM module
**
** @param   ev event identifier
**
** @return  if the event has been processed or not
**
******************************************************************************/
static bool_t Process_Comm_GPS(EV_COMM_e ev)
{
    bool_t event_processed = TRUE; // event processed variable, initialize as processed

    // Process the event
    switch (ev)
    {
        case eEV_COMM_GPS_INITIALIZE:

            err = Comm_GPS_Initialize();
            if(err != eERR_NONE)
            {
                Set_NV_Data_Error_IF_NOT_SAT_Comms(err);
            }
            else
            {
                //Comm_Modules_Print_Configuration(eMOD_GPS);
            }

        break;

        case eEV_COMM_GPS_GET_POS_DATA:

            err = Comm_GPS_Get_Position_Data();
            if(err != eERR_NONE)
            {
                Set_NV_Data_Error_IF_NOT_SAT_Comms(err);
            }

        break;

        case eEV_COMM_GPS_EXECUTE_TEST:

            gps_hw_test_finished = FALSE;
            gps_hw_test_error = Comm_GPS_Execute_Test();
            if(gps_hw_test_error != eERR_NONE)
            {
                Set_NV_Data_Error_IF_NOT_SAT_Comms(gps_hw_test_error);
            }
            gps_hw_test_finished = TRUE;
        break;

        case eEV_COMM_GPS_SEND_CMD:
            gps_cmd_send = Comm_GPS_Send_Command();
            if(gps_cmd_send != eERR_NONE)
            {
                Set_NV_Data_Error_IF_NOT_SAT_Comms(gps_cmd_send);
            }

        break;

        default:
            event_processed = FALSE;
        break;
    }

    // return event processed or not!!!
    return event_processed;
}

/******************************************************************************
** Name:    Comm_Sigfox_Open
*****************************************************************************/
/**
** @brief   Open Sigfox Communication
** @param   void
**
** @return  none
*****************************************************************************/
void Comm_Sigfox_Open(void)
{
    // check if the Sigfox option is enabled
    if (!my_Device_Info.enable_b.sigfox) return;

    // Initialize Sigfox
    Send_Comm_Queue(eEV_COMM_SIGFOX_INITIALIZE);
}

/******************************************************************************
** Name:    Comm_Sigfox_Generate_Send_Test
*****************************************************************************/
/**
** @brief   Function to send the test result to Sigfox cloud
**
** @param   pResult     pointer to result parameters
** @param   auto_test   if the test is automatic or forced by an user
**
** @return  none
**
******************************************************************************/
void Comm_Sigfox_Generate_Send_Test (DB_TEST_RESULT_t *pResult)
{
    // check if the Sigfox option is enabled
    if (!my_Device_Info.enable_b.sigfox) return;

    // format the info to report
    memset ((uint8_t *) &sigfox_test_result, 0, sizeof (SIGFOX_TEST_RESULT_t));

    if(pResult->test_id < TEST_FULL_ID)
    {
        sigfox_test_result.info_type = MSG_ID_DIARY_TEST;
    }
    else if(pResult->test_id == TEST_FULL_ID)
    {
        sigfox_test_result.info_type = MSG_ID_MONTHLY_TEST;
    }
    else if(pResult->test_id == TEST_MANUAL_ID)
    {
        sigfox_test_result.info_type = MSG_ID_MANUAL_TEST;
    }
    else if(pResult->test_id == TEST_INITIAL_ID)
    {
        //if (Is_Battery_Mode_Demo()) return;
        sigfox_test_result.info_type = MSG_ID_POWER_ON_TEST;
    }
    else if(pResult->test_id == TEST_FUNC_ID)
    {
        //if (Is_Battery_Mode_Demo()) return;
        sigfox_test_result.info_type = MSG_ID_FUNC_TEST;
    }

    sigfox_test_result.error_code_defib = (uint8_t) pResult->defib.error_code;
    if (pResult->patmon.ADS_cal)   sigfox_test_result.error_code_ADS = (uint8_t) pResult->patmon.ADS_cal;
    if (pResult->patmon.ADS_comms) sigfox_test_result.error_code_ADS = (uint8_t) pResult->patmon.ADS_comms;
    sigfox_test_result.error_code_dc_18v  = (uint8_t) pResult->voltage.dc_18v_error;
    sigfox_test_result.error_code_dc_main = (uint8_t) pResult->voltage.dc_main_error;
    sigfox_test_result.error_code_audio   = (uint8_t) pResult->misc.audio;
    sigfox_test_result.error_electrodes   = (uint8_t) pResult->electrodes.error_code;
    sigfox_test_result.battery_soc  = (uint8_t) pResult->battery.rem_charge;
    sigfox_test_result.battery_volt = (uint8_t) (Defib_Get_Vbatt() / 100);
    sigfox_test_result.battery_temp = (int8_t) pResult->battery.bat_temperature;
    sigfox_test_result.error_code = (uint8_t) pResult->error_code;
    sigfox_test_result.must_be_0x55 = 0x55;

    // Send Test Result
    Send_Comm_Queue (eEV_COMM_SIGFOX_SEND_TEST);
}

/******************************************************************************
** Name:    Comm_Is_Sigfox_Test_Sended
*****************************************************************************/
/**
** @brief   Checks if sigfox test sended process completed
** @param   void
**
** @return  TRUE if sigfox test sended and FALSE if not
*****************************************************************************/
bool_t Comm_Is_Sigfox_Test_Sended(void)
{
    // check if the Sigfox option is enabled
    if (!my_Device_Info.enable_b.sigfox) return FALSE;

    return sigfox_test_sended;
}

/******************************************************************************
 ** Name:    Comm_Sigfox_Generate_Send_Alert
 *****************************************************************************/
/**
 ** @brief   Generate and send R100 Sigfox Alert
 ** @param   void
 **
 ** @return  ERROR_ID_e error code
 ******************************************************************************/
void Comm_Sigfox_Generate_Send_Alert(SIGFOX_MSG_ID_e alert_id)
{
    DRD_RESULT_t diagnosis;
    DRD_Get_Diagnosis(&diagnosis);

    // check if the Sigfox option is enabled
    if (!my_Device_Info.enable_b.sigfox) return;

    // format the info to report
    memset ((uint8_t *) &sigfox_func_alert, 0, sizeof (SIGFOX_FUNC_ALERT_t));
    sigfox_func_alert.info_type = alert_id;
    sigfox_func_alert.electrodes_sign = Electrodes_Get_Signature();
    sigfox_func_alert.battery_soc  = (uint8_t) Battery_Get_Charge();
    sigfox_func_alert.patient_type = (uint8_t) pMisc_Settings->glo_patient_adult;
    if(alert_id == MSG_ID_SHOCK_DONE_ALERT) tx_thread_sleep(OSTIME_200MSEC);
    sigfox_func_alert.zp = (uint32_t) patMon_Get_Zp_Ohms();
    sigfox_func_alert.drd_diag = (uint8_t) diagnosis.rhythm;;
    sigfox_func_alert.must_be_0x55 = 0x55;

    // if a shock has been performed, set the shock energy
    if(alert_id == MSG_ID_SHOCK_DONE_ALERT) sigfox_func_alert.shock_energy = FSM_shock_energy;

    // Send Test Result
    Send_Comm_Queue (eEV_COMM_SIGFOX_SEND_ALERT);
}

/******************************************************************************
** Name:    Comm_Is_Sigfox_Alert_Sended
*****************************************************************************/
/**
** @brief   Checks if Sigfox alert sended process completed
** @param   void
**
** @return  TRUE if sigfox alert sended and FALSE if not
*****************************************************************************/
bool_t Comm_Is_Sigfox_Alert_Sended(void)
{
    // check if the Sigfox option is enabled
    if (!my_Device_Info.enable_b.sigfox) return FALSE;


    return sigfox_alert_frame_sended;
}

/******************************************************************************
 ** Name:    Comm_Sigfox_Generate_Send_Expiration
 *****************************************************************************/
/**
 ** @brief   Generate and send electrodes and battery expiration dates
 ** @param   void
 **
 ** @return  ERROR_ID_e error code
 ******************************************************************************/
void Comm_Sigfox_Generate_Send_Expiration(DB_TEST_RESULT_t *pResult)
{
    uint8_t    open_cover_flag = 0;
    uint8_t    pin_state;          // Cover pin state

    // check if the Sigfox option is enabled
    if (!my_Device_Info.enable_b.sigfox) return;

    // check if cover is open
    g_ioport.p_api->pinRead (KEY_COVER, &pin_state);

    if (pin_state)
    {
        open_cover_flag = 1;
    }

    // format the info to report
    memset ((uint8_t *) &sigfox_expiration, 0, sizeof (SIGFOX_ELEC_BATT_EXPIRATION_t));
    sigfox_expiration.info_type = (uint8_t)MSG_ID_BATT_ELECT_EXP;
    sigfox_expiration.batt_expiration = (uint32_t)pResult->battery.manufacture_date;
    sigfox_expiration.elec_expiration = (uint32_t)pResult->electrodes.expiration_date;
    if(Is_Battery_Mode_Demo())
    {
        sigfox_expiration.demo_mode = (uint8_t)1;
    }
    else 
    {
        sigfox_expiration.demo_mode = (uint8_t)0;
    }
    sigfox_expiration.open_cover = (uint8_t)open_cover_flag;

    sigfox_expiration.must_be_0x55 = (uint8_t)0x55;

    // Send Test Result
    Send_Comm_Queue (eEV_COMM_SIGFOX_SEND_EXP);
}

/******************************************************************************
** Name:    Comm_Is_Sigfox_Exp_Sended
*****************************************************************************/
/**
** @brief   Checks if sigfox exp sended process completed
** @param   void
**
** @return  TRUE if sigfox test sended and FALSE if not
*****************************************************************************/
bool_t Comm_Is_Sigfox_Exp_Sended(void)
{
    // check if the Sigfox option is enabled
    if (!my_Device_Info.enable_b.sigfox) return FALSE;

    return sigfox_exp_sended;
}

/******************************************************************************
 ** Name:    Comm_Sigfox_Generate_Send_Position
 *****************************************************************************/
/**
 ** @brief   Generate R100 Sigfox GPS position
 ** @param   void
 **
 ** @return  ERROR_ID_e error code
 ******************************************************************************/
void Comm_Sigfox_Generate_Send_Position(SIGFOX_MSG_ID_e gps_id)
{
    // check if the Sigfox option is enabled
    if (!my_Device_Info.enable_b.sigfox) return;

    // format the info to report
    memset ((uint8_t *) &sigfox_gps_position, 0, sizeof (SIGFOX_GPS_POSITION_t));
    sigfox_gps_position.info_type = gps_id;

    // FORMAT sigfox_gps_position from ASCII to decimal values
    // latitude    GGMM.MMMM  N or S
    // longitude  GGGMM.MMMM  E or W
    sigfox_gps_position.lat_grados    = (uint8_t) (ASCII_TO_DEC(R100_gps.lat_data[0])*10 +
                                                   ASCII_TO_DEC(R100_gps.lat_data[1]));
    sigfox_gps_position.lat_minutos_1 = (uint8_t) (ASCII_TO_DEC(R100_gps.lat_data[2])*10 +
                                                   ASCII_TO_DEC(R100_gps.lat_data[3]));
    sigfox_gps_position.lat_minutos_2 = (uint16_t)(ASCII_TO_DEC(R100_gps.lat_data[5])*1000 +
                                                   ASCII_TO_DEC(R100_gps.lat_data[6])*100  +
                                                   ASCII_TO_DEC(R100_gps.lat_data[7])*10   +
                                                   ASCII_TO_DEC(R100_gps.lat_data[8]));

    sigfox_gps_position.long_grados    = (uint16_t) (ASCII_TO_DEC(R100_gps.long_data[0])*100 +
                                                     ASCII_TO_DEC(R100_gps.long_data[1])*10  +
                                                     ASCII_TO_DEC(R100_gps.long_data[2]));
    sigfox_gps_position.long_minutos_1 = (uint8_t)  (ASCII_TO_DEC(R100_gps.long_data[3])*10 +
                                                     ASCII_TO_DEC(R100_gps.long_data[4]));
    sigfox_gps_position.long_minutos_2 = (uint16_t) (ASCII_TO_DEC(R100_gps.long_data[6])*1000 +
                                                     ASCII_TO_DEC(R100_gps.long_data[7])*100  +
                                                     ASCII_TO_DEC(R100_gps.long_data[8])*10   +
                                                     ASCII_TO_DEC(R100_gps.long_data[9]));

    sigfox_gps_position.lat_long_dir = 0;
    if (R100_gps.N_S == 'N') sigfox_gps_position.lat_long_dir = 0x2;
    if (R100_gps.E_W == 'E') sigfox_gps_position.lat_long_dir |= 0x1;

    sigfox_gps_position.must_be_0x55 = 0x55;

    // Send GPS frame
    Send_Comm_Queue (eEV_COMM_SIGFOX_SEND_POSITION);
}

/******************************************************************************
** Name:    Comm_Is_Sigfox_Exp_Sended
*****************************************************************************/
/**
** @brief   Checks if sigfox exp sended process completed
** @param   void
**
** @return  TRUE if sigfox test sended and FALSE if not
*****************************************************************************/
bool_t Comm_Is_Sigfox_GPS_Sended(void)
{
    // check if the Sigfox option is enabled
    if (!my_Device_Info.enable_b.sigfox) return FALSE;

    return sigfox_gps_sended;
}


/******************************************************************************
 ** Name:    Comm_Sigfox_Execute_HW_Test
 *****************************************************************************/
/**
 ** @brief   Execute Sigfox HW Test
 ** @param   void
 **
 ** @return  ERROR_ID_e error code
 ******************************************************************************/
void Comm_Sigfox_Execute_HW_Test(void)
{
    // check if the Sigfox option is enabled
    if (!my_Device_Info.enable_b.sigfox) return;

    // Send Test Result
    Send_Comm_Queue (eEV_COMM_SIGFOX_EXECUTE_TEST);
}

/******************************************************************************
** Name:    Comm_Is_Sigfox_Initialized
*****************************************************************************/
/**
** @brief   Checks if sigfox initialization process completed
** @param   void
**
** @return  TRUE if sigfox initialized and FALSE if not
*****************************************************************************/
bool_t Comm_Is_Sigfox_Initialized(void)
{
    // check if the Sigfox option is enabled
    if (!my_Device_Info.enable_b.sigfox) return FALSE;

    return sigfox_initialized;
}

/******************************************************************************
** Name:    Comm_Is_Sigfox_HW_Test_Finished
*****************************************************************************/
/**
** @brief   Checks if sigfox hw test process is finished
** @param   void
**
** @return  TRUE if sigfox hw test finished and FALSE if not
*****************************************************************************/
bool_t    Comm_Is_Sigfox_HW_Test_Finished(void)
{
    return sigfox_hw_test_finished;
}

/******************************************************************************
** Name:    Comm_Get_Sigfox_HW_Test_Result
*****************************************************************************/
/**
** @brief   Get sigfox hw test result
** @param   void
**
** @return  ERROR_ID_e sigfox hw test result
*****************************************************************************/
ERROR_ID_e    Comm_Get_Sigfox_HW_Test_Result(void)
{
    return sigfox_hw_test_error;
}

/******************************************************************************
** Name:    Comm_Get_Sigfox_Error
*****************************************************************************/
/**
** @brief   Get sigfox status
** @param   void
**
** @return  sigfox status
*****************************************************************************/
ERROR_ID_e    Comm_Get_Sigfox_Error(void)
{
    return sigfox_error;
}

/******************************************************************************
** Name:    Comm_Wifi_Open
*****************************************************************************/
/**
** @brief   Open Wifi Communication
** @param   void
**
** @return  none
*****************************************************************************/
void Comm_Wifi_Open(void)
{
    // check if the Wifi option is enabled
    if (!my_Device_Info.enable_b.wifi) return;

    // Initialize WIFI
    Send_Comm_Queue (eEV_COMM_WIFI_INITIALIZE);
}

/******************************************************************************
** Name:    Comm_Is_Wifi_Initialized
*****************************************************************************/
/**
** @brief   Checks if wifi initialization process completed
** @param   void
**
** @return  TRUE if wifi initialized and connected FALSE if not
*****************************************************************************/
bool_t Comm_Is_Wifi_Initialized(void)
{
    // check if the Wifi option is enabled
    if (!my_Device_Info.enable_b.wifi) return FALSE;

    return wifi_initialized;
}

/******************************************************************************
** Name:    Comm_Is_Wifi_Init_Finished
*****************************************************************************/
/**
** @brief   Checks if wifi initialization process finished
** @param   void
**
** @return  TRUE if wifi finished and connected FALSE if not
*****************************************************************************/
bool_t Comm_Is_Wifi_Init_Finished(void)
{
    // check if the Wifi option is enabled
    if (!my_Device_Info.enable_b.wifi) return FALSE;

    return wifi_init_finished;
}

/******************************************************************************
** Name:    Comm_Is_Wifi_Free
*****************************************************************************/
/**
** @brief   Returns if Comms UART is free
**
** @param   none
**
** @return  true or false
******************************************************************************/
bool_t Comm_Is_Wifi_Free (void)
{
    return is_wifi_free;
}

/******************************************************************************
** Name:    Comm_Wifi_Is_Host_Alive
*****************************************************************************/
/**
** @brief   Function to check if server host is up and running
** @param   void
**
** @return  none
*****************************************************************************/
void Comm_Wifi_Is_Host_Alive(void)
{
    // check if the Wifi option is enabled
    if (!my_Device_Info.enable_b.wifi) return;

    Send_Comm_Queue (eEV_COMM_WIFI_IS_SERVER_ALIVE);
}

/******************************************************************************
** Name:    Comm_Is_Wifi_Host_Alive_Finished
*****************************************************************************/
/**
 * @brief   Checks if server host is up and running process finished
** @param   void
**
** @return  TRUE if finished, FALSE if not
*****************************************************************************/
bool_t Comm_Is_Wifi_Host_Alive_Finished(void)
{
    return wifi_host_alive_finished;
}

/******************************************************************************
** Name:    Comm_Wifi_Generate_Send_Test_Frame
*****************************************************************************/
/**
** @brief   Function to send the test result using wifi connection
**
** @param   pResult     pointer to result parameters
** @param   auto_test   if the test is automatic or forced by an user
**
** @return  none
**
******************************************************************************/
void Comm_Wifi_Generate_Send_Test_Frame (DB_TEST_RESULT_t *pResult)
{
    uint8_t    open_cover_flag = 0;
    uint8_t    pin_state;          // Cover pin state

    // check if the Wifi option is enabled
    if (!my_Device_Info.enable_b.wifi) return;

    // format the info to report
    memset ((uint8_t *) &wifi_test_result, 0, sizeof (WIFI_TEST_RESULT_t));

    if(pResult->test_id < TEST_FULL_ID)
    {
        wifi_test_result.info_type = WIFI_MSG_ID_DIARY_TEST;
    }
    else if(pResult->test_id == TEST_FULL_ID)
    {
        wifi_test_result.info_type = WIFI_MSG_ID_MONTHLY_TEST;
    }
    else if(pResult->test_id == TEST_MANUAL_ID)
    {
        wifi_test_result.info_type = WIFI_MSG_ID_MANUAL_TEST;
    }
    else if(pResult->test_id == TEST_INITIAL_ID)
    {
        wifi_test_result.info_type = WIFI_MSG_ID_POWER_ON_TEST;
    }
    else if(pResult->test_id == TEST_FUNC_ID)
    {
        wifi_test_result.info_type = WIFI_MSG_ID_FUNC_TEST;
    }

    wifi_test_result.error_code_defib = (uint8_t) pResult->defib.error_code;
    if (pResult->patmon.ADS_cal)   wifi_test_result.error_code_ADS = (uint8_t) pResult->patmon.ADS_cal;
    if (pResult->patmon.ADS_comms) wifi_test_result.error_code_ADS = (uint8_t) pResult->patmon.ADS_comms;
    wifi_test_result.error_code_dc_18v  = (uint8_t) pResult->voltage.dc_18v_error;
    wifi_test_result.error_code_dc_main = (uint8_t) pResult->voltage.dc_main_error;
    wifi_test_result.error_code_audio   = (uint8_t) pResult->misc.audio;
    wifi_test_result.error_electrodes   = (uint8_t) pResult->electrodes.error_code;
    wifi_test_result.battery_soc  = (uint8_t) pResult->battery.rem_charge;
    wifi_test_result.battery_volt = (uint8_t) (Defib_Get_Vbatt() / 100);
    wifi_test_result.battery_temp = (int8_t) pResult->battery.bat_temperature;
    wifi_test_result.error_code = (uint8_t) pResult->error_code;
    wifi_test_result.must_be_0x55 = (uint8_t) 0x55;

    // check if cover is open
    g_ioport.p_api->pinRead (KEY_COVER, &pin_state);

    if (pin_state)
    {
        open_cover_flag = 1;
    }

    wifi_test_result.info_type_exp = (uint8_t) WIFI_MSG_ID_BATT_ELECT_EXP;
    wifi_test_result.batt_expiration = (uint32_t) pResult->battery.manufacture_date;
    wifi_test_result.elec_expiration = (uint32_t) pResult->electrodes.expiration_date;
    if(Is_Battery_Mode_Demo())
    {
        wifi_test_result.demo_mode = (uint8_t)1;
    }
    else 
    {
        wifi_test_result.demo_mode = (uint8_t)0;
    }
    wifi_test_result.open_cover = (uint8_t) open_cover_flag;

    wifi_test_result.must_be_0x55_exp = (uint8_t) 0x55;

    // Send Test Result
    Send_Comm_Queue (eEV_COMM_WIFI_SEND_FRAME_TEST);
}

/******************************************************************************
** Name:    Comm_Is_Wifi_Test_Frame_Sended
*****************************************************************************/
/**
** @brief   Checks if wifi test frame sended process completed
** @param   void
**
** @return  TRUE if wifi test frame sended and FALSE if not
*****************************************************************************/
bool_t Comm_Is_Wifi_Test_Frame_Sended(void)
{
    // check if the Wifi option is enabled
    if (!my_Device_Info.enable_b.wifi) return FALSE;


    return wifi_test_frame_sended;
}

/******************************************************************************
 ** Name:    Comm_Wifi_Generate_Send_Alert_Frame
 *****************************************************************************/
/**
 ** @brief   Generate and send R100 Wifi Alert
 ** @param   void
 **
 ** @return  ERROR_ID_e error code
 ******************************************************************************/
void Comm_Wifi_Generate_Send_Alert_Frame(WIFI_MSG_ID_e alert_id)
{
    // check if the Wifi option is enabled
    if (!my_Device_Info.enable_b.wifi) return;

    // format the info to report
    if(switch_alert == false)
    {
        memset ((uint8_t *) &wifi_func_alert, 0, sizeof (WIFI_FUNC_ALERT_t));
        wifi_func_alert.info_type = alert_id;
        wifi_func_alert.electrodes_sign = Electrodes_Get_Signature();
        wifi_func_alert.battery_soc  = (uint8_t) Battery_Get_Charge();
        wifi_func_alert.patient_type = (uint8_t) pMisc_Settings->glo_patient_adult;
        wifi_func_alert.zp = (uint32_t) patMon_Get_Zp_Ohms();
        wifi_func_alert.drd_diag = (uint8_t) DRD_Get_Diagnosis(NULL);
        wifi_func_alert.must_be_0x55 = 0x55;

        // if a shock has been performed, set the shock energy
        if(alert_id == (WIFI_MSG_ID_e) WIFI_MSG_ID_SHOCK_DONE_ALERT) wifi_func_alert.shock_energy = FSM_shock_energy;

        // Send Alert Frame
        Send_Comm_Queue (eEV_COMM_WIFI_SEND_FRAME_ALERT);
    }

    if(switch_alert == true)
    {
        memset ((uint8_t *) &wifi_func_alert_aux, 0, sizeof (WIFI_FUNC_ALERT_t));
        wifi_func_alert_aux.info_type = alert_id;
        wifi_func_alert_aux.electrodes_sign = Electrodes_Get_Signature();
        wifi_func_alert_aux.battery_soc  = (uint8_t) Battery_Get_Charge();
        wifi_func_alert_aux.patient_type = (uint8_t) pMisc_Settings->glo_patient_adult;
        wifi_func_alert_aux.zp = (uint32_t) patMon_Get_Zp_Ohms();
        wifi_func_alert_aux.drd_diag = (uint8_t) DRD_Get_Diagnosis(NULL);
        wifi_func_alert_aux.must_be_0x55 = 0x55;

        // if a shock has been performed, set the shock energy
        if(alert_id == (WIFI_MSG_ID_e) WIFI_MSG_ID_SHOCK_DONE_ALERT) wifi_func_alert_aux.shock_energy = FSM_shock_energy;

        // Send Alert Frame
        Send_Comm_Queue (eEV_COMM_WIFI_SEND_FRAME_ALERT_AUX);
    }
    if(switch_alert == false) switch_alert = true;
}

/******************************************************************************
** Name:    Comm_Is_Wifi_Alert_Frame_Sended
*****************************************************************************/
/**
** @brief   Checks if wifi test frame sended process completed
** @param   void
**
** @return  TRUE if wifi test frame sended and FALSE if not
*****************************************************************************/
bool_t Comm_Is_Wifi_Alert_Frame_Sended(void)
{
    // check if the Wifi option is enabled
    if (!my_Device_Info.enable_b.wifi) return FALSE;

    return wifi_alert_frame_sended;
}

/******************************************************************************
 ** Name:    Comm_Wifi_Generate_Send_Position_Frame
 *****************************************************************************/
/**
 ** @brief   Generate R100 Wifi GPS position Frame
 ** @param   void
 **
 ** @return  ERROR_ID_e error code
 ******************************************************************************/
void Comm_Wifi_Generate_Send_Position_Frame(WIFI_MSG_ID_e gps_id)
{
    // check if the Wifi option is enabled
    if (!my_Device_Info.enable_b.wifi) return;

    // format the info to report
    memset ((uint8_t *) &wifi_gps_position, 0, sizeof (WIFI_GPS_POSITION_t));
    wifi_gps_position.info_type = gps_id;

    // FORMAT wifi_gps_position from ASCII to decimal values
    // latitude    GGMM.MMMM  N or S
    // longitude  GGGMM.MMMM  E or W
    wifi_gps_position.lat_grados = (uint8_t)(ASCII_TO_DEC(R100_gps.lat_data[0])*10 +
                                             ASCII_TO_DEC(R100_gps.lat_data[1]));
    wifi_gps_position.lat_minutos_1 = (uint8_t)(ASCII_TO_DEC(R100_gps.lat_data[2])*10 +
                                                ASCII_TO_DEC(R100_gps.lat_data[3]));
    wifi_gps_position.lat_minutos_2 = (uint16_t)(ASCII_TO_DEC(R100_gps.lat_data[5])*1000 +
                                                 ASCII_TO_DEC(R100_gps.lat_data[6])*100   +
                                                 ASCII_TO_DEC(R100_gps.lat_data[7])*10   +
                                                 ASCII_TO_DEC(R100_gps.lat_data[8]));

    wifi_gps_position.long_grados = (uint16_t)(ASCII_TO_DEC(R100_gps.long_data[0])*100 +
                                               ASCII_TO_DEC(R100_gps.long_data[1])*10 +
                                               ASCII_TO_DEC(R100_gps.long_data[2]));
    wifi_gps_position.long_minutos_1 = (uint8_t)(ASCII_TO_DEC(R100_gps.long_data[3])*10 +
                                                 ASCII_TO_DEC(R100_gps.long_data[4]));
    wifi_gps_position.long_minutos_2 = (uint16_t)(ASCII_TO_DEC(R100_gps.long_data[6])*1000 +
                                                  ASCII_TO_DEC(R100_gps.long_data[7])*100   +
                                                  ASCII_TO_DEC(R100_gps.long_data[8])*10   +
                                                  ASCII_TO_DEC(R100_gps.long_data[9]));

    wifi_gps_position.lat_long_dir = 0;
    if (R100_gps.N_S == 'N') wifi_gps_position.lat_long_dir = 0x2;
    if (R100_gps.E_W == 'E') wifi_gps_position.lat_long_dir |= 0x1;

    wifi_gps_position.must_be_0x55 = 0x55;

    // Send Position Frame
    Send_Comm_Queue (eEV_COMM_WIFI_SEND_FRAME_GPS);
}

/******************************************************************************
** Name:    Comm_Is_Wifi_Position_Frame_Sended
*****************************************************************************/
/**
** @brief   Checks if wifi position frame sended process completed
** @param   void
**
** @return  TRUE if wifi position frame sended and FALSE if not
*****************************************************************************/
bool_t Comm_Is_Wifi_Position_Frame_Sended(void)
{
    // check if the Wifi option is enabled
    if (!my_Device_Info.enable_b.wifi) return FALSE;

    // In DEMO mode do not send anything related to wifi
    if (Is_Battery_Mode_Demo()) return FALSE;

    return wifi_position_frame_sended;
}

/******************************************************************************
** Name:    Comm_Wifi_Send_Test
*****************************************************************************/
/**
** @brief   Function to send the test file using wifi
** @param   void
**
** @return  none
*****************************************************************************/
void Comm_Wifi_Send_Test(void)
{
    // check if the Wifi option is enabled
    if (!my_Device_Info.enable_b.wifi) return;

    Send_Comm_Queue (eEV_COMM_WIFI_SEND_TEST);
}

/******************************************************************************
** Name:    Comm_Is_Wifi_Test_Sended
*****************************************************************************/
/**
** @brief   Checks if wifi test sended process completed
** @param   void
**
** @return  TRUE if wifi test sended and FALSE if not
*****************************************************************************/
bool_t Comm_Is_Wifi_Test_Sended(void)
{
    // check if the Wifi option is enabled
    if (!my_Device_Info.enable_b.wifi) return FALSE;

    return wifi_test_sended;
}

/******************************************************************************
** Name:    Comm_Wifi_Send_Episode
*****************************************************************************/
/**
** @brief   Function to send the episode file using wifi
** @param   void
**
** @return  none
*****************************************************************************/
void Comm_Wifi_Send_Episode(void)
{
    // check if the Wifi option is enabled
    if (!my_Device_Info.enable_b.wifi) return;

    // In DEMO mode do not send anything related to wifi
    if (Is_Battery_Mode_Demo()) return;

    Send_Comm_Queue (eEV_COMM_WIFI_SEND_EPISODE);
}

/******************************************************************************
** Name:    Comm_Is_Wifi_Episode_Sended
*****************************************************************************/
/**
** @brief   Checks if wifi episode sended process completed
** @param   void
**
** @return  TRUE if wifi test sended and FALSE if not
*****************************************************************************/
bool_t Comm_Is_Wifi_Episode_Sended(void)
{
    // check if the Wifi option is enabled
    if (!my_Device_Info.enable_b.wifi) return FALSE;

    // In DEMO mode do not send anything related to wifi
    if (Is_Battery_Mode_Demo()) return FALSE;

    return wifi_episode_sended;
}

/******************************************************************************
** Name:    Comm_Wifi_Get_Firmware
*****************************************************************************/
/**
** @brief   Function to get the firmware file using wifi
** @param   void
**
** @return  none
*****************************************************************************/
void Comm_Wifi_Get_Firmware(void)
{
    // check if the Wifi option is enabled
    if (!my_Device_Info.enable_b.wifi) return;

    Send_Comm_Queue (eEV_COMM_WIFI_GET_UPGRADE_FILE);
}

/******************************************************************************
** Name:    Comm_Is_Wifi_Upgrade_Received
*****************************************************************************/
/**
** @brief   Checks if wifi upgrade received process completed
** @param   void
**
** @return  TRUE if wifi upgrade received and FALSE if not
*****************************************************************************/
bool_t Comm_Is_Wifi_Upgrade_Received(void)
{
    // check if the Wifi option is enabled
    if (!my_Device_Info.enable_b.wifi) return FALSE;

    return wifi_firmware_received;
}

/******************************************************************************
** Name:    Comm_Wifi_Get_Configuration
*****************************************************************************/
/**
** @brief   Function to get the config file using wifi
** @param   void
**
** @return  none
*****************************************************************************/
void Comm_Wifi_Get_Configuration(void)
{
    // check if the Wifi option is enabled
    if (!my_Device_Info.enable_b.wifi) return;

    Send_Comm_Queue (eEV_COMM_WIFI_GET_CONFIG_FILE);
}

/******************************************************************************
** Name:    Comm_Is_Wifi_Config_Received
*****************************************************************************/
/**
** @brief   Checks if wifi config received process completed
** @param   void
**
** @return  TRUE if wifi upgrade received and FALSE if not
*****************************************************************************/
bool_t Comm_Is_Wifi_Config_Received(void)
{
    // check if the Wifi option is enabled
    if (!my_Device_Info.enable_b.wifi) return FALSE;

    return wifi_config_received;
}

/******************************************************************************
 ** Name:    Comm_Wifi_Execute_HW_Test
 *****************************************************************************/
/**
 ** @brief   Execute Wifi HW Test
 ** @param   void
 **
 ** @return  void
 ******************************************************************************/
void Comm_Wifi_Execute_HW_Test(void)
{
    // check if the Wifi option is enabled
    if (!my_Device_Info.enable_b.wifi) return;

    // Send Test Result
    Send_Comm_Queue (eEV_COMM_WIFI_EXECUTE_TEST);
}

/******************************************************************************
** Name:    Comm_Is_Wifi_HW_Test_Finished
*****************************************************************************/
/**
** @brief   Checks if wifi hw test process is finished
** @param   void
**
** @return  TRUE if wifi hw test finished and FALSE if not
*****************************************************************************/
bool_t    Comm_Is_Wifi_HW_Test_Finished(void)
{
    return wifi_hw_test_finished;
}

/******************************************************************************
** Name:    Comm_Get_Wifi_HW_Test_Result
*****************************************************************************/
/**
** @brief   Get wifi hw test result
** @param   void
**
** @return  ERROR_ID_e wifi hw test result
*****************************************************************************/
ERROR_ID_e    Comm_Get_Wifi_HW_Test_Result(void)
{
    return wifi_hw_test_error;
}

/******************************************************************************
** Name:    Comm_Get_Wifi_Host_Alive_Flag
*****************************************************************************/
/**
** @brief   Gets wifi host is alive and running flag
** @param   void
**
** @return  TRUE if wifi host running and FALSE if not
*****************************************************************************/
bool_t Comm_Get_Wifi_Host_Alive_Flag(void)
{
    // check if the Wifi option is enabled
    if (!my_Device_Info.enable_b.wifi) return FALSE;

    return wifi_host_alive_flag;
}

/******************************************************************************
** Name:    Comm_GPS_Open
*****************************************************************************/
/**
** @brief   Open GPS Communication
** @param   void
**
** @return  none
*****************************************************************************/
void Comm_GPS_Open(void)
{
    // check if the GPS option is enabled
    if (!my_Device_Info.enable_b.gps) return;

    // Initialize GPS
    Send_Comm_Queue (eEV_COMM_GPS_INITIALIZE);
}

/******************************************************************************
** Name:    Comm_GPS_Send_Cmd
*****************************************************************************/
/**
** @brief   Send GPS Commands
** @param   void
**
** @return  none
*****************************************************************************/
void Comm_GPS_Send_Cmd(void)
{
    // check if the GPS option is enabled
    if (!my_Device_Info.enable_b.gps) return;

    Send_Comm_Queue (eEV_COMM_GPS_SEND_CMD);
}

/******************************************************************************
** Name:    Comm_GPS_Get_Position
*****************************************************************************/
/**
** @brief   Ask GPS Position
** @param   void
**
** @return  none
*****************************************************************************/
void Comm_GPS_Get_Position(void)
{
    // check if the GPS option is enabled
    if (!my_Device_Info.enable_b.gps) return;

    // In DEMO mode do not send anything related to gps
    if (Is_Battery_Mode_Demo()) return;

    Send_Comm_Queue (eEV_COMM_GPS_GET_POS_DATA);
}

/******************************************************************************
 ** Name:    Comm_GPS_Execute_HW_Test
 *****************************************************************************/
/**
 ** @brief   Execute gps HW Test
 ** @param   void
 **
 ** @return  void
 ******************************************************************************/
void Comm_GPS_Execute_HW_Test(void)
{
    // check if the GPS option is enabled
    if (!my_Device_Info.enable_b.gps) return;

    // In DEMO mode do not send anything related to gps
    // if (Is_Mode_Demo()) return;

    // Send Test Result
    Send_Comm_Queue (eEV_COMM_GPS_EXECUTE_TEST);
}

/******************************************************************************
** Name:    Comm_Is_GPS_HW_Test_Finished
*****************************************************************************/
/**
** @brief   Checks if gps hw test process is finished
** @param   void
**
** @return  TRUE if gps hw test finished and FALSE if not
*****************************************************************************/
bool_t    Comm_Is_GPS_HW_Test_Finished(void)
{
    return gps_hw_test_finished;
}

/******************************************************************************
** Name:    Comm_Get_GPS_HW_Test_Result
*****************************************************************************/
/**
** @brief   Get gps hw test result
** @param   void
**
** @return  ERROR_ID_e gps hw test result
*****************************************************************************/
ERROR_ID_e    Comm_Get_GPS_HW_Test_Result(void)
{
    return gps_hw_test_error;
}

/******************************************************************************
** Name:    Comm_Is_GPS_Cmd_Send
*****************************************************************************/
/**
** @brief   Check if GPS command is send
** @param   void
**
** @return  ERROR_ID_e gps hw test result
*****************************************************************************/
ERROR_ID_e    Comm_Is_GPS_Cmd_Send(void)
{
    return gps_cmd_send;
}

/******************************************************************************
 ** Name:    Comm_ACC_Execute_HW_Test
 *****************************************************************************/
/**
 ** @brief   Execute ACC HW Test
 **
 ** @param   force      force test
 **
 ** @return  void
 ******************************************************************************/
void Comm_ACC_Execute_HW_Test(bool_t force)
{
    // check if the ACC option is enabled
    if (!my_Device_Info.enable_b.accelerometer && force != TRUE) return;

    // In DEMO mode do not send anything related to acc
    if (Is_Battery_Mode_Demo()) return;

    // Send Test Result
    tx_event = eEV_COMM_ACC_EXECUTE_TEST;
    (void) tx_queue_send(&queue_comm, &tx_event, TX_WAIT_FOREVER);
}


/******************************************************************************
** Name:    Process_Comm_ACC
*****************************************************************************/
/**
** @brief   Process events related to ACC COMM module
**
** @param   ev event identifier
**
** @return  if the event has been processed or not
**
******************************************************************************/
static bool_t Process_Comm_ACC(EV_COMM_e ev)
{
    bool_t event_processed = TRUE; // event processed variable, initialize as processed

    // Process the event
    switch (ev)
    {
        case eEV_COMM_ACC_EXECUTE_TEST:
            acc_hw_test_finished = FALSE;
            acc_hw_test_error = Comm_ACC_Execute_Test();
            if(acc_hw_test_error != eERR_NONE)
            {
                Set_NV_Data_Error_IF_NOT_SAT_Comms(acc_hw_test_error);
            }
            acc_hw_test_finished = TRUE;
            break;
        default:
            event_processed = FALSE;
            break;

    }

    // return event processed or not!!!
    return event_processed;
}

/******************************************************************************
** Name:    Comm_Is_ACC_HW_Test_Finished
*****************************************************************************/
/**
** @brief   Checks if accelerometer hw test process is finished
** @param   void
**
** @return  TRUE if acc hw test finished and FALSE if not
*****************************************************************************/
bool_t    Comm_Is_ACC_HW_Test_Finished(void)
{
    return acc_hw_test_finished;
}

/******************************************************************************
** Name:    Comm_Get_ACC_HW_Test_Result
*****************************************************************************/
/**
** @brief   Get acc hw test result
** @param   void
**
** @return  ERROR_ID_e acc hw test result
*****************************************************************************/
ERROR_ID_e    Comm_Get_ACC_HW_Test_Result(void)
{
    return acc_hw_test_error;
}

