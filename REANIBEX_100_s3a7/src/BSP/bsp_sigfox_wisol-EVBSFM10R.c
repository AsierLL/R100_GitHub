/******************************************************************************
 * Name      : S3A7_REANIBEX_100                                              *
 * Copyright : Osatu S. Coop                                                  *
 * Compiler  : MinGW32                                                        *
 * Target    : Reanibex Series                                                *
 ******************************************************************************/

/*!
 * @file        bsp_sigfox_wisol-EVBSFM10R.c
 * @brief       BSP for Wisol EVBSFM10R board
 *
 * @version     v1
 * @date        27/05/2019
 * @author      jarribas
 * @warning     author   date    version     .- Edition
 * @bug
 *
 */

/******************************************************************************
 ** Includes
 */

#include "BSP/bsp_sigfox_wisol-EVBSFM10R.h"
#include "HAL/thread_comm_hal.h"
#include "Comm.h"
#include "hal_data.h"

/******************************************************************************
 ** Macros
 */

/******************************************************************************
 ** Defines
 */
#define MAX_SIGFOX_CMD_LENGTH 100
#define MAX_SIGFOX_DATA_LENGTH 13  ///< 12 bytes payload + NULL
/******************************************************************************
 ** Typedefs
 */

/******************************************************************************
 ** Constants
 */

/******************************************************************************
 ** Externals
 */
extern sigfox_config_t R100_sigfox;

/******************************************************************************
 ** Globals
 */
static char_t sigfox_cmd [MAX_SIGFOX_CMD_LENGTH];

/******************************************************************************
 ** Locals
 */

/******************************************************************************
 ** Prototypes
 */
static void   Sigfox_Send_Command     (const char_t *command);
static bool_t Sigfox_Command_Response (char_t* ack_1, char_t* ack_2, uint32_t tout);
static void   Sigfox_Command_Response_Get_Data (char_t* data, uint32_t data_length);

/******************************************************************************
** Name:    Sigfox_Check_Module_Status
*****************************************************************************/
/**
** @brief   Configure Wifi module
**
** @param   none
**
** @return  ERROR_ID_e error code
******************************************************************************/
ERROR_ID_e Sigfox_Check_Module_Status(void)
{
    Sigfox_Send_Command(SIGFOX_CMD_AT);
    if (Sigfox_Command_Response (SIGFOX_RESP_OK, NULL, OSTIME_3SEC) == FALSE)
    {
        Sigfox_Send_Command(SIGFOX_CMD_AT);
        if (Sigfox_Command_Response (SIGFOX_RESP_OK, NULL, OSTIME_3SEC) == FALSE)
        {
            return eERR_COMM_SIGFOX_STATUS;
        }
    }

    return eERR_NONE;
}

/******************************************************************************
** Name:    Sigfox_Get_Module_DeviceID
*****************************************************************************/
/**
** @brief   Get Sigfox Module DeviceID
**
** @param   none
**
** @return  ERROR_ID_e error code
******************************************************************************/
ERROR_ID_e Sigfox_Get_Module_DeviceID(void)
{
    Sigfox_Send_Command(SIGFOX_CMD_DEVID);
    Sigfox_Command_Response_Get_Data(R100_sigfox.device_id, MAX_SIGFOX_DEV_ID_LENGTH);
    return eERR_NONE;
}

/******************************************************************************
** Name:    Sigfox_Get_Module_PAC
*****************************************************************************/
/**
** @brief   Get Sigfox Module PAC
**
** @param   none
**
** @return  ERROR_ID_e error code
******************************************************************************/
ERROR_ID_e Sigfox_Get_Module_PAC(void)
{
    Sigfox_Send_Command(SIGFOX_CMD_PAC);
    Sigfox_Command_Response_Get_Data(R100_sigfox.device_pac, MAX_SIGFOX_DEV_PAC_LENGTH);
    return eERR_NONE;
}

/******************************************************************************
** Name:    Sigfox_Get_TX_Frequency
*****************************************************************************/
/**
** @brief   Get Sigfox Module TX Frequency
**
** @param   none
**
** @return  ERROR_ID_e error code
******************************************************************************/
ERROR_ID_e Sigfox_Get_TX_Frequency(void)
{
    Sigfox_Send_Command(SIGFOX_CMD_TX_FREQ);
    Sigfox_Command_Response_Get_Data(R100_sigfox.device_tx_freq, MAX_SIGFOX_FREQ_LENGTH);
    return eERR_NONE;
}

/******************************************************************************
** Name:    Sigfox_Get_RX_Frequency
*****************************************************************************/
/**
** @brief   Get Sigfox Module RX Frequency
**
** @param   none
**
** @return  ERROR_ID_e error code
******************************************************************************/
ERROR_ID_e Sigfox_Get_RX_Frequency(void)
{
    Sigfox_Send_Command(SIGFOX_CMD_RX_FREQ);
    Sigfox_Command_Response_Get_Data(R100_sigfox.device_rx_freq, MAX_SIGFOX_FREQ_LENGTH);
    return eERR_NONE;
}

/******************************************************************************
** Name:    Sigfox_Get_Module_Temp
*****************************************************************************/
/**
** @brief   Get Sigfox Module Temperature
**
** @param   none
**
** @return  ERROR_ID_e error code
******************************************************************************/
ERROR_ID_e Sigfox_Get_Module_Temp(void)
{
    Sigfox_Send_Command(SIGFOX_CMD_TEMP);
    return eERR_NONE;
}

/******************************************************************************
** Name:    Sigfox_Get_Module_Voltages
*****************************************************************************/
/**
** @brief   Get Sigfox Module Voltages
**
** @param   none
**
** @return  ERROR_ID_e error code
******************************************************************************/
ERROR_ID_e Sigfox_Get_Module_Voltages(void)
{
    Sigfox_Send_Command(SIGFOX_CMD_VOLTAGES);
    return eERR_NONE;
}

/******************************************************************************
** Name:    Sigfox_Compare_Test_Frequencies
*****************************************************************************/
/**
** @brief   Compare Sigfox module TX & RX Test Frequencies
**
** @param   none
**
** @return  none
******************************************************************************/
ERROR_ID_e Sigfox_Compare_Test_Frequencies(void)
{
    if(memcmp(R100_sigfox.device_tx_freq, SIGFOX_TEST_TX_FREQ, MAX_SIGFOX_FREQ_LENGTH) != 0)
    {
        return eERR_COMM_SIGFOX_TEST_TX_FREQ;
    }

    if(memcmp(R100_sigfox.device_rx_freq, SIGFOX_TEST_RX_FREQ, MAX_SIGFOX_FREQ_LENGTH) != 0)
    {
        return eERR_COMM_SIGFOX_TEST_RX_FREQ;
    }

    return eERR_NONE;
}

/******************************************************************************
** Name:    Sigfox_Start_CW_Test
*****************************************************************************/
/**
** @brief   Start Continuous Wave Test
**
** @param   none
**
** @return  ERROR_ID_e error code
******************************************************************************/
ERROR_ID_e Sigfox_Start_CW_Test(void)
{
    Sigfox_Send_Command(SIGFOX_CMD_CW_TEST_ON);
    if (Sigfox_Command_Response (SIGFOX_RESP_OK, NULL, OSTIME_3SEC) == FALSE)
    {
        return eERR_COMM_SIGFOX_CW_TEST_START;
    }

    return eERR_NONE;
}

/******************************************************************************
** Name:    Sigfox_Stop_CW_Test
*****************************************************************************/
/**
** @brief   Stop Continuous Wave Test
**
** @param   none
**
** @return  ERROR_ID_e error code
******************************************************************************/
ERROR_ID_e Sigfox_Stop_CW_Test(void)
{
    Sigfox_Send_Command(SIGFOX_CMD_CW_TEST_OFF);
    if (Sigfox_Command_Response (SIGFOX_RESP_OK, NULL, OSTIME_3SEC) == FALSE)
    {
        return eERR_COMM_SIGFOX_CW_TEST_STOP;
    }

    return eERR_NONE;
}

/******************************************************************************
** Name:    Sigfox_Start_RX_GFSK_Test
*****************************************************************************/
/**
** @brief   Start RX GFSK test mode
**
** @param   none
**
** @return  ERROR_ID_e error code
******************************************************************************/
ERROR_ID_e Sigfox_Start_RX_GFSK_Test(void)
{
    Sigfox_Send_Command(SIGFOX_CMD_SE_TEST_ON);
    if (Sigfox_Command_Response (SIGFOX_RESP_SE_TEST_ON, NULL, OSTIME_3SEC) == FALSE)
    {
        return eERR_COMM_SIGFOX_RX_GFSK_TEST;
    }

    return eERR_NONE;
}

/******************************************************************************
** Name:    Sigfox_Send_Data_Frame
*****************************************************************************/
/**
** @brief   Send data frame through Sigfox
**
** @param   none
**
** @return  ERROR_ID_e error code
******************************************************************************/
ERROR_ID_e Sigfox_Send_Data_Frame(uint8_t* data)
{
    uint8_t frame[MAX_SIGFOX_DATA_LENGTH];
    char_t data_hex[25];

    memset (frame, 0, MAX_SIGFOX_DATA_SZ);
    memset (data_hex, 0, 25);

    memcpy(frame, data, MAX_SIGFOX_DATA_SZ);

    for (uint8_t i = 0 ; i != MAX_SIGFOX_DATA_LENGTH ; i++)
    {
        //lint -e586 Admissible use of sprintf (deprecated warning)
        sprintf(&data_hex[2*i], "%02X", frame[i]);
    }
    //insert NULL at the end of the output string
    data_hex[24] = '\0';

    Sigfox_Send_Command(SIGFOX_CMD_MAX_POWER);
    if (Sigfox_Command_Response (SIGFOX_RESP_OK, NULL, OSTIME_3SEC) == FALSE)
    {
        Sigfox_Send_Command(SIGFOX_CMD_MAX_POWER);
        if (Sigfox_Command_Response (SIGFOX_RESP_OK, NULL, OSTIME_3SEC) == FALSE)
        {
            return eERR_COMM_SIGFOX_MAX_POWER;
        }
    }

    // Clear command variable
    memset (sigfox_cmd, 0, MAX_SIGFOX_CMD_LENGTH);

    strcpy(sigfox_cmd, SIGFOX_CMD_SEND_FRAME);
    memcpy(&sigfox_cmd[6], data_hex, 25);

    Sigfox_Send_Command(sigfox_cmd);
    if (Sigfox_Command_Response (SIGFOX_RESP_OK, NULL, OSTIME_20SEC) == FALSE)
    {
        Sigfox_Send_Command(sigfox_cmd);
        if (Sigfox_Command_Response (SIGFOX_RESP_OK, NULL, OSTIME_20SEC) == FALSE)
        {
            return eERR_COMM_SIGFOX_RX;
        }
    }

    return eERR_NONE;
}

/******************************************************************************
** Name:    Sigfox_Send_Command
*****************************************************************************/
/**
** @brief   Send a command to the Sigfox module through UART
**
** @param   command pointer to the command str
**
** @return  none
******************************************************************************/
static void Sigfox_Send_Command(const char_t *command)
{
    if(strlen(command) > MAX_SIGFOX_CMD_LENGTH)
    {
        return;
    }

    Comm_Uart_Send(command);
}

/******************************************************************************
** Name:    Sigfox_Command_Response
*****************************************************************************/
/**
** @brief   Waits for a response to a given command
**
** @param   ack   expected response number 1
** @param   ack   expected response number 2 (NULL if only 1 response expected)
** @param   tout  timeout for the response
**
** @return  bool_t  true if received expected response, false if timeout expired
******************************************************************************/
static bool_t Sigfox_Command_Response (char_t* ack_1, char_t* ack_2, uint32_t tout)
{
    ERROR_ID_e  retError;
    uint32_t    ref_timeout;
    char        sigfox_rx_msg[256];

    memset(sigfox_rx_msg,0,sizeof(sigfox_rx_msg));

    ref_timeout = tx_time_get() + tout;

    while(tx_time_get() <= ref_timeout)
    {
        retError = Comm_Uart_Wifi_Receive (sigfox_rx_msg, (uint8_t) strlen (ack_1), tout, 0);

        if(retError != eERR_NONE)
        {
            return FALSE;
        }

        if(memcmp(sigfox_rx_msg, ack_1, strlen (ack_1)) == 0)
        {
            return TRUE;
        }

        //Clear to make another read
        memset(sigfox_rx_msg,0,sizeof(sigfox_rx_msg));

        if(ack_2 != NULL)
        {

            retError = Comm_Uart_Wifi_Receive (sigfox_rx_msg, (uint8_t) strlen (ack_2), tout, 0);

            if(retError != eERR_NONE)
            {
                return FALSE;
            }

            if(memcmp(sigfox_rx_msg, ack_2, strlen (ack_2)) == 0)
            {
                return TRUE;
            }

            //Clear to make another read
            memset(sigfox_rx_msg,0,sizeof(sigfox_rx_msg));
        }
    }

    return FALSE;
}

/******************************************************************************
** Name:    Sigfox_Command_Response_Get_Data
*****************************************************************************/
/**
** @brief   Waits for a response to a given command
**
** @param   data         obtained data
** @param   data_length  expected data length
**
** @return  none
******************************************************************************/
static void Sigfox_Command_Response_Get_Data (char_t* data, uint32_t data_length)
{
    ERROR_ID_e retError;

    retError = Comm_Uart_Wifi_Receive (data, (uint8_t) data_length, OSTIME_3SEC, 0);

    if(retError != eERR_NONE)
    {
        memset(data, 0, data_length);
    }
}
