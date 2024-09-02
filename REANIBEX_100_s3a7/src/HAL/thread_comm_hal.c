/******************************************************************************
 * Name      : S3A7_REANIBEX_100                                              *
 * Copyright : Osatu S. Coop                                                  *
 * Compiler  : MinGW32                                                        *
 * Target    : Reanibex Series                                                *
 ******************************************************************************/

/*!
 * @file        thread_comm_hal.c
 * @brief       HAL for Comm Thread
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

#include <ctype.h>

#include "BSP/bsp_sigfox_wisol-EVBSFM10R.h"
#include "BSP/bsp_wifi_silabs-WGM110.h"
#include "BSP/bsp_gps_quectel-L96.h"
#include "BSP/bsp_acc_LIS3DH.h"
#include "HAL/thread_comm_hal.h"
#include "thread_comm.h"
#include "thread_api.h"
#include "Comm.h"
#include "Trace.h"
#include "I2C_2.h"

#include "thread_defibrillator_hal.h"
#include "thread_core_entry.h"
#include "thread_drd_entry.h"
#include "event_ids.h"
#include "sysMon_Battery.h"

/******************************************************************************
 ** Macros
 */

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

/******************************************************************************
 ** Globals
 */

wifi_config_t   R100_wifi;
sigfox_config_t R100_sigfox;
gps_config_t    R100_gps;
acc_config_t    R100_acc;

SIGFOX_TEST_RESULT_t            sigfox_test_result;     ///< Test result structure
SIGFOX_FUNC_ALERT_t             sigfox_func_alert;      ///< Functional alert structure
SIGFOX_GPS_POSITION_t           sigfox_gps_position;    ///< Functional gps structure
SIGFOX_ELEC_BATT_EXPIRATION_t   sigfox_expiration;      ///< Battery and electrodes expiration dates structure

WIFI_TEST_RESULT_t              wifi_test_result;       ///< Test result structure
WIFI_FUNC_ALERT_t               wifi_func_alert;        ///< Functional alert structure
WIFI_FUNC_ALERT_t               wifi_func_alert_aux;    ///< Auxiliar functional alert structure
WIFI_GPS_POSITION_t             wifi_gps_position;      ///< Functional gps structure

static COMM_MOD_e           comm_selected_uart = eMOD_SIGFOX;
static bool_t               is_sigfox_free = true;
//static bool_t               donwlink_fisrt_msg = true;

static ERROR_ID_e           retError = eERR_NONE;

static WIFI_HOST_RESPONSE_t host_response;
static WIFI_SERVER_RESPONSE_t server_response;

extern WIFI_CERT_t R100_wifi_cert;
extern bool_t       first_time_gps;             ///< Flag to transmit gps position only once

/******************************************************************************
 ** Locals
 */

/******************************************************************************
 ** Prototypes
 */

/******************************************************************************
 ** Name:    Comm_Initialize
 *****************************************************************************/
/**
 ** @brief   Initialization of Comm UART
 ** @param   void
 **
 ******************************************************************************/
void Comm_Modules_Initialize(void)
{
/******************************************************************************************/
    // 0.- Reset WIFI SIGFOX GPS module & Initialize Comm UART
/******************************************************************************************/
    //Comm_Hardware_Reset(eMOD_SIGFOX);

    Comm_Uart_Init();

    //retError = Comm_Uart_Set_Baud_Rate(BR_115200);
    retError = Comm_Uart_Set_Baud_Rate(BR_9600);
}


/******************************************************************************
** Name:    Comm_Hardware_Off
*****************************************************************************/
/**
** @brief   Reset Wifi, Sigfox or GPS module
**
** @param   none
******************************************************************************/
void Comm_Hardware_Off(COMM_MOD_e comm_mod)
{
    uint8_t  i2c_data;

    if((comm_mod != eMOD_WIFI) && (comm_mod != eMOD_SIGFOX) && (comm_mod != eMOD_GPS)) return;

    // Read PCF8574 previous status
    I2C2_ReadByte (i2C_ADD_GPIO, &i2c_data);

    switch(comm_mod)
    {
        case eMOD_WIFI:
            i2c_data = (uint8_t)(i2c_data & (~(1<<1))) | (0<<1);
            break;
        case eMOD_SIGFOX:
            i2c_data = (uint8_t)(i2c_data & (~(1<<2))) | (0<<2);
            break;
        case eMOD_GPS:
            i2c_data = (uint8_t)(i2c_data & ~1) | 0;
            break;
        default:
            break;
    }

    I2C2_WriteByte (i2C_ADD_GPIO, i2c_data);

    (void) tx_thread_sleep (OSTIME_200MSEC);
}

/******************************************************************************
** Name:    Comm_Hardware_On
*****************************************************************************/
/**
** @brief   Reset Wifi, Sigfox or GPS module
**
** @param   none
******************************************************************************/
void Comm_Hardware_On(COMM_MOD_e comm_mod)
{
    uint8_t  i2c_data;

    if((comm_mod != eMOD_WIFI) && (comm_mod != eMOD_SIGFOX) && (comm_mod != eMOD_GPS)) return;

    // Read PCF8574 previous status
    I2C2_ReadByte (i2C_ADD_GPIO, &i2c_data);

    switch(comm_mod)
    {
        case eMOD_WIFI:
            i2c_data = (uint8_t)(i2c_data & (~(1<<1))) | (1<<1);
            break;
        case eMOD_SIGFOX:
            i2c_data = (uint8_t)(i2c_data & (~(1<<2))) | (1<<2);
            break;
        case eMOD_GPS:
            i2c_data = (uint8_t)(i2c_data & ~1) | 1;
            break;
        default:
            break;
    }

    I2C2_WriteByte (i2C_ADD_GPIO, i2c_data);

    (void) tx_thread_sleep (OSTIME_200MSEC);
}


/******************************************************************************
** Name:    Comm_Hardware_Reset
*****************************************************************************/
/**
** @brief   Reset Wifi, Sigfox or GPS module
**
** @param   none
******************************************************************************/
void Comm_Hardware_Reset(COMM_MOD_e comm_mod)
{
    if((comm_mod != eMOD_WIFI) && (comm_mod != eMOD_SIGFOX) && (comm_mod != eMOD_GPS)) return;

    Comm_Hardware_Off(comm_mod);

    Comm_Hardware_On(comm_mod);
}

/******************************************************************************
 ** Name:    Comm_Select_Uart
 *****************************************************************************/
/**
 ** @brief   Select Uart to send and receive data. This function sets the SN74CBT
 **          Multiplexer/Demultiplexer via the PCF8574 I/O Expander for I2C.
 ** @param   COMM_MOD_e Uart to select
 **
 ** @return ERROR_ID_e error code
 ******************************************************************************/
ERROR_ID_e Comm_Select_Uart(COMM_MOD_e comm_uart)
{
    uint8_t  i2c_data;

    if((comm_uart != eMOD_NONE) && (comm_uart != eMOD_WIFI) && (comm_uart != eMOD_SIGFOX) && (comm_uart != eMOD_GPS)) return eERR_COMM_UART_SELECT;

    // Read PCF8574 previous status
    I2C2_ReadByte (i2C_ADD_GPIO, &i2c_data);

    // Clear UART bits
    i2c_data &= (uint8_t) ~( (1 << 6) | (1 << 7) );

    if(first_time_gps)
    {
        //Disable Enables bits !!!! Do not disable WIFI & GPS until position is found !!!!!
        i2c_data |= (uint8_t) ( (1 << 3)); // | (1 << 4) ); // | (1 << 5)           // 3->Sigfox, 4->WIFI, 5->GPS 
    }
    else
    {
        //Disable Enables bits !!!! Disable GPS because it has found the position !!!!!
        i2c_data |= (uint8_t) ( (1 << 3) /*| (1 << 4)*/ | (1 << 5));
    }

    switch(comm_uart)
    {
        case eMOD_NONE:
            i2c_data |= (uint8_t) ( (1 << 3) | (1 << 4) | (1 << 5));
            comm_selected_uart = eMOD_NONE;
            break;
        case eMOD_WIFI:
            i2c_data = (uint8_t) (i2c_data & (~(1<<7))) | (1<<7);
            i2c_data &= (uint8_t) ~( 1 << 4);
            comm_selected_uart = eMOD_WIFI;
//Trace ((TRACE_NEWLINE),"TRACE WIFI");
            break;
        case eMOD_SIGFOX:
            is_sigfox_free = false;
            i2c_data = (uint8_t) (i2c_data & (~(1<<6))) | (1<<6);
            i2c_data = (uint8_t) (i2c_data & (~(1<<7))) | (1<<7);
            i2c_data &= (uint8_t) ~( 1 << 3);
            comm_selected_uart = eMOD_SIGFOX;
//Trace ((TRACE_NEWLINE),"TRACE SIGFOX");
            break;
        case eMOD_GPS:
            i2c_data = (uint8_t) (i2c_data & (~(1<<6))) | (1<<6);
            i2c_data &= (uint8_t) ~( 1 << 5);
            comm_selected_uart = eMOD_GPS;
//Trace ((TRACE_NEWLINE),"TRACE GPS");
            break;
        default:
//Trace ((TRACE_NEWLINE),"TRACE ERROR");
            return eERR_COMM_UART_SELECT;
            //break;
    }

    I2C2_WriteByte (i2C_ADD_GPIO, i2c_data);

    (void) tx_thread_sleep (OSTIME_100MSEC);

    return eERR_NONE;
}

/******************************************************************************
** Name:    Comm_Get_Selected_Uart
*****************************************************************************/
/**
** @brief   Returns the selected Comms UART
**
** @param   none
**
** @return  Selected Comms Uart
******************************************************************************/
COMM_MOD_e Comm_Get_Selected_Uart (void)
{
    return comm_selected_uart;
}

/******************************************************************************
** Name:    Comm_Is_Sigfox_Free
*****************************************************************************/
/**
** @brief   Returns if Comms UART is free
**
** @param   none
**
** @return  true or false
******************************************************************************/
bool_t Comm_Is_Sigfox_Free (void)
{
    return is_sigfox_free;
}

/******************************************************************************
** Name:    Comm_Modules_Print_Configuration
*****************************************************************************/
/**
** @brief   Prints Module Information
**
 ** @param  COMM_MOD_e module to print info
**
** @return  none
******************************************************************************/
void Comm_Modules_Print_Configuration(COMM_MOD_e comm_mod)
{
    Trace ((TRACE_NEWLINE), " ");
    (void) tx_thread_sleep(OSTIME_100MSEC);
    Trace ((TRACE_TIME_STAMP | TRACE_NEWLINE), "======================================");
    (void) tx_thread_sleep(OSTIME_100MSEC);

    switch(comm_mod)
    {
        case eMOD_WIFI:
            Trace ((TRACE_TIME_STAMP | TRACE_NEWLINE), "          WLAN CONFIGURATION");
            break;
        case eMOD_SIGFOX:
            Trace ((TRACE_TIME_STAMP | TRACE_NEWLINE), "         SIGFOX DEVICE INFO");
            break;
        case eMOD_GPS:
            Trace ((TRACE_TIME_STAMP | TRACE_NEWLINE), "          GPS CONFIGURATION");
            break;
        default:
            break;

    }

    (void) tx_thread_sleep(OSTIME_100MSEC);
    Trace ((TRACE_TIME_STAMP | TRACE_NEWLINE), "======================================");
    (void) tx_thread_sleep(OSTIME_100MSEC);

    switch(comm_mod)
    {
        case eMOD_WIFI:
            Trace ((TRACE_TIME_STAMP), "WLAN SSID:    ");
            (void) tx_thread_sleep(OSTIME_100MSEC);
            Trace_Arg (TRACE_NO_FLAGS | TRACE_NEWLINE, " %s", (uint32_t) R100_wifi.wlan_ssid);
            (void) tx_thread_sleep(OSTIME_100MSEC);
            /*Trace ((TRACE_TIME_STAMP), "WLAN PASSKEY: ");
            (void) tx_thread_sleep(OSTIME_100MSEC);
            Trace_Arg (TRACE_NO_FLAGS | TRACE_NEWLINE, " %s", (uint32_t) R100_wifi.wlan_pass);
            (void) tx_thread_sleep(OSTIME_100MSEC);*/
            Trace ((TRACE_TIME_STAMP), "TYPE OF EAP: ");
            (void) tx_thread_sleep(OSTIME_100MSEC);
            Trace_Arg (TRACE_NO_FLAGS | TRACE_NEWLINE, "  %d", (uint32_t) R100_wifi.wlan_eap_security);
            (void) tx_thread_sleep(OSTIME_100MSEC);
            /*Trace ((TRACE_TIME_STAMP), "IPv4 ADDRESS: ");
            (void) tx_thread_sleep(OSTIME_100MSEC);
            Trace_Arg (TRACE_NO_FLAGS | TRACE_NEWLINE, " %s", (uint32_t) R100_wifi.ip_address);
            (void) tx_thread_sleep(OSTIME_100MSEC);*/
            /*Trace ((TRACE_TIME_STAMP), "MAC ADDRESS:  ");
            (void) tx_thread_sleep(OSTIME_100MSEC);
            Trace_Arg (TRACE_NO_FLAGS | TRACE_NEWLINE, " %s", (uint32_t) R100_wifi.mac_address);
            (void) tx_thread_sleep(OSTIME_100MSEC);*/
            Trace ((TRACE_TIME_STAMP), "HOST NAME:    ");
            (void) tx_thread_sleep(OSTIME_100MSEC);
            Trace_Arg (TRACE_NO_FLAGS | TRACE_NEWLINE, " %s", (uint32_t) R100_wifi.host_name);
            break;
        case eMOD_SIGFOX:
            Trace ((TRACE_TIME_STAMP), "DEVICE ID:      ");
            (void) tx_thread_sleep(OSTIME_100MSEC);
            Trace_Arg (TRACE_NO_FLAGS | TRACE_NEWLINE, " %s", (uint32_t) R100_sigfox.device_id);
            (void) tx_thread_sleep(OSTIME_100MSEC);
            Trace ((TRACE_TIME_STAMP), "PAC ID:         ");
            (void) tx_thread_sleep(OSTIME_100MSEC);
            Trace_Arg (TRACE_NO_FLAGS | TRACE_NEWLINE, " %s", (uint32_t) R100_sigfox.device_pac);
            (void) tx_thread_sleep(OSTIME_100MSEC);
            Trace ((TRACE_TIME_STAMP), "TX FREQ:        ");
            (void) tx_thread_sleep(OSTIME_100MSEC);
            Trace_Arg (TRACE_NO_FLAGS | TRACE_NEWLINE, " %s", (uint32_t) R100_sigfox.device_tx_freq);
            (void) tx_thread_sleep(OSTIME_100MSEC);
            Trace ((TRACE_TIME_STAMP), "RX FREQ:        ");
            (void) tx_thread_sleep(OSTIME_100MSEC);
            Trace_Arg (TRACE_NO_FLAGS | TRACE_NEWLINE, " %s", (uint32_t) R100_sigfox.device_rx_freq);
            break;
        case eMOD_GPS:
            Trace ((TRACE_TIME_STAMP), "DEVICE ID:      ");
            (void) tx_thread_sleep(OSTIME_100MSEC);
            Trace_Arg (TRACE_NO_FLAGS | TRACE_NEWLINE, " %s", (uint32_t) R100_gps.device_id);
            break;
        default:
            break;

    }

    (void) tx_thread_sleep(OSTIME_100MSEC);
    Trace ((TRACE_TIME_STAMP | TRACE_NEWLINE), "======================================");
    (void) tx_thread_sleep(OSTIME_100MSEC);

    // Store Sigfox registration info in a file
    if (comm_mod == eMOD_SIGFOX) DB_Sigfox_Store_Info (R100_sigfox.device_id, R100_sigfox.device_pac);
}

/*************************************************************************************************************
**************************************************************************************************************
                                           WIFI MODULE FUNCTIONS
**************************************************************************************************************
*************************************************************************************************************/

/******************************************************************************
 ** Name:    Comm_Wifi_Initialize_Fast
 *****************************************************************************/
/**
 ** @brief   Fast initialization of Wifi module
 ** @param   void
 **
 ** @return none
 ******************************************************************************/
ERROR_ID_e Comm_Wifi_Initialize_Fast(void)
{
    // Select UART for WIFI
    Comm_Select_Uart(eMOD_WIFI);

    Comm_Uart_Set_Baud_Rate(BR_115200);

    return eERR_NONE;
}

/******************************************************************************
 ** Name:    Wifi_Initialize
 *****************************************************************************/
/**
 ** @brief   Initialization of Wifi module
 ** @param   void
 **
 ** @return ERROR_ID_e error code
 ******************************************************************************/
ERROR_ID_e Comm_Wifi_Initialize(void)
{
    bool_t peap_cert_loaded = false;

    // Select UART for WIFI
    retError = Comm_Select_Uart(eMOD_WIFI);
    if(retError != eERR_NONE) return retError;

    Comm_Hardware_Reset(eMOD_WIFI);

    retError = Comm_Uart_Set_Baud_Rate(BR_115200);

    // Initialize BGLib library for comms with the module
    Wifi_Initialize_BGLib();

/******************************************************************************************
 *  1.- Configure WIFI module
******************************************************************************************/
    retError = Wifi_Set_Operating_Mode();
    if(retError != eERR_NONE) return retError;
    Trace ((TRACE_TIME_STAMP | TRACE_NEWLINE), "Wifi Module Check Comm.... OK");
/*
    // Check module <-> host comms
    retError = Wifi_Check_Module_Status();
    if(retError != eERR_NONE) return retError;

    Trace ((TRACE_TIME_STAMP | TRACE_NEWLINE), "Wifi Module Check Comm.... OK");

    // Check (Configure?) module Uart settings
    retError = Wifi_Configure_Module_Uart();
    if(retError != eERR_NONE) return retError;

    Trace ((TRACE_TIME_STAMP | TRACE_NEWLINE), "Wifi Module Uart Configuration.... OK");
*/
/******************************************************************************************
 * 2. - Get module status
******************************************************************************************/
    //retError = Wifi_Get_Module_Status();
    //if(retError != eERR_NONE) return retError;

    retError = Wifi_Get_Module_MAC_Address();
    if(retError != eERR_NONE) return retError;

//    Trace ((TRACE_TIME_STAMP | TRACE_NEWLINE), "Wifi Module MAC configuration.... OK");

/******************************************************************************************
 * 3. - Get credentials and configuration settings
******************************************************************************************/
    retError = Wifi_Scan_Network();
    if(retError != eERR_NONE) return retError;

    Trace ((TRACE_TIME_STAMP | TRACE_NEWLINE), "Wifi Module Scan Network.... OK");

/******************************************************************************************
 *  4. - Configure TCP/IP Settings
******************************************************************************************/

    if (R100_wifi.wlan_advs == eWIFI_NO)
    {
        retError = Wifi_Configure_TCPIP_DHCP_Settings(DHCP_ON);
        if(retError != eERR_NONE) return retError;

        Trace ((TRACE_TIME_STAMP | TRACE_NEWLINE), "Wifi Module Configure TCP/IP DHCP ON.... OK");
    }

    if (R100_wifi.wlan_advs == eWIFI_ONLY_IP || R100_wifi.wlan_advs == eWIFI_IP_DNS)
    {
        retError = Wifi_Configure_TCPIP_DHCP_Settings(DHCP_OFF);
        if(retError != eERR_NONE) return retError;

        Trace ((TRACE_TIME_STAMP | TRACE_NEWLINE), "Wifi Module Configure TCP/IP DHCP OFF.... OK");
    }
    if (R100_wifi.wlan_advs == eWIFI_ONLY_DNS || R100_wifi.wlan_advs == eWIFI_IP_DNS)
    {
        retError = Wifi_Configure_DNS_Settings();
        if(retError != eERR_NONE) return retError;

        Trace ((TRACE_TIME_STAMP | TRACE_NEWLINE), "Wifi Module Configure DNS.... OK");
    }

    retError = Wifi_Set_Wifi_Radio_On();
    if(retError != eERR_NONE) return retError;

    Trace ((TRACE_TIME_STAMP | TRACE_NEWLINE), "Wifi Module Radio ON.... OK");

/******************************************************************************************
 * 5. - Configure WPA-EAP Certificate
******************************************************************************************/
    // Read certificates configuration
    Wifi_Config_Certs(WIFI_CERT_READ, NONE, NONE, NONE);
    if (R100_wifi_cert.wifi_load_eap == 0 && R100_wifi_cert.wifi_load_tls == 0)
    {
        retError = Wifi_Reset_Certs_Store();
        if(retError != eERR_NONE) return retError;
    }

    if(R100_wifi.wlan_eap_security == eWIFI_EAP_TLS_SEC)
    {
        if(R100_wifi_cert.wifi_load_eap == 0)
        {
            retError = Wifi_Load_EAP_TLS_Cert();
            if(retError == TRY_AGAIN)
            {
                retError = Wifi_Load_EAP_TLS_Cert();
            }
            if(retError != eERR_NONE) return retError;
            // Write EAP certificates configuration
            peap_cert_loaded = true;
        }
        if(R100_wifi_cert.wifi_load_eap == 1)
        {
            retError = Wifi_No_Load_EAP_TLS_Cert(R100_wifi_cert.wifi_fingerprint);
            if(retError != eERR_NONE) return retError;
        }

        Trace ((TRACE_TIME_STAMP | TRACE_NEWLINE), "Wifi Module Configure WPA ENTERPRISE EAP TLS Cert.... OK");
    }
    if(R100_wifi.wlan_eap_security == eWIFI_EAP_PEAP_SEC)
    {
        retError = Wifi_Load_EAP_PEAP_Cert(R100_wifi_cert.wifi_load_eap);
        if(retError != eERR_NONE) return retError;
        // Write EAP certificates configuration
        peap_cert_loaded = true;

        Trace ((TRACE_TIME_STAMP | TRACE_NEWLINE), "Wifi Module Configure WPA ENTERPRISE EAP PEAP MSCHAPv2 Cert.... OK");
    }


/******************************************************************************************
 * 6. - Configure TLS CA cert
******************************************************************************************/
    if(R100_wifi_cert.wifi_load_tls == 0)
    {
        retError = Wifi_Load_Server_TLS_Cacert();
        if(retError == TRY_AGAIN)
        {
            retError = Wifi_Load_Server_TLS_Cacert();
        }
        if(retError != eERR_NONE) return retError;
        // Write certificates configuration
        if(peap_cert_loaded == false)Wifi_Config_Certs(WIFI_CERT_WRITE, NONE, WIFI_TLS_LOADED, NONE);
        if(peap_cert_loaded == true) Wifi_Config_Certs(WIFI_CERT_WRITE, WIFI_EAP_LOADED, WIFI_TLS_LOADED, NONE);
    }

    Trace ((TRACE_TIME_STAMP | TRACE_NEWLINE), "Wifi Module Configure TLS CA Cert.... OK");

/******************************************************************************************
 * 7. - Bring Network Up
******************************************************************************************/
    retError = Wifi_Set_Password(R100_wifi.wlan_eap_security);
    if(retError != eERR_NONE) return retError;

    retError = Wifi_Connect_Network();
    if(retError != eERR_NONE) return retError;

    Trace ((TRACE_TIME_STAMP | TRACE_NEWLINE), "Wifi Module Connect Network.... OK");

/******************************************************************************************
 * 8. - Get IPv4 address
******************************************************************************************/
//    retError = Wifi_Get_IPv4_Address();
//    if(retError != eERR_NONE) return retError;

/******************************************************************************************
 * 9. - Wifi Get RSSI
******************************************************************************************/
    retError = Wifi_Get_RSSI();
    if(retError != eERR_NONE) return retError;

    return eERR_NONE;
}

/******************************************************************************
 ** Name:    Comm_Wifi_Radio_Off
 *****************************************************************************/
/**
 ** @brief   Set Radio Wifi Off
 ** @param   void
 **
 ** @return  ERROR_ID_e error code
 ******************************************************************************/
ERROR_ID_e Comm_Wifi_Radio_Off(void)
{
    retError = Wifi_Set_Wifi_Radio_Off();
    if(retError != eERR_NONE) return retError;

    Trace ((TRACE_TIME_STAMP | TRACE_NEWLINE), "Wifi Module Radio OFF.... OK");

    return eERR_NONE;
}

/******************************************************************************
 ** Name:    Comm_Wifi_Reset
 *****************************************************************************/
/**
 ** @brief   Reset Wifi
 ** @param   void
 **
 ** @return  ERROR_ID_e error code
 ******************************************************************************/
ERROR_ID_e Comm_Wifi_Reset(void)
{
    retError = Wifi_Module_Reset();

    Trace ((TRACE_TIME_STAMP | TRACE_NEWLINE), "Wifi Module Reset.... OK");

    return eERR_NONE;
}

/******************************************************************************
 ** Name:    Comm_Wifi_Is_Host_Server_Alive
 *****************************************************************************/
/**
 ** @brief   Check if host server is alive
 ** @param   void
 **
 ** @return  ERROR_ID_e error code
 ******************************************************************************/
ERROR_ID_e Comm_Wifi_Is_Host_Server_Alive(void)
{
    if(Wifi_Is_Host_Alive() != eERR_NONE) return eERR_COMM_WIFI_SERVER_ALIVE;

    return eERR_NONE;
}

/******************************************************************************
 ** Name:    Comm_Wifi_Send_Frame_Test
 *****************************************************************************/
/**
 ** @brief   Send Test frame using wifi connection
 ** @param   void
 **
 ** @return  ERROR_ID_e error code
 ******************************************************************************/
ERROR_ID_e    Comm_Wifi_Send_Frame_Test(void)
{
    uint8_t wifi_rps;

    //lint -e64 Known type mismatch passing a struct as a byte array
    if(Comm_Is_Wifi_Initialized() == FALSE) 
    {
        if(wifi_test_result.info_type != WIFI_MSG_ID_POWER_ON_TEST) Inc_WifiTestNoConnect(&wifi_test_result);
        return eERR_COMM_WIFI_SEND_FRAME_TEST;
    }

    if(wifi_test_result.info_type != WIFI_MSG_ID_POWER_ON_TEST) Inc_WifiTestConnect(&wifi_test_result);
    wifi_rps = Wifi_Send_Data_Frame((uint8_t *) &wifi_test_result, MAX_WIFI_DATA_TEST_SZ, WIFI_TEST_FRAME);
    if(wifi_rps == eRR_COMM_WIFI_SERVER_RPS_ER) 
    {
        return eRR_COMM_WIFI_SERVER_RPS_ER;
    }
    if(wifi_rps != eERR_NONE) 
    {
        return eERR_COMM_WIFI_SEND_FRAME_TEST;
    }
    return eERR_NONE;
}

/******************************************************************************
 ** Name:    Comm_Wifi_Send_Frame_Alert
 *****************************************************************************/
/**
 ** @brief   Send Alert frame using wifi connection
 ** @param   void
 **
 ** @return  ERROR_ID_e error code
 ******************************************************************************/
ERROR_ID_e    Comm_Wifi_Send_Frame_Alert(void)
{
    //lint -e64 Known type mismatch passing a struct as a byte array
    if(Comm_Is_Wifi_Initialized() == FALSE) 
    {
        if(wifi_func_alert.info_type == WIFI_MSG_ID_COVER_OPEN_ALERT || wifi_func_alert.info_type == WIFI_MSG_ID_COVER_CLOSE_ALERT) Inc_WifiCoverNoConnect();
        return eERR_COMM_WIFI_SEND_FRAME_ALERT;
    }
    if(wifi_func_alert.info_type == WIFI_MSG_ID_COVER_OPEN_ALERT || wifi_func_alert.info_type == WIFI_MSG_ID_COVER_CLOSE_ALERT) Inc_WifiCoverConnect();

    if(Wifi_Send_Data_Frame((uint8_t *) &wifi_func_alert, MAX_WIFI_DATA_SZ, WIFI_ALERT_FRAME) != eERR_NONE) return eERR_COMM_WIFI_SEND_FRAME_ALERT;

    return eERR_NONE;
}

/******************************************************************************
 ** Name:    Comm_Wifi_Send_Frame_Alert_Aux
 *****************************************************************************/
/**
 ** @brief   Send Alert frame using wifi connection
 ** @param   void
 **
 ** @return  ERROR_ID_e error code
 ******************************************************************************/
ERROR_ID_e    Comm_Wifi_Send_Frame_Alert_Aux(void)
{
    //lint -e64 Known type mismatch passing a struct as a byte array
    if(Comm_Is_Wifi_Initialized() == FALSE) 
    {
        if(wifi_func_alert.info_type == WIFI_MSG_ID_COVER_OPEN_ALERT || wifi_func_alert.info_type == WIFI_MSG_ID_COVER_CLOSE_ALERT) Inc_WifiCoverNoConnect();
        return eERR_COMM_WIFI_SEND_FRAME_ALERT;
    }
    if(wifi_func_alert.info_type == WIFI_MSG_ID_COVER_OPEN_ALERT || wifi_func_alert.info_type == WIFI_MSG_ID_COVER_CLOSE_ALERT) Inc_WifiCoverConnect();

    if(Wifi_Send_Data_Frame((uint8_t *) &wifi_func_alert_aux, MAX_WIFI_DATA_SZ, WIFI_ALERT_FRAME) != eERR_NONE) return eERR_COMM_WIFI_SEND_FRAME_ALERT;

    return eERR_NONE;
}

/******************************************************************************
 ** Name:    Comm_Wifi_Send_Frame_Gps
 *****************************************************************************/
/**
 ** @brief   Send Gps position frame using wifi connection
 ** @param   void
 **
 ** @return  ERROR_ID_e error code
 ******************************************************************************/
ERROR_ID_e    Comm_Wifi_Send_Frame_Gps(void)
{
    if(Comm_Is_Wifi_Initialized() == FALSE) return eERR_COMM_WIFI_SEND_FRAME_GPS;
    //lint -e64 Known type mismatch passing a struct as a byte array
    if(Wifi_Send_Data_Frame((uint8_t *) &wifi_gps_position, MAX_WIFI_DATA_SZ, WIFI_GPS_FRAME) != eERR_NONE) return eERR_COMM_WIFI_SEND_FRAME_GPS;

    return eERR_NONE;
}

/******************************************************************************
 ** Name:    Comm_Wifi_Send_Last_Test
 *****************************************************************************/
/**
 ** @brief   Send Last Test File
 ** @param   void
 **
 ** @return  ERROR_ID_e error code
 ******************************************************************************/
ERROR_ID_e    Comm_Wifi_Send_Last_Test(void)
{
    if(Wifi_Send_Test_Result() != eERR_NONE) return eERR_COMM_WIFI_SEND_TEST;

    return eERR_NONE;
}

/******************************************************************************
 ** Name:    Comm_Wifi_Send_Last_Episode
 *****************************************************************************/
/**
 ** @brief   Send Last Episode File
 ** @param   void
 **
 ** @return  ERROR_ID_e error code
 ******************************************************************************/
ERROR_ID_e    Comm_Wifi_Send_Last_Episode(void)
{
    if(Wifi_Send_Episode_Result() != eERR_NONE) return eERR_COMM_WIFI_SEND_EPISODE;

    return eERR_NONE;
}

/******************************************************************************
 ** Name:    Comm_Wifi_Get_Upgrade_Firmware
 *****************************************************************************/
/**
 ** @brief   Get upgrade firmware from host server
 ** @param   void
 **
 ** @return  ERROR_ID_e error code
 ******************************************************************************/
ERROR_ID_e    Comm_Wifi_Get_Upgrade_Firmware(void)
{
    uint8_t err;
    bool is_verified= false;

    if(Wifi_Receive_Firmware_Result() != eERR_NONE)
    {
        return eERR_COMM_WIFI_RECEIVE_UPGRADE_FILE;
    }

    if(err = Crypto_ATECC_Verify(&is_verified), err != eERR_NONE)  // Verify Firmware
    {
        Wifi_Delete_Firmware();
        return eERR_COMM_WIFI_RECEIVE_UPGRADE_FILE;
    }
    if(is_verified == false)
    {
        Wifi_Delete_Firmware();
        return eERR_COMM_WIFI_RECEIVE_UPGRADE_FILE;
    }

    return eERR_NONE;
}

/******************************************************************************
 ** Name:    Comm_Wifi_Get_Configuration_File
 *****************************************************************************/
/**
 ** @brief   Get configuration file from host server
 ** @param   void
 **
 ** @return  ERROR_ID_e error code
 ******************************************************************************/
ERROR_ID_e    Comm_Wifi_Get_Configuration_File(void)
{
    if(Wifi_Receive_Configuration_Result() != eERR_NONE) return eERR_COMM_WIFI_RECEIVE_CONFIG_FILE;

    return eERR_NONE;
}

/******************************************************************************
 ** Name:    Comm_Wifi_Execute_Test
 *****************************************************************************/
/**
 ** @brief   Test of Wifi module
 ** @param   void
 **
 ** @return ERROR_ID_e error code
 ******************************************************************************/
ERROR_ID_e Comm_Wifi_Execute_Test(void)
{
    // Select UART for WIFI
    retError = Comm_Select_Uart(eMOD_WIFI);
    if(retError != eERR_NONE) return retError;

    Comm_Hardware_Reset(eMOD_WIFI);

    retError = Comm_Uart_Set_Baud_Rate(BR_115200);

    // Initialize BGLib library for comms with the module
    Wifi_Initialize_BGLib();

    // Check module <-> host comms
    retError = Wifi_Check_Module_Status();
    if(retError != eERR_NONE) return retError;

    return eERR_NONE;
}

/******************************************************************************
** Name:    Wifi_Is_MAC_Valid
*****************************************************************************/
/**
** @brief   Checks if a MAC address has a valid format
**
** @param   s   pointer of the MAC address string
**
** @return  bool_t  true if MAC format OK, false if not
******************************************************************************/
bool_t Comm_Wifi_Is_MAC_Valid(char_t *s)
{
    uint8_t i = 0;

    for(i = 0; i < 17; i++)
    {
        if (((i % 3) != 2) && (!isxdigit((uint8_t) s[i])))
        {
            return FALSE;
        }
        if(i % 3 == 2 && s[i] != ':')
        {
            return FALSE;
        }
    }
    if(s[17] != '\0')
    {
        return FALSE;
    }

    return TRUE;
}

/******************************************************************************
** Name:    Wifi_Is_IPv4_Valid
*****************************************************************************/
/**
** @brief   Checks if a IPv4 address has a valid format
**
** @param   s   pointer of the IPv4 address string
**
** @return  bool_t  true if IPv4 format OK, false if not
******************************************************************************/
bool_t Comm_Wifi_Is_IPv4_Valid(char_t *s)
{
    static uint8_t i = 0;
    static uint8_t j = 0;
    static uint8_t numDots = 0;
    static char_t number[4] = {0000};
    static uint8_t len = 0;

    len = (uint8_t) strlen(s);

    if (!isdigit((uint8_t) s[0])) return FALSE;

    if (len < 7 || len > 15) return FALSE;

    number[3] = 0;

    for (i = 0 ; i< len; i++)
    {
        if (isdigit((uint8_t) s[i]))
        {
            number[j] = s[i];
            j++;
            if (j>3)
            {
                return FALSE;
            }
        }
        else if (s[i] == '.')
        {
            if (atoi(number) > 255)
            {
                return FALSE;
            }
            memset(number, '0', 3);
            number[3] = 0;
            j = 0;
            numDots++;
            if(numDots > 3)
            {
                return FALSE;
            }
        }
    }

    if (numDots == 3)
    {
        return TRUE;
    }
    else
    {
        return FALSE;
    }

}

void  Comm_Wifi_Save_Host_Response(char_t *resp)
{
    memset(&host_response, 0, sizeof(host_response));

    host_response.must_be_0xAA = (uint8_t) resp[0];

    host_response.timestamp = ( ((uint32_t)resp[1] & 0xFF) << 24) |
                              ( ((uint32_t)resp[2] & 0xFF) << 16) |
                              ( ((uint32_t)resp[3] & 0xFF) <<  8) |
                              ( ((uint32_t)resp[4] & 0xFF) <<  0) ;

    host_response.cfg_available = (uint8_t) resp[5];

    host_response.frmw_available = (uint8_t) resp[6];

    host_response.must_be_0xBB = (uint8_t) resp[7];
}

uint8_t  Comm_Wifi_Is_Cfg_Available(void)
{
   return host_response.cfg_available;
}

uint8_t  Comm_Wifi_Is_Frmw_Available(void)
{
    return host_response.frmw_available;
}

void Comm_Wifi_Save_Server_Response(char_t *resp)
{
    memset(&server_response, 0, sizeof(server_response));

    for(int i=0; i<4; i++)
    {
        server_response.version |= ( ((uint32_t)(resp[3-i]) & 0xFF) <<  i*8);
    }

    /* server_response.version = ( ((uint32_t)resp[0] & 0xFF) << 24) |
                              ( ((uint32_t)resp[1] & 0xFF) << 16) |
                              ( ((uint32_t)resp[2] & 0xFF) <<  8) |
                              ( ((uint32_t)resp[3] & 0xFF) <<  0) ; */

    for(int i=0; i<16; i++)
    {
        server_response.s_n[i] = (char_t)( ((char_t)resp[4+i] & 0xFF));
    }

    for(int i=0; i<64; i++)
    {
        server_response.pass[i] = (char_t)( ((char_t)resp[21+i] & 0xFF));
    }

    for(int i=0; i<8; i++)
    {
        server_response.file_size |= ( ((uint64_t)resp[92-i] & 0xFF) <<  i*8);
    }

    /* server_response.file_size = ( ((uint64_t)resp[53] & 0xFF) << 56) |
                                ( ((uint64_t)resp[54] & 0xFF) << 48) |
                                ( ((uint64_t)resp[55] & 0xFF) << 40) |
                                ( ((uint64_t)resp[56] & 0xFF) << 32) |
                                ( ((uint64_t)resp[57] & 0xFF) << 24) |
                                ( ((uint64_t)resp[58] & 0xFF) << 16) |
                                ( ((uint64_t)resp[59] & 0xFF) <<  8) |
                                ( ((uint64_t)resp[60] & 0xFF) <<  0) ; */

    for(int i=0; i<2; i++)
    {
        server_response.crc_file |= (uint16_t)( ((uint16_t)resp[94-i] & 0xFF) <<  i*8);
    }

    /* server_response.crc_file = (uint16_t)( ( ((uint16_t)resp[61] & 0xFF) <<  8) |
                                           ( ((uint16_t)resp[62] & 0xFF) <<  0) ); */
}

void Comm_Wifi_PASS(uint8_t *password)
{
    memcpy(password, (uint8_t *)&server_response.pass, 64);
}

uint64_t Comm_Wifi_SZ(void)
{
    return server_response.file_size;
}

uint16_t Comm_Wifi_CRC(void)
{
    return server_response.crc_file;
}

/*************************************************************************************************************
**************************************************************************************************************
                                           SIGFOX MODULE FUNCTIONS
**************************************************************************************************************
*************************************************************************************************************/


/******************************************************************************
 ** Name:    Comm_Sigfox_Initialize
 *****************************************************************************/
/**
 ** @brief   Initialization of Sigfox module
 ** @param   void
 **
 ** @return ERROR_ID_e error code
 ******************************************************************************/
ERROR_ID_e Comm_Sigfox_Initialize(void)
{
    static bool_t first_time = FALSE;
    is_sigfox_free = false;

    memset(&R100_sigfox,0,sizeof(sigfox_config_t));

    retError = Comm_Select_Uart(eMOD_SIGFOX);
    if(retError != eERR_NONE)
    {
        is_sigfox_free = true;
        return retError;
    }

    Comm_Hardware_Reset(eMOD_SIGFOX);

    retError = Comm_Uart_Set_Baud_Rate(BR_9600);
    if(retError != eERR_NONE)
    {
        is_sigfox_free = true;
        return retError;
    }

    retError = Sigfox_Check_Module_Status();
    if(retError != eERR_NONE)
    {
        is_sigfox_free = true;
        return retError;
    }

    if(first_time == FALSE)
    {
        retError = Sigfox_Get_Module_DeviceID();
        if(retError != eERR_NONE)
        {
            is_sigfox_free = true;
            return retError;
        }

        retError = Sigfox_Get_Module_PAC();
        if(retError != eERR_NONE)
        {
            is_sigfox_free = true;
            return retError;
        }

        retError = Sigfox_Get_TX_Frequency();
        if(retError != eERR_NONE)
        {
            is_sigfox_free = true;
            return retError;
        }

        retError = Sigfox_Get_RX_Frequency();
        if(retError != eERR_NONE)
        {
            is_sigfox_free = true;
            return retError;
        }

        /*retError = Sigfox_Get_Module_Temp();
        if(retError != eERR_NONE)
        {
            is_sigfox_free = true;
            return retError;
        }

        retError = Sigfox_Get_Module_Voltages();
        if(retError != eERR_NONE)
        {
            is_sigfox_free = true;
            return retError;
        }*/

        first_time = TRUE;
    }
    is_sigfox_free = true;
    return eERR_NONE;
}

/******************************************************************************
 ** Name:    Comm_Sigfox_Execute_Test
 *****************************************************************************/
/**
 ** @brief   Test of Sigfox module
 ** @param   void
 **
 ** @return ERROR_ID_e error code
 ******************************************************************************/
ERROR_ID_e Comm_Sigfox_Execute_Test(void)
{
    retError = Comm_Select_Uart(eMOD_SIGFOX);
    if(retError != eERR_NONE)
    {
        is_sigfox_free = true;
        return retError;
    }

    Comm_Hardware_Reset(eMOD_SIGFOX);

    retError = Comm_Uart_Set_Baud_Rate(BR_9600);
    if(retError != eERR_NONE)
    {
        is_sigfox_free = true;
        return retError;
    }

    retError = Sigfox_Check_Module_Status();
    if(retError != eERR_NONE)
    {
        is_sigfox_free = true;
        return retError;
    }

    is_sigfox_free = true;
    return eERR_NONE;
}

/******************************************************************************
 ** Name:    Comm_Sigfox_Send_Test
 *****************************************************************************/
/**
 ** @brief   Send R100 Test
 ** @param   void
 **
 ** @return  ERROR_ID_e error code
 ******************************************************************************/
ERROR_ID_e Comm_Sigfox_Send_Test(void)
{
    is_sigfox_free = false;
    //lint -e64 Known type mismatch passing a struct as a byte array
    retError = Sigfox_Send_Data_Frame((uint8_t *) &sigfox_test_result);
    if(retError != eERR_NONE)
    {
        is_sigfox_free = true;
        return retError;
    }

    is_sigfox_free = true;
    return eERR_NONE;
}

/******************************************************************************
 ** Name:    Comm_Sigfox_Send_Alert
 *****************************************************************************/
/**
 ** @brief   Send R100 Functional Alert
 ** @param   void
 **
 ** @return  ERROR_ID_e error code
 ******************************************************************************/
ERROR_ID_e Comm_Sigfox_Send_Alert(void)
{
    /*bool_t donwlink = false;
    if((sigfox_func_alert.info_type == MSG_ID_NO_SHOCK_ALERT || sigfox_func_alert.info_type == MSG_ID_SHOCK_DISC_ALERT 
        || sigfox_func_alert.info_type == MSG_ID_SHOCK_TIMEOUT_ALERT || sigfox_func_alert.info_type == MSG_ID_SHOCK_DONE_ALERT) && donwlink_fisrt_msg)
    {
        donwlink = true;
        donwlink_fisrt_msg = false;
    }
    else
    {
        donwlink = false;
    }*/
    is_sigfox_free = false;
    //lint -e64 Known type mismatch passing a struct as a byte array
    retError = Sigfox_Send_Data_Frame((uint8_t *) &sigfox_func_alert);
    if(retError != eERR_NONE)
    {
        retError = Sigfox_Send_Data_Frame((uint8_t *) &sigfox_func_alert);
        if(retError != eERR_NONE)
        {
            is_sigfox_free = true;
            return retError;
        }
    }

    is_sigfox_free = true;
    return eERR_NONE;
}

/******************************************************************************
 ** Name:    Comm_Sigfox_Send_Exp
 *****************************************************************************/
/**
 ** @brief   Send battery and electrodes expiration date
 ** @param   void
 **
 ** @return  ERROR_ID_e error code
 ******************************************************************************/
ERROR_ID_e Comm_Sigfox_Send_Exp(void)
{
    is_sigfox_free = false;
    //lint -e64 Known type mismatch passing a struct as a byte array
    retError = Sigfox_Send_Data_Frame((uint8_t *) &sigfox_expiration);
    if(retError != eERR_NONE)
    {
        is_sigfox_free = true;
        return retError;
    }

    is_sigfox_free = true;
    return eERR_NONE;
}

/******************************************************************************
 ** Name:    Comm_Sigfox_Send_Position
 *****************************************************************************/
/**
 ** @brief   Send R100 GPS Position
 ** @param   void
 **
 ** @return  ERROR_ID_e error code
 ******************************************************************************/
ERROR_ID_e Comm_Sigfox_Send_Position(void)
{
    is_sigfox_free = false;
    //lint -e64 Known type mismatch passing a struct as a byte array
    retError = Sigfox_Send_Data_Frame((uint8_t *) &sigfox_gps_position);
    if(retError != eERR_NONE)
    {
        is_sigfox_free = true;
        return retError;
    }

    is_sigfox_free = true;
    return eERR_NONE;
}

/******************************************************************************
 ** Name:    Comm_Sigfox_Start_Uplink_Test
 *****************************************************************************/
/**
 ** @brief   Perform sigfox module Uplink Test
 ** @param   void
 **
 ** @return ERROR_ID_e error code
 ******************************************************************************/
ERROR_ID_e Comm_Sigfox_Start_Uplink_Test(void)
{
    retError = Comm_Sigfox_Device_Info_Check();
    if(retError != eERR_NONE)
    {
        is_sigfox_free = true;
        return retError;
    }

    Comm_Modules_Print_Configuration(eMOD_SIGFOX);

    retError = Sigfox_Start_CW_Test();
    if(retError != eERR_NONE)
    {
        is_sigfox_free = true;
        return retError;
    }

    Trace ((TRACE_TIME_STAMP | TRACE_NEWLINE), "Sigfox Uplink Test Started....");

    is_sigfox_free = true;
    return eERR_NONE;
}

/******************************************************************************
 ** Name:    Comm_Sigfox_Stop_Uplink_Test
 *****************************************************************************/
/**
 ** @brief   Stop sigfox module Uplink Test
 ** @param   void
 **
 ** @return ERROR_ID_e error code
 ******************************************************************************/
ERROR_ID_e Comm_Sigfox_Stop_Uplink_Test(void)
{
    retError = Sigfox_Stop_CW_Test();
    if(retError != eERR_NONE)
    {
        is_sigfox_free = true;
        return retError;
    }

    Trace ((TRACE_TIME_STAMP | TRACE_NEWLINE), "Sigfox Uplink Test Stopped....");
    is_sigfox_free = true;
    return eERR_NONE;
}

/******************************************************************************
 ** Name:    Comm_Sigfox_Start_Downlink_Test
 *****************************************************************************/
/**
 ** @brief   Start sigfox module Downlink Test (with RX GFSK test mode)
 ** @param   void
 **
 ** @return ERROR_ID_e error code
 ******************************************************************************/
ERROR_ID_e Comm_Sigfox_Start_Downlink_Test(void)
{
    retError = Sigfox_Start_RX_GFSK_Test();
    if(retError != eERR_NONE)
    {
        is_sigfox_free = true;
        return retError;
    }
    Trace ((TRACE_TIME_STAMP | TRACE_NEWLINE), "Sigfox Downlink Test Started....");
    is_sigfox_free = true;
    return eERR_NONE;
}

/******************************************************************************
 ** Name:    Comm_Sigfox_Device_Info_Check
 *****************************************************************************/
/**
 ** @brief   Device information Check
 ** @param   void
 **
 ** @return ERROR_ID_e error code
 ******************************************************************************/
ERROR_ID_e Comm_Sigfox_Device_Info_Check(void)
{
/******************************************************************************************/
    // 0.-  SIGFOX Check Module Communication
    retError = Sigfox_Check_Module_Status();
    if(retError != eERR_NONE)
    {
        is_sigfox_free = true;
        return retError;
    }
/******************************************************************************************/
    // 1.-  SIGFOX ID Check
    retError = Sigfox_Get_Module_DeviceID();
    if(retError != eERR_NONE)
    {
        is_sigfox_free = true;
        return retError;
    }
/******************************************************************************************/
    // 2.-  SIGFOX PAC Code Check
    retError = Sigfox_Get_Module_PAC();
    if(retError != eERR_NONE)
    {
        is_sigfox_free = true;
        return retError;
    }
/******************************************************************************************/
    // 3.-  SIGFOX RF Frequency Setup Check
    retError = Sigfox_Get_TX_Frequency();
    if(retError != eERR_NONE)
    {
        is_sigfox_free = true;
        return retError;
    }

    retError = Sigfox_Get_RX_Frequency();
    if(retError != eERR_NONE)
    {
        is_sigfox_free = true;
        return retError;
    }

    //Compare Sigfox TX & RX FREQ are the same as expected
    retError = Sigfox_Compare_Test_Frequencies();
    if(retError != eERR_NONE)
    {
        is_sigfox_free = true;
        return retError;
    }

/******************************************************************************************/
    is_sigfox_free = true;
    return eERR_NONE;
}

/*************************************************************************************************************
**************************************************************************************************************
                                           GPS MODULE FUNCTIONS
**************************************************************************************************************
*************************************************************************************************************/

/******************************************************************************
 ** Name:    Comm_GPS_Initialize
 *****************************************************************************/
/**
 ** @brief   Initialization of GPS module
 ** @param   void
 **
 ** @return ERROR_ID_e error code
 ******************************************************************************/
ERROR_ID_e Comm_GPS_Initialize(void)
{
    memset(&R100_gps,0,sizeof(R100_gps));

    retError = Comm_Select_Uart(eMOD_GPS);
    if(retError != eERR_NONE) return retError;

    //Comm_Hardware_Reset(eMOD_GPS);

    retError = Comm_Uart_Set_Baud_Rate(BR_9600);
    if(retError != eERR_NONE) return retError;

    return eERR_NONE;
}

/******************************************************************************
 ** Name:    Comm_GPS_Send_Command
 *****************************************************************************/
/**
 ** @brief   Send GPS commands
 ** @param   void
 **
 ** @return ERROR_ID_e error code
 ******************************************************************************/
ERROR_ID_e Comm_GPS_Send_Command(void)
{
    if(Comm_Is_Sigfox_Free() && Comm_Is_Wifi_Free())
    {
        if (Comm_Get_Selected_Uart() != eMOD_GPS)
        {
            retError = Comm_Select_Uart(eMOD_GPS);
            if(retError != eERR_NONE) return retError;

            retError = Comm_Uart_Set_Baud_Rate(BR_9600);
            if(retError != eERR_NONE) return retError;
        }

        // In addition to these commands it is possible to load an EPO file with satellite predictions for a faster TTFF
        Comm_Uart_Send("$PMTK286,1*23");        // AIC - Active Interference Cancellation
        Comm_Uart_Send("$PMTK313,1*2E");        // Set SBAS enabled. Signal correction
        Comm_Uart_Send("$PMTK301,2*2E");        // Set DGPS mode. Differential GPS
        Comm_Uart_Send("$PMTK220,1000*1F");     // Set possition fix every 1 seconds. Update satellites registers
        Comm_Uart_Send("$PMTK353,1,1,0,0,0*2B"); // GPS + GLONASS
        //Comm_Uart_Send("$PMTK869,1,1*35");  // EASY default enabled
    }
    else return eERR_COMM_GPS;

    return eERR_NONE;
}

/******************************************************************************
 ** Name:    Comm_GPS_Execute_Test
 *****************************************************************************/
/**
 ** @brief   Test of GPS module
 ** @param   void
 **
 ** @return ERROR_ID_e error code
 ******************************************************************************/
ERROR_ID_e Comm_GPS_Execute_Test(void)
{
    retError = Comm_Select_Uart(eMOD_GPS);
    if(retError != eERR_NONE) return retError;

    Comm_Hardware_Reset(eMOD_GPS);

    retError = Comm_Uart_Set_Baud_Rate(BR_9600);
    if(retError != eERR_NONE) return retError;

    // Wait a little time...
    (void) tx_thread_sleep(OSTIME_3SEC);

    if(GPS_Is_Test_Package_Received() == FALSE)
    {
        return eERR_COMM_GPS;
    }

    return eERR_NONE;
}

/******************************************************************************
 ** Name:    Comm_GPS_Get_Position_Data
 *****************************************************************************/
/**
 ** @brief   Get GPS Data
 ** @param   void
 **
 ** @return ERROR_ID_e error code
 ******************************************************************************/
ERROR_ID_e    Comm_GPS_Get_Position_Data(void)
{
    if(Gps_Is_New_Position_Available() == TRUE)
    {
        // Select UART for GPS
        retError = Comm_Select_Uart(eMOD_GPS);
        if(retError != eERR_NONE) return retError;

        retError = Comm_Uart_Set_Baud_Rate(BR_9600);
        if(retError != eERR_NONE) return retError;

        retError = Gps_Get_Lat_Long_Data();
        if(retError != eERR_NONE) return retError;

        // Proccessed new position
        Gps_Set_New_Position_Available(FALSE);
    }

    Trace ((TRACE_TIME_STAMP), "GPS ==> ");
    (void) tx_thread_sleep(OSTIME_100MSEC);
    Trace_Arg (TRACE_NO_FLAGS, " LAT:  %s  ", (uint32_t) R100_gps.lat_data);
    (void) tx_thread_sleep(OSTIME_100MSEC);
    Trace_Arg (TRACE_NEWLINE, " LNG:  %s  ", (uint32_t) R100_gps.long_data);
    (void) tx_thread_sleep(OSTIME_100MSEC);
    return eERR_NONE;
}

/*************************************************************************************************************
**************************************************************************************************************
                                        ACCELEROMETER MODULE FUNCTIONS
**************************************************************************************************************
*************************************************************************************************************/

/******************************************************************************
 ** Name:    Comm_ACC_Execute_Test
 *****************************************************************************/
/**
 ** @brief   Test of ACC module
 ** @param   void
 **
 ** @return ERROR_ID_e error code
 ******************************************************************************/
ERROR_ID_e Comm_ACC_Execute_Test(void)
{
    if(retError = Accelerometer_Presence(), retError != eERR_NONE)
    {
        return eERR_COMM_ACC_PRESENCE;
    }
    return eERR_NONE;
}

/**
 * @brief If accelerometer presence is OK, then initialize Accelerometer setup
 *
 * @param void
 *
 * @return eERR_NONE
 * @return eERR_COMM_ACC_READ_PRESENCE
 * @return retError
 */
ERROR_ID_e Comm_ACC_Initialize(void)
{

    if(retError = Accelerometer_Presence(), retError != eERR_NONE)
    {
        if(retError = Accelerometer_Presence(), retError != eERR_NONE)
        {
            return retError;
        }
    }

    if(retError = Accelerometer_Setup(ACC_ENABLE_XYZ, ACC_SAMPLE_RATE_25, ACC_2G_ACCEL_RANGE), retError != eERR_NONE) // rate = 25 Hz Hz and scale = 2g
    {
        return retError;
    }
    /* if(retError = Accel_Interrupt_Init(ACC_DISABLE_INT, 0, 0, 0), retError != eERR_NONE) // thresold = 50 and duration = 50 sec
    {
        return retError;
    } */

    R100_acc.device_id[0] = 'A';
    R100_acc.device_id[1] = 'C';
    R100_acc.device_id[2] = 'C';
    R100_acc.device_id[3] = '\0';

    R100_acc.xyz_config = ((ACC_SAMPLE_RATE_25 << 4) | ACC_2G_ACCEL_RANGE);
    R100_acc.hp_filter = 0;

    return eERR_NONE;
}

/**
 * @brief Read XYZ axis acceleration values
 *
 * @param void
 *
 * @return eERR_NONE
 */
ERROR_ID_e Comm_ACC_Get_Acceleration_Data(void)
{
    int i = 0;
    uint8_t acc_data[6] = {0};
    int16_t symbol[3] = {0};
    //uint8_t error = 0;
    //int16_t aux_x=0,aux_y=0,aux_z=0;

    /* for(i=0; i<6; i++)
    { */
        //error = ACC_Get_Axis_Data(0x28+i, &acc_data[i]);
        I2C1_ReadRegister_ACC(0x28, &acc_data[0]);
        I2C1_ReadRegister_ACC(0x29, &acc_data[1]);

        I2C1_ReadRegister_ACC(0x2A, &acc_data[2]);
        I2C1_ReadRegister_ACC(0x2B, &acc_data[3]);

        I2C1_ReadRegister_ACC(0x2C, &acc_data[4]);
        I2C1_ReadRegister_ACC(0x2D, &acc_data[5]);
    //}

    // Format the data to add the sign
    for(i=0;i<3;i++)
    {
        if(acc_data[2*i+1] & 0x80)
        {
            symbol[i] = (int16_t)0xF000;
        }
        else symbol[i] = 0x0000;
    }

    /*aux_x = (int16_t) ( (acc_data[1] << 8) | acc_data[0] );
    aux_y = (int16_t) ( (acc_data[3] << 8) | acc_data[2] );
    aux_z = (int16_t) ( (acc_data[5] << 8) | acc_data[4] );

    // convert to m/s2
    aux_x = (aux_x * 10) / 16384;
    aux_y = (aux_y * 10) / 16384;
    aux_z = (aux_z * 10) / 16384;

    R100_acc.x_data = (int16_t) aux_x;
    R100_acc.y_data = (int16_t) aux_y;
    R100_acc.z_data = (int16_t) aux_z; */

    R100_acc.x_data = (int16_t)((( ( (int16_t)acc_data[1] ) << 8 ) + (int16_t)acc_data[0] ) >> 4);
    R100_acc.y_data = (int16_t)((( ( (int16_t)acc_data[3] ) << 8 ) + (int16_t)acc_data[2] ) >> 4);
    R100_acc.z_data = (int16_t)((( ( (int16_t)acc_data[5] ) << 8 ) + (int16_t)acc_data[4] ) >> 4);

    R100_acc.x_data = (int16_t)(symbol[0] | R100_acc.x_data);
    R100_acc.y_data = (int16_t)(symbol[1] | R100_acc.y_data);
    R100_acc.z_data = (int16_t)(symbol[2] | R100_acc.z_data);

    // Convert data acceleration to mg
    R100_acc.x_data = (int16_t)(R100_acc.x_data * 2000 / 32768);
    R100_acc.y_data = (int16_t)(R100_acc.y_data * 2000 / 32768);
    R100_acc.z_data = (int16_t)(R100_acc.z_data * 2000 / 32768);

    /* Trace     (TRACE_NEWLINE, "  ");
    Trace_Arg (TRACE_NO_FLAGS, "  X data : %d", (uint32_t) R100_acc.x_data);
    Trace_Arg (TRACE_NO_FLAGS, "  Y data : %d", (uint32_t) R100_acc.y_data);
    Trace_Arg (TRACE_NEWLINE, "  Z data : %d", (uint32_t) R100_acc.z_data);*/

    return eERR_NONE;
}

/**
 * @brief Read INT2 register to check if interrupt is activated or not
 *
 * @param void
 *
 * @return If interrupt is enabled or not(0=disable 1=enable)
 * @return eERR_COMM_ACC_INT2
 */
uint8_t Comm_ACC_Get_INT2(void)
{
    uint8_t read_int2 = 0;
    //uint8_t error = 0;

    ACC_Get_Axis_Data(0x35, &read_int2);
    return (read_int2 & 0x40);
    /*if(error == 0)
    {
        //R100_acc.interrupt = (read_int2 & 0x40) >> 6;
        return eERR_NONE;
    }
    return eERR_COMM_ACC_INT2;*/
}
