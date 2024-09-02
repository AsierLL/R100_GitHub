/******************************************************************************
 * Copyright : Osatu S. Coop                                                  *
 * Compiler  : GNU ARM Embedded                                               *
 * Target    : Reanibex Serie 100                                             *
 ******************************************************************************/

/*!
 * @file        sysMon_RF_Comms.c
 * @brief       RF communications executed in the context of the SysMon thread
 *
 * @version     v1
 * @date        24/11/2021
 * @author      ilazkanoiturburu
 * @warning     author   date    version     .- Edition
 * @bug
 *
 */

/******************************************************************************
 ** Includes
 */
#include <stdio.h>
#include <ctype.h>
#include <time.h>

#include <device_init.h>

#include "DB_Episode.h"
#include "sysMon_RF_Comms.h"


#ifdef UNIT_TESTS
#include "unit_tests.h"
#endif

/******************************************************************************
 ** Macros
 */

#define GPS_ZERO                        "\0\0\0\0"

/******************************************************************************
 ** Defines
 */


/******************************************************************************
 ** Typedefs
 */


/******************************************************************************
 ** Constants
 */

/******************************************************************************
 ** Externals
 */

extern gps_config_t R100_gps;                   ///< GPS data
extern acc_config_t R100_acc;                   ///< ACC data


extern bool_t       first_time_gps;                         ///< Flag to transmit gps position only once
//static bool_t       flag_transmission_message = FALSE;    ///< Flag for transmission in course

/******************************************************************************
 ** Globals
 */

/******************************************************************************
 ** Locals
 */

/******************************************************************************
 ** Prototypes
 */



/******************************************************************************
** Name:    Execute_Get_Gps_Position_Test
*****************************************************************************/
/**
** @brief   Function that executes the gps position after test
**
** @param   pResult     pointer to result parameters
** @param   tout        time out
**
** @return  0 - RTC position is invalid
**          1 - RTC position is valid
**          2 - device must be powered-off
******************************************************************************/
uint32_t Execute_Get_Gps_Position_Test (DB_TEST_RESULT_t *pResult, uint32_t tout)
{
    uint32_t ref_timeout = 0;
    uint32_t audio_timeout = 0;
    uint8_t test_id = pResult->test_id;

    Refresh_Wdg ();

    // Open Gps Comm.
    Comm_GPS_Open();
    Pasatiempos (OSTIME_5SEC);

    // send commands to the GPS to configure it
    Comm_GPS_Send_Cmd();

    if (GPS_Is_Test_Package_Received() == FALSE) return 0;

    ref_timeout = tx_time_get() + tout;
    audio_timeout = tx_time_get() + OSTIME_1SEC;

    while (tx_time_get() <= ref_timeout)
    {
        // check if cover is closed or ON_OFF key is pressed
        if (Check_Test_Abort_User(pResult))
        {
            return 2;
        }

        Refresh_Wdg ();
        if ((memcmp(R100_gps.lat_data,  GPS_ZERO, 4) == 0) &&
            (memcmp(R100_gps.long_data, GPS_ZERO, 4) == 0))
        {
            // Set GPS Get Data Command
            Comm_GPS_Get_Position();
            Refresh_Wdg ();
            Pasatiempos (OSTIME_10SEC);
            if(test_id == TEST_MANUAL_ID && tx_time_get() > audio_timeout)
            {
                Audio_Message (eAUDIO_CMD_CONCAT, eAUDIO_GETTING_LOCATION, TRUE);
                Audio_Message (eAUDIO_CMD_CONCAT, eAUDIO_TAKE_SEVERAL_MINUTES, TRUE);
                //flag_transmission_message = TRUE;
                audio_timeout = tx_time_get() + OSTIME_30SEC;
            }
        }
        else
        {
            // a valid position has been obtained!!
            //Pasatiempos(OSTIME_20SEC);
            //Comm_GPS_Get_Position();
            return 1;
        }
    }

    // Timeout --> no GPS position has been received
    return 0;
}

/******************************************************************************
** Name:    Execute_Periodic_Gps_Init
*****************************************************************************/
/**
** @brief   Function that executes the gps position periodically
**
** @param   none
**
** @return  none
******************************************************************************/
void Execute_Periodic_Gps_Init (void)
{
    if ((Comm_Get_Selected_Uart() != eMOD_GPS) && (Comm_Is_Sigfox_Free() && Comm_Is_Wifi_Free()))
    {
        Comm_GPS_Open();
        Comm_GPS_Get_Position();
    }
}

/******************************************************************************
** Name:    Execute_Periodic_Gps_Position
*****************************************************************************/
/**
** @brief   Function that executes the gps position periodically
**
** @param   pNV_data to the structure to fill
**
** @return  none
******************************************************************************/
void Execute_Periodic_Gps_Position (NV_DATA_t *pNV_data, NV_DATA_BLOCK_t *pNV_data_block)
{
    if((memcmp(R100_gps.lat_data, GPS_ZERO, 4) != 0) &&
       (memcmp(R100_gps.long_data, GPS_ZERO, 4) != 0))
    {
        if((memcmp(R100_gps.lat_data, pNV_data->latitude, 4) != 0) &&
           (memcmp(R100_gps.long_data, pNV_data->longitude, 4) != 0))
        {
            // save gps position to non-volatile memory
            memcpy (pNV_data->latitude,  R100_gps.lat_data, 4);
            memcpy (pNV_data->longitude, R100_gps.long_data, 4);

            // update the non volatile data
            NV_Data_Write(pNV_data, pNV_data_block);
        }


        // In DEMO mode do not send anything
        if (Is_Battery_Mode_Demo()) return;

        if(Is_Sigfox_TX_Enabled() == SIGFOX_ONLY || Is_Sigfox_TX_Enabled() == SIGFOX_PRIOR)
        {
            Send_Sigfox_Position(MSG_ID_CHANGED_POS_GPS);

            first_time_gps = false;
        }
        // Check if must send by wifi
        if(Is_Wifi_TX_Enabled() == WIFI_ONLY || Is_Wifi_TX_Enabled() == WIFI_PRIOR) //WIFI_WITH_PATIENT
        {
            Send_Wifi_Position(WIFI_MSG_ID_CHANGED_POS_GPS);    // Generate and Send position report using Wifi

            first_time_gps = false;
        }
    }
}


/******************************************************************************
** Name:    Execute_Save_Gps_Position
*****************************************************************************/
/**
** @brief   Function that executes the storing of the GPS position
**
** @param   pNV_data to the structure to fill
**
** @return  none
******************************************************************************/
void Execute_Save_Gps_Position (NV_DATA_t *pNV_data, NV_DATA_BLOCK_t *pNV_data_block)
{
    // give some time
    Pasatiempos (OSTIME_1SEC);

    // Open Gps Comm.
    Comm_GPS_Open();

    // refresh the internal watchdog timer
    Refresh_Wdg ();

    while((memcmp(R100_gps.lat_data,  GPS_ZERO, 4) == 0) &&
          (memcmp(R100_gps.long_data, GPS_ZERO, 4) == 0))
    {
        // Set GPS Get Data Command
        Comm_GPS_Get_Position();

        Pasatiempos (OSTIME_4SEC);
    }

    // refresh the internal watchdog timer
    Refresh_Wdg ();

    // Open Sigfox Comm.
    Comm_Sigfox_Open();

    Pasatiempos (OSTIME_8SEC);

    Comm_Sigfox_Generate_Send_Position(MSG_ID_SAVED_POS_GPS);

    // refresh the internal watchdog timer
    Refresh_Wdg ();

    if((memcmp(R100_gps.lat_data,  pNV_data->latitude, 12) != 0) &&
       (memcmp(R100_gps.long_data, pNV_data->longitude, 12) != 0))
    {
        // save gps position to non-volatile memory
        memcpy (pNV_data->latitude, R100_gps.lat_data, 12);
        memcpy (pNV_data->longitude, R100_gps.long_data, 12);

        pNV_data->lat_long_dir = 0;
        if (R100_gps.N_S == 'N') pNV_data->lat_long_dir = 0x2;
        if (R100_gps.E_W == 'E') pNV_data->lat_long_dir |= 0x1;

        // refresh the internal watchdog timer
        Refresh_Wdg ();

        // update the non volatile data
        NV_Data_Write(pNV_data, pNV_data_block);

        // refresh the internal watchdog timer
        Refresh_Wdg ();
    }

    Pasatiempos (OSTIME_8SEC);
}

/******************************************************************************
** Name:    Execute_Send_Sigfox_Test
*****************************************************************************/
/**
** @brief   Function that executes the sending of the Sigfox Test
**
** @param   pResult     pointer to result parameters
** @param   auto_test   if the test is automatic or forced by an user
**
** @return  False if device must be power-off
******************************************************************************/
bool_t Execute_Send_Sigfox_Test (DB_TEST_RESULT_t *pResult, bool_t auto_test)
{
    uint32_t ref_timeout = 0;
    uint32_t tout = 0;

    Refresh_Wdg ();

    if((Comm_Is_Sigfox_Initialized() == FALSE) || (Comm_Get_Selected_Uart() != eMOD_SIGFOX))
    {
        // Open wifi Comm.
        Comm_Sigfox_Open();
    }

    tout = (OSTIME_60SEC*5);

    ref_timeout = tx_time_get() + tout;

    Refresh_Wdg ();

    while((tx_time_get() <= ref_timeout) &&
          (Comm_Is_Sigfox_Initialized() == FALSE) &&
          (Comm_Get_Sigfox_Error() == eERR_NONE))
    {
        Refresh_Wdg ();
        tx_thread_sleep (OSTIME_50MSEC);
        // check if cover is closed or ON_OFF key is pressed
        if (Check_Test_Abort_User(pResult))
        {
            return ABORT_USER;
        }

    }

    if(Comm_Is_Sigfox_Initialized())
    {
        Refresh_Wdg ();

        //tout = (OSTIME_60SEC*5);

        //ref_timeout = tx_time_get() + tout;

        //Refresh_Wdg ();

        //Generate and Send Test report using Sigfox
        Comm_Sigfox_Generate_Send_Test(pResult);

        Refresh_Wdg ();

        for(uint8_t i = 0; i<200; i++)
        {
            // check if cover is closed or ON_OFF key is pressed
            if (Check_Test_Abort_User(pResult))
            {
                return ABORT_USER;
            }

            if ((i%100) == 0)     //Advice to the user periodically
            {
                if(!auto_test)
                {
                    //flag_transmission_message = TRUE;
                    Audio_Message (eAUDIO_CMD_CONCAT, eAUDIO_TRANSMISSION_ONGOING, TRUE);  // warn about transmission
                }
            }
            tx_thread_sleep (OSTIME_100MSEC);
            Refresh_Wdg ();
            // break if test sended
            if(Comm_Is_Sigfox_Test_Sended() == TRUE) break;
        }

        /*if(!auto_test)
        {
            Audio_Message (eAUDIO_CMD_CONCAT, eAUDIO_TRANSMISSION_DONE, TRUE);
        }*/

        // wait while playing all pending audio messages ...
        Pasatiempos_Listening();
    }
    else
    {
        return false;
    }

    return true;
}

/******************************************************************************
** Name:    Execute_Send_Sigfox_Exp
*****************************************************************************/
/**
** @brief   Function that sends battery and electrodes expiration dates
**
** @param   pResult     pointer to result parameters
** @param   auto_test   if the test is automatic or forced by an user
**
** @return  False if device must be power-off
******************************************************************************/
bool_t Execute_Send_Sigfox_Exp (DB_TEST_RESULT_t *pResult, bool_t auto_test)
{
    uint32_t ref_timeout = 0;
    uint32_t tout = 0;

    Refresh_Wdg ();

    if((Comm_Is_Sigfox_Initialized() == FALSE) || (Comm_Get_Selected_Uart() != eMOD_SIGFOX))
    {
        // Open wifi Comm.
        Comm_Sigfox_Open();
    }

    tout = (OSTIME_60SEC*5);

    ref_timeout = tx_time_get() + tout;

    Refresh_Wdg ();

    while((tx_time_get() <= ref_timeout) &&
          (Comm_Is_Sigfox_Initialized() == FALSE) &&
          (Comm_Get_Sigfox_Error() == eERR_NONE))
    {
        Refresh_Wdg ();
        tx_thread_sleep (OSTIME_50MSEC);
        // check if cover is closed or ON_OFF key is pressed
        if (Check_Test_Abort_User(pResult))
        {
            return ABORT_USER;
        }

    }

    if(Comm_Is_Sigfox_Initialized())
    {
        Refresh_Wdg ();

        //Generate and Send 
        Comm_Sigfox_Generate_Send_Expiration(pResult);

        Refresh_Wdg ();

        for(uint8_t i = 0; i<200; i++)
        {
            // check if cover is closed or ON_OFF key is pressed
            if (Check_Test_Abort_User(pResult))
            {
                return ABORT_USER;
            }

            if ((i%100) == 0)     //Advice to the user periodically
            {
                if(!auto_test)
                {
                    //flag_transmission_message = TRUE;
                    Audio_Message (eAUDIO_CMD_CONCAT, eAUDIO_TRANSMISSION_ONGOING, TRUE);  // warn about transmission
                }
            }
            tx_thread_sleep (OSTIME_100MSEC);
            Refresh_Wdg ();
            // break if test sended
            if(Comm_Is_Sigfox_Exp_Sended() == TRUE) break;
        }

        /*if(!auto_test)
        {
            Audio_Message (eAUDIO_CMD_CONCAT, eAUDIO_TRANSMISSION_DONE, TRUE);
        }*/

        // wait while playing all pending audio messages ...
        Pasatiempos_Listening();
    }
    else
    {
        return false;
    }

    return true;
}

/******************************************************************************
** Name:    Execute_Send_Sigfox_GPS
*****************************************************************************/
/**
** @brief   Function that sends GPS positiion
**
** @param   pResult     pointer to result parameters
** @param   auto_test   if the test is automatic or forced by an user
** @param   gps_id      GPS ID to send position
**
** @return  False if device must be power-off
******************************************************************************/
bool_t Execute_Send_Sigfox_GPS (DB_TEST_RESULT_t *pResult, bool_t auto_test, SIGFOX_MSG_ID_e gps_id)
{
    uint32_t ref_timeout = 0;
    uint32_t tout = 0;

    Refresh_Wdg ();

    if((Comm_Is_Sigfox_Initialized() == FALSE) || (Comm_Get_Selected_Uart() != eMOD_SIGFOX))
    {
        // Open wifi Comm.
        Comm_Sigfox_Open();
    }

    tout = (OSTIME_60SEC*5);

    ref_timeout = tx_time_get() + tout;

    Refresh_Wdg ();

    while((tx_time_get() <= ref_timeout) &&
          (Comm_Is_Sigfox_Initialized() == FALSE) &&
          (Comm_Get_Sigfox_Error() == eERR_NONE))
    {
        Refresh_Wdg ();
        tx_thread_sleep (OSTIME_50MSEC);
        // check if cover is closed or ON_OFF key is pressed
        if (Check_Test_Abort_User(pResult))
        {
            return ABORT_USER;
        }

    }

    if(Comm_Is_Sigfox_Initialized())
    {
        Refresh_Wdg ();

        //Generate and Send 
        Comm_Sigfox_Generate_Send_Position(gps_id);

        Refresh_Wdg ();

        for(uint8_t i = 0; i<200; i++)
        {
            // check if cover is closed or ON_OFF key is pressed
            if (Check_Test_Abort_User(pResult))
            {
                return ABORT_USER;
            }

            if ((i%100) == 0)     //Advice to the user periodically
            {
                if(!auto_test)
                {
                    //flag_transmission_message = TRUE;
                    Audio_Message (eAUDIO_CMD_CONCAT, eAUDIO_TRANSMISSION_ONGOING, TRUE);  // warn about transmission
                }
            }
            tx_thread_sleep (OSTIME_100MSEC);
            Refresh_Wdg ();
            // break if test sended
            if(Comm_Is_Sigfox_Exp_Sended() == TRUE) break;
        }
        // wait while playing all pending audio messages ...
        //Pasatiempos_Listening();
    }
    else
    {
        return false;
    }

    return true;
}

/******************************************************************************
** Name:    Execute_Wifi_Open
*****************************************************************************/
/**
** @brief   Function that executes the wifi initialization
**
** @param   pResult     pointer to result parameters
** @param   auto_test   if the test is automatic or forced by an user
**
** @return  Check if cover is closed or ON_OFF key is pressed
******************************************************************************/
bool_t Execute_Wifi_Open(DB_TEST_RESULT_t *pResult, bool_t auto_test)
{
    uint32_t ref_timeout = 0;
    uint32_t audio_timeout = 0;

    // Open wifi Comm.
    Comm_Wifi_Open();

    // 5 minute timeout for Initialize Wifi
    ref_timeout = tx_time_get() + (OSTIME_60SEC*3);

    Refresh_Wdg ();

    //audio_timeout = tx_time_get() + OSTIME_20SEC;

    while((tx_time_get() <= ref_timeout) && ((Comm_Is_Wifi_Initialized() == FALSE) && Comm_Is_Wifi_Init_Finished() == FALSE))
    {
        Refresh_Wdg ();
        tx_thread_sleep (OSTIME_50MSEC);
        // check if cover is closed or ON_OFF key is pressed
        if (Check_Test_Abort_User(pResult))
        {
            return ABORT_USER;
        }
        if(tx_time_get() > audio_timeout && !auto_test)
        {
            //flag_transmission_message = TRUE;
            Audio_Message (eAUDIO_CMD_CONCAT, eAUDIO_TRANSMISSION_ONGOING, TRUE);  // warn about transmission
            audio_timeout = tx_time_get() + OSTIME_20SEC;
        }
    }
    return true;
}

/******************************************************************************
** Name:    Execute_Wifi_Init_Host_Alive
*****************************************************************************/
/**
** @brief   Function that executes the wifi initialization and checks if the host is alive
**
** @param   pResult     pointer to result parameters
** @param   auto_test   if the test is automatic or forced by an user
**
** @return  Check if cover is closed or ON_OFF key is pressed
******************************************************************************/
bool_t Execute_Wifi_Init_Host_Alive (DB_TEST_RESULT_t *pResult, bool_t auto_test)
{
    uint32_t ref_timeout = 0;
    uint32_t audio_timeout = 0;

    Refresh_Wdg ();

    if((Comm_Is_Wifi_Initialized() == FALSE) || (Comm_Get_Selected_Uart() != eMOD_WIFI))
    {
        if(Execute_Wifi_Open(pResult, auto_test) == ABORT_USER)
        {
            return ABORT_USER;
        }
    }

    if(Comm_Is_Wifi_Initialized())
    {
        Refresh_Wdg ();

        // 3 minute timeout for checking server alive
        ref_timeout = tx_time_get() + (OSTIME_60SEC*3);

        Refresh_Wdg ();

        Comm_Wifi_Is_Host_Alive();

        audio_timeout = tx_time_get() + OSTIME_20SEC;

        while((tx_time_get() <= ref_timeout) && (Comm_Is_Wifi_Host_Alive_Finished() == FALSE))
        {
            Refresh_Wdg ();
            tx_thread_sleep (OSTIME_50MSEC);
            // check if cover is closed or ON_OFF key is pressed
            if (Check_Test_Abort_User(pResult))
            {
                return ABORT_USER;
            }
            if(tx_time_get() > audio_timeout)
            {
                //flag_transmission_message = TRUE;
                Audio_Message (eAUDIO_CMD_CONCAT, eAUDIO_TRANSMISSION_ONGOING, TRUE);  // warn about transmission
                audio_timeout = tx_time_get() + OSTIME_20SEC;
            }
        }
        if(Save_Comms_Error() == eERR_COMM_WIFI_SERVER_ALIVE)
        {
            Audio_Message (eAUDIO_CMD_CONCAT, eAUDIO_CONN_ERROR, TRUE);
            Pasatiempos_Listening();
        }
    }
    else
    {
        Audio_Message (eAUDIO_CMD_CONCAT, eAUDIO_CONN_ERROR, TRUE);
        Pasatiempos_Listening();
    }

    return true;
}

/******************************************************************************
** Name:    Execute_Send_Wifi_Test_Frame
*****************************************************************************/
/**
** @brief   Function that executes the sending of the last performed test frame
**
** @param   pResult     pointer to result parameters
** @param   auto_test   if the test is automatic or forced by an user
**
** @return  Check if cover is closed or ON_OFF key is pressed
******************************************************************************/
bool_t Execute_Send_Wifi_Test_Frame (DB_TEST_RESULT_t *pResult, bool_t auto_test)
{
    uint32_t ref_timeout = 0;
    uint32_t audio_timeout = 0;

    // Establish Wifi connection and check if host is available
    //if(Execute_Wifi_Init_Host_Alive() == TRUE)
    //if(Comm_Get_Wifi_Host_Alive_Flag() == TRUE)

    Refresh_Wdg ();

    if((Comm_Is_Wifi_Initialized() == FALSE) || (Comm_Get_Selected_Uart() != eMOD_WIFI))
    {
        if(Execute_Wifi_Open(pResult, auto_test) == ABORT_USER)
        {
            return ABORT_USER;
        }
    }
    if(Comm_Is_Wifi_Initialized() == TRUE)
    {
        Refresh_Wdg ();

        // 1 minute timeout for sending test frame
        ref_timeout = tx_time_get() + (OSTIME_60SEC);

        Refresh_Wdg ();

        Comm_Wifi_Generate_Send_Test_Frame(pResult);

        audio_timeout = tx_time_get() + OSTIME_20SEC;

        while((tx_time_get() <= ref_timeout))
        {
            Refresh_Wdg ();
            tx_thread_sleep (OSTIME_50MSEC);
            // check if cover is closed or ON_OFF key is pressed
            if (Check_Test_Abort_User(pResult))
            {
                return ABORT_USER;
            }
            if((Is_Wifi_TX_Enabled() == WIFI_ONLY && Comm_Is_Wifi_Test_Frame_Sended()) || 
               (Is_Wifi_TX_Enabled() == WIFI_PRIOR && Comm_Is_Wifi_Test_Frame_Sended() && Comm_Is_Sigfox_Exp_Sended()))
            {
                break;
            }
            if(tx_time_get() > audio_timeout && !auto_test)
            {
                //flag_transmission_message = TRUE;
                Audio_Message (eAUDIO_CMD_CONCAT, eAUDIO_TRANSMISSION_ONGOING, TRUE);  // warn about transmission
                audio_timeout = tx_time_get() + OSTIME_20SEC;
            }
        }
        if(Is_Wifi_TX_Enabled() == WIFI_ONLY)
        {
            if(Save_Comms_Error() == eERR_COMM_WIFI_SEND_FRAME_TEST)
            {
                if(!auto_test)
                {
                    Audio_Message (eAUDIO_CMD_CONCAT, eAUDIO_CONN_ERROR, TRUE);
                    Pasatiempos_Listening();
                }
                return SERVER_NO_CONNECT;
            }
            if(Is_GPS_TX_Enabled() && Save_Comms_Error() == eRR_COMM_WIFI_SERVER_RPS_ER)
            {
                if(!auto_test)
                {
                    Audio_Message (eAUDIO_CMD_CONCAT, eAUDIO_CONN_ERROR, TRUE);
                    Pasatiempos_Listening();
                }
                return ROUTER_CONNECT;
            }
            if(!Is_GPS_TX_Enabled() && Save_Comms_Error() == eRR_COMM_WIFI_SERVER_RPS_ER)
            {
                if(!auto_test)
                {
                    Audio_Message (eAUDIO_CMD_CONCAT, eAUDIO_CONN_ERROR, TRUE);
                    Pasatiempos_Listening();
                }
                return SERVER_NO_CONNECT;
            }
        }
    }
    else
    {
        if(Is_Wifi_TX_Enabled() == WIFI_ONLY)
        {
            if(!auto_test)
            {
                Audio_Message (eAUDIO_CMD_CONCAT, eAUDIO_CONN_ERROR, TRUE);
                Pasatiempos_Listening();
            }
            return ROUTER_NO_CONECT;
        }
        Execute_Send_Sigfox_Test(pResult, auto_test);
        Execute_Send_Sigfox_Exp(pResult, auto_test);
    }

    return ROUTER_CONNECT;
}

/******************************************************************************
** Name:    Execute_Send_Wifi_Alert_Frame
*****************************************************************************/
/**
** @brief   Function that executes the sending alert
**
** @param   pResult          pointer to result parameters
** @param   wifi_msg_id      Message ID to send alert
**
** @return  Check if cover is closed or ON_OFF key is pressed
******************************************************************************/
bool_t Execute_Send_Wifi_Alert_Frame (DB_TEST_RESULT_t *pResult, WIFI_MSG_ID_e wifi_msg_id)
{
    uint32_t ref_timeout = 0;
    uint32_t audio_timeout = 0;

    UNUSED(wifi_msg_id);

    // Establish Wifi connection and check if host is available
    //if(Execute_Wifi_Init_Host_Alive() == TRUE)
    //if(Comm_Get_Wifi_Host_Alive_Flag() == TRUE)
    {
        Refresh_Wdg ();

        // 1 minute timeout for sending alert
        ref_timeout = tx_time_get() + (OSTIME_60SEC);

        Refresh_Wdg ();

        Comm_Wifi_Generate_Send_Alert_Frame(wifi_msg_id);

        audio_timeout = tx_time_get() + OSTIME_20SEC;

        while((tx_time_get() <= ref_timeout) && (Comm_Is_Wifi_Alert_Frame_Sended() == FALSE))
        {
            Refresh_Wdg ();
            tx_thread_sleep (OSTIME_50MSEC);
            // check if cover is closed or ON_OFF key is pressed
            if (Check_Test_Abort_User(pResult))
            {
                return ABORT_USER;
            }
            if(tx_time_get() > audio_timeout)
            {
                //flag_transmission_message = TRUE;
                Audio_Message (eAUDIO_CMD_CONCAT, eAUDIO_TRANSMISSION_ONGOING, TRUE);  // warn about transmission
                audio_timeout = tx_time_get() + OSTIME_20SEC;
            }
        }
    }

    return true;
}

/******************************************************************************
** Name:    Execute_Send_Wifi_Position_Frame
*****************************************************************************/
/**
** @brief   Function that executes the sending of the gps position
**
** @param   pResult     pointer to result parameters
** @param   gps_id      GPS ID to send position
** @param   auto_test   if the test is automatic or forced by an user
**
** @return  Check if cover is closed or ON_OFF key is pressed
******************************************************************************/
bool_t Execute_Send_Wifi_Position_Frame (DB_TEST_RESULT_t *pResult, SIGFOX_MSG_ID_e gps_id, bool_t auto_test)
{
    uint32_t ref_timeout = 0;
    uint32_t audio_timeout = 0;

    Refresh_Wdg ();

    if((Comm_Is_Wifi_Initialized() == FALSE) || (Comm_Get_Selected_Uart() != eMOD_WIFI))
    {
        if(Execute_Wifi_Open(pResult, auto_test) == ABORT_USER)
        {
            return ABORT_USER;
        }
    }

    Refresh_Wdg ();
    if(Comm_Is_Wifi_Initialized() == TRUE)
    {
        // 1 minute timeout for sending position
        ref_timeout = tx_time_get() + (OSTIME_60SEC);

        Refresh_Wdg ();

        Comm_Wifi_Generate_Send_Position_Frame(gps_id);

        audio_timeout = tx_time_get() + OSTIME_20SEC;

        while((tx_time_get() <= ref_timeout))
        {
            Refresh_Wdg ();
            tx_thread_sleep (OSTIME_50MSEC);
            // check if cover is closed or ON_OFF key is pressed
            if (Check_Test_Abort_User(pResult))
            {
                return ABORT_USER;
            }
            if((Is_Wifi_TX_Enabled() == WIFI_ONLY && Comm_Is_Wifi_Position_Frame_Sended()) || 
                (Is_Wifi_TX_Enabled() == WIFI_PRIOR && Comm_Is_Wifi_Position_Frame_Sended() && Comm_Is_Sigfox_GPS_Sended()))
            {
                break;
            }
            if(tx_time_get() > audio_timeout && !auto_test)
            {
                //flag_transmission_message = TRUE;
                Audio_Message (eAUDIO_CMD_CONCAT, eAUDIO_TRANSMISSION_ONGOING, TRUE);  // warn about transmission
                audio_timeout = tx_time_get() + OSTIME_20SEC;
            }
        }
        if(Is_Wifi_TX_Enabled() == WIFI_ONLY && Save_Comms_Error() == eERR_COMM_WIFI_SEND_FRAME_GPS && !auto_test)
        {
            Audio_Message (eAUDIO_CMD_CONCAT, eAUDIO_CONN_ERROR, TRUE);
            Pasatiempos_Listening();
        }
    }
    else
    {
        if(Is_Wifi_TX_Enabled() == WIFI_ONLY && !auto_test)
        {
            Audio_Message (eAUDIO_CMD_CONCAT, eAUDIO_CONN_ERROR, TRUE);
            Pasatiempos_Listening();
        }
        Execute_Send_Sigfox_GPS(pResult, auto_test, gps_id);
    }

    return true;
}

/******************************************************************************
** Name:    Execute_Send_Wifi_Test_File
*****************************************************************************/
/**
** @brief   Function that executes the sending of the last performed test File
**
** @param   pResult     pointer to result parameters
** @param   auto_test   if the test is automatic or forced by an user
**
** @return  Check if cover is closed or ON_OFF key is pressed
******************************************************************************/
bool_t Execute_Send_Wifi_Test_File (DB_TEST_RESULT_t *pResult, bool_t auto_test)
{
    uint32_t ref_timeout = 0;
    uint32_t audio_timeout = 0;

    // Establish Wifi connection and check if host is available
    //if(Execute_Wifi_Init_Host_Alive() == TRUE)
    {
        Refresh_Wdg ();

        // 30 minutes timeout for sending test file
        ref_timeout = tx_time_get() + (OSTIME_60SEC*30);

        Refresh_Wdg ();

        Comm_Wifi_Send_Test();

        Refresh_Wdg ();

        audio_timeout = tx_time_get() + OSTIME_20SEC;

        while((tx_time_get() <= ref_timeout) && (Comm_Is_Wifi_Test_Sended() == FALSE))
        {
            Refresh_Wdg ();
            tx_thread_sleep (OSTIME_50MSEC);
            // check if cover is closed or ON_OFF key is pressed
            if (Check_Test_Abort_User(pResult))
            {
                return ABORT_USER;
            }
            if(tx_time_get() > audio_timeout && !auto_test)
            {
                // flag_transmission_message = TRUE;
                Audio_Message (eAUDIO_CMD_CONCAT, eAUDIO_TRANSMISSION_ONGOING, TRUE);  // warn about transmission
                Audio_Message (eAUDIO_CMD_CONCAT, eAUDIO_TAKE_SEVERAL_MINUTES, TRUE);
                audio_timeout = tx_time_get() + OSTIME_20SEC;
            }
        }
        if(Save_Comms_Error() == eERR_COMM_WIFI_SEND_TEST && !auto_test)
        {
            Audio_Message (eAUDIO_CMD_CONCAT, eAUDIO_CONN_ERROR, TRUE);
            Pasatiempos_Listening();
        }
    }
    return true;
}

/******************************************************************************
** Name:    Execute_Send_Wifi_Episode
*****************************************************************************/
/**
** @brief   Function that executes the sending of the wee episodes
**
** @param   pResult     pointer to result parameters
** @param   auto_test   if the test is automatic or forced by an user
**
** @return  Check if cover is closed or ON_OFF key is pressed
******************************************************************************/
bool_t Execute_Send_Wifi_Episode (DB_TEST_RESULT_t *pResult, bool_t auto_test)
{
    uint32_t ref_timeout = 0;
    char_t  sn_str [8];         // serial number string (last 4 characters)
    uint8_t sn_len = 0;         // serial number string length
    static DEVICE_INFO_t         my_Device_Info;          // pointer to the device info
    char_t  filename [32];      // file name
    uint32_t audio_timeout = 0;

    Refresh_Wdg ();

    // get the device info
    memcpy(&my_Device_Info, Get_Device_Info(), sizeof(DEVICE_INFO_t));

    sn_len = (uint8_t) strlen(my_Device_Info.sn);
    if ((sn_len >= 4) && (sn_len < RSC_NAME_SZ))
    {
        sn_str[0] = my_Device_Info.sn[sn_len - 5];
        sn_str[1] = my_Device_Info.sn[sn_len - 4];
        sn_str[2] = my_Device_Info.sn[sn_len - 3];
        sn_str[3] = my_Device_Info.sn[sn_len - 2];
        sn_str[4] = my_Device_Info.sn[sn_len - 1];
    }
    else {
        sn_str[0] = sn_str[1] = sn_str[2] = sn_str[3] = sn_str[4] ='0';
    }
    sn_str[5] = 0;

    sprintf (filename, "R100_%s_0X.WEE", sn_str);

    for (uint8_t i=0; i<NUM_EPISODES; i++)
    {
        //filename[11] = (char_t) (i + '0');

        filename[11] = (char_t) ((i/10) + '0');
        filename[12] = (char_t) ((i%10) + '0');

        if(Check_Episode_Present(filename) == TRUE)
        {
            DB_Episode_Set_Current_Filename(filename);
            // In case of a DIARY Test, perform the Wifi connection just in case
            if(pResult->test_id < TEST_FULL_ID)
            {
                // Establish Wifi connection and check if host is available
                if(Execute_Wifi_Init_Host_Alive(pResult, auto_test) == false)
                {
                    return false;
                }
            }

            // If server connection is Alive
            if(Comm_Get_Wifi_Host_Alive_Flag() == TRUE)
            {

                Refresh_Wdg ();

                // 30 minutes timeout for sending episode file
                ref_timeout = tx_time_get() + (OSTIME_60SEC*30);

                Refresh_Wdg ();

                Comm_Wifi_Send_Episode();

                Refresh_Wdg ();

                audio_timeout = tx_time_get() + OSTIME_20SEC;

                while((tx_time_get() <= ref_timeout) && (Comm_Is_Wifi_Episode_Sended() == FALSE))
                {
                    Refresh_Wdg ();
                    tx_thread_sleep (OSTIME_50MSEC);
                    // check if cover is closed or ON_OFF key is pressed
                    if (Check_Test_Abort_User(pResult))
                    {
                        return ABORT_USER;
                    }
                    if(tx_time_get() > audio_timeout)
                    {
                        //flag_transmission_message = TRUE;
                        Audio_Message (eAUDIO_CMD_CONCAT, eAUDIO_TRANSMISSION_ONGOING, TRUE);  // warn about transmission
                        Audio_Message (eAUDIO_CMD_CONCAT, eAUDIO_TAKE_SEVERAL_MINUTES, TRUE);
                        audio_timeout = tx_time_get() + OSTIME_20SEC;
                    }
                }
                if(Save_Comms_Error() == eERR_COMM_WIFI_SEND_EPISODE)
                {
                    Audio_Message (eAUDIO_CMD_CONCAT, eAUDIO_CONN_ERROR, TRUE);
                    Pasatiempos_Listening();
                }

                /*if(Comm_Is_Wifi_Episode_Sended() == TRUE)
                {
                    DB_Episode_Delete(filename);
                }*/
            }
            else
            {
                Audio_Message (eAUDIO_CMD_CONCAT, eAUDIO_CONN_ERROR, TRUE);
                Pasatiempos_Listening();
            }
        }
    }

    return true;
}

/******************************************************************************
** Name:    Execute_Receive_Wifi_Upgrade
*****************************************************************************/
/**
** @brief   Function that executes the receiving of the last firmware
**
** @param   pResult     pointer to result parameters
** @param   auto_test   if the test is automatic or forced by an user
**
** @return  Check if cover is closed or ON_OFF key is pressed
******************************************************************************/
bool_t Execute_Receive_Wifi_Upgrade (DB_TEST_RESULT_t *pResult, bool_t auto_test)
{
    uint32_t ref_timeout = 0;
    uint32_t audio_timeout = 0;

    //Refresh_Wdg ();

    // Establish Wifi connection and check if host is available
    //if(Execute_Wifi_Init_Host_Alive() == TRUE)
    //{
        Refresh_Wdg ();

        // 30 minutes timeout for receibe upgrade file
        ref_timeout = tx_time_get() + (OSTIME_60SEC * 30);

        Refresh_Wdg ();

        Comm_Wifi_Get_Firmware();

        Refresh_Wdg ();

        audio_timeout = tx_time_get() + OSTIME_20SEC;

        while((tx_time_get() <= ref_timeout) && (Comm_Is_Wifi_Upgrade_Received() == FALSE))
        {
            Refresh_Wdg ();
            tx_thread_sleep (OSTIME_500MSEC);
            // check if cover is closed or ON_OFF key is pressed
            if (Check_Test_Abort_User(pResult))
            {
                return ABORT_USER;
            }
            if(tx_time_get() > audio_timeout && !auto_test)
            {
                //flag_transmission_message = TRUE;
                Audio_Message (eAUDIO_CMD_CONCAT, eAUDIO_TRANSMISSION_ONGOING, TRUE);  // warn about transmission
                Audio_Message (eAUDIO_CMD_CONCAT, eAUDIO_TAKE_SEVERAL_MINUTES, TRUE);
                audio_timeout = tx_time_get() + OSTIME_20SEC;
            }
        }
        if(Save_Comms_Error() == eERR_COMM_WIFI_RECEIVE_UPGRADE_FILE && !auto_test)
        {
            Audio_Message (eAUDIO_CMD_CONCAT, eAUDIO_CONN_ERROR, TRUE);
            Pasatiempos_Listening();
        }
    //}
    return true;
}

/******************************************************************************
** Name:    Execute_Receive_Wifi_Configuration
*****************************************************************************/
/**
** @brief   Function that executes the receiving of the last configuration
**
** @param   pResult     pointer to result parameters
** @param   auto_test   if the test is automatic or forced by an user
**
** @return  Check if cover is closed or ON_OFF key is pressed
******************************************************************************/
bool_t Execute_Receive_Wifi_Configuration (DB_TEST_RESULT_t *pResult, bool_t auto_test)
{
    uint32_t ref_timeout = 0;
    uint32_t audio_timeout = 0;

    //Refresh_Wdg ();

    // Establish Wifi connection and check if host is available
    //if(Execute_Wifi_Init_Host_Alive() == TRUE)
    //{
        Refresh_Wdg ();

        // 5 minutes timeout for receibe configuration file
        ref_timeout = tx_time_get() + (OSTIME_60SEC*5);

        Refresh_Wdg ();

        Comm_Wifi_Get_Configuration();

        Refresh_Wdg ();

        audio_timeout = tx_time_get() + OSTIME_20SEC;

        while((tx_time_get() <= ref_timeout) && (Comm_Is_Wifi_Config_Received() == FALSE))
        {
            Refresh_Wdg ();
            tx_thread_sleep (OSTIME_50MSEC);
            // check if cover is closed or ON_OFF key is pressed
            if (Check_Test_Abort_User(pResult))
            {
                return ABORT_USER;
            }
            if(tx_time_get() > audio_timeout && !auto_test)
            {
                //flag_transmission_message = TRUE;
                Audio_Message (eAUDIO_CMD_CONCAT, eAUDIO_TRANSMISSION_ONGOING, TRUE);  // warn about transmission
                Audio_Message (eAUDIO_CMD_CONCAT, eAUDIO_TAKE_SEVERAL_MINUTES, TRUE);
                audio_timeout = tx_time_get() + OSTIME_20SEC;
            }
        }
        if(Save_Comms_Error() == eERR_COMM_WIFI_RECEIVE_CONFIG_FILE && !auto_test)
        {
            Audio_Message (eAUDIO_CMD_CONCAT, eAUDIO_CONN_ERROR, TRUE);
            Pasatiempos_Listening();
        }
    //}
    return true;
}

/******************************************************************************
 ** Name:    Send_Sigfox_Alert
 *****************************************************************************/
/**
 ** @brief   Function to send alerts by Sigfox
 **
 ** @param   alert_id   alert identifier
 **
 ** @return  none
 ******************************************************************************/
void Send_Sigfox_Alert(SIGFOX_MSG_ID_e alert_id)
{
    // In DEMO mode do not send anything related to sigfox
    if ((Is_Battery_Mode_Demo())  &&
       (alert_id != MSG_ID_COVER_OPEN_ALERT)    &&
       (alert_id != MSG_ID_COVER_CLOSE_ALERT)) return;

    if((Comm_Is_Sigfox_Initialized() == FALSE) || (Comm_Get_Selected_Uart() != eMOD_SIGFOX))
    {
        Comm_Sigfox_Open(); // Open Sigfox Comm.
    }
    Comm_Sigfox_Generate_Send_Alert(alert_id);   // Generate and Send Alert using Sigfox
}

/******************************************************************************
 ** Name:    Send_Sigfox_Test
 *****************************************************************************/
/**
 ** @brief   Function to send test results by Sigfox
 **
 ** @param   pResult   test result
 **
 ** @return  none
 ******************************************************************************/
void Send_Sigfox_Test(DB_TEST_RESULT_t *pResult)
{
    if(pResult->test_id == TEST_INITIAL_ID || pResult->test_id == TEST_FUNC_ID)
    {
        if (Is_Battery_Mode_Demo()) return;
    }

    if((Comm_Is_Sigfox_Initialized() == FALSE) || (Comm_Get_Selected_Uart() != eMOD_SIGFOX))
    {
        Comm_Sigfox_Open(); // Open Sigfox Comm.
    }
    Comm_Sigfox_Generate_Send_Test(pResult); // Generate and Send Test report using Sigfox
    Comm_Sigfox_Generate_Send_Expiration(pResult);
}

/******************************************************************************
 ** Name:    Send_Sigfox_Position
 *****************************************************************************/
/**
 ** @brief   Function to send position by Sigfox
 **
 ** @param   gps_id   gps identifier
 **
 ** @return  none
 ******************************************************************************/
void Send_Sigfox_Position(SIGFOX_MSG_ID_e gps_id)
{
    // In DEMO mode do not send anything related to sigfox
    if (Is_Battery_Mode_Demo()) return;

    if((Comm_Is_Sigfox_Initialized() == FALSE) || (Comm_Get_Selected_Uart() != eMOD_SIGFOX))
    {
        Comm_Sigfox_Open(); // Open Sigfox Comm.
    }
    Comm_Sigfox_Generate_Send_Position(gps_id); // Generate and Send Test report using Sigfox
}

/******************************************************************************
 ** Name:    Send_Wifi_Alert
 *****************************************************************************/
/**
 ** @brief   Function to send alerts by Wifi
 **
 ** @param   alert_id   alert identifier
 **
 ** @return  none
 ******************************************************************************/
void Send_Wifi_Alert(WIFI_MSG_ID_e alert_id)
{
    // In DEMO mode do not send anything related to wifi
    if (Is_Battery_Mode_Demo()  &&
       (alert_id != WIFI_MSG_ID_COVER_OPEN_ALERT)    &&
       (alert_id != WIFI_MSG_ID_COVER_CLOSE_ALERT)) return;

    if(Comm_Is_Wifi_Initialized() == FALSE || (Comm_Get_Selected_Uart() != eMOD_WIFI))
    {
        Comm_Wifi_Open(); // Open wifi Comm.
    }
    Comm_Wifi_Generate_Send_Alert_Frame(alert_id); // Generate and Send Test report using Wifi
}

/******************************************************************************
 ** Name:    Send_Wifi_Test_Frame
 *****************************************************************************/
/**
 ** @brief   Function to send test results by Wifi
 **
 ** @param   pResult   test results
 **
 ** @return  none
 ******************************************************************************/
void Send_Wifi_Test_Frame(DB_TEST_RESULT_t *pResult)
{
    if(pResult->test_id == TEST_INITIAL_ID || pResult->test_id == TEST_FUNC_ID)
    {
        if (Is_Battery_Mode_Demo()) return;
    }

    if(Comm_Is_Wifi_Initialized() == FALSE || (Comm_Get_Selected_Uart() != eMOD_WIFI))
    {
        Comm_Wifi_Open(); // Open wifi Comm.
    }
    Comm_Wifi_Generate_Send_Test_Frame(pResult); // Generate and Send Test report using Wifi
}

/******************************************************************************
 ** Name:    Send_Wifi_Position
 *****************************************************************************/
/**
 ** @brief   Function to send position by Wifi
 **
 ** @param   alert_id   gps identifier
 **
 ** @return  none
 ******************************************************************************/
void Send_Wifi_Position(WIFI_MSG_ID_e alert_id)
{
    // In DEMO mode do not send anything related to sigfox
    if (Is_Battery_Mode_Demo()) return;

    if(Comm_Is_Wifi_Initialized() == FALSE || (Comm_Get_Selected_Uart() != eMOD_WIFI))
    {
        Comm_Wifi_Open(); // Open wifi Comm.
    }
    Comm_Wifi_Generate_Send_Position_Frame(alert_id); // Generate and Send Test report using Wifi
}

/******************************************************************************
 ** Name:    Is_GPS_TX_Enabled
 *****************************************************************************/
/**
 ** @brief   Check if the GPS transmission is enabled
 **
 ** @param   none
 **
 ** @return  TRUE if enabled, else FALSE
 ******************************************************************************/
bool_t Is_GPS_TX_Enabled (void)
{
    if (Get_Device_Info()->enable_b.sigfox == 0 && Get_Device_Info()->enable_b.wifi == 0) return FALSE;
    if (Get_Device_Info()->enable_b.gps == 1 &&
        Get_Device_Settings()->misc.alert_options.gps == 1) return TRUE;

    return FALSE;
}

/******************************************************************************
 ** Name:    Is_Sigfox_TX_Enabled
 *****************************************************************************/
/**
 ** @brief   Check if the Sigfox transmission is enabled
 **
 ** @param   none
 **
 ** @return  TRUE if enabled, else FALSE
 ******************************************************************************/
uint8_t Is_Sigfox_TX_Enabled (void)
{
    if (Get_Device_Info()->enable_b.sigfox == 0) return FALSE;

    if (Get_Device_Settings()->misc.alert_options.sigfox_only == 1) return SIGFOX_ONLY;
    if (Get_Device_Settings()->misc.alert_options.sigfox_pri == 1) return SIGFOX_PRIOR;
    if (Get_Device_Settings()->misc.alert_options.wifi_pri == 1) return WIFI_PRIOR;

    return FALSE;
}

/******************************************************************************
** Name:    Is_Wifi_TX_Enabled
*****************************************************************************/
/**
** @brief   Check if the Wifi transmission enabled
**
** @param   none
**
** @return  TRUE if enabled, else FALSE
******************************************************************************/
uint8_t Is_Wifi_TX_Enabled (void)
{
    if (Get_Device_Info()->enable_b.wifi == 0 || Get_Device_Settings()->misc.glo_transmis_mode == 0) return FALSE;

    if (Get_Device_Settings()->misc.alert_options.wifi_only == 1) return WIFI_ONLY;
    if (Get_Device_Settings()->misc.alert_options.wifi_pri == 1) return WIFI_PRIOR;
    if (Get_Device_Settings()->misc.alert_options.sigfox_pri == 1) return SIGFOX_PRIOR;

    return FALSE;
}

/******************************************************************************
** Name:    Wifi_Trans_Days
*****************************************************************************/
/**
** @brief   Check if the Wifi transmission enabled
**
** @param   none
**
** @return  TRUE if enabled, else FALSE
******************************************************************************/
uint8_t Wifi_Trans_Days(void)
{
    if (Get_Device_Info()->enable_b.wifi == 0 || Get_Device_Settings()->misc.glo_transmis_mode == 0) return FALSE;

    if (Get_Device_Settings()->misc.alert_options.wifi_1_day == 1) return WIFI_1_DAY;
    if (Get_Device_Settings()->misc.alert_options.wifi_7_day == 1) return WIFI_7_DAY;
    if (Get_Device_Settings()->misc.alert_options.wifi_30_day == 1) return WIFI_30_DAY;

    return FALSE;
}


/**
 * @brief This function initialize and configure acclerometer 
 *        According to the XYZ values in stable position, the x, y or z axes will be enabled.
 *        If it is necessary to reverse the polarity of the interrupt, INT1 will also be configured.
 * 
 * @param pNV_data pointer to NV data
 * 
 * @return acc errors
 */
uint32_t Lis3dh_ACC_Init(NV_DATA_BLOCK_t *pNV_data)
{
    uint32_t err_acc = 0;

    if(err_acc = Comm_ACC_Initialize(), err_acc != 0) //IÑIGO
    {
        Trace_Arg (TRACE_NO_FLAGS | TRACE_NEWLINE , "ACC ERROR INIT : %d", (uint32_t) err_acc);
        // Disable interrupt and not configure again if the acc is not present
        if(err_acc == eERR_COMM_ACC_PRESENCE)
        {
            pNV_data->acc_pos_hvi = 4;
        }
        //Accel_Interrupt_Init (0, 0, 0, 0);
        return err_acc;
    }

    // Get acceleration data
    Comm_ACC_Get_Acceleration_Data();
    Refresh_Wdg ();
    // Check if the R100 device is horizontal and configure the interruption
    if((R100_acc.x_data < 20 && R100_acc.x_data > -20) && (R100_acc.y_data < 20 && R100_acc.y_data > -20))
    {
        //Trace (TRACE_TIME_STAMP | TRACE_NEWLINE, "HORIZONTAL");
        if(err_acc = Accel_Interrupt_Init (ACC_ENABLE_XY_HIGH_AXIS, ACC_THRESOLD, ACC_DURATION, ACC_POLARITY_HIGH), err_acc != eERR_NONE)
        {
            Trace_Arg (TRACE_NO_FLAGS | TRACE_NEWLINE , "ACC ERROR INTERRUPT INIT : %d", (uint32_t) err_acc);
            return err_acc;
        }
        pNV_data->acc_pos_hvi = 1;
        return err_acc;
    }
    // Check if the R100 device is vertical and configure the interruption
    if(R100_acc.z_data < 25 && R100_acc.z_data > -25)
    {
        //Trace (TRACE_TIME_STAMP | TRACE_NEWLINE, "VERTICAL");
        if(err_acc = Accel_Interrupt_Init (ACC_ENABLE_Z_HIGH_AXIS, ACC_THRESOLD_Z, ACC_DURATION, ACC_POLARITY_HIGH), err_acc != eERR_NONE)
        {
            Trace_Arg (TRACE_NO_FLAGS | TRACE_NEWLINE , "ACC ERROR INTERRUPT INIT : %d", (uint32_t) err_acc);
            return err_acc;
        }
        pNV_data->acc_pos_hvi = 2;
        return err_acc;
    }
    // Check if the R100 device is inclined and configure the interruption
    if(R100_acc.z_data > 25)
    {
        //Trace (TRACE_TIME_STAMP | TRACE_NEWLINE, "INCLINED");
        if(err_acc = Accel_Interrupt_Init (ACC_ENABLE_XY_HIGH_AXIS, ACC_THRESOLD, ACC_DURATION, ACC_POLARITY_LOW), err_acc != eERR_NONE)
        {
            Trace_Arg (TRACE_NO_FLAGS | TRACE_NEWLINE , "ACC ERROR INTERRUPT INIT : %d", (uint32_t) err_acc);
            return err_acc;
        }
        pNV_data->acc_pos_hvi = 3;
        return err_acc;
    }
    return eERR_NONE;
}

/******************************************************************************
** Name:    Send_Test_By_Comms
*****************************************************************************/
/**
** @brief   Function to check the wireless comms. devices
**
** @param   pResult     pointer to result parameters
** @param   auto_test   if the test is automatic or forced by an user
**
** @return  none
******************************************************************************/
void Send_Test_By_Comms (DB_TEST_RESULT_t *pResult, bool_t auto_test)
{
            SIGFOX_MSG_ID_e gps_id;
            uint32_t        position;
            uint32_t        tout = (OSTIME_60SEC*3);    // 3 minutes timeout for GPS
            uint8_t         fx_res, wifi_rps;
    static  FX_FILE         update_file;

    // check if the Sigfox option is enabled AND no sigfox error in comms test
    if ((Is_Sigfox_TX_Enabled() == SIGFOX_ONLY || Is_Sigfox_TX_Enabled() == SIGFOX_PRIOR) && (pResult->comms.sigfox == eERR_NONE))
    {
        // 1. SIGFOX: Send test result through SIGFOX
        if (Execute_Send_Sigfox_Test(pResult, auto_test) == ABORT_USER) { return; }
        Refresh_Wdg ();
        if (Execute_Send_Sigfox_Exp(pResult, auto_test) == ABORT_USER) { return; }
        Refresh_Wdg ();
        if(pResult->test_id == TEST_FULL_ID || pResult->test_id == TEST_MANUAL_ID)
        {
            Inc_SigfoxManualTest();
        }
        else Inc_SigfoxDailyTest();
    }

    // check if the wifi option is enabled AND no wifi error in comms test
    if ((Is_Wifi_TX_Enabled() == WIFI_ONLY || Is_Wifi_TX_Enabled() == WIFI_PRIOR) && (pResult->comms.wifi == eERR_NONE))
    {
        // 0. WIFI:
        // In case of a MANUAL or AUTOTEST (Not MONTHLY!!!) Test, send test frame result through WIFI IF sigfox option active AND in error or NO sigfox option active
        //if ((pResult->test_id < TEST_FULL_ID) || (pResult->test_id == TEST_MANUAL_ID))
        if((Wifi_Trans_Days() == WIFI_1_DAY || 
            ((Wifi_Trans_Days() == WIFI_7_DAY && (pResult->test_id == 8 || pResult->test_id == 15 || pResult->test_id == 22 || pResult->test_id == TEST_FULL_ID)) || pResult->error_code != eERR_NONE) || 
            ((Wifi_Trans_Days() == WIFI_30_DAY && pResult->test_id == TEST_FULL_ID)) || pResult->error_code != eERR_NONE) || 
            (pResult->test_id == TEST_MANUAL_ID))
        {
            // Establish Wifi connection and check if host is available
            //if (Execute_Wifi_Init_Host_Alive(pResult) == false) { return; }

            // If server connection is Alive
            //if (Comm_Get_Wifi_Host_Alive_Flag() == TRUE)
            {
                if(wifi_rps = Execute_Send_Wifi_Test_Frame(pResult, auto_test), wifi_rps == ABORT_USER) { return; }
            }
        }

        // 0.5 WIFI:
        // In case of an AUTOTEST (Not MONTHLY!!!) Test, send Episode .WEE File through Wifi if there are pending
        /*if (pResult->test_id < TEST_FULL_ID) // WIFI DESCOMENTAR
        {
            // Send Test File, it will check if its a DIARY Test, and will connect to Wifi
            if (Execute_Send_Wifi_Episode(pResult, auto_test) == false) { return; }
        }*/

        // 1. WIFI:
        // In case of a MANUAL or MONTHLY Test, send Test File through Wifi
        // 2. WIFI:
        // In case of a MANUAL or MONTHLY Test, send Episode .WEE File through Wifi if there are pending
        // 3. WIFI:
        // In case of a MANUAL or MONTHLY Test, If there is a cfg or firmware available download them
        /*if ((pResult->test_id == TEST_MANUAL_ID) || (pResult->test_id == TEST_FULL_ID)) // WIFI DESCOMENTAR
        {
            // Establish Wifi connection and check if host is available
            if (Execute_Wifi_Init_Host_Alive(pResult, auto_test) == false) { return; }
            // If server connection is Alive
            if (Comm_Get_Wifi_Host_Alive_Flag() == TRUE)
            {
                // Send Test File
                if (Execute_Send_Wifi_Test_File(pResult, auto_test) == false) { return; }

                // Send Episode .WEE File if any pending
                if (Execute_Send_Wifi_Episode(pResult, auto_test) == false) { return; }

                // check if the .cfg is available
                if (Comm_Wifi_Is_Cfg_Available())
                {
                    // Receive Wifi Configuration File
                    if (Execute_Receive_Wifi_Configuration(pResult, auto_test) == false) { return; }
                }

                // check if firmware is available
                if (Comm_Wifi_Is_Frmw_Available())
                {
                    // Receive Wifi Upgrade File
                    if (Execute_Receive_Wifi_Upgrade(pResult, auto_test)  == false) { return; }
                }
            }
        }*/
    }

    // Load and read configuration file on SD
    fx_res = (uint8_t) fx_file_open(&sd_fx_media, &update_file, UPGRADE_FILENAME, FX_OPEN_FOR_READ);
    if (fx_res == 0)
    {
        // Do not perform GPS operations....
        Trace (TRACE_NEWLINE, "");
        fx_res = (uint8_t) fx_file_close (&update_file);
    }
    else
    {
       // check if the GPS option is enabled
        if (Is_GPS_TX_Enabled() && (pResult->comms.gps == eERR_NONE)  &&                    // Check if GPS is activate
           ((Is_Sigfox_TX_Enabled() != FALSE && (pResult->comms.sigfox == eERR_NONE)) ||    // Must be sent by sigfox?
           (Is_Wifi_TX_Enabled() != FALSE && (pResult->comms.wifi == eERR_NONE))) &&        // Must be sent by wifi?
           Is_Battery_Mode_Demo() == FALSE &&                                               // In DEMO mode do not send anything related to GPS
           Get_NV_Data()->update_flag == FALSE)                                                    // After an update GPS does not run
        {
            // If a Monthly or Manual Test has been performed, send also the GPS Position
            if((pResult->test_id == TEST_FULL_ID) || (pResult->test_id == TEST_MANUAL_ID))
            {
                if(Is_Wifi_TX_Enabled() == WIFI_ONLY && (wifi_rps == ROUTER_NO_CONECT || wifi_rps == SERVER_NO_CONNECT))
                {

                }
                else
                {
                    Refresh_Wdg ();

                    // Wait some time to get the GPS position...
                    position = Execute_Get_Gps_Position_Test (pResult, tout);

                    if ((Is_Sigfox_TX_Enabled() == SIGFOX_ONLY || Is_Sigfox_TX_Enabled() == SIGFOX_PRIOR) && (pResult->comms.sigfox == eERR_NONE))
                    {
                        Inc_SigfoxManualTestGPS();
                    }

                    if ((Is_Wifi_TX_Enabled() == WIFI_ONLY || Is_Wifi_TX_Enabled() == WIFI_PRIOR) && (pResult->comms.wifi == eERR_NONE))
                    {
                        Inc_WifiTestConnectGPS();
                    }

                    if (position == 2) { return; }

                    // position obtained
                    if (position == 1)
                    {
                        Refresh_Wdg ();

                        // the GPS test is classified as manual or periodic (automatic)
                        gps_id = (pResult->test_id == TEST_MANUAL_ID) ? MSG_ID_MAN_TEST_GPS : MSG_ID_MON_TEST_GPS;

                        //must be sent by sigfox?
                        if ((Is_Sigfox_TX_Enabled() == SIGFOX_ONLY || Is_Sigfox_TX_Enabled() == SIGFOX_PRIOR) && (pResult->comms.sigfox == eERR_NONE))
                        {
                            if(wifi_rps = Execute_Send_Sigfox_GPS(pResult, auto_test, gps_id), wifi_rps == ABORT_USER) return;
                        }

                        //must be sent by wifi
                        if ((Is_Wifi_TX_Enabled() == WIFI_ONLY || Is_Wifi_TX_Enabled() == WIFI_PRIOR) && (pResult->comms.wifi == eERR_NONE))
                        {
                            // Establish Wifi connection and check if host is available (just in case)
                            //if (Execute_Wifi_Init_Host_Alive(pResult) == false) { return; }

                            // If server connection is Alive
                            //if (Comm_Get_Wifi_Host_Alive_Flag() == TRUE)
                            {
                                if (wifi_rps = Execute_Send_Wifi_Position_Frame(pResult, gps_id, auto_test), wifi_rps == ABORT_USER) return;
                            }
                        }

                        Refresh_Wdg ();

                        // wait while playing all pending audio messages ...
                        Pasatiempos_Listening();
                    }
                }
            }
        }
    }

    // Play last transmission message
    //if(flag_transmission_message)
    if(!auto_test)
    {
        Audio_Message (eAUDIO_CMD_CONCAT, eAUDIO_TRANSMISSION_DONE, TRUE);
        Pasatiempos_Listening();
    }
}
