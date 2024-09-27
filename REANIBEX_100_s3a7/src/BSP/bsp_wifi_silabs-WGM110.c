/******************************************************************************
 * Name      : S3A7_REANIBEX_100                                              *
 * Copyright : Osatu S. Coop                                                  *
 * Compiler  : MinGW32                                                        *
 * Target    : Reanibex Series                                                *
 ******************************************************************************/

/*!
 * @file        bsp_wifi_silabs-WGM110.c
 * @brief       BSP for Silicon Labs WGM110 module
 *
 * @version     v1
 * @date        10/10/2019
 * @author      jarribas
 * @warning     author   date    version     .- Edition
 * @bug
 *
 */

/******************************************************************************
 ** Includes
 */
#include "BSP/bsp_wifi_silabs-WGM110.h"
#include "HAL/thread_comm_hal.h"
#include "Comm.h"
#include "device_init.h"
#include "hal_data.h"
#include "BSP/bglib_api/wifi_bglib.h"
#include "Trace.h"
#include "DB_Episode.h"
#include "SCEF/sha2.h"
#include "SCEF/aes.h"

/******************************************************************************
 ** Macros
 */
/**
 * Define BGLIB library
 */
//lint -e19 BGLIB api macro declaration
BGLIB_DEFINE();

/******************************************************************************
 ** Defines
 */
#define MAX_WIFI_CMD_LENGTH  256
#define MAX_WIFI_DATA_LENGTH 256

#define BGLIB_RX_BYTES(x,y) (BGLIB_MSG_HEADER_LEN+x+y)

#define TEST_FOLDER_NAME      "TEST"             ///< Folder name for Test
#define EPI_FOLDER_NAME       "EPISODES"         ///< Folder name for Episodes

#define SERVER_RESP_OK           "OK"
#define SERVER_RESP_ER           "ER"

#define TEST_FILE_SZ          32768              ///< File Size allocated memory (32KB)

#define WIFI_EAP_POS            4                 ///< EAP position
#define WIFI_SSID_POS           12                ///< SSID position (WIFI_EAP_POS plus 7 because EAP is always same)
#define WIFI_PASS_POS           6                 ///< Password position
#define WIFI_KEY_PASS_POS       10                ///< Key password position
#define WIFI_EAP_CNF_POS        10                ///< EAP configuration password position
#define WIFI_EAP_PASS_POS       11                ///< EAP password position
#define WIFI_ADVS_POS           7                 ///< Advenced settings position
#define WIFI_IP_POS             6                 ///< IP position
#define WIFI_MASK_POS           7                 ///< Subnet mask position
#define WIFI_GATEWAY_POS        10                ///< Gateway position
#define WIFI_DNSP_POS           7                 ///< Primary DNS position
#define WIFI_DNSS_POS           7                 ///< Secondary DNS position





#define MAX_WIFI_DATA_TX_SZ    255                  ///< Max packet size for Wifi transfer
//#define MAX_WIFI_DATA_TX_SZ   1024                ///< Max packet size for Wifi transfer
#define MAX_WIFI_CERT_SZ        1024                ///< Max size of certificale file

#define WIFI_SWAP_UINT16(x)              (x = (uint16_t) ( ((x>>8)&0x00FF) | ((x<<8)&0xFF00) ))
#define WIFI_SWAP_UINT32(x)              (x = (uint32_t) ( ((x>>24)&0x000000FF) | ((x>>8)&0x0000FF00) | ((x<<8)&0x00FF0000) | ((x<<24)&0xFF000000) ))
#define WIFI_SWAP_UINT64(x)              (x = (uint64_t) ( ((x>>56)&0x00000000000000FF) | ((x>>40)&0x000000000000FF00) | ((x>>24)&0x0000000000FF0000) | ((x>>8)&0x00000000FF000000) | ((x<<8)&0x000000FF00000000) | ((x<<24)&0x0000FF0000000000) | ((x<<40)&0x00FF000000000000) | ((x<<56)&0xFF00000000000000)))

/******************************************************************************
 ** Typedefs
 */

/******************************************************************************
 ** Constants
 */

/******************************************************************************
 ** Externals
 */
extern wifi_config_t R100_wifi;
extern WIFI_HEADER_t wifi_header;

/******************************************************************************
 ** Globals
 */
       WIFI_CERT_t R100_wifi_cert;
static WIFI_PAYLOAD_t wifi_payload;

static char_t endpoint_data[MAX_WIFI_DATA_TX_SZ];

/** Pointer to wifi packet*/
struct wifi_cmd_packet* pck;
struct wifi_cmd_packet* pck_resp;
struct wifi_cmd_packet* pck_evt[4];

/** TCP endpoint. */
static uint8_t tcp_endpoint;
/** UDP endpoint. */
static uint8_t udp_client_endpoint;
static uint8_t udp_server_endpoint;
/** The destination IP address */
static uint32_t dest_ip = 0;
/** The destination TCP port. */
static uint16_t dest_port = 0;
/** The local UDP port. */
static uint16_t udp_local_port = 0;
//static FX_FILE  test_file;
//static FX_FILE  upgrade_file;
//static FX_FILE  config_file;

static FX_FILE  wifi_file;

uint8_t      access_key[12] = {0};    // USB asscess key
/** File name. */
//static char_t  fname[32];

/** Buffer for storing data from the serial port. */
//static char wifi_buffer[BGLIB_MSG_MAXLEN];

/******************************************************************************
 ** Locals
 */

/******************************************************************************
 ** Prototypes
 */
static void   Wifi_Send_Command              (uint8 msg_len, uint8* msg_data, uint16 data_len, uint8* data);
static bool_t Wifi_Command_Response          (uint32_t ack, uint8_t length, uint32_t tout);
static bool_t Wifi_Command_Response_Data     (uint32_t ack, uint32_t tout);
static bool_t Wifi_Command_Response_CFG_FW   (uint32_t ack, uint8_t length, uint32_t tout, bool_t wait_length);

static void   Wifi_Save_MAC_Address          (void);
//static void   Wifi_Save_IPv4_Address         (void);

static void Wifi_Fill_Header(uint8_t id, uint64_t file_size, uint16_t crc_16);

/******************************************************************************
** Name:    Get_Checksum_Add
*****************************************************************************/
/**
** @brief   Calculates the data array checksum (addition)
**
** @param   pBuffer pointer to the array to process
** @param   size    number of bytes to process
**
** @return  checksum result
******************************************************************************/
static uint16_t Get_Checksum_Add_16 (uint8_t *pBuffer, uint32_t size, uint16_t crc)
{
    uint32_t    i;          // global use index
    uint32_t    add = 0;    // add count

    add = (uint32_t)crc;
    // calculate the data checksum
    for (i=0; i<size; i++)
    {
        add += (uint32_t) pBuffer [i];
    }
    return ((uint16_t) add);
}

/*lint -save -e641 BGLIB api typedefs are checked as integers*/
/******************************************************************************
** Name:    Wifi_Initialize_BGLib
*****************************************************************************/
/**
** @brief   Initialize library,and provide output function
**
** @param   none
**
** @return  ERROR_ID_e error code
******************************************************************************/
void Wifi_Initialize_BGLib(void)
{
    // Initialize BGLIB with our output function for sending messages.
    BGLIB_INITIALIZE(Wifi_Send_Command);

    // This command is used to verify that communication works between the external host and the device
    wifi_cmd_system_hello();

    (void) tx_thread_sleep (OSTIME_5SEC);  // wait a little time
}

/******************************************************************************
** Name:    Wifi_Set_Operating_Mode
*****************************************************************************/
/**
** @brief   Wifi set operating mode: 1-client;2-Access Point;4-WIFI direct
**
** @param   none
**
** @return  ERROR_ID_e error code
******************************************************************************/
ERROR_ID_e Wifi_Set_Operating_Mode(void)
{
    uint8_t read_bytes = 0;
    (void) tx_thread_sleep (OSTIME_100MSEC);  // wait a little time

    read_bytes = BGLIB_RX_BYTES(BGLIB_RESP_2_BYTES,BGLIB_NO_EVT);
    wifi_cmd_sme_set_operating_mode(1);
    if (Wifi_Command_Response (wifi_rsp_sme_set_operating_mode_id, read_bytes, OSTIME_3SEC) == FALSE)
    {
        (void) tx_thread_sleep (OSTIME_1SEC);  // wait a little time
        wifi_cmd_sme_set_operating_mode(1);
        if (Wifi_Command_Response (wifi_rsp_sme_set_operating_mode_id, read_bytes, OSTIME_3SEC) == FALSE)
        {
            return eERR_COMM_WIFI_CHECK_STATUS;
        }
    }

    return eERR_NONE;
}

/******************************************************************************
** Name:    Wifi_Check_Module_Status
*****************************************************************************/
/**
** @brief   Check Wifi module Status
**
** @param   none
**
** @return  ERROR_ID_e error code
******************************************************************************/
ERROR_ID_e Wifi_Check_Module_Status(void)
{
    uint8_t read_bytes = 0;
    (void) tx_thread_sleep (OSTIME_100MSEC);  // wait a little time

    read_bytes = BGLIB_RX_BYTES(BGLIB_RESP_NO_BYTES,BGLIB_NO_EVT);
    wifi_cmd_system_hello();
    if (Wifi_Command_Response (wifi_rsp_system_hello_id, read_bytes, OSTIME_3SEC) == FALSE)
    {
        (void) tx_thread_sleep (OSTIME_1SEC);  // wait a little time
        wifi_cmd_system_hello();
        if (Wifi_Command_Response (wifi_rsp_system_hello_id, read_bytes, OSTIME_3SEC) == FALSE)
        {
            return eERR_COMM_WIFI_CHECK_STATUS;
        }
    }

    // wait a little time
    (void) tx_thread_sleep(OSTIME_50MSEC);

    return eERR_NONE;
}

/******************************************************************************
** Name:    Wifi_Configure_Module_Uart
*****************************************************************************/
/**
** @brief   Configure Wifi module Uart
**
** @param   none
**
** @return  ERROR_ID_e error code
******************************************************************************/
ERROR_ID_e Wifi_Configure_Module_Uart(void)
{
    uint8_t read_bytes = 0;

    read_bytes = BGLIB_RX_BYTES(BGLIB_RESP_2_BYTES,BGLIB_EVT_HW_UART_CONF);

    // This command is used to read the current configuration of a UART interface
    wifi_cmd_hardware_uart_conf_get(0);

    if (Wifi_Command_Response (wifi_rsp_hardware_uart_conf_get_id, read_bytes, OSTIME_10SEC) == FALSE)
    {
        return eERR_COMM_WIFI_INIT;
    }
    //lint -e621  Identifier clash suppressed for individual identifiers
    if(pck_resp->rsp_hardware_uart_conf_get.result != wifi_err_success)
    {
        return eERR_COMM_WIFI_INIT;
    }

    if(pck_evt[0]->evt_hardware_uart_conf.rate       != BR_115384)    return eERR_COMM_WIFI_INIT_CONFIG_UART_BAUD;
    if(pck_evt[0]->evt_hardware_uart_conf.data_bits  != DATA_BITS_8)  return eERR_COMM_WIFI_INIT_CONFIG_UART_DATA;
    if(pck_evt[0]->evt_hardware_uart_conf.stop_bits  != STOP_BITS_1)  return eERR_COMM_WIFI_INIT_CONFIG_UART_STOP;
    if(pck_evt[0]->evt_hardware_uart_conf.parity     != NO_PARITY)    return eERR_COMM_WIFI_INIT_CONFIG_UART_PARITY;
    if(pck_evt[0]->evt_hardware_uart_conf.flow_ctrl  != NO_FLOW_CTRL) return eERR_COMM_WIFI_INIT_CONFIG_UART_FLOW;

    // retry ????
    // This command is used to configure a UART interface
    //wifi_cmd_hardware_uart_conf_set(uint8 id, uint32 rate, uint8 data_bits, uint8 stop_bits, uint8 parity, uint8 flow_ctrl);

    (void) tx_thread_sleep (OSTIME_50MSEC);  // wait a little time

    return eERR_NONE;
}

/******************************************************************************
** Name:    Wifi_Get_Module_Status
*****************************************************************************/
/**
** @brief   Get Wifi module status
**
** @param   none
**
** @return  ERROR_ID_e error code
******************************************************************************/
ERROR_ID_e Wifi_Get_Module_Status(void)
{
    // This command is used to retrieve the device status
    wifi_cmd_system_sync();

    if (Wifi_Command_Response (wifi_rsp_system_sync_id, BGLIB_RX_BYTES(BGLIB_RESP_NO_BYTES,BGLIB_NO_EVT), OSTIME_10SEC) == FALSE)
    {
        return eERR_COMM_WIFI_INIT_SYNC;
    }

    (void) tx_thread_sleep (OSTIME_50MSEC); // wait a little time

    return eERR_NONE;
}

/******************************************************************************
** Name:    Wifi_Get_Module_MAC_Address
*****************************************************************************/
/**
** @brief   Get Wifi module MAC Address
**
** @param   none
**
** @return  ERROR_ID_e error code
******************************************************************************/
ERROR_ID_e Wifi_Get_Module_MAC_Address(void)
{
    uint8_t fx_res, read_bytes = 0;
    uint32_t attributes;
    char_t mac_add[17] = {0};
    #define  MAC "mac.mac"

    fx_res = (uint8_t) fx_directory_default_set(&sd_fx_media, CERT_FOLDER_NAME);
    if (fx_res == FX_SUCCESS)
    {
        fx_res = (uint8_t) fx_file_attributes_read (&sd_fx_media, MAC, (UINT *)&attributes);
        if (fx_res != FX_SUCCESS)
        {

            read_bytes = BGLIB_RX_BYTES(BGLIB_RESP_3_BYTES,BGLIB_EVT_GET_MAC);

            // This command is used to read the MAC address of the device
            wifi_cmd_config_get_mac(0);

            if (Wifi_Command_Response (wifi_rsp_config_get_mac_id, read_bytes, OSTIME_10SEC) == FALSE)
            {
                return eERR_COMM_WIFI_INIT_GET_MAC_ADDRESS;
            }

            if(pck_resp->rsp_config_get_mac.result != wifi_err_success)
            {
                return eERR_COMM_WIFI_INIT;
            }

            // Parse WGM110 MAC Address format to ASCII
            Wifi_Save_MAC_Address();

            // Check if MAC Address is valid
            if(Comm_Wifi_Is_MAC_Valid(R100_wifi.mac_address) == FALSE) return eERR_COMM_WIFI_INIT_GET_MAC_ADDRESS;

            (void) tx_thread_sleep (OSTIME_50MSEC);  // wait a little time

            fx_res = (uint8_t) fx_file_create(&sd_fx_media, MAC);
            if (fx_res == 0)
            {
                fx_res = (uint8_t) fx_file_open(&sd_fx_media, &wifi_file, MAC, FX_OPEN_FOR_WRITE);
                if (fx_res == 0)
                {
                    sprintf (mac_add, "%s", R100_wifi.mac_address);
                    fx_res = (uint8_t) fx_file_write(&wifi_file, mac_add, sizeof(mac_add));
                    fx_res = (uint8_t) fx_media_flush (&sd_fx_media);
                    fx_res = (uint8_t) fx_file_close(&wifi_file);
                }
            }
            fx_res = (uint8_t) fx_directory_default_set(&sd_fx_media, FX_NULL);
        }
    }

    return eERR_NONE;
}

/******************************************************************************
** Name:    Wifi_Set_Wifi_Radio_On
*****************************************************************************/
/**
** @brief   Turn On Wifi module Radio
**
** @param   none
**
** @return  ERROR_ID_e error code
******************************************************************************/
ERROR_ID_e Wifi_Set_Wifi_Radio_On(void)
{
    uint8_t read_bytes = 0;

    read_bytes = BGLIB_RX_BYTES(BGLIB_RESP_2_BYTES, BGLIB_EVT_WIFI_ON_OFF);

    // This command is used to switch on the Wi-Fi radio
    wifi_cmd_sme_wifi_on();

    if (Wifi_Command_Response (wifi_rsp_sme_wifi_on_id, read_bytes, OSTIME_10SEC) == FALSE)
    {
        //Wifi_Set_Wifi_Radio_Off();
        return eERR_COMM_WIFI_INIT;
        /*wifi_cmd_sme_wifi_on();

        if (Wifi_Command_Response (wifi_rsp_sme_wifi_on_id, read_bytes, OSTIME_10SEC) == FALSE)
        {
           return eERR_COMM_WIFI_INIT;
        }*/
    }

    if(pck_resp->rsp_sme_wifi_on.result != wifi_err_success)
    {
       return eERR_COMM_WIFI_INIT;
    }

    if(pck_evt[0]->evt_sme_wifi_is_on.result != wifi_err_success)
    {
       return eERR_COMM_WIFI_INIT;
    }

    return eERR_NONE;
}

/******************************************************************************
** Name:    Wifi_Set_Wifi_Radio_Off
*****************************************************************************/
/**
** @brief   Turn Off Wifi module Radio
**
** @param   none
**
** @return  ERROR_ID_e error code
******************************************************************************/
ERROR_ID_e Wifi_Set_Wifi_Radio_Off(void)
{
    uint8_t read_bytes = 0;

    read_bytes = BGLIB_RX_BYTES(BGLIB_RESP_2_BYTES,BGLIB_EVT_WIFI_ON_OFF);
    // This command is used to turn off the Wi-Fi radio
    wifi_cmd_sme_wifi_off();

    if (Wifi_Command_Response (wifi_rsp_sme_wifi_off_id, read_bytes, OSTIME_10SEC) == FALSE)
    {
       return eERR_COMM_WIFI_INIT;
    }

    if(pck_resp->rsp_sme_wifi_off.result != wifi_err_success)
    {
       return eERR_COMM_WIFI_INIT;
    }

    if(pck_evt[0]->evt_sme_wifi_is_off.result != wifi_err_success)
    {
       return eERR_COMM_WIFI_INIT;
    }

    return eERR_NONE;
}

/******************************************************************************
** Name:    Wifi_Scan_Network
*****************************************************************************/
/**
** @brief   Scan for an Access Point
**
** @param   none
**
** @return  ERROR_ID_e error code
******************************************************************************/
ERROR_ID_e Wifi_Scan_Network(void)
{
    //FX_FILE  wifi_file;
    uint8_t  fx_res;
    uint32_t nBytes = 0;
    uint8_t  i, j, k, h, z, ii, jj, kk, hh, zz;
    uint16_t  offset = 0;
    //uint8  wlan_secure = 0;
    //uint8_t read_bytes = 0;
    char_t     r_w_data[WIFI_SIZE];

    memset(&R100_wifi, 0, sizeof(wifi_config_t));

    //strcpy(R100_wifi.wlan_ssid, "Corporativa");//strcpy(wifi.wlan_ssid, "OSATU");
    //strcpy(R100_wifi.wlan_pass, "bexen$220");

    //strcpy(R100_wifi.wlan_ssid, "OSATU_2");
    //strcpy(R100_wifi.wlan_pass, "Bexencardio_123");

    //strcpy(R100_wifi.wlan_ssid, "AndroidAP_7710");
    //strcpy(R100_wifi.wlan_pass, "jonnwifi");

    //strcpy(R100_wifi.wlan_ssid, "ASUS_30_2G");
    //strcpy(R100_wifi.wlan_pass, "jonPass1235");

    //strcpy(R100_wifi.host_name, "192.168.106.53"); // Corporativa
    //strcpy(R100_wifi.host_name, "192.168.2.167"); // ASUS_30_2G WIFI
    //strcpy(R100_wifi.host_name, "192.168.2.209"); // ASUS_30_2G ETHERNET
    //strcpy(R100_wifi.host_name, "192.168.1.161"); // PC INAXIO
    //strcpy(R100_wifi.host_name, "192.168.105.3"); // Estefania
    //strcpy(R100_wifi.host_name, "4.231.129.117"); // Estefania wififront.bexencardio.com
    strcpy(R100_wifi.host_name, "wififront.bexencardio.com");

    memset(endpoint_data, 0, MAX_WIFI_DATA_TX_SZ);

    fx_res = (uint8_t) fx_directory_default_set(&sd_fx_media, FX_NULL); // Reset default directory

    // Load and read Test file on SD
    fx_res = (uint8_t) fx_file_open(&sd_fx_media, &wifi_file, WIFI_FILENAME, FX_OPEN_FOR_READ);
    if (fx_res == 0)
    {
        fx_res = (uint8_t) fx_file_read(&wifi_file, r_w_data, WIFI_SIZE, &nBytes);     //Read 100 bytes
        if (fx_res == 0)
        {
            if(fx_res = Crypto_ATECC_Decrypt((uint8_t *)r_w_data), fx_res == 0)
            {
                R100_wifi.wlan_eap_security = (uint8_t)r_w_data[WIFI_EAP_POS] - '0';

                // SSID offset position
                offset = (uint8_t) (WIFI_SSID_POS);

                for(i=0;i<32;i++)
                {
                    if(r_w_data[offset+i] == '\r') break;
                    R100_wifi.wlan_ssid[i] = r_w_data[offset+i];
                }

                // SSID password offset position
                offset = (uint8_t) (offset + i + WIFI_PASS_POS);

                for(j=0;j<32;j++)
                {
                    if(r_w_data[offset+j] == '\r') break;
                    R100_wifi.wlan_pass[j] = r_w_data[offset+j];
                }

                // key password offset position
                offset = (uint8_t) (offset + j + WIFI_KEY_PASS_POS);

                for(k=0;k<32;k++)
                {
                    if(r_w_data[offset+k] == '\r') break;
                    R100_wifi.router_pass_key[k] = r_w_data[offset+k];
                }

                // EAP configuration offset position
                offset = (uint8_t) (offset + k + WIFI_EAP_CNF_POS);

                for(h=0;h<32;h++)
                {
                    if(r_w_data[offset+h] == '\r') break;
                    R100_wifi.eap_cnf[h] = r_w_data[offset+h];
                }

                // EAP configuration offset position
                offset = (uint8_t) (offset + h + WIFI_EAP_PASS_POS);

                for(z=0;z<32;z++)
                {
                    if(r_w_data[offset+z] == '\r') break;
                    R100_wifi.eap_pass[z] = r_w_data[offset+z];
                }

                // EAP configuration offset position
                offset = (uint8_t) (offset + z + WIFI_ADVS_POS);

                R100_wifi.wlan_advs = (uint8_t)r_w_data[offset] - '0';

                // EAP configuration offset position
                offset = (uint8_t) (offset + WIFI_IP_POS);

                for(ii=0;ii<32;ii++)
                {
                    if(r_w_data[offset+ii] == '\r') break;
                    R100_wifi.ip[ii] = r_w_data[offset+ii];
                }

                // Subnet mask configuration offset position
                offset = (uint8_t) (offset + ii + WIFI_MASK_POS);

                for(jj=0;jj<32;jj++)
                {
                    if(r_w_data[offset+jj] == '\r') break;
                    R100_wifi.mask[jj] = r_w_data[offset+jj];
                }

                // Gateway configuration offset position
                offset = (uint8_t) (offset + jj + WIFI_GATEWAY_POS);

                for(kk=0;kk<32;kk++)
                {
                    if(r_w_data[offset+kk] == '\r') break;
                    R100_wifi.gateway[kk] = r_w_data[offset+kk];
                }

                // DNSP configuration offset position
                offset = (uint8_t) (offset + kk + WIFI_DNSP_POS);

                for(hh=0;hh<32;hh++)
                {
                    if(r_w_data[offset+hh] == '\r') break;
                    R100_wifi.pdns[hh] = r_w_data[offset+hh];
                }

                // DNSS configuration offset position
                offset = (uint8_t) (offset + hh + WIFI_DNSS_POS);

                for(zz=0;zz<32;zz++)
                {
                    if(r_w_data[offset+zz] == '\r') break;
                    R100_wifi.sdns[zz] = r_w_data[offset+zz];
                }

            }
            else 
            {
                Set_NV_Data_Error_IF_NOT_SAT_Comms(fx_res);
                return eERR_COMM_WIFI_INIT;
            }
        }
    }
    else return eERR_COMM_WIFI_INIT;

    fx_res = (uint8_t) fx_file_close (&wifi_file);

    /*read_bytes = BGLIB_RX_BYTES(BGLIB_RESP_2_BYTES, (BGLIB_EVT_SCAN_RESULT + (uint8_t)strlen(R100_wifi.wlan_ssid)));
    // This command initiates a scan for an Access Point
    wifi_cmd_sme_start_ssid_scan((uchar_t)strlen(R100_wifi.wlan_ssid), R100_wifi.wlan_ssid);

    if (Wifi_Command_Response (wifi_rsp_sme_start_ssid_scan_id, read_bytes, OSTIME_30SEC) == FALSE)
    {
        return eERR_COMM_WIFI_INIT;
    }

    if(pck_resp->rsp_sme_start_scan.result != wifi_err_success)
    {
        return eERR_COMM_WIFI_INIT;
    }

    wlan_secure = pck_evt[0]->evt_sme_scan_result.secure;

    //if(wlan_secure & 0x01) // bit 0: defines whether the Access Point supports secure connections
    //if(wlan_secure & 0x02) // bit 1: defines whether the Access Point supports WPS
    //if(wlan_secure & 0x04) // bit 2: defines whether the Access Point supports WPA-Enterprise
    //if(wlan_secure & 0x08) // bit 3: defines whether SSID is hidden

    if(wlan_secure & 0x04)
    {
        R100_wifi.wlan_eap_security = eWIFI_WPA_EAP_SEC;
    }
    else
    {
        R100_wifi.wlan_eap_security = eWIFI_DEFAULT_SEC;
    }*/

    //(void) tx_thread_sleep (OSTIME_2SEC);  // wait a little time

    return eERR_NONE;
}

/******************************************************************************
** Name:    Wifi_Config_Certs
*****************************************************************************/
/**
 * @brief   Update of the cert.dat file to know which certificates have been loaded
 *
 * @param   write_read Bit for read/write in the file: 0 read / 1 write
 * @param   eap_tls To know which certificates have been loaded
 * @param   write_fingerprint Certificate fingerprint data
 *
 * @return  fx_res
 */
ERROR_ID_e Wifi_Config_Certs(bool_t write_read, uint8_t eap, uint8_t tls, char_t* write_fingerprint)
{
    //FX_FILE  cert_file;
    uint8_t  fx_res = 0, i;
    uint32_t nBytes = 0;

    memset(endpoint_data, 0, MAX_WIFI_DATA_TX_SZ);

    // Hidden folder
    fx_res = (uint8_t) fx_directory_attributes_set(&sd_fx_media, CERT_FOLDER_NAME, FX_HIDDEN);
    fx_res = (uint8_t) fx_directory_default_set(&sd_fx_media, CERT_FOLDER_NAME);
    if (fx_res == FX_SUCCESS)
    {
        // Load and read Test file on SD
        fx_res = (uint8_t) fx_file_open(&sd_fx_media, &wifi_file, CERT_FILENAME, write_read);
        if (fx_res == 0)
        {
            if(write_read == WIFI_CERT_READ)
            {
                memset(&R100_wifi_cert, 0, sizeof(WIFI_CERT_t));
                fx_res = (uint8_t) fx_file_read(&wifi_file, endpoint_data, 255, &nBytes);     //Read 100 bytes
                if (fx_res == 0)
                {
                    R100_wifi_cert.wifi_load_eap = (uint8_t)(endpoint_data[9] - '0');
                    R100_wifi_cert.wifi_load_tls = (uint8_t)(endpoint_data[21] - '0');
                    for(i=0;i<16;i++)
                    {
                        if(endpoint_data[27+i] == '\r') break;
                        R100_wifi_cert.wifi_fingerprint[i] = endpoint_data[27+i];
                    }
                }
            }
            else
            {
                if(eap == WIFI_EAP_LOADED)
                {
                    fx_res = (uint8_t) fx_file_seek(&wifi_file, (ULONG) (9));
                    fx_res = (uint8_t) fx_file_write (&wifi_file, "1", 1);
                }
                if(tls == WIFI_TLS_LOADED)
                {
                    fx_res = (uint8_t) fx_file_seek(&wifi_file, (ULONG) (21));
                    fx_res = (uint8_t) fx_file_write (&wifi_file, "1", 1);
                }
                if(eap == WIFI_FP_LOADED)
                {
                    fx_res = (uint8_t) fx_file_seek(&wifi_file, (ULONG) (27));
                    fx_res = (uint8_t) fx_file_write (&wifi_file, write_fingerprint, 16);
                }
                fx_res = (uint8_t) fx_media_flush (&sd_fx_media);
            }
        }
        fx_res = (uint8_t) fx_file_close (&wifi_file);
        fx_res = (uint8_t) fx_directory_default_set(&sd_fx_media, FX_NULL);
    }
    return fx_res;
}

/******************************************************************************
** Name:    Wifi_Reset_Certs_Store
*****************************************************************************/
/**
** @brief   Reset Certificates Store
**
** @param   none
**
** @return  ERROR_ID_e error code
******************************************************************************/
ERROR_ID_e Wifi_Reset_Certs_Store()
{
    uint8_t read_bytes = 0;
    read_bytes = BGLIB_RX_BYTES(BGLIB_RESP_2_BYTES, BGLIB_NO_EVT);
    wifi_cmd_x509_reset_store();
    if (Wifi_Command_Response (wifi_rsp_x509_reset_store_id, read_bytes, OSTIME_3SEC) == FALSE)
    {
        return eERR_COMM_WIFI_CERT_RESET_STORE;
    }
    if(pck_resp->rsp_x509_reset_store.result != wifi_err_success)
    {
        return eERR_COMM_WIFI_CERT_RESET_STORE;
    }
    return eERR_NONE;
}

/******************************************************************************
** Name:    Wifi_Load_Server_TLS_Cacert
*****************************************************************************/
/**
** @brief   Load Server TLS CA cert if present in SD and if not present in module flash
**
** @param   none
**
** @return  ERROR_ID_e error code
******************************************************************************/
ERROR_ID_e Wifi_Load_Server_TLS_Cacert(void) //estaba uint32_t bytes, porque si size es uint16_t?
{
    //FX_FILE  server_ca_cert_file;
    uint8_t  fx_res;
    uint8_t read_bytes = 0;
    uint32_t read_nBytes = 0;
    uint32_t frameId, nFrame;
    char_t cert_data[MAX_WIFI_DATA_TX_SZ];

    fx_res = (uint8_t) fx_directory_default_set(&sd_fx_media, FX_NULL); // Reset default directory
    fx_res = (uint8_t) fx_directory_default_set(&sd_fx_media, CERT_FOLDER_NAME); // Reset default directory

    // Load and read certificate file on SD
    fx_res = (uint8_t) fx_file_open(&sd_fx_media, &wifi_file, TLS_CA_CERT_FILENAME, FX_OPEN_FOR_READ);
    if (fx_res == 0)
    {
        // Command to add a certificate to a certificate store.
        read_bytes = BGLIB_RX_BYTES(BGLIB_RESP_2_BYTES, BGLIB_NO_EVT);
        wifi_cmd_x509_add_certificate(0, (uint16_t)wifi_file.fx_file_current_file_size); // (Store in SD, size)
        if (Wifi_Command_Response (wifi_rsp_x509_add_certificate_id, read_bytes, OSTIME_3SEC) == FALSE)
        {
            return eERR_COMM_WIFI_CERT_SERVER_TLS_CA;
        }
        if(pck_resp->rsp_x509_add_certificate.result != wifi_err_success)
        {
            return eERR_COMM_WIFI_CERT_SERVER_TLS_CA;
        }

        nFrame = (uint32_t) ((wifi_file.fx_file_current_file_size) / 255 );
        if (((wifi_file.fx_file_current_file_size) % 255) != 0)
        {
            nFrame++;
        }
        for(frameId=0; frameId < nFrame; frameId++)
        {
            fx_res = (uint8_t) fx_file_read(&wifi_file, cert_data, MAX_WIFI_DATA_TX_SZ, &read_nBytes);
            if (fx_res == 0)
            {
                // Command to upload a block of certificate data.
                read_bytes = BGLIB_RX_BYTES(BGLIB_RESP_2_BYTES, BGLIB_NO_EVT);
                wifi_cmd_x509_add_certificate_data((uint8_t)read_nBytes, cert_data);
                if (Wifi_Command_Response (wifi_rsp_x509_add_certificate_data_id, read_bytes, OSTIME_5SEC) == FALSE)
                {
                    return eERR_COMM_WIFI_CERT_SERVER_TLS_CA;
                }
                if(pck_resp->rsp_x509_add_certificate_data.result != wifi_err_success)
                {
                    return eERR_COMM_WIFI_CERT_SERVER_TLS_CA;
                }
            }
            else
            {
                return eERR_COMM_WIFI_CERT_SERVER_TLS_CA;
            }
        }
        fx_res = (uint8_t) fx_file_close (&wifi_file);
        fx_res = (uint8_t) fx_directory_default_set(&sd_fx_media, FX_NULL); // Reset default directory

        // This command is used to finish adding a certificate.
        //read_bytes = BGLIB_RX_BYTES(BGLIB_RESP_3_BYTES, BGLIB_NO_EVT);
        wifi_cmd_x509_add_certificate_finish(); 
        if (Wifi_Command_Response_Data (wifi_rsp_x509_add_certificate_finish_id, OSTIME_10SEC) == FALSE)
        {
            return eERR_COMM_WIFI_CERT_SERVER_TLS_CA;
        }
        if(pck_resp->rsp_x509_add_certificate_finish.result != wifi_err_success)
        {
            if(pck_resp->rsp_x509_add_certificate_finish.result == wifi_err_already_exists)
            {
                return eERR_NONE;
            }
            return TRY_AGAIN;
            //return eERR_COMM_WIFI_CERT_SERVER_TLS_CA;
        }

        /*// This command is used to set the required CA certificate of an EAP type. // 1: TLS type
        read_bytes = BGLIB_RX_BYTES(BGLIB_RESP_2_BYTES, BGLIB_NO_EVT);
        wifi_cmd_sme_set_eap_type_ca_certificate(1, pck_evt[0]->evt_x509_certificate.fingerprint.len, pck_evt[0]->evt_x509_certificate.fingerprint.data);
        if (Wifi_Command_Response (wifi_rsp_sme_set_eap_type_ca_certificate_id, read_bytes, OSTIME_3SEC) == FALSE)
        {
            return eERR_COMM_WIFI_INIT_CONFIG_TLS_CACERT;
        }
        if(pck_resp->rsp_sme_set_eap_type_ca_certificate.result != wifi_err_success)
        {
            return eERR_COMM_WIFI_INIT_CONFIG_TLS_CACERT;
        }*/
    }
    else
    {
        return eERR_COMM_WIFI_INIT_OPEN;
    }

    return eERR_NONE;
}

/******************************************************************************
** Name:    Wifi_Load_EAP_CA_Cert
*****************************************************************************/
/**
** @brief   Load EAP CA certificate to connect
**
** @param   none
**
** @return  ERROR_ID_e error code
******************************************************************************/
ERROR_ID_e Wifi_Load_EAP_CA_Cert(void)
{
    uint8_t  fx_res;
    uint8_t read_bytes = 0;
    uint32_t read_nBytes = 0;
    uint32_t frameId, nFrame;
    char_t cert_data[MAX_WIFI_DATA_TX_SZ];

    fx_res = (uint8_t) fx_directory_default_set(&sd_fx_media, FX_NULL); // Reset default directory
    fx_res = (uint8_t) fx_directory_default_set(&sd_fx_media, CERT_FOLDER_NAME);

    // Load and read Router CA certificate file on SD
    fx_res = (uint8_t) fx_file_open(&sd_fx_media, &wifi_file, WPA_EAP_CA_CERT_FILENAME, FX_OPEN_FOR_READ);
    if (fx_res == 0)
    {
        // Command to add a certificate to a certificate store.
        read_bytes = BGLIB_RX_BYTES(BGLIB_RESP_2_BYTES, BGLIB_NO_EVT);
        wifi_cmd_x509_add_certificate(0, (uint16_t)wifi_file.fx_file_current_file_size); // (Store in SD, size)
        if (Wifi_Command_Response (wifi_rsp_x509_add_certificate_id, read_bytes, OSTIME_3SEC) == FALSE)
        {
            return eERR_COMM_WIFI_CERT_CLIENT_ROUTER_CA;
        }
        if(pck_resp->rsp_x509_add_certificate.result != wifi_err_success)
        {
            return eERR_COMM_WIFI_CERT_CLIENT_ROUTER_CA;
        }

        nFrame = (uint32_t) ((wifi_file.fx_file_current_file_size) / 255 );
        if (((wifi_file.fx_file_current_file_size) % 255) != 0)
        {
            nFrame++;
        }

        for(frameId=0; frameId < nFrame; frameId++)
        {
            fx_res = (uint8_t) fx_file_read(&wifi_file, cert_data, MAX_WIFI_DATA_TX_SZ, &read_nBytes);
            if (fx_res == 0)
            {
                // Command to upload a block of certificate data.
                read_bytes = BGLIB_RX_BYTES(BGLIB_RESP_2_BYTES, BGLIB_NO_EVT);
                wifi_cmd_x509_add_certificate_data((uint8_t)read_nBytes, cert_data);
                if (Wifi_Command_Response (wifi_rsp_x509_add_certificate_data_id, read_bytes, OSTIME_5SEC) == FALSE)
                {
                    return eERR_COMM_WIFI_CERT_CLIENT_ROUTER_CA;
                }
                if(pck_resp->rsp_x509_add_certificate_data.result != wifi_err_success)
                {
                    return eERR_COMM_WIFI_CERT_CLIENT_ROUTER_CA;
                }
            }
            else
            {
                return eERR_COMM_WIFI_CERT_CLIENT_ROUTER_CA;
            }
        }
        fx_res = (uint8_t) fx_file_close (&wifi_file);

        // This command is used to finish adding a certificate.
        //read_bytes = BGLIB_RX_BYTES(BGLIB_RESP_19_BYTES, BGLIB_NO_EVT);
        wifi_cmd_x509_add_certificate_finish(); 
        if (Wifi_Command_Response_Data (wifi_rsp_x509_add_certificate_finish_id, OSTIME_10SEC) == FALSE)
        {
            return eERR_COMM_WIFI_CERT_CLIENT_ROUTER_CA;
        }
        if(pck_resp->rsp_x509_add_certificate_finish.result != wifi_err_success)
        {
            if(pck_resp->rsp_x509_add_certificate_finish.result == wifi_err_already_exists)
            {
                if(pck_resp->rsp_x509_add_certificate_finish.result == wifi_err_hardware_ps_store_full)
                {
                    // This command is used to reset the certificate stores.
                    read_bytes = BGLIB_RX_BYTES(BGLIB_RESP_2_BYTES, BGLIB_NO_EVT);
                    wifi_cmd_x509_reset_store();
                    if (Wifi_Command_Response (wifi_rsp_x509_reset_store_id, read_bytes, OSTIME_5SEC) == FALSE)
                    {
                        return eERR_COMM_WIFI_CERT_RESET_STORE;
                    }
                    if(pck_resp->rsp_x509_reset_store.result != wifi_err_success)
                    {
                        return eERR_COMM_WIFI_CERT_RESET_STORE;
                    }
                    return TRY_AGAIN;
                }

                // This command is used to retrieve information about added certificates.
                read_bytes = BGLIB_RX_BYTES(BGLIB_RESP_2_BYTES, BGLIB_EVT_X509_CERT + BGLIB_EVT_X509_CERT_FINGER + BGLIB_EVT_X509_CERT_SUB + BGLIB_EVT_X509_CERT_SUB_LEN + BGLIB_EVT_X509_CERT_LIST);
                wifi_cmd_x509_list_certificates();
                if (Wifi_Command_Response (wifi_rsp_x509_list_certificates_id, read_bytes, OSTIME_10SEC) == FALSE)
                {
                    return eERR_COMM_WIFI_CERT_LIST;
                }
                if(pck_resp->rsp_x509_list_certificates.result != wifi_err_success)
                {
                    return eERR_COMM_WIFI_CERT_LIST;
                }
            }
        }
    }
    else
    {
        return eERR_COMM_WIFI_INIT_OPEN;
    }
    return eERR_NONE;
}

/******************************************************************************
** Name:    Wifi_Load_EAP_TLS_Cert
*****************************************************************************/
/**
** @brief   Load EAP certs to connect using EAP-TLS
**
** @param   none
**
** @return  ERROR_ID_e error code
******************************************************************************/
ERROR_ID_e Wifi_Load_EAP_TLS_Cert(void)
{
    //FX_FILE  router_cert_key_file;
    //FX_FILE  router_cert_file;
    //FX_FILE  router_ca_cert_file;
    uint8_t  fx_res;
    uint8_t read_bytes = 0;
    uint32_t read_nBytes = 0;
    uint32_t frameId, nFrame;
    char_t cert_data[MAX_WIFI_DATA_TX_SZ];
    char recv_resp[16] = {0};

    fx_res = Wifi_Load_EAP_CA_Cert();
    if(fx_res != eERR_NONE) return eERR_COMM_WIFI_INIT_OPEN;

    fx_res = (uint8_t) fx_directory_default_set(&sd_fx_media, FX_NULL); // Reset default directory
    fx_res = (uint8_t) fx_directory_default_set(&sd_fx_media, CERT_FOLDER_NAME);

    // Load and read Router certificate file on SD
    fx_res = (uint8_t) fx_file_open(&sd_fx_media, &wifi_file, WPA_EAP_CERT_FILENAME, FX_OPEN_FOR_READ);
    if (fx_res == 0)
    {
        // Command to add a certificate to a certificate store.
        read_bytes = BGLIB_RX_BYTES(BGLIB_RESP_2_BYTES, BGLIB_NO_EVT);
        wifi_cmd_x509_add_certificate(0, (uint16_t)wifi_file.fx_file_current_file_size); // (Store in SD, size)
        if (Wifi_Command_Response (wifi_rsp_x509_add_certificate_id, read_bytes, OSTIME_3SEC) == FALSE)
        {
            return eERR_COMM_WIFI_CERT_CLIENT_ROUTER;
        }
        if(pck_resp->rsp_x509_add_certificate.result != wifi_err_success)
        {
            return eERR_COMM_WIFI_CERT_CLIENT_ROUTER;
        }

        nFrame = (uint32_t) ((wifi_file.fx_file_current_file_size) / 255 );
        if (((wifi_file.fx_file_current_file_size) % 255) != 0)
        {
            nFrame++;
        }

        for(frameId=0; frameId < nFrame; frameId++)
        {
            fx_res = (uint8_t) fx_file_read(&wifi_file, cert_data, MAX_WIFI_DATA_TX_SZ, &read_nBytes);
            if (fx_res == 0)
            {
                // Command to upload a block of certificate data.
                read_bytes = BGLIB_RX_BYTES(BGLIB_RESP_2_BYTES, BGLIB_NO_EVT);
                wifi_cmd_x509_add_certificate_data((uint8_t)read_nBytes, cert_data);
                if (Wifi_Command_Response (wifi_rsp_x509_add_certificate_data_id, read_bytes, OSTIME_5SEC) == FALSE)
                {
                    return eERR_COMM_WIFI_CERT_CLIENT_ROUTER;
                }
                if(pck_resp->rsp_x509_add_certificate_data.result != wifi_err_success)
                {
                    return eERR_COMM_WIFI_CERT_CLIENT_ROUTER;
                }
            }
            else
            {
                return eERR_COMM_WIFI_CERT_CLIENT_ROUTER;
            }
        }
        fx_res = (uint8_t) fx_file_close (&wifi_file);

        // This command is used to finish adding a certificate.
        //read_bytes = BGLIB_RX_BYTES(BGLIB_RESP_19_BYTES, BGLIB_NO_EVT);
        wifi_cmd_x509_add_certificate_finish(); 
        if (Wifi_Command_Response_Data (wifi_rsp_x509_add_certificate_finish_id, OSTIME_10SEC) == FALSE)
        {
            return eERR_COMM_WIFI_CERT_CLIENT_ROUTER;
        }
        if(pck_resp->rsp_x509_add_certificate_finish.result != wifi_err_success)
        {
            if(pck_resp->rsp_x509_add_certificate_finish.result == wifi_err_already_exists)
            {
                if(pck_resp->rsp_x509_add_certificate_finish.result == wifi_err_hardware_ps_store_full)
                {
                    // This command is used to reset the certificate stores.
                    read_bytes = BGLIB_RX_BYTES(BGLIB_RESP_2_BYTES, BGLIB_NO_EVT);
                    wifi_cmd_x509_reset_store();
                    if (Wifi_Command_Response (wifi_rsp_x509_reset_store_id, read_bytes, OSTIME_5SEC) == FALSE)
                    {
                        return eERR_COMM_WIFI_CERT_RESET_STORE;
                    }
                    if(pck_resp->rsp_x509_reset_store.result != wifi_err_success)
                    {
                        return eERR_COMM_WIFI_CERT_RESET_STORE;
                    }
                    return TRY_AGAIN;
                }

                // This command is used to retrieve information about added certificates.
                read_bytes = BGLIB_RX_BYTES(BGLIB_RESP_2_BYTES, BGLIB_EVT_X509_CERT + BGLIB_EVT_X509_CERT_FINGER + BGLIB_EVT_X509_CERT_SUB + BGLIB_EVT_X509_CERT_SUB_LEN + BGLIB_EVT_X509_CERT_LIST);
                wifi_cmd_x509_list_certificates();
                if (Wifi_Command_Response (wifi_rsp_x509_list_certificates_id, read_bytes, OSTIME_10SEC) == FALSE)
                {
                    return eERR_COMM_WIFI_CERT_LIST;
                }
                if(pck_resp->rsp_x509_list_certificates.result != wifi_err_success)
                {
                    return eERR_COMM_WIFI_CERT_LIST;
                }
                /*if(pck_evt[0]->evt_x509_certificate.type == 1)
                {
                    // This command is used to delete a certificate from the certificate store.
                    read_bytes = BGLIB_RX_BYTES(BGLIB_RESP_2_BYTES, BGLIB_NO_EVT);
                    wifi_cmd_x509_delete_certificate(pck_evt[0]->evt_x509_certificate.fingerprint.len, pck_evt[0]->evt_x509_certificate.fingerprint.data);
                    if (Wifi_Command_Response (wifi_rsp_x509_delete_certificate_id, read_bytes, OSTIME_5SEC) == FALSE)
                    {
                        return eERR_COMM_WIFI_INIT_CONFIG_TLS_CACERT;
                    }
                    if(pck_resp->rsp_x509_delete_certificate.result != wifi_err_success)
                    {
                        return eERR_COMM_WIFI_INIT_CONFIG_TLS_CACERT;
                    }
                    // After delete a certificate try again to load
                    Wifi_Load_EAP_TLS_Cert();
                }*/
            }
        }
        //memcpy(recv_resp, pck_evt[0]->evt_x509_certificate.fingerprint.data, pck_evt[0]->evt_x509_certificate.fingerprint.len);
        memcpy(recv_resp, pck_resp->rsp_x509_add_certificate_finish.fingerprint.data, pck_resp->rsp_x509_add_certificate_finish.fingerprint.len);

        // Load and read Router certificate KEY file on SD
        fx_res = (uint8_t) fx_file_open(&sd_fx_media, &wifi_file, WPA_EAP_CERT_KEY_FILENAME, FX_OPEN_FOR_READ);
        if(fx_res == 0)
        {
            // This command is used to add a private key to the RAM certificate store.
            read_bytes = BGLIB_RX_BYTES(BGLIB_RESP_2_BYTES, BGLIB_NO_EVT);
            wifi_cmd_x509_add_private_key((uint16_t)wifi_file.fx_file_current_file_size, pck_resp->rsp_x509_add_certificate_finish.fingerprint.len, pck_resp->rsp_x509_add_certificate_finish.fingerprint.data);
            if (Wifi_Command_Response (wifi_rsp_x509_add_private_key_id, read_bytes, OSTIME_3SEC) == FALSE)
            {
                return eERR_COMM_WIFI_CERT_CLIENT_ROUTER_KEY;
            }
            if(pck_resp->rsp_x509_add_private_key.result != wifi_err_success)
            {
                return eERR_COMM_WIFI_CERT_CLIENT_ROUTER_KEY;
            }

            nFrame = (uint32_t) ((wifi_file.fx_file_current_file_size) / 255 );
            if (((wifi_file.fx_file_current_file_size) % 255) != 0)
            {
                nFrame++;
            }
            for(frameId=0; frameId < nFrame; frameId++)
            {
                fx_res = (uint8_t) fx_file_read(&wifi_file, cert_data, MAX_WIFI_DATA_TX_SZ, &read_nBytes);
                if (fx_res == 0)
                {
                    // This command is used to upload a block of private key data.
                    read_bytes = BGLIB_RX_BYTES(BGLIB_RESP_2_BYTES, BGLIB_NO_EVT);
                    wifi_cmd_x509_add_private_key_data((uint8_t)read_nBytes, cert_data);
                    if (Wifi_Command_Response (wifi_rsp_x509_add_private_key_data_id, read_bytes, OSTIME_5SEC) == FALSE)
                    {
                        return eERR_COMM_WIFI_CERT_CLIENT_ROUTER_KEY;
                    }
                    if(pck_resp->rsp_x509_add_private_key_data.result != wifi_err_success)
                    {
                        return eERR_COMM_WIFI_CERT_CLIENT_ROUTER_KEY;
                    }
                }
                else
                {
                    return eERR_COMM_WIFI_CERT_CLIENT_ROUTER_KEY;
                }
            }
            fx_res = (uint8_t) fx_file_close (&wifi_file);
            fx_res = (uint8_t) fx_directory_default_set(&sd_fx_media, FX_NULL); // Reset default directory

            // This command is used to finish adding a certificate.
            //read_bytes = BGLIB_RX_BYTES(BGLIB_RESP_19_BYTES, BGLIB_NO_EVT);
            wifi_cmd_x509_add_private_key_finish((uint8_t) strlen(R100_wifi.router_pass_key), R100_wifi.router_pass_key); 
            if (Wifi_Command_Response_Data (wifi_rsp_x509_add_private_key_finish_id, OSTIME_10SEC) == FALSE)
            {
                return eERR_COMM_WIFI_CERT_CLIENT_ROUTER_KEY;
            }
        }

        // This command is used to set the client certificate of an EAP type. // 0: CA certificate 1: User/client certificate
        read_bytes = BGLIB_RX_BYTES(BGLIB_RESP_2_BYTES, BGLIB_NO_EVT);
        wifi_cmd_sme_set_eap_type_user_certificate(TLS, 16, recv_resp);
        if (Wifi_Command_Response (wifi_rsp_sme_set_eap_type_user_certificate_id, read_bytes, OSTIME_3SEC) == FALSE)
        {
            return eERR_COMM_WIFI_CERT_SERVER_TLS_CONFIG;
        }
        if(pck_resp->rsp_sme_set_eap_type_user_certificate.result != wifi_err_success)
        {
            return eERR_COMM_WIFI_CERT_SERVER_TLS_CONFIG;
        }

        // This command is used to set the EAP configuration, which is used when authenticating with a secure Access Point.
        read_bytes = BGLIB_RX_BYTES(BGLIB_RESP_2_BYTES, BGLIB_NO_EVT);
        wifi_cmd_sme_set_eap_configuration(TLS, NONE, (uint8_t) strlen(R100_wifi.eap_cnf), R100_wifi.eap_cnf);
        if (Wifi_Command_Response (wifi_rsp_sme_set_eap_configuration_id, read_bytes, OSTIME_3SEC) == FALSE)
        {
            return eERR_COMM_WIFI_CERT_SERVER_TLS_CONFIG;
        }
        if(pck_resp->rsp_sme_set_eap_configuration.result != wifi_err_success)
        {
            return eERR_COMM_WIFI_CERT_SERVER_TLS_CONFIG;
        }
        // Write EAP certificate fingerprint
        Wifi_Config_Certs(WIFI_CERT_WRITE, WIFI_FP_LOADED, NONE, recv_resp);
    }
    else
    {
        return eERR_COMM_WIFI_INIT_OPEN;
    }

    // This command is used to delete a certificate from the certificate store.
    /* read_bytes = BGLIB_RX_BYTES(BGLIB_RESP_2_BYTES, BGLIB_NO_EVT);
    wifi_cmd_x509_delete_certificate(pck_evt[0]->evt_x509_certificate.fingerprint.len, pck_evt[0]->evt_x509_certificate.fingerprint.data);
    if (Wifi_Command_Response (wifi_rsp_x509_delete_certificate_id, read_bytes, OSTIME_3SEC) == FALSE)
    {
        return eERR_COMM_WIFI_INIT_CONFIG_TLS_CACERT;
    } */

    return eERR_NONE;
}

/******************************************************************************
** Name:    Wifi_No_Load_EAP_TLS_Cert
*****************************************************************************/
/**
 * @brief   Configuration EAP certs to connect using EAP-TLS
 *
 * @param   write_fingerprint Certificate fingerprint data
 *
 * @return  ERROR_ID_e error code
 */
ERROR_ID_e Wifi_No_Load_EAP_TLS_Cert(char_t* fingerprint)
{
    //FX_FILE  router_cert_key_file;
    uint8_t  fx_res;
    uint8_t read_bytes = 0;
    uint32_t read_nBytes = 0;
    uint32_t frameId, nFrame;
    char_t cert_data[MAX_WIFI_DATA_TX_SZ];

    fx_res = (uint8_t) fx_directory_default_set(&sd_fx_media, FX_NULL); // Reset default directory
    fx_res = (uint8_t) fx_directory_default_set(&sd_fx_media, CERT_FOLDER_NAME); // Reset default directory
    // Load and read Router certificate KEY file on SD
    fx_res = (uint8_t) fx_file_open(&sd_fx_media, &wifi_file, WPA_EAP_CERT_KEY_FILENAME, FX_OPEN_FOR_READ);
    if(fx_res == 0)
    {
        // This command is used to add a private key to the RAM certificate store.
        read_bytes = BGLIB_RX_BYTES(BGLIB_RESP_2_BYTES, BGLIB_NO_EVT);
        wifi_cmd_x509_add_private_key((uint16_t)wifi_file.fx_file_current_file_size, 16, fingerprint);
        if (Wifi_Command_Response (wifi_rsp_x509_add_private_key_id, read_bytes, OSTIME_3SEC) == FALSE)
        {
            return eERR_COMM_WIFI_CERT_CLIENT_ROUTER_KEY;
        }
        if(pck_resp->rsp_x509_add_private_key.result != wifi_err_success)
        {
            return eERR_COMM_WIFI_CERT_CLIENT_ROUTER_KEY;
        }

        nFrame = (uint32_t) ((wifi_file.fx_file_current_file_size) / 255 );
        if (((wifi_file.fx_file_current_file_size) % 255) != 0)
        {
            nFrame++;
        }
        for(frameId=0; frameId < nFrame; frameId++)
        {
            fx_res = (uint8_t) fx_file_read(&wifi_file, cert_data, MAX_WIFI_DATA_TX_SZ, &read_nBytes);
            if (fx_res == 0)
            {
                // This command is used to upload a block of private key data.
                read_bytes = BGLIB_RX_BYTES(BGLIB_RESP_2_BYTES, BGLIB_NO_EVT);
                wifi_cmd_x509_add_private_key_data((uint8_t)read_nBytes, cert_data);
                if (Wifi_Command_Response (wifi_rsp_x509_add_private_key_data_id, read_bytes, OSTIME_5SEC) == FALSE)
                {
                    return eERR_COMM_WIFI_CERT_CLIENT_ROUTER_KEY;
                }
                if(pck_resp->rsp_x509_add_private_key_data.result != wifi_err_success)
                {
                    return eERR_COMM_WIFI_CERT_CLIENT_ROUTER_KEY;
                }
            }
            else
            {
                return eERR_COMM_WIFI_CERT_CLIENT_ROUTER_KEY;
            }
        }
        fx_res = (uint8_t) fx_file_close (&wifi_file);
        fx_res = (uint8_t) fx_directory_default_set(&sd_fx_media, FX_NULL); // Reset default directory

        // This command is used to finish adding a certificate.
        //read_bytes = BGLIB_RX_BYTES(BGLIB_RESP_19_BYTES, BGLIB_NO_EVT);
        wifi_cmd_x509_add_private_key_finish((uint8_t) strlen(R100_wifi.router_pass_key), R100_wifi.router_pass_key); 
        if (Wifi_Command_Response_Data (wifi_rsp_x509_add_private_key_finish_id, OSTIME_10SEC) == FALSE)
        {
            return eERR_COMM_WIFI_CERT_CLIENT_ROUTER_KEY;
        }
    }

    // This command is used to set the client certificate of an EAP type. // 0: CA certificate 1: User/client certificate
    read_bytes = BGLIB_RX_BYTES(BGLIB_RESP_2_BYTES, BGLIB_NO_EVT);
    wifi_cmd_sme_set_eap_type_user_certificate(TLS, 16, fingerprint);
    if (Wifi_Command_Response (wifi_rsp_sme_set_eap_type_user_certificate_id, read_bytes, OSTIME_3SEC) == FALSE)
    {
        return eERR_COMM_WIFI_CERT_EAP_CONFIG;
    }
    if(pck_resp->rsp_sme_set_eap_type_user_certificate.result != wifi_err_success)
    {
        return eERR_COMM_WIFI_CERT_EAP_CONFIG;
    }

    // This command is used to set the EAP configuration, which is used when authenticating with a secure Access Point.
    read_bytes = BGLIB_RX_BYTES(BGLIB_RESP_2_BYTES, BGLIB_NO_EVT);
    wifi_cmd_sme_set_eap_configuration(TLS, NONE, (uint8_t) strlen(R100_wifi.eap_cnf), R100_wifi.eap_cnf);
    if (Wifi_Command_Response (wifi_rsp_sme_set_eap_configuration_id, read_bytes, OSTIME_3SEC) == FALSE)
    {
        return eERR_COMM_WIFI_CERT_EAP_CONFIG;
    }
    if(pck_resp->rsp_sme_set_eap_configuration.result != wifi_err_success)
    {
        return eERR_COMM_WIFI_CERT_EAP_CONFIG;
    }

    fx_res = (uint8_t) fx_file_close (&wifi_file);
    fx_res = (uint8_t) fx_directory_default_set(&sd_fx_media, FX_NULL); // Reset default directory
    return fx_res;
}

/******************************************************************************
** Name:    Wifi_Load_EAP_PEAP_Cert
*****************************************************************************/
/**
** @brief   Load EAP certs to connect using PEAP-MSCHAPv2
**
** @param   none
**
** @return  ERROR_ID_e error code
******************************************************************************/
ERROR_ID_e Wifi_Load_EAP_PEAP_Cert(uint8_t peap)
{
    uint8_t fx_res = 0;
    uint8_t read_bytes = 0;

    if(peap == 0)
    {
        fx_res = Wifi_Load_EAP_CA_Cert();
        if(fx_res != eERR_NONE) return eERR_COMM_WIFI_INIT_OPEN;
    }

    read_bytes = BGLIB_RX_BYTES(BGLIB_RESP_2_BYTES, BGLIB_NO_EVT);

    // This command is used to set the user name of an EAP type
    wifi_cmd_sme_set_eap_type_username(MSCHAPv2, (uint8_t) strlen(R100_wifi.eap_cnf), R100_wifi.eap_cnf)
    if (Wifi_Command_Response (wifi_rsp_sme_set_eap_type_username_id, read_bytes, OSTIME_3SEC) == FALSE)
    {
        return eERR_COMM_WIFI_CERT_EAP_CONFIG;
    }
    if(pck_resp->rsp_sme_set_eap_configuration.result != wifi_err_success)
    {
        return eERR_COMM_WIFI_CERT_EAP_CONFIG;
    }

    // This command is used to set the password of an EAP type.
    wifi_cmd_sme_set_eap_type_password(MSCHAPv2, (uint8_t) strlen(R100_wifi.eap_pass), R100_wifi.eap_pass)
    if (Wifi_Command_Response (wifi_rsp_sme_set_eap_type_password_id, read_bytes, OSTIME_3SEC) == FALSE)
    {
        return eERR_COMM_WIFI_CERT_EAP_CONFIG;
    }
    if(pck_resp->rsp_sme_set_eap_configuration.result != wifi_err_success)
    {
        return eERR_COMM_WIFI_CERT_EAP_CONFIG;
    }

    // This command is used to set the EAP configuration, which is used when authenticating with a secure Access Point.
    wifi_cmd_sme_set_eap_configuration(PEAP, MSCHAPv2, (uint8_t) strlen(R100_wifi.eap_cnf), R100_wifi.eap_cnf);
    if (Wifi_Command_Response (wifi_rsp_sme_set_eap_configuration_id, read_bytes, OSTIME_3SEC) == FALSE)
    {
        return eERR_COMM_WIFI_CERT_EAP_CONFIG;
    }
    if(pck_resp->rsp_sme_set_eap_configuration.result != wifi_err_success)
    {
        return eERR_COMM_WIFI_CERT_EAP_CONFIG;
    }

    return eERR_NONE;
}

/******************************************************************************
** Name:    Wifi_Configure_TCPIP_DHCP_Settings
*****************************************************************************/
/**
** @brief   Configure IP settings of the device
**
** @param   dhcp    ON/OFF DHCP
**
** @return  ERROR_ID_e error code
******************************************************************************/
ERROR_ID_e    Wifi_Configure_TCPIP_DHCP_Settings(bool_t dhcp)
{
#define IP_ADDRESS_WIFI(a,b,c,d)    ((uint32_t)(a << 24)   + \
		                            (uint32_t)(b << 16)    +  \
                                    (uint32_t)(c << 8)     +  \
                                    (uint32_t)(d))

#define IP_BUILD(a,b,c)     (uint8_t)((a - '0') * 100 + (b - '0') * 10 + (c - '0'));

    uint32_t add;
    uint32_t net;
    uint32_t gate;
    uint8_t  read_bytes = 0;
    uint8_t ipadd1 = 0, ipadd2 = 0, ipadd3 = 0, ipadd4 = 0;
    uint8_t mask1 = 0, mask2 = 0, mask3 = 0, mask4 = 0;
    uint8_t gate1 = 0, gate2 = 0, gate3 = 0, gate4 = 0;

    if(dhcp == DHCP_OFF)
    {
        ipadd1 = IP_BUILD(R100_wifi.ip[0], R100_wifi.ip[1], R100_wifi.ip[2]);
        ipadd2 = IP_BUILD(R100_wifi.ip[4], R100_wifi.ip[5], R100_wifi.ip[6]);
        ipadd3 = IP_BUILD(R100_wifi.ip[8], R100_wifi.ip[9], R100_wifi.ip[10]);
        ipadd4 = IP_BUILD(R100_wifi.ip[12], R100_wifi.ip[13], R100_wifi.ip[14]);

        mask1 = IP_BUILD(R100_wifi.mask[0], R100_wifi.mask[1], R100_wifi.mask[2]);
        mask2 = IP_BUILD(R100_wifi.mask[4], R100_wifi.mask[5], R100_wifi.mask[6]);
        mask3 = IP_BUILD(R100_wifi.mask[8], R100_wifi.mask[9], R100_wifi.mask[10]);
        mask4 = IP_BUILD(R100_wifi.mask[12], R100_wifi.mask[13], R100_wifi.mask[14]);

        gate1 = IP_BUILD(R100_wifi.gateway[0], R100_wifi.gateway[1], R100_wifi.gateway[2]);
        gate2 = IP_BUILD(R100_wifi.gateway[4], R100_wifi.gateway[5], R100_wifi.gateway[6]);
        gate3 = IP_BUILD(R100_wifi.gateway[8], R100_wifi.gateway[9], R100_wifi.gateway[10]);
        gate4 = IP_BUILD(R100_wifi.gateway[12], R100_wifi.gateway[13], R100_wifi.gateway[14]);
    }

    add  = IP_ADDRESS_WIFI(ipadd4,ipadd3,ipadd2,ipadd1);
    net  = IP_ADDRESS_WIFI(mask4,mask3,mask2,mask1);
    gate = IP_ADDRESS_WIFI(gate4,gate3,gate2,gate1);

    read_bytes = BGLIB_RX_BYTES(BGLIB_RESP_2_BYTES,BGLIB_EVT_TCPIP_CONF);

    // This command is used to configure IP settings of the device
    if(dhcp == DHCP_OFF)
    {
        wifi_cmd_tcpip_configure(add, net, gate, DHCP_OFF);
    }
    else
    {
        wifi_cmd_tcpip_configure(add, net, gate, DHCP_ON);
    }
    if (Wifi_Command_Response (wifi_rsp_tcpip_configure_id, read_bytes, OSTIME_3SEC) == FALSE)
    {
        return eERR_COMM_WIFI_INIT;
    }

    /*if(pck_resp->rsp_tcpip_configure.result != wifi_err_success)
    {
        return eERR_COMM_WIFI_INIT;
    }*/


    /*if(pck_evt[0]->evt_tcpip_configuration.use_dhcp == DHCP_ON)
    {
        return eERR_COMM_WIFI_INIT;
    }*/

    return eERR_NONE;
}

/******************************************************************************
** Name:    Wifi_Configure_DNS_Settings
*****************************************************************************/
/**
** @brief   Configure DNS settings of the device
**
** @param   none
**
** @return  ERROR_ID_e error code
******************************************************************************/
ERROR_ID_e    Wifi_Configure_DNS_Settings(void)
{
#define IP_ADDRESS_WIFI(a,b,c,d)    ((uint32_t)(a << 24)   + \
                                    (uint32_t)(b << 16)    +  \
                                    (uint32_t)(c << 8)     +  \
                                    (uint32_t)(d))

#define IP_BUILD(a,b,c)     (uint8_t)((a - '0') * 100 + (b - '0') * 10 + (c - '0'));

#define PRIMARY_DNS     0
#define SECONDARY_DNS   1

    uint32_t p_dns;
    uint32_t s_dns;

    uint8_t  read_bytes = 0;
    uint8_t p_dns1, p_dns2, p_dns3, p_dns4;
    uint8_t s_dns1, s_dns2, s_dns3, s_dns4;

    p_dns1 = IP_BUILD(R100_wifi.pdns[0], R100_wifi.pdns[1], R100_wifi.pdns[2]);
    p_dns2 = IP_BUILD(R100_wifi.pdns[4], R100_wifi.pdns[5], R100_wifi.pdns[6]);
    p_dns3 = IP_BUILD(R100_wifi.pdns[8], R100_wifi.pdns[9], R100_wifi.pdns[10]);
    p_dns4 = IP_BUILD(R100_wifi.pdns[12], R100_wifi.pdns[13], R100_wifi.pdns[14]);

    s_dns1 = IP_BUILD(R100_wifi.sdns[0], R100_wifi.sdns[1], R100_wifi.sdns[2]);
    s_dns2 = IP_BUILD(R100_wifi.sdns[4], R100_wifi.sdns[5], R100_wifi.sdns[6]);
    s_dns3 = IP_BUILD(R100_wifi.sdns[8], R100_wifi.sdns[9], R100_wifi.sdns[10]);
    s_dns4 = IP_BUILD(R100_wifi.sdns[12], R100_wifi.sdns[13], R100_wifi.sdns[14]);


    p_dns  = IP_ADDRESS_WIFI(p_dns1,p_dns2,p_dns3,p_dns4);
    s_dns  = IP_ADDRESS_WIFI(s_dns1,s_dns2,s_dns3,s_dns4);

    read_bytes = BGLIB_RX_BYTES(BGLIB_RESP_2_BYTES,BGLIB_EVT_DNS);

    // This command is used to configure IP settings of the device
    wifi_cmd_tcpip_dns_configure(PRIMARY_DNS, p_dns);

    if (Wifi_Command_Response (wifi_rsp_tcpip_dns_configure_id, read_bytes, OSTIME_3SEC) == FALSE)
    {
        return eERR_COMM_WIFI_INIT;
    }

    /*if(pck_resp->rsp_tcpip_dns_configure.result != wifi_err_success)
    {
        return eERR_COMM_WIFI_INIT;
    }*/

    /*if(pck_evt[0]->evt_tcpip_dns_configuration.index == PRIMARY_DNS)
    {
        return eERR_COMM_WIFI_INIT;
    }*/

    // This command is used to configure IP settings of the device
    wifi_cmd_tcpip_dns_configure(SECONDARY_DNS, s_dns);

    if (Wifi_Command_Response (wifi_rsp_tcpip_dns_configure_id, read_bytes, OSTIME_3SEC) == FALSE)
    {
        return eERR_COMM_WIFI_INIT;
    }

    /*if(pck_resp->rsp_tcpip_dns_configure.result != wifi_err_success)
    {
        return eERR_COMM_WIFI_INIT;
    }*/

    /*if(pck_evt[0]->evt_tcpip_dns_configuration.index == SECONDARY_DNS)
    {
        return eERR_COMM_WIFI_INIT;
    }*/

    return eERR_NONE;
}

/******************************************************************************
** Name:    Wifi_Set_Password
*****************************************************************************/
/**
** @brief   Set password to connection
**
** @param   none
**
** @return  ERROR_ID_e error code
******************************************************************************/
ERROR_ID_e Wifi_Set_Password(uint8_t eap)
{
    uint8_t read_bytes = 0;

    if(eap == eWIFI_WPA)
    {
        read_bytes = BGLIB_RX_BYTES(BGLIB_RESP_1_BYTE, BGLIB_NO_EVT);

        // This command is used to set the password when authenticating with a secure Access Point
        wifi_cmd_sme_set_password((uint8_t) strlen(R100_wifi.wlan_pass), R100_wifi.wlan_pass);

        if (Wifi_Command_Response (wifi_rsp_sme_set_password_id, read_bytes, OSTIME_10SEC) == FALSE)
        {
            return eERR_COMM_WIFI_INIT;
        }

        if(pck_resp->rsp_sme_set_password.status != wifi_err_success)
        {
            return eERR_COMM_WIFI_INIT;
        }
    }
    return eERR_NONE;
}

/******************************************************************************
** Name:    Wifi_Connect_Network
*****************************************************************************/
/**
** @brief   Start a connection establishment procedure with an Access Point
**
** @param   none
**
** @return  ERROR_ID_e error code
******************************************************************************/
ERROR_ID_e Wifi_Connect_Network(void)
{
    uint8_t read_bytes = 0;
    //read_bytes = BGLIB_RX_BYTES(9,12);
    //read_bytes = BGLIB_RX_BYTES(9, 12 + 17);
    //read_bytes = BGLIB_RX_BYTES(BGLIB_RESP_9_BYTES, (BGLIB_EVT_CONNECTED + BGLIB_EVT_TCPIP_CONF + 25));
    read_bytes = 66; // read_bytes = 76;

    // This command is used to start a connection establishment procedure with an Access Point with the given SSID
    wifi_cmd_sme_connect_ssid((uint8_t) strlen(R100_wifi.wlan_ssid), R100_wifi.wlan_ssid);
    if (Wifi_Command_Response (wifi_rsp_sme_connect_ssid_id, read_bytes, OSTIME_20SEC) == FALSE)
    {
        //return eERR_COMM_WIFI_INIT;
        wifi_cmd_sme_connect_ssid((uint8_t) strlen(R100_wifi.wlan_ssid), R100_wifi.wlan_ssid);
        if (Wifi_Command_Response (wifi_rsp_sme_connect_ssid_id, read_bytes, OSTIME_20SEC) == FALSE)
        {
            return eERR_COMM_WIFI_INIT;
        }
    }
    if(pck_resp->rsp_sme_connect_ssid.result != wifi_err_success)
    {
        return eERR_COMM_WIFI_INIT;
    }

    if((pck_evt[0]->evt_sme_connected.status == wifi_err_success && pck_evt[0]->evt_sme_connected.hw_interface == wifi_err_success) ||
       (pck_evt[1]->evt_sme_connected.status == wifi_err_success && pck_evt[1]->evt_sme_connected.hw_interface == wifi_err_success) ||
       (pck_evt[2]->evt_sme_connected.status == wifi_err_success && pck_evt[2]->evt_sme_connected.hw_interface == wifi_err_success) ||
       (pck_evt[3]->evt_sme_connected.status == wifi_err_success && pck_evt[3]->evt_sme_connected.hw_interface == wifi_err_success))
    {
        return eERR_NONE;
    }
    else return eERR_COMM_WIFI_INIT;

    //ip_address = pck_evt_2->evt_tcpip_configuration.address;
    //Wifi_Save_IPv4_Address(); Iñigo WIFI

    /*if(Comm_Wifi_Is_IPv4_Valid(wifi.ip_address) == FALSE)
    {
        return eERR_COMM_WIFI_INIT_GET_IP_ADDRESS;
    }*/

    // wait a little time
    //(void) tx_thread_sleep(OSTIME_50MSEC);

    return eERR_NONE;
}

/******************************************************************************
** Name:    Wifi_Get_RSSI
*****************************************************************************/
/**
** @brief   The received signal strength indication (RSSI) in dBm units.
**          RSSI: < -30 dBm = Very good; -40 / -60 dBm = Good; -70 dBm == Weak; -80 dBm = Poor; -90 dBm = No connection;
**          SNR (Signal to Noise Ratio): 10 - 24 dB = Poor; 25 - 40 dB = Good; > 41 dB = Very Good;
**
** @param   none
**
** @return  ERROR_ID_e error code
******************************************************************************/
ERROR_ID_e Wifi_Get_RSSI(void)
{
    uint8_t read_bytes = 0;

    // This command is used to get a value indicating the signal quality of the connection.
    read_bytes = BGLIB_RX_BYTES(BGLIB_RESP_3_BYTES, 6);
    wifi_cmd_sme_get_signal_quality();
    if (Wifi_Command_Response (wifi_rsp_sme_get_signal_quality_id, read_bytes, OSTIME_10SEC) == FALSE)
    {
        //return eERR_COMM_WIFI_INIT;
    }
    R100_wifi.signal_rssi = pck_evt[0]->evt_sme_signal_quality.rssi;
    Trace_Arg (TRACE_NO_FLAGS | TRACE_NEWLINE, " RSSI: %d", (uint32_t)R100_wifi.signal_rssi);

    return eERR_NONE;
}

/******************************************************************************
** Name:    Wifi_Disconnect_Network
*****************************************************************************/
/**
** @brief   Disconnect from the currently connected Access Point
**
** @param   none
**
** @return  ERROR_ID_e error code
******************************************************************************/
ERROR_ID_e Wifi_Disconnect_Network(void)
{
    // This command is used to cancel an ongoing connection attempt or disconnect from the currently connected Access Point
    wifi_cmd_sme_disconnect();
    if (Wifi_Command_Response_Data (wifi_rsp_sme_disconnect_id, OSTIME_1SEC) == FALSE)
    {
        return eERR_COMM_WIFI_INIT;
    }

    /*if(pck->rsp_sme_disconnect.result != wifi_err_success)
    {
        return eERR_COMM_WIFI_INIT;
    }

    if(pck->evt_sme_disconnected.reason != wifi_err_success)
    {
        return eERR_COMM_WIFI_INIT;
    }*/

    (void) tx_thread_sleep(OSTIME_100MSEC);

    return eERR_NONE;
}

/******************************************************************************
** Name:    Wifi_Module_Reset
*****************************************************************************/
/**
** @brief   Perform Wifi module software commanded Reset
**
** @param   none
**
** @return  ERROR_ID_e error code
******************************************************************************/
ERROR_ID_e Wifi_Module_Reset(void)
{
    // This command is used to reset the device
    wifi_cmd_system_reset(0);
    //wifi_cmd_dfu_reset(0);
    /* Command does not have a response */
    (void) tx_thread_sleep(OSTIME_5SEC);

    return eERR_NONE;
}

/******************************************************************************
** Name:    Wifi_Open_TCP_Client
*****************************************************************************/
/**
** @brief   Open the tcp endpoint client connection
**
** @param   none
**
** @return  ERROR_ID_e error code
******************************************************************************/
ERROR_ID_e Wifi_Open_TCP_Client(void)
{
    uint8_t  read_bytes = 0;
    uint8_t  ip[4];

    if (sscanf (R100_wifi.host_name, "%u.%u.%u.%u",
               (unsigned int *) &ip[0],
               (unsigned int *) &ip[1],
               (unsigned int *) &ip[2],
               (unsigned int *) &ip[3]) != 4)
    {
       return eERR_COMM_WIFI_OPEN_TCP;
    }

    dest_ip = (uint32_t) (ip[3] << 24 | ip[2] << 16 | ip[1] << 8 | ip[0]);

    //dest_port = 12345;
    read_bytes = BGLIB_RX_BYTES(BGLIB_RESP_3_BYTES, (BGLIB_EVT_TCPIP_ENDP_STS + BGLIB_EVT_ENDP_STS + BGLIB_EVT_TCPIP_ENDP_STS + BGLIB_EVT_ENDP_STS));

    // This command is used to create a new TCP connection to a TCP server
    wifi_cmd_tcpip_tcp_connect(dest_ip, dest_port, -1);

    if (Wifi_Command_Response (wifi_rsp_tcpip_tcp_connect_id, read_bytes, OSTIME_10SEC) == FALSE)
    {
        return eERR_COMM_WIFI_OPEN_TCP;
    }

    if(pck_resp->rsp_tcpip_tcp_connect.result != wifi_err_success)
    {
        return eERR_COMM_WIFI_OPEN_TCP;
    }

    tcp_endpoint = pck_resp->rsp_tcpip_tcp_connect.endpoint;

    /*if(pck_evt[3]->evt_endpoint_status.active == TRUE)
    {

    }*/

    return eERR_NONE;
}

/******************************************************************************
** Name:    Wifi_Close_TCP_Client
*****************************************************************************/
/**
** @brief   Close the tcp endpoint client connection
**
** @param   none
**
** @return  ERROR_ID_e error code
******************************************************************************/
ERROR_ID_e Wifi_Close_TCP_Client(void)
{
    uint8_t read_bytes = 0;

    //read_bytes = BGLIB_RX_BYTES(BGLIB_RESP_3_BYTES, BGLIB_NO_EVT);
    read_bytes = BGLIB_RX_BYTES(BGLIB_RESP_3_BYTES, BGLIB_EVT_ENDP_STS);

    // This command is used to close a protocol endpoint
    wifi_cmd_endpoint_close(tcp_endpoint);

    if (Wifi_Command_Response (wifi_rsp_endpoint_close_id, read_bytes, OSTIME_10SEC) == FALSE)
    {
        return eERR_COMM_WIFI_CLOSE_TCP;
    }

    if(pck_resp->rsp_endpoint_close.result != wifi_err_success)
    {
        return eERR_COMM_WIFI_CLOSE_TCP;
    }

    if((pck_evt[0]->evt_endpoint_status.endpoint == tcp_endpoint) &&
       (pck_evt[0]->evt_endpoint_status.active != 0))
    {
        return eERR_COMM_WIFI_CLOSE_TCP;
    }

    return eERR_NONE;
}

/******************************************************************************
** Name:    Wifi_Open_UDP_Client
*****************************************************************************/
/**
** @brief   Open the udp endpoint client connection
**
** @param   none
**
** @return  ERROR_ID_e error code
******************************************************************************/
ERROR_ID_e Wifi_Open_UDP_Client(void)
{
    uint8_t read_bytes = 0;
    uint8_t ip[4];

    if (sscanf (R100_wifi.host_name, "%u.%u.%u.%u",
               (unsigned int *) &ip[0],
               (unsigned int *) &ip[1],
               (unsigned int *) &ip[2],
               (unsigned int *) &ip[3]) != 4)
    {
       return eERR_COMM_WIFI_OPEN_UDP;
    }

    dest_ip = (uint32_t) (ip[3] << 24 | ip[2] << 16 | ip[1] << 8 | ip[0]);

    dest_port = 10000;

    read_bytes = BGLIB_RX_BYTES(BGLIB_RESP_3_BYTES, (BGLIB_EVT_TCPIP_ENDP_STS + BGLIB_EVT_ENDP_STS));

    // This command is used to create a new UDP connection
    wifi_cmd_tcpip_udp_connect(dest_ip, dest_port, -1);

    if (Wifi_Command_Response (wifi_rsp_tcpip_udp_connect_id, read_bytes, OSTIME_10SEC) == FALSE)
    {
        return eERR_COMM_WIFI_OPEN_UDP;
    }

    if(pck_resp->rsp_tcpip_udp_connect.result != wifi_err_success)
    {
        return eERR_COMM_WIFI_OPEN_UDP;
    }

    udp_client_endpoint = pck_resp->rsp_tcpip_udp_connect.endpoint;

    udp_local_port = pck_evt[0]->evt_tcpip_endpoint_status.local_port;

    /*if(pck_evt[1]->evt_endpoint_status.active == TRUE)
    {

    }*/

    return eERR_NONE;
}

/******************************************************************************
** Name:    Wifi_Close_UDP_Client
*****************************************************************************/
/**
** @brief   Close the udp endpoint client connection
**
** @param   none
**
** @return  ERROR_ID_e error code
******************************************************************************/
ERROR_ID_e Wifi_Close_UDP_Client(void)
{
    uint8_t read_bytes = 0;

    read_bytes = BGLIB_RX_BYTES(BGLIB_RESP_3_BYTES, BGLIB_EVT_ENDP_STS);

    // This command is used to close a protocol endpoint
    wifi_cmd_endpoint_close(udp_client_endpoint);

    if (Wifi_Command_Response (wifi_rsp_endpoint_close_id, read_bytes, OSTIME_10SEC) == FALSE)
    {
        return eERR_COMM_WIFI_CLOSE_UDP;
    }

    if(pck_resp->rsp_endpoint_close.result != wifi_err_success)
    {
        return eERR_COMM_WIFI_CLOSE_UDP;
    }

    if((pck_evt[0]->evt_endpoint_status.endpoint == udp_client_endpoint) &&
       (pck_evt[0]->evt_endpoint_status.active != 0))
    {
        return eERR_COMM_WIFI_CLOSE_UDP;
    }

    return eERR_NONE;
}

/******************************************************************************
** Name:    Wifi_Start_UDP_Server
*****************************************************************************/
/**
** @brief   Start the udp server
**
** @param   none
**
** @return  ERROR_ID_e error code
******************************************************************************/
ERROR_ID_e Wifi_Start_UDP_Server(void)
{
    uint8_t read_bytes = 0;

    read_bytes = BGLIB_RX_BYTES(BGLIB_RESP_3_BYTES, (BGLIB_EVT_TCPIP_ENDP_STS + BGLIB_EVT_ENDP_STS));

    // Start UDP Server to receive data
    wifi_cmd_tcpip_start_udp_server(udp_local_port, -1);

    if (Wifi_Command_Response (wifi_rsp_tcpip_start_udp_server_id, read_bytes, OSTIME_10SEC) == FALSE)
    {
        return eERR_COMM_WIFI_OPEN_UDP_SERVER;
    }

    udp_server_endpoint = pck_resp->rsp_tcpip_start_udp_server.endpoint;

    /*if(pck_evt[1]->evt_endpoint_status.active == TRUE)
    {

    }*/

    return eERR_NONE;
}

/******************************************************************************
** Name:    Wifi_Close_UDP_Server
*****************************************************************************/
/**
** @brief   Close the udp server
**
** @param   none
**
** @return  ERROR_ID_e error code
******************************************************************************/
ERROR_ID_e Wifi_Close_UDP_Server(void)
{
    uint8_t read_bytes = 0;

    read_bytes = BGLIB_RX_BYTES(BGLIB_RESP_3_BYTES, BGLIB_EVT_ENDP_STS);

    // This command is used to close a protocol endpoint
    wifi_cmd_endpoint_close(udp_server_endpoint);

    if (Wifi_Command_Response (wifi_rsp_endpoint_close_id, read_bytes, OSTIME_10SEC) == FALSE)
    {
        return eERR_COMM_WIFI_CLOSE_UDP_SERVER;
    }

    if(pck_resp->rsp_endpoint_close.result != wifi_err_success)
    {
        return eERR_COMM_WIFI_CLOSE_UDP_SERVER;
    }

    if((pck_evt[0]->evt_endpoint_status.endpoint == udp_server_endpoint) &&
       (pck_evt[0]->evt_endpoint_status.active != 0))
    {
        return eERR_COMM_WIFI_CLOSE_UDP_SERVER;
    }

    return eERR_NONE;
}

/******************************************************************************
** Name:    Wifi_Open_TLS_Client
*****************************************************************************/
/**
** @brief   Open the tls endpoint client connection
**
** @param   none
**
** @return  ERROR_ID_e error code
******************************************************************************/
ERROR_ID_e Wifi_Open_TLS_Client(void)
{
    uint8_t  read_bytes = 0;
    //uint8_t  ip[4];
    ipv4 ip_address;

    read_bytes = (uint8_t)BGLIB_RX_BYTES(BGLIB_RESP_2_BYTES,(BGLIB_EVT_TCPIP_DNS_HOSTNAME + (uint8_t)strlen (R100_wifi.host_name)));
    wifi_cmd_tcpip_dns_gethostbyname((uint8_t)strlen (R100_wifi.host_name), R100_wifi.host_name);

    if (Wifi_Command_Response (wifi_rsp_tcpip_dns_gethostbyname_id, read_bytes, OSTIME_3SEC) == FALSE)
    {
        return eERR_COMM_WIFI_OPEN_TLS;
    }
    if(pck_resp->rsp_tcpip_dns_gethostbyname.result != wifi_err_success)
    {
        return eERR_COMM_WIFI_OPEN_TLS;
    }
    ip_address = pck_evt[0]->evt_tcpip_dns_gethostbyname_result.address;

    /*if (sscanf (R100_wifi.host_name, "%u.%u.%u.%u",
                (unsigned int *) &ip[0],
                (unsigned int *) &ip[1],
                (unsigned int *) &ip[2],
                (unsigned int *) &ip[3]) != 4)
    {
       return eERR_COMM_WIFI_OPEN_TLS;
    }*/

    //dest_ip = (uint32_t) (ip[3] << 24 | ip[2] << 16 | ip[1] << 8 | ip[0]);

    dest_ip = (uint32_t) (ip_address.a[3] << 24 | ip_address.a[2] << 16 | ip_address.a[1] << 8 | ip_address.a[0]);

    read_bytes = BGLIB_RX_BYTES(BGLIB_RESP_NO_BYTES,BGLIB_NO_EVT);

    wifi_cmd_tcpip_tls_set_authmode(2); //2: mandatory verification, verification failure causes connection to fail

    if (Wifi_Command_Response (wifi_rsp_tcpip_tls_set_authmode_id, read_bytes, OSTIME_3SEC) == FALSE)
    {
        (void) tx_thread_sleep (OSTIME_1SEC);  // wait a little time
        if (Wifi_Command_Response (wifi_rsp_tcpip_tls_set_authmode_id, read_bytes, OSTIME_3SEC) == FALSE)
        {
            return eERR_COMM_WIFI_OPEN_TLS;
        }
    }

    read_bytes = BGLIB_RX_BYTES(BGLIB_RESP_3_BYTES, (BGLIB_EVT_TCPIP_ENDP_STS + BGLIB_EVT_ENDP_STS + BGLIB_EVT_TCPIP_ENDP_STS + BGLIB_EVT_ENDP_STS + BGLIB_EVT_TLS_VERIFY));

    // This command is used to create a new TLS connection to a TLS server
    wifi_cmd_tcpip_tls_connect(dest_ip, dest_port, -1);

    if (Wifi_Command_Response_Data (wifi_rsp_tcpip_tls_connect_id, OSTIME_10SEC) == FALSE)
    {
        (void) tx_thread_sleep (OSTIME_1SEC);  // wait a little time
        if (Wifi_Command_Response_Data (wifi_rsp_tcpip_tls_connect_id, OSTIME_10SEC) == FALSE)
        {
            return eERR_COMM_WIFI_OPEN_TLS;
        }
    }

    if(pck_resp->rsp_tcpip_tls_connect.result != wifi_err_success)
    {
        return eERR_COMM_WIFI_OPEN_TLS;
    }

    // This command is used to activate or deactivate endpoints. 0: Desactivate 1: Activate
    read_bytes = BGLIB_RX_BYTES(BGLIB_RESP_3_BYTES, BGLIB_EVT_TCPIP_ENDP_STS + BGLIB_EVT_ENDP_STS);
    wifi_cmd_endpoint_set_active(pck_resp->rsp_tcpip_tls_connect.endpoint, 1);
    if (Wifi_Command_Response (wifi_rsp_endpoint_set_active_id, read_bytes, OSTIME_10SEC) == FALSE)
    {
        return eERR_COMM_WIFI_OPEN_TLS;
    }

    tcp_endpoint = pck_resp->rsp_tcpip_tls_connect.endpoint;

    /*if(pck_evt[3]->evt_endpoint_status.active == TRUE)
    {

    }*/

    return eERR_NONE;
}

/******************************************************************************
** Name:    Wifi_Close_TLS_Client
*****************************************************************************/
/**
** @brief   Close the tls endpoint client connection
**
** @param   none
**
** @return  ERROR_ID_e error code
******************************************************************************/
ERROR_ID_e Wifi_Close_TLS_Client(void)
{
    uint8_t read_bytes = 0;

    //read_bytes = BGLIB_RX_BYTES(BGLIB_RESP_3_BYTES, BGLIB_NO_EVT);
    read_bytes = BGLIB_RX_BYTES(BGLIB_RESP_3_BYTES, BGLIB_EVT_ENDP_STS);

    // This command is used to close a protocol endpoint
    wifi_cmd_endpoint_close(tcp_endpoint);

    if (Wifi_Command_Response (wifi_rsp_endpoint_close_id, read_bytes, OSTIME_10SEC) == FALSE)
    {
        return eERR_COMM_WIFI_CLOSE_TLS;
    }

    if(pck_resp->rsp_endpoint_close.result != wifi_err_success)
    {
        return eERR_COMM_WIFI_CLOSE_TLS;
    }

    /*if((pck_evt[0]->evt_endpoint_status.endpoint == tcp_endpoint) &&
       (pck_evt[0]->evt_endpoint_status.active != 0))
    {
        return eERR_COMM_WIFI_CLOSE_TLS;
    }*/

    return eERR_NONE;
}

/******************************************************************************
** Name:    Wifi_Is_Host_Alive
*****************************************************************************/
/**
** @brief   Check if host server is alive
**
** @param   none
**
** @return  ERROR_ID_e error code
******************************************************************************/
ERROR_ID_e Wifi_Is_Host_Alive(void)
{
    char        recv_resp[10];
    uint8_t     read_bytes = 0;
    ERROR_ID_e  retError = eERR_NONE;
    //char_t     sn[RSC_NAME_SZ] = {0};     ///< Device serial number
    uint8_t     wifi_header_data[MAX_WIFI_HEADER_BYTES] = {0};

    // Initialize and reset header
    memset (recv_resp, 0, sizeof(recv_resp));
    memset(endpoint_data, 0, MAX_WIFI_DATA_TX_SZ);

    // destination port of the host alive service
    dest_port = PORT_HOST_ALIVE;

    // Open TCP connection
    if(retError = Wifi_Open_TLS_Client(), retError != eERR_NONE)
    {
        if(retError = Wifi_Disconnect_Network(), retError != eERR_NONE) return retError;
        if(retError = Wifi_Connect_Network(), retError != eERR_NONE) return retError;
        if(retError = Wifi_Open_TLS_Client(), retError != eERR_NONE) return retError;
    }

    if(pck_evt[1]->evt_endpoint_status.active == TRUE) //With TCP use pck_evt[2]
    {
        //read_bytes = BGLIB_RX_BYTES(BGLIB_RESP_3_BYTES, (BGLIB_EVT_ENDP_DATA + strlen(SERVER_RESP_OK)));
        //read_bytes = BGLIB_RX_BYTES(BGLIB_RESP_3_BYTES, (6 + strlen(SERVER_RESP_OK)));
        read_bytes = BGLIB_RX_BYTES(BGLIB_RESP_3_BYTES, (BGLIB_EVT_ENDP_DATA_6 + BGLIB_EVT_ENDP_DATA_2));
        //read_bytes = BGLIB_RX_BYTES(BGLIB_RESP_3_BYTES, (6 + 8 + 8 + 11));
        //memcpy(&sn, Get_Device_Info()->sn, RSC_NAME_SZ);

        // Fill Wifi Header
        Wifi_Fill_Header(WIFI_FILE_FRAME, 0, 0);

        memcpy(wifi_header_data, (uint8_t *) &wifi_header, sizeof(WIFI_HEADER_t));
        memcpy(&endpoint_data[0], wifi_header_data, MAX_WIFI_HEADER_BYTES);

        // This command is used to send data to a given endpoint
        //wifi_cmd_endpoint_send(tcp_endpoint, (uint8_t) strlen("OK"), "OK");
        //wifi_cmd_endpoint_send(tcp_endpoint, RSC_NAME_SZ, sn);
        wifi_cmd_endpoint_send(tcp_endpoint, MAX_WIFI_HEADER_BYTES, endpoint_data);

        if (Wifi_Command_Response (wifi_rsp_endpoint_send_id, read_bytes, OSTIME_3SEC) == FALSE)
        {
            return eERR_COMM_WIFI_SERVER_ALIVE;
        }

        if(pck_resp->rsp_endpoint_send.result != wifi_err_success)
        {
            wifi_cmd_endpoint_send(tcp_endpoint, MAX_WIFI_HEADER_BYTES, endpoint_data);
            if (Wifi_Command_Response (wifi_rsp_endpoint_send_id, BGLIB_EVT_ENDP_DATA, OSTIME_3SEC) == FALSE)
            {
                return eERR_COMM_WIFI_SERVER_ALIVE;
            }
            if(pck_resp->rsp_endpoint_send.result != wifi_err_success)
            {
                return eERR_COMM_WIFI_SERVER_ALIVE;
            }
        }

        //pck_evt[0]->evt_endpoint_data.endpoint;
        if(pck_evt[0]->evt_endpoint_data.data.len != 0)
        {
            //pck_evt[0]->evt_endpoint_data.data.data;

            memcpy(recv_resp, pck_evt[0]->evt_endpoint_data.data.data, pck_evt[0]->evt_endpoint_data.data.len);

            // The server responds if the CFG and SREC are available
            Comm_Wifi_Save_Host_Response(recv_resp);

            // Before closing the session, an OK is received / sent to know that the communication has been correct.
            //read_bytes = BGLIB_RX_BYTES(BGLIB_RESP_3_BYTES, (BGLIB_EVT_ENDP_DATA_6 + BGLIB_EVT_ENDP_DATA_5 + BGLIB_EVT_RESPONSE));
            wifi_cmd_endpoint_send(tcp_endpoint, (uint8_t) strlen("OK"), "OK");
            if (Wifi_Command_Response (wifi_rsp_endpoint_send_id, BGLIB_EVT_ENDP_STS, OSTIME_1SEC) == FALSE)
            {
                return eERR_COMM_WIFI_SERVER_ALIVE;
            }
            if(pck_resp->rsp_endpoint_send.result != wifi_err_success)
            {
                return eERR_COMM_WIFI_SERVER_ALIVE;
            }

            /*if(strcmp(recv_resp, SERVER_RESP_OK) != 0)
            {
                return eERR_COMM_WIFI_SERVER_ALIVE;
            }*/

            // Close TCP connection
            retError = Wifi_Close_TLS_Client();

            return retError;
        }
        else
        {
            return eERR_COMM_WIFI_SERVER_ALIVE;
        }
    }
    else
    {
        return eERR_COMM_WIFI_SERVER_ALIVE;
    }
}

/******************************************************************************
** Name:    Wifi_Send_Data_Frame
*****************************************************************************/
/**
** @brief   Send Data Frame using Wifi connection (for Sigfox data frames)
**
** @param   none
**
** @return  ERROR_ID_e error code
******************************************************************************/
ERROR_ID_e Wifi_Send_Data_Frame(uint8_t* data, uint8_t size_data, uint8_t frame_type)
{
    char        recv_resp[2];
    uint16_t    crc_16 = 0;
    uint8_t   read_bytes = 0;
    ERROR_ID_e  retError = eERR_NONE;
    uint8_t     wifi_header_data[MAX_WIFI_HEADER_BYTES] = {0};

    // Initialize and reset header
    memset (recv_resp, 0, sizeof(recv_resp));
    memset(endpoint_data, 0, MAX_WIFI_DATA_TX_SZ);

    // destination port of the data Frame send service
    dest_port = PORT_DATA_FRAME;

    // Open TCP connection
    if(retError = Wifi_Open_TLS_Client(), retError != eERR_NONE)
    {
        if(retError = Wifi_Disconnect_Network(), retError != eERR_NONE) return retError;
        if(retError = Wifi_Connect_Network(), retError != eERR_NONE) return retError;
        if(retError = Wifi_Open_TLS_Client(), retError != eERR_NONE) return retError;
    }

    if(pck_evt[1]->evt_endpoint_status.active == TRUE)
    {
        crc_16 = Get_Checksum_Add_16(data, MAX_WIFI_DATA_TEST_SZ, crc_16);

        // Fill Wifi Header
        Wifi_Fill_Header(frame_type, 0 , crc_16);

        memcpy(wifi_header_data, (uint8_t *) &wifi_header, sizeof(WIFI_HEADER_t));
        memcpy(&endpoint_data[0], wifi_header_data, MAX_WIFI_HEADER_BYTES);
        memcpy(&endpoint_data[MAX_WIFI_HEADER_BYTES], data, size_data);

        read_bytes = BGLIB_RX_BYTES(BGLIB_RESP_3_BYTES, BGLIB_EVT_ENDP_DATA_6 + BGLIB_EVT_ENDP_DATA_2);

        // This command is used to send data to a given endpoint
        wifi_cmd_endpoint_send(tcp_endpoint, (uint8_t)(MAX_WIFI_HEADER_BYTES+size_data), endpoint_data);

        if (Wifi_Command_Response (wifi_rsp_endpoint_send_id, read_bytes, OSTIME_3SEC) == FALSE)
        {
            return eERR_COMM_WIFI_SEND_DATA;
        }

        if(pck_resp->rsp_endpoint_send.result != wifi_err_success)
        {
            wifi_cmd_endpoint_send(tcp_endpoint, (uint8_t)(MAX_WIFI_HEADER_BYTES+size_data), endpoint_data);
            if (Wifi_Command_Response (wifi_rsp_endpoint_send_id, read_bytes, OSTIME_3SEC) == FALSE)
            {
                return eERR_COMM_WIFI_SEND_DATA;
            }
            if(pck_resp->rsp_endpoint_send.result != wifi_err_success)
            {
                return eERR_COMM_WIFI_SEND_DATA;
            }
        }

        if(pck_evt[0]->evt_endpoint_data.data.len != 0)
        {
            //pck_evt[0]->evt_endpoint_data.data.data;

            memcpy(recv_resp, pck_evt[0]->evt_endpoint_data.data.data, pck_evt[0]->evt_endpoint_data.data.len);

            if(strncmp(recv_resp, SERVER_RESP_ER, 2) == 0)
            {
                return eRR_COMM_WIFI_SERVER_RPS_ER;
            }
            if(strncmp(recv_resp, SERVER_RESP_OK, 2) != 0)
            {
                return eERR_COMM_WIFI_SEND_DATA;
            }

            // Close TCP connection
            retError = Wifi_Close_TLS_Client();

            return retError;
        }
        else
        {
            return eERR_COMM_WIFI_SEND_DATA;
        }
    }
    else
    {
        return eERR_COMM_WIFI_SEND_DATA;
    }
}

/******************************************************************************
** Name:    Wifi_Send_Test_Result
******************************************************************************/
/**
** @brief   Check if an error has occurred and reset directory.
**
** @param   none
**
** @return  ERROR_ID_e error code
******************************************************************************/
ERROR_ID_e Wifi_Send_Test_Result(void)
{
    ERROR_ID_e  retError = eERR_NONE;

    retError = Wifi_Send_Test();
    fx_directory_default_set(&sd_fx_media, FX_NULL); // Reset default directory

    return retError;
}

/******************************************************************************
** Name:    Wifi_Send_Test
*****************************************************************************/
/**
** @brief   Send a File from a uSD to the Server
**
** @param   none
**
** @return  ERROR_ID_e error code
******************************************************************************/
ERROR_ID_e Wifi_Send_Test(void)
{
    //FX_FILE  test_file;
    char_t  fname[32];
    ERROR_ID_e  retError = eERR_NONE;
    uint8_t     fx_res, tries_cnt=0;
    //uint8_t   read_bytes = 0;
    char_t      read_file_data[255]= {0}, recv_resp[2] = {0};
    uint32_t    frameId = 0, nFrame = 0, nBytes = 0, i = 0;
    uint64_t    file_size = 0, file_size_ptr = 0;
    uint8_t     wifi_header_data[MAX_WIFI_HEADER_BYTES] = {0};
    uint16_t    crc_16 = 0, aux_id_payload = 0;

    // Initialize and reset header
    memset (recv_resp, 0, sizeof(recv_resp));
    memset ((uint8_t *) &wifi_payload, 0, sizeof(WIFI_PAYLOAD_t));
    memset(endpoint_data, 0, MAX_WIFI_DATA_TX_SZ);

    // destination port of the Test send service
    dest_port = PORT_TEST;

    // Open TCP connection
    if(retError = Wifi_Open_TLS_Client(), retError != eERR_NONE)
    {
        if(retError = Wifi_Disconnect_Network(), retError != eERR_NONE) return retError;
        if(retError = Wifi_Connect_Network(), retError != eERR_NONE) return retError;
        if(retError = Wifi_Open_TLS_Client(), retError != eERR_NONE) return retError;
    }

    if(pck_evt[1]->evt_endpoint_status.active == TRUE)
    {
        //read_bytes = BGLIB_RX_BYTES(BGLIB_RESP_3_BYTES, BGLIB_NO_EVT);

        // Clear filename variable
        memset (fname, 0, sizeof(fname));

        memcpy(fname, DB_Test_Get_Filename(), 32);

        //sprintf (fname, "%s\\R100_0000_11.TXT", TEST_FOLDER_NAME);

        //fx_res = (uint8_t) fx_directory_default_set(&sd_fx_media, FX_NULL); // Reset default directory

        // Load and read Test file on SD
        fx_res = (uint8_t) fx_file_open(&sd_fx_media, &wifi_file, fname, FX_OPEN_FOR_READ);

        memcpy(&file_size, &wifi_file.fx_file_current_file_size, sizeof(uint64_t));
        // Check file length
        nFrame = (uint32_t) ((wifi_file.fx_file_current_file_size) / MAX_WIFI_READ_FILE );
        if ((wifi_file.fx_file_current_file_size) % MAX_WIFI_READ_FILE )
        {
            nFrame++;
        }
        // Calculate file CRC 
        for(i=0; i < nFrame; i++)
        {
            fx_res = (uint8_t) fx_file_read(&wifi_file, endpoint_data, MAX_WIFI_READ_FILE, &nBytes);
            if (fx_res == 0)
            {
                crc_16 = Get_Checksum_Add_16((uint8_t*)&endpoint_data, nBytes, crc_16);
            }
        }
        // Fill Wifi Header
        Wifi_Fill_Header(WIFI_FILE_FRAME, file_size, crc_16);

        memcpy(wifi_header_data, (uint8_t *) &wifi_header, sizeof(WIFI_HEADER_t));
        memcpy(&endpoint_data[0], wifi_header_data, MAX_WIFI_HEADER_BYTES);

        if (fx_res == 0)
        {
            //if ( test_file.fx_file_current_file_size <= TEST_FILE_SZ )
            {
                // Send wifi header
                wifi_cmd_endpoint_send(tcp_endpoint, MAX_WIFI_HEADER_BYTES, endpoint_data);
                if (Wifi_Command_Response (wifi_rsp_endpoint_send_id, BGLIB_EVT_ENDP_DATA, OSTIME_1SEC) == FALSE)
                {
                    return eERR_COMM_WIFI_INIT;
                }
                if(pck_resp->rsp_endpoint_send.result != wifi_err_success)
                {
                    return eERR_COMM_WIFI_INIT;
                }
                if(pck_evt[0]->evt_endpoint_data.data.len != 0)
                {
                    memset(endpoint_data, 0, MAX_WIFI_DATA_TX_SZ);

                    memcpy(endpoint_data, pck_evt[0]->evt_endpoint_data.data.data, MAX_WIFI_DATA_TX_SZ);

                    sscanf(endpoint_data, "%u", (unsigned int *) &frameId);
                }
                fx_res = (uint8_t) fx_file_seek(&wifi_file, (ULONG) (frameId));

                while (frameId <= nFrame)
                {
                    fx_res = (uint8_t) fx_file_read(&wifi_file, read_file_data, MAX_WIFI_READ_FILE, &nBytes);     //Read 1024 bytes
                    aux_id_payload ++;
                    if(aux_id_payload >= UINT16_MAX) aux_id_payload = 0;
                    wifi_payload.id_payload = WIFI_SWAP_UINT16(aux_id_payload);
                    memcpy(&endpoint_data[0], &wifi_payload.id_payload, sizeof(wifi_payload.id_payload));
                    memcpy(&endpoint_data[2], read_file_data, nBytes);
                    if (fx_res == 0)
                    {
                        if(frameId >= nFrame-1)
                        {
                            // Because in the last frame receive OK message
                            //read_bytes = BGLIB_RX_BYTES(BGLIB_RESP_3_BYTES, (BGLIB_EVT_ENDP_DATA_6 + BGLIB_EVT_ENDP_DATA_5 + BGLIB_EVT_RESPONSE + BGLIB_EVT_RESPONSE));
                            wifi_cmd_endpoint_send(tcp_endpoint, (uint8_t)(nBytes + WIFI_ID_PAYLOAD_SIZE), &endpoint_data);
                            if (Wifi_Command_Response (wifi_rsp_endpoint_send_id, BGLIB_EVT_ENDP_DATA, OSTIME_1SEC) == FALSE)
                            {
                                return eERR_COMM_WIFI_SEND_TEST;
                            }
                            if(pck_resp->rsp_endpoint_send.result != wifi_err_success)
                            {
                                wifi_cmd_endpoint_send(tcp_endpoint, (uint8_t)(nBytes + WIFI_ID_PAYLOAD_SIZE), &endpoint_data);
                                if (Wifi_Command_Response (wifi_rsp_endpoint_send_id, BGLIB_EVT_ENDP_DATA, OSTIME_1SEC) == FALSE)
                                {
                                    return eERR_COMM_WIFI_INIT;
                                }

                                if(pck_resp->rsp_endpoint_send.result != wifi_err_success)
                                {
                                    return eERR_COMM_WIFI_INIT;
                                }
                            }
                            break;
                        }
                        //file_length = nBytes;
                        wifi_cmd_endpoint_send(tcp_endpoint, (uint8_t)(nBytes + WIFI_ID_PAYLOAD_SIZE), &endpoint_data);
                        if (Wifi_Command_Response (wifi_rsp_endpoint_send_id, BGLIB_EVT_ENDP_DATA, OSTIME_100MSEC) == FALSE)
                        {
                            (void) tx_thread_sleep(OSTIME_1SEC);
                            wifi_cmd_endpoint_send(tcp_endpoint, (uint8_t)(nBytes+WIFI_ID_PAYLOAD_SIZE), &endpoint_data);
                            if (Wifi_Command_Response (wifi_rsp_endpoint_send_id, BGLIB_EVT_ENDP_DATA, OSTIME_100MSEC) == FALSE)
                            {
                                tries_cnt++;
                                //return eERR_COMM_WIFI_SEND_TEST;
                            }
                        }

                        if(pck_resp->rsp_endpoint_send.result != wifi_err_success)
                        {
                            Trace ((TRACE_TIME_STAMP | TRACE_NEWLINE), "=== Retry ====");
                            tx_thread_sleep(OSTIME_1SEC);
                            wifi_cmd_endpoint_send(tcp_endpoint, (uint8_t)(nBytes + WIFI_ID_PAYLOAD_SIZE), &endpoint_data);
                            if (Wifi_Command_Response (wifi_rsp_endpoint_send_id, BGLIB_EVT_ENDP_DATA, OSTIME_100MSEC) == FALSE)
                            {
                                tries_cnt++;
                                //return eERR_COMM_WIFI_SEND_TEST;
                            }

                            /*if(pck_resp->rsp_endpoint_send.result != wifi_err_success)
                            {
                                return eERR_COMM_WIFI_SEND_TEST;
                            }*/
                        }
                        if(pck_evt[0]->evt_endpoint_data.data.len != 0)
                        {
                            memset(endpoint_data, 0, MAX_WIFI_DATA_TX_SZ);

                            memcpy(endpoint_data, pck_evt[0]->evt_endpoint_data.data.data, MAX_WIFI_DATA_TX_SZ);

                            sscanf(endpoint_data, "%u", (unsigned int *) &frameId);
                        
                            file_size_ptr = frameId * nBytes;
                            fx_res = (uint8_t) fx_file_seek(&wifi_file, (ULONG) (file_size_ptr));
                            fx_res = (uint8_t) fx_media_flush (&sd_fx_media);
                            tries_cnt=0;
                            //tx_thread_sleep(OSTIME_100MSEC);
                        }
                        Trace_Arg (TRACE_NO_FLAGS | TRACE_NEWLINE, " TEST data len: %d", (uint32_t)file_size_ptr);
                        (void) tx_thread_sleep(OSTIME_50MSEC);

                        if(tries_cnt > 20) return eERR_COMM_WIFI_SEND_TEST;
                    }
                    else
                    {
                        // Error: Could not read file
                        return eERR_COMM_WIFI_SEND_TEST;
                    }
                }
           }
           /*else
           {
                // Error: Length exceeded
               return eERR_COMM_WIFI_INIT_CACERT_LENGTH;
           }*/

            fx_res = (uint8_t) fx_file_close (&wifi_file);
        }
        else
        {
            // Close TCP connection
            retError = Wifi_Close_TLS_Client();
            return eERR_COMM_WIFI_INIT_OPEN;
        }

        fx_res = (uint8_t) fx_file_close (&wifi_file);

        if(pck_evt[0]->evt_endpoint_data.data.len != 0)
        {
            memcpy(recv_resp, pck_evt[0]->evt_endpoint_data.data.data, pck_evt[0]->evt_endpoint_data.data.len);

            if(strncmp(recv_resp, SERVER_RESP_ER, 2) == 0)
            {
                return eERR_COMM_WIFI_SEND_TEST;
            }
            if(strncmp(recv_resp, SERVER_RESP_OK, 2) != 0)
            {
                return eERR_COMM_WIFI_SEND_TEST;
            }

            // Close TCP connection
            retError = Wifi_Close_TLS_Client();

            return retError;
        }

        retError = Wifi_Close_TLS_Client();
    }
    else
    {
        return eERR_COMM_WIFI_SEND_TEST;
    }

    return eERR_NONE;
}

/******************************************************************************
** Name:    Wifi_Send_Episode_Result
******************************************************************************/
/**
** @brief   Check if an error has occurred and reset directory
**
** @param   none
**
** @return  ERROR_ID_e error code
******************************************************************************/
ERROR_ID_e Wifi_Send_Episode_Result(void)
{
    ERROR_ID_e  retError = eERR_NONE;

    retError = Wifi_Send_Episode();
    fx_directory_default_set(&sd_fx_media, FX_NULL); // Reset default directory

    return retError;
}

/******************************************************************************
** Name:    Wifi_Send_Episode
*****************************************************************************/
/**
** @brief   Send a File from a uSD to the Server
**
** @param   none
**
** @return  ERROR_ID_e error code
******************************************************************************/
ERROR_ID_e Wifi_Send_Episode(void)
{
    //FX_FILE  test_file;
    char_t  fname[32];
    ERROR_ID_e  retError = eERR_NONE;
    uint8_t     fx_res;
    //uint8_t   read_bytes = 0;
    uint32_t    frameId = 0, nFrame = 0, nBytes = 0, i = 0;
    uint32_t    my_strlen = 0;
    uint64_t    file_size = 0, file_size_ptr = 0;
    char_t      my_fname[32] = {0};
    char_t      read_file_data[255]= {0}, recv_resp[2] = {0};
    uint8_t     wifi_header_data[MAX_WIFI_HEADER_BYTES] = {0};
    uint16_t    crc_16 = 0, aux_id_payload = 0;;

    // Initialize and reset header
    memset (recv_resp, 0, sizeof(recv_resp));
    memset ((uint8_t *) &wifi_payload, 0, sizeof(WIFI_PAYLOAD_t));
    memset(endpoint_data, 0, MAX_WIFI_DATA_TX_SZ);

    // destination port of the episode send service
    dest_port = PORT_EPI;

    // Open TCP connection
    if(retError = Wifi_Open_TLS_Client(), retError != eERR_NONE)
    {
        if(retError = Wifi_Disconnect_Network(), retError != eERR_NONE) return retError;
        if(retError = Wifi_Connect_Network(), retError != eERR_NONE) return retError;
        if(retError = Wifi_Open_TLS_Client(), retError != eERR_NONE) return retError;
    }

    if(pck_evt[1]->evt_endpoint_status.active == TRUE)
    {
        //read_bytes = BGLIB_RX_BYTES(BGLIB_RESP_3_BYTES, BGLIB_NO_EVT);

        // Clear filename variable
        memset (fname, 0, sizeof(fname));
        memset (my_fname, 0, sizeof(my_fname));

        //sprintf (fname, "%s\\R100_0637_03.TXT", TEST_FOLDER_NAME);

        memcpy(my_fname, DB_Episode_Get_Current_Filename(), 32);
        //DB_Episode_Resample(my_fname);

        //sprintf (fname, "%s\\%s", EPI_FOLDER_NAME, my_fname);
        strncpy(fname, my_fname, sizeof(my_fname));
        my_strlen = strlen (fname);
        //////////////////////////////
        // the file must be "wee"
        fname[my_strlen-3] = 'W';
        fname[my_strlen-2] = 'E';
        fname[my_strlen-1] = 'E';

        Trace ((TRACE_TIME_STAMP | TRACE_NEWLINE), fname);

        //memcpy(fname, DB_Episode_Get_Wee_Filename(), 32);

        //fx_res = (uint8_t) fx_directory_default_set(&sd_fx_media, FX_NULL); // Reset default directory
        fx_res = (uint8_t) fx_directory_default_set(&sd_fx_media, EPI_FOLDER_NAME); // Reset default directory

        // Load and read Test file on SD
        fx_res = (uint8_t) fx_file_open(&sd_fx_media, &wifi_file, fname, FX_OPEN_FOR_READ);

        memcpy(&file_size, &wifi_file.fx_file_current_file_size, sizeof(uint64_t));
        // Check file length
        nFrame = (uint32_t) ((wifi_file.fx_file_current_file_size) / MAX_WIFI_READ_FILE );
        if ((wifi_file.fx_file_current_file_size) % MAX_WIFI_READ_FILE )
        {
            nFrame++;
        }
        // Calculate file CRC
        for(i=0; i < nFrame; i++)
        {
            fx_res = (uint8_t) fx_file_read(&wifi_file, endpoint_data, MAX_WIFI_READ_FILE, &nBytes);
            if (fx_res == 0)
            {
                crc_16 = Get_Checksum_Add_16((uint8_t*)&endpoint_data, nBytes, crc_16);
            }
        }
        // Fill Wifi Header
        Wifi_Fill_Header(WIFI_FILE_FRAME, file_size, crc_16);

        memcpy(wifi_header_data, (uint8_t *) &wifi_header, sizeof(WIFI_HEADER_t));
        memcpy(&endpoint_data[0], wifi_header_data, MAX_WIFI_HEADER_BYTES);

        if (fx_res == 0)
        {
            //if ( test_file.fx_file_current_file_size <= TEST_FILE_SZ )
            {
                wifi_cmd_endpoint_send(tcp_endpoint, MAX_WIFI_HEADER_BYTES, endpoint_data);
                if (Wifi_Command_Response (wifi_rsp_endpoint_send_id, BGLIB_EVT_ENDP_DATA, OSTIME_1SEC) == FALSE)
                {
                    return eERR_COMM_WIFI_INIT;
                }
                if(pck_resp->rsp_endpoint_send.result != wifi_err_success)
                {
                    return eERR_COMM_WIFI_INIT;
                }
                if(pck_evt[0]->evt_endpoint_data.data.len != 0)
                {
                    memset(endpoint_data, 0, MAX_WIFI_DATA_TX_SZ);

                    memcpy(endpoint_data, pck_evt[0]->evt_endpoint_data.data.data, MAX_WIFI_DATA_TX_SZ);

                    sscanf(endpoint_data, "%u", (unsigned int *) &frameId);
                }
                fx_res = (uint8_t) fx_file_seek(&wifi_file, (ULONG) (frameId));

                while (frameId <= nFrame)
                {
                    fx_res = (uint8_t) fx_file_read(&wifi_file, read_file_data, MAX_WIFI_READ_FILE, &nBytes);     //Read 1024 bytes
                    aux_id_payload ++;
                    if(aux_id_payload >= UINT16_MAX) aux_id_payload = 0;
                    wifi_payload.id_payload = WIFI_SWAP_UINT16(aux_id_payload);
                    memcpy(&endpoint_data[0], &wifi_payload.id_payload, sizeof(wifi_payload.id_payload));
                    memcpy(&endpoint_data[2], read_file_data, nBytes);
                    if (fx_res == 0)
                    {
                        if(frameId >= nFrame-1)
                        {
                            // Because in the last frame receive OK message
                            //read_bytes = BGLIB_RX_BYTES(BGLIB_RESP_3_BYTES, 8/*(BGLIB_EVT_ENDP_DATA_6 + BGLIB_EVT_ENDP_DATA_5 + BGLIB_EVT_RESPONSE + BGLIB_EVT_RESPONSE)*/);
                            wifi_cmd_endpoint_send(tcp_endpoint, (uint8_t)(nBytes + WIFI_ID_PAYLOAD_SIZE), &endpoint_data);
                            if (Wifi_Command_Response (wifi_rsp_endpoint_send_id, BGLIB_EVT_ENDP_DATA, OSTIME_1SEC) == FALSE)
                            {
                                return eERR_COMM_WIFI_INIT;
                            }
                            if(pck_resp->rsp_endpoint_send.result != wifi_err_success)
                            {
                                wifi_cmd_endpoint_send(tcp_endpoint, (uint8_t)(nBytes + WIFI_ID_PAYLOAD_SIZE), &endpoint_data);
                                if (Wifi_Command_Response (wifi_rsp_endpoint_send_id, BGLIB_EVT_ENDP_DATA, OSTIME_1SEC) == FALSE)
                                {
                                    return eERR_COMM_WIFI_INIT;
                                }

                                if(pck_resp->rsp_endpoint_send.result != wifi_err_success)
                                {
                                    return eERR_COMM_WIFI_INIT;
                                }
                            }
                            break;
                        }
                        //file_length = nBytes;
                        wifi_cmd_endpoint_send(tcp_endpoint, (uint8_t)(nBytes + WIFI_ID_PAYLOAD_SIZE), &endpoint_data);
                        if (Wifi_Command_Response (wifi_rsp_endpoint_send_id, BGLIB_EVT_ENDP_DATA, OSTIME_100MSEC) == FALSE)
                        {
                            (void) tx_thread_sleep(OSTIME_5SEC);
                            wifi_cmd_endpoint_send(tcp_endpoint, (uint8_t)(nBytes + WIFI_ID_PAYLOAD_SIZE), &endpoint_data);
                            if (Wifi_Command_Response (wifi_rsp_endpoint_send_id, BGLIB_EVT_ENDP_DATA, OSTIME_100MSEC) == FALSE)
                            {
                                return eERR_COMM_WIFI_SEND_EPISODE;
                            }
                        }

                        if(pck_resp->rsp_endpoint_send.result != wifi_err_success)
                        {
                            Trace ((TRACE_TIME_STAMP | TRACE_NEWLINE), "=== Retry ====");

                            wifi_cmd_endpoint_send(tcp_endpoint, (uint8_t)(nBytes + WIFI_ID_PAYLOAD_SIZE), &endpoint_data);
                            if (Wifi_Command_Response (wifi_rsp_endpoint_send_id, BGLIB_EVT_ENDP_DATA, OSTIME_100MSEC) == FALSE)
                            {
                                return eERR_COMM_WIFI_SEND_EPISODE;
                            }

                            if(pck_resp->rsp_endpoint_send.result != wifi_err_success)
                            {
                                return eERR_COMM_WIFI_SEND_EPISODE;
                            }
                        }
                        if(pck_evt[0]->evt_endpoint_data.data.len != 0)
                        {
                            memset(endpoint_data, 0, MAX_WIFI_DATA_TX_SZ);

                            memcpy(endpoint_data, pck_evt[0]->evt_endpoint_data.data.data, MAX_WIFI_DATA_TX_SZ);

                            sscanf(endpoint_data, "%u", (unsigned int *) &frameId);
                        
                            file_size_ptr = frameId * nBytes;
                            fx_res = (uint8_t) fx_file_seek(&wifi_file, (ULONG) (file_size_ptr));
                            fx_res = (uint8_t) fx_media_flush (&sd_fx_media);
                            //tx_thread_sleep(OSTIME_100MSEC);
                        }
                        Trace_Arg (TRACE_NO_FLAGS | TRACE_NEWLINE, " WEE data len: %d", (uint32_t)file_size_ptr);
                    }
                    else
                    {
                        // Error: Could not read file
                        return eERR_COMM_WIFI_SEND_EPISODE;
                    }

                    // wait a little time
                    //(void) tx_thread_sleep(OSTIME_10MSEC);
                }
            }
           /*else
           {
                // Error: Length exceeded
               return eERR_COMM_WIFI_INIT_CACERT_LENGTH;
           }*/

            //fx_res = (uint8_t) fx_file_close (&test_file);
            //fx_res = (uint8_t) fx_file_delete(&sd_fx_media, fname);
        }
        else
        {
            // Close TCP connection
            retError = Wifi_Close_TLS_Client();
            return eERR_COMM_WIFI_INIT_OPEN;
        }

        fx_res = (uint8_t) fx_file_close (&wifi_file);

        if(pck_evt[0]->evt_endpoint_data.data.len != 0)
        {
            memcpy(recv_resp, pck_evt[0]->evt_endpoint_data.data.data, pck_evt[0]->evt_endpoint_data.data.len);

            if(strncmp(recv_resp, SERVER_RESP_ER, 2) == 0)
            {
                return eERR_COMM_WIFI_SEND_EPISODE;
            }
            if(strncmp(recv_resp, SERVER_RESP_OK, 2) != 0)
            {
                return eERR_COMM_WIFI_SEND_EPISODE;
            }

            fx_res = (uint8_t) fx_file_delete(&sd_fx_media, fname);
            fx_res = (uint8_t) fx_media_flush (&sd_fx_media);

            // Close TCP connection
            retError = Wifi_Close_TLS_Client();

            return retError;
        }

        retError = Wifi_Close_TLS_Client();
    }
    else
    {
        return eERR_COMM_WIFI_SEND_EPISODE;
    }

    return eERR_NONE;
}

/******************************************************************************
** Name:    Wifi_Receive_Firmware_Result
******************************************************************************/
/**
** @brief   Check if an error has occurred and if it has occurred, delete the firmware.srec so as not to record it because it may be corrupt
**
** @param   none
**
** @return  ERROR_ID_e error code
******************************************************************************/
ERROR_ID_e Wifi_Receive_Firmware_Result(void)
{
    //FX_FILE  upgrade_file;
    ERROR_ID_e retError = eERR_NONE;
    if(retError = Wifi_Receive_Firmware(), retError != eERR_NONE)
    {
        fx_file_close(&wifi_file);
        fx_file_delete(&sd_fx_media, UPGRADE_FILENAME);
        fx_media_flush (&sd_fx_media);
    }
    return retError;
}

/******************************************************************************
** Name:    Wifi_Delete_Firmware
******************************************************************************/
/**
** @brief   Delete Firmware
**
** @param   none
**
** @return  void
******************************************************************************/
void Wifi_Delete_Firmware(void)
{

    fx_file_close(&wifi_file);
    fx_file_delete(&sd_fx_media, UPGRADE_FILENAME);
    fx_media_flush (&sd_fx_media);
}

/******************************************************************************
** Name:    Wifi_Receive_Firmware
******************************************************************************/
/**
** @brief   Receive firmware update file
**
** @param   none
**
** @return  ERROR_ID_e error code
******************************************************************************/
ERROR_ID_e Wifi_Receive_Firmware(void)
{
    //FX_FILE  upgrade_file;
    ERROR_ID_e retError = eERR_NONE;
    static uint8_t  fx_res = 0, read_bytes = 0, tries_cnt = 0;
    uint32_t attributes = 0;
    static uint32_t nFrame = 0, /*nSwapFrame = 0,*/ data_rec = 0, max_frames = 0;
    static bool_t find_null = false;
    //unsigned char bytes[4];
    uint8_t wifi_header_data[MAX_WIFI_HEADER_BYTES] = {0};
    static uint16_t crc_16 = 0;
    static uint16_t id_payload_rcv = 0, id_payload_cnt = 0;
    static char_t rcv_id[2] = {1,1};

    // Initialize and reset header
    memset(endpoint_data, 0, MAX_WIFI_DATA_TX_SZ);

    // destination port of the firmware receive service
    dest_port = PORT_FW;

    // Open TCP connection
    if(retError = Wifi_Open_TLS_Client(), retError != eERR_NONE)
    {
        if(retError = Wifi_Disconnect_Network(), retError != eERR_NONE) return retError;
        if(retError = Wifi_Connect_Network(), retError != eERR_NONE) return retError;
        if(retError = Wifi_Open_TLS_Client(), retError != eERR_NONE) return retError;
    }

    if(pck_evt[1]->evt_endpoint_status.active == TRUE)
    {
        Wifi_Fill_Header(WIFI_FILE_FRAME, 0, 0);

        memcpy(wifi_header_data, (uint8_t *) &wifi_header, sizeof(WIFI_HEADER_t));
        memcpy(&endpoint_data[0], wifi_header_data, MAX_WIFI_HEADER_BYTES);

        // This command is used to send data to a given endpoint
        wifi_cmd_endpoint_send(tcp_endpoint, MAX_WIFI_HEADER_BYTES, endpoint_data);

        // This command is used to send data to a given endpoint
        //wifi_cmd_endpoint_send(tcp_endpoint, (uint8_t) strlen("OK"), "OK");

        if (Wifi_Command_Response (wifi_rsp_endpoint_send_id, BGLIB_EVT_ENDP_DATA, OSTIME_1SEC) == FALSE)
        {
            return eERR_COMM_WIFI_RECEIVE_UPGRADE_FILE;
        }

        if(pck_resp->rsp_endpoint_send.result != wifi_err_success)
        {
            return eERR_COMM_WIFI_RECEIVE_UPGRADE_FILE;
        }

        if(pck_evt[0]->evt_endpoint_data.data.len != 0)
        {
            memset(endpoint_data, 0, MAX_WIFI_DATA_TX_SZ);

            memcpy(endpoint_data, pck_evt[0]->evt_endpoint_data.data.data, MAX_WIFI_DATA_TX_SZ);

            //sscanf(endpoint_data, "%u", (unsigned int *) &upgrade_size);
            Comm_Wifi_Save_Server_Response(endpoint_data);

            wifi_header.file_size = Comm_Wifi_SZ();
            wifi_header.wifi_cheksum = Comm_Wifi_CRC();
            Comm_Wifi_PASS(wifi_header.password);

            max_frames = (uint32_t) (wifi_header.file_size / 250 );
            if (wifi_header.file_size % 250 )
            {
                max_frames++;
            }
        }

        fx_res = (uint8_t) fx_directory_default_set(&sd_fx_media, FX_NULL);

        // ... check if the requested file exist
        fx_res = (uint8_t) fx_file_attributes_read (&sd_fx_media, UPGRADE_FILENAME, (UINT *)&attributes);
        if ((fx_res == FX_SUCCESS) || (attributes & FX_ARCHIVE))
        {
            fx_res = (uint8_t) fx_file_delete (&sd_fx_media, UPGRADE_FILENAME);
            fx_res = (uint8_t) fx_media_flush (&sd_fx_media);
        }

        fx_res = (uint8_t) fx_file_create(&sd_fx_media, UPGRADE_FILENAME);

        // ... open the upgrade file
        fx_res = (uint8_t) fx_file_open(&sd_fx_media, &wifi_file, UPGRADE_FILENAME, FX_OPEN_FOR_WRITE);
        if (fx_res == FX_SUCCESS)
        {
            while(wifi_header.file_size > data_rec)
            {
                // Swap nFrame value to be sent
                //nSwapFrame = nFrame;

                //WIFI_SWAP_UINT32(nSwapFrame);

                if(nFrame <= max_frames-1)
                {
                    // This command is used to send data to a given endpoint
                    wifi_cmd_endpoint_send(tcp_endpoint, sizeof(uint32_t), &nFrame);
                    if (Wifi_Command_Response_CFG_FW (wifi_rsp_endpoint_send_id, 250, OSTIME_200MSEC, TRUE) == FALSE)
                    {
                        // wait a little time
                        (void) tx_thread_sleep(OSTIME_500MSEC);
                        wifi_cmd_endpoint_send(tcp_endpoint, sizeof(uint32_t), &nFrame);
                        if (Wifi_Command_Response (wifi_rsp_endpoint_send_id, BGLIB_EVT_ENDP_DATA, OSTIME_200MSEC) == FALSE)
                        {
                            tries_cnt++;
                            //return eERR_COMM_WIFI_RECEIVE_UPGRADE_FILE;
                        }
                    }
                }
                else
                {
                    wifi_cmd_endpoint_send(tcp_endpoint, sizeof(uint32_t), &nFrame);
                    if (Wifi_Command_Response_CFG_FW (wifi_rsp_endpoint_send_id, BGLIB_EVT_ENDP_DATA, OSTIME_200MSEC, FALSE) == FALSE)
                    {
                        tries_cnt++;
                    }
                }
                //Trace ((TRACE_TIME_STAMP | TRACE_NEWLINE), "=== TX ====");
                if(pck_resp->rsp_endpoint_send.result != wifi_err_success)
                {
                    Trace ((TRACE_TIME_STAMP | TRACE_NEWLINE), "=== Retry ====");
                    (void) tx_thread_sleep(OSTIME_500MSEC);
                    // This command is used to send data to a given endpoint. Retry....
                    //wifi_cmd_endpoint_send(tcp_endpoint, (uint8_t) strlen("NOK"), "NOK");
                    if(nFrame <= max_frames-1)
                    {
                        wifi_cmd_endpoint_send(tcp_endpoint, sizeof(uint32_t), &nFrame);
                        if (Wifi_Command_Response_CFG_FW (wifi_rsp_endpoint_send_id, 250, OSTIME_400MSEC, TRUE) == FALSE)
                        {
                            tries_cnt++;
                            //return eERR_COMM_WIFI_RECEIVE_UPGRADE_FILE;
                        }
                    }
                    else
                    {
                        wifi_cmd_endpoint_send(tcp_endpoint, sizeof(uint32_t), &nFrame);
                        if (Wifi_Command_Response_CFG_FW (wifi_rsp_endpoint_send_id, BGLIB_EVT_ENDP_DATA, OSTIME_400MSEC, FALSE) == FALSE)
                        {
                            tries_cnt++;
                            //return eERR_COMM_WIFI_RECEIVE_UPGRADE_FILE;
                        }
                    }

                    /*if(pck_resp->rsp_endpoint_send.result != wifi_err_success)
                    {
                        return eERR_COMM_WIFI_RECEIVE_UPGRADE_FILE;
                    }*/
                }

                if(pck_evt[0]->evt_endpoint_data.data.len != 0)
                {
                    /*if(memcmp(pck_evt[0]->evt_endpoint_data.data.data, (uchar_t*)&rcv_id, 2) != 0)
                    {*/
                        /*if(pck_evt[0]->evt_endpoint_data.data.len != MAX_WIFI_DATA_TX_SZ)
                        {
                            return eERR_COMM_WIFI_RECEIVE_UPGRADE_FILE;
                        }*/

                        //memset(&endpoint_data, 0, MAX_WIFI_DATA_TX_SZ);
                        memcpy(rcv_id, pck_evt[0]->evt_endpoint_data.data.data, 2);
                        // Receive Frame ID to check if is spected ID
                        id_payload_rcv = (uint16_t)(((uint8_t)rcv_id[0] << 8) | (uint8_t)rcv_id[1]);
                        //Trace_Arg (TRACE_NO_FLAGS | TRACE_NEWLINE, " id_payload_rcv: %d", (uint32_t)id_payload_rcv);
                        //Trace_Arg (TRACE_NO_FLAGS | TRACE_NEWLINE, " id_payload_cnt: %d", (uint32_t)id_payload_cnt);
                        // Frame ID counter to check the frame we received

                        //if(id_payload_rcv == id_payload_cnt)
                        if(id_payload_rcv == nFrame)
                        {
                            memcpy(endpoint_data, &pck_evt[0]->evt_endpoint_data.data.data[2], MAX_WIFI_DATA_TX_SZ);
                            for(int i=0; i<pck_evt[0]->evt_endpoint_data.data.len-WIFI_ID_PAYLOAD_SIZE; i++)
                            {
                                if((endpoint_data[i] == '\0')) // (char_t*)&endpoint_data[i] == NULL || endpoint_data[i] == ' ' ||
                                {
                                    find_null = true;
                                }
                            }
                            if(find_null == false)
                            {
                                // ... write  the settings in file
                                //fx_res = fx_file_seek(&upgrade_file, 252*nFrame);
                                fx_res = (uint8_t) fx_file_write (&wifi_file, &endpoint_data, (ULONG)(pck_evt[0]->evt_endpoint_data.data.len - WIFI_ID_PAYLOAD_SIZE));
                                //fx_res = (uint8_t) fx_file_write (&upgrade_file, (uint8_t *) &pck_evt[0]->evt_endpoint_data.data.data, pck_evt[0]->evt_endpoint_data.data.len);

                                // ... flush data to physical media
                                fx_res = (uint8_t) fx_media_flush (&sd_fx_media);
                                if (fx_res == 0)
                                {
                                    crc_16 = Get_Checksum_Add_16((uint8_t*)&endpoint_data, (uint32_t)(pck_evt[0]->evt_endpoint_data.data.len - WIFI_ID_PAYLOAD_SIZE), crc_16);
                                }

                                data_rec = data_rec + (uint32_t)(pck_evt[0]->evt_endpoint_data.data.len - WIFI_ID_PAYLOAD_SIZE);
                                nFrame++;
                                tries_cnt = 0;
                            }
                            if(find_null == true)
                            {
                                (void) tx_thread_sleep(OSTIME_100MSEC);
                                find_null = false;
                            }
                        }
                        else
                        {
                            tries_cnt ++;
                            if(tries_cnt > 20) return eERR_COMM_WIFI_RECEIVE_UPGRADE_FILE;
                        }
                    //}
                    /*else
                    {
                        tries_cnt ++;
                        if(tries_cnt > 50) return eERR_COMM_WIFI_RECEIVE_UPGRADE_FILE;
                    }*/
                }
                //id_payload_cnt ++;
                if(id_payload_cnt >= UINT16_MAX) id_payload_cnt = 0;

                if(wifi_header.file_size <= data_rec)//(max_frames <= nFrame)
                {
                    if(crc_16 == wifi_header.wifi_cheksum)
                    {
                        // Before closing the session, an OK is received / sent to know that the communication has been correct.
                        read_bytes = BGLIB_RX_BYTES(BGLIB_RESP_3_BYTES, (BGLIB_EVT_ENDP_DATA_6 + BGLIB_EVT_ENDP_DATA_5 + BGLIB_EVT_RESPONSE));
                        wifi_cmd_endpoint_send(tcp_endpoint, (uint8_t) strlen("OK"), "OK");
                        if (Wifi_Command_Response (wifi_rsp_endpoint_send_id, read_bytes, OSTIME_3SEC) == FALSE)
                        {
                            return eERR_COMM_WIFI_RECEIVE_UPGRADE_FILE;
                        }
                        if(pck_resp->rsp_endpoint_send.result != wifi_err_success)
                        {
                            return eERR_COMM_WIFI_RECEIVE_UPGRADE_FILE;
                        }
                    }
                    else
                    {
                        // Before closing the session, an OK is received / sent to know that the communication has been correct.
                        read_bytes = BGLIB_RX_BYTES(BGLIB_RESP_3_BYTES, (BGLIB_EVT_ENDP_DATA_6 + BGLIB_EVT_ENDP_DATA_5 + BGLIB_EVT_RESPONSE));
                        wifi_cmd_endpoint_send(tcp_endpoint, (uint8_t) strlen("ER"), "ER");
                        if (Wifi_Command_Response (wifi_rsp_endpoint_send_id, read_bytes, OSTIME_3SEC) == FALSE)
                        {
                            return eERR_COMM_WIFI_RECEIVE_UPGRADE_FILE;
                        }
                        if(pck_resp->rsp_endpoint_send.result != wifi_err_success)
                        {
                            return eERR_COMM_WIFI_RECEIVE_UPGRADE_FILE;
                        }
                    }
                }

                Trace_Arg (TRACE_NO_FLAGS | TRACE_NEWLINE, " SREC data len: %d", data_rec);

                // wait a little time
                (void) tx_thread_sleep(OSTIME_50MSEC);
            }
        }
        else
        {
            // Close TCP connection
            retError = Wifi_Close_TLS_Client();
            return eERR_COMM_WIFI_INIT_OPEN;
        }

        //Trace ((TRACE_TIME_STAMP | TRACE_NEWLINE), "=== End Of Transmission ====");

        // ... close the file
        fx_res = (uint8_t) fx_file_close(&wifi_file);

        // Close TCP connection
        retError = Wifi_Close_TLS_Client();

        /*if(wifi_header.wifi_cheksum != wifi_file_xor_checksum)
        {
            return eERR_COMM_WIFI_CRC;
        }*/

        return retError;
    }
    else
    {
        return eERR_COMM_WIFI_RECEIVE_UPGRADE_FILE;
    }

    return eERR_NONE;
}

/******************************************************************************
** Name:    Wifi_Receive_Configuration_Result
******************************************************************************/
/**
** @brief   Check if an error has occurred and if it has occurred, delete the R100.cfg so as not to record it because it may be corrupt
**
** @param   none
**
** @return  ERROR_ID_e error code
******************************************************************************/
ERROR_ID_e Wifi_Receive_Configuration_Result(void)
{
    //FX_FILE  config_file;
    ERROR_ID_e retError = eERR_NONE;
    if(retError = Wifi_Receive_Configuration(), retError != eERR_NONE)
    {
        fx_file_close(&wifi_file);
        fx_file_delete(&sd_fx_media, CFG_FILENAME);
        fx_media_flush (&sd_fx_media);
    }
    return retError;
}

/******************************************************************************
Name:    Wifi_Receive_Configuration
******************************************************************************/
/**
** @brief   Receive configuration update file
**
** @param   none
**
** @return  ERROR_ID_e error code
******************************************************************************/
ERROR_ID_e Wifi_Receive_Configuration(void)
{
    //FX_FILE  config_file;
    ERROR_ID_e retError = eERR_NONE;
    uint8_t  fx_res, read_bytes = 0, tries_cnt = 0;
    uint32_t nFrame = 0, nSwapFrame = 0;
    uint32_t attributes = 0, data_rec = 0;
    uint8_t  wifi_header_data[MAX_WIFI_HEADER_BYTES] = {0};
    uint16_t crc_16 = 0;
    uint16_t id_payload_rcv = 0, id_payload_cnt = 0;
    char_t rcv_id[2] = {1,1};

    // Initialize and reset header
    memset(endpoint_data, 0, MAX_WIFI_DATA_TX_SZ);

    // destination port of the host .cfg receive service
    dest_port = PORT_CFG;

    // Open TCP connection
    if(retError = Wifi_Open_TLS_Client(), retError != eERR_NONE)
    {
        if(retError = Wifi_Disconnect_Network(), retError != eERR_NONE) return retError;
        if(retError = Wifi_Connect_Network(), retError != eERR_NONE) return retError;
        if(retError = Wifi_Open_TLS_Client(), retError != eERR_NONE) return retError;
    }

    if(pck_evt[1]->evt_endpoint_status.active == TRUE)
    {
        // Fill Wifi Header
        Wifi_Fill_Header(WIFI_FILE_FRAME, 0, 0);

        memcpy(wifi_header_data, (uint8_t *) &wifi_header, sizeof(WIFI_HEADER_t));
        memcpy(&endpoint_data[0], wifi_header_data, MAX_WIFI_HEADER_BYTES);

        // This command is used to send data to a given endpoint
        wifi_cmd_endpoint_send(tcp_endpoint, MAX_WIFI_HEADER_BYTES, endpoint_data);

        // This command is used to send data to a given endpoint
        //wifi_cmd_endpoint_send(tcp_endpoint, (uint8_t) strlen("OK"), "OK");

        if (Wifi_Command_Response_Data (wifi_rsp_endpoint_send_id, OSTIME_1SEC) == FALSE)
        {
            return eERR_COMM_WIFI_RECEIVE_CONFIG_FILE;
        }

        if(pck_resp->rsp_endpoint_send.result != wifi_err_success)
        {
            return eERR_COMM_WIFI_RECEIVE_CONFIG_FILE;
        }

        if(pck_evt[0]->evt_endpoint_data.data.len != 0)
        {
            memset(endpoint_data, 0, MAX_WIFI_DATA_TX_SZ);

            memcpy(endpoint_data, pck_evt[0]->evt_endpoint_data.data.data, MAX_WIFI_DATA_TX_SZ);

            Comm_Wifi_Save_Server_Response(endpoint_data);

            wifi_header.file_size = Comm_Wifi_SZ();
            wifi_header.wifi_cheksum = Comm_Wifi_CRC();
        }

        fx_res = (uint8_t) fx_directory_default_set(&sd_fx_media, FX_NULL);

        // ... check if the requested file exist
        fx_res = (uint8_t) fx_file_attributes_read (&sd_fx_media, CFG_FILENAME, (UINT *)&attributes);
        if ((fx_res == FX_SUCCESS) || (attributes & FX_ARCHIVE))
        {
            fx_res = (uint8_t) fx_file_delete (&sd_fx_media, CFG_FILENAME);
            fx_res = (uint8_t) fx_media_flush (&sd_fx_media);
        }

        fx_res = (uint8_t) fx_file_create(&sd_fx_media, CFG_FILENAME);

        // ... open the upgrade file
        fx_res = (uint8_t) fx_file_open(&sd_fx_media, &wifi_file, CFG_FILENAME, FX_OPEN_FOR_WRITE);
        if (fx_res == FX_SUCCESS)
        {
            while(wifi_header.file_size > data_rec)
            {
                // Swap nFrame value to be sent
                nSwapFrame = nFrame;

                WIFI_SWAP_UINT32(nSwapFrame);

                // This command is used to send data to a given endpoint
                //wifi_cmd_endpoint_send(tcp_endpoint, (uint8_t) strlen("OK"), "OK");
                wifi_cmd_endpoint_send(tcp_endpoint, sizeof(uint32_t), &nSwapFrame);
                if (Wifi_Command_Response_Data (wifi_rsp_endpoint_send_id, OSTIME_200MSEC) == FALSE)
                {
                    // wait a little time
                    (void) tx_thread_sleep(OSTIME_5SEC);
                    wifi_cmd_endpoint_send(tcp_endpoint, sizeof(uint32_t), &nSwapFrame);
                    if (Wifi_Command_Response_Data (wifi_rsp_endpoint_send_id, OSTIME_200MSEC) == FALSE)
                    {
                        return eERR_COMM_WIFI_RECEIVE_CONFIG_FILE;
                    }
                }
                //Trace ((TRACE_TIME_STAMP | TRACE_NEWLINE), "=== TX ====");
                if(pck_resp->rsp_endpoint_send.result != wifi_err_success)
                {
                    Trace ((TRACE_TIME_STAMP | TRACE_NEWLINE), "=== Retry ====");
                    // This command is used to send data to a given endpoint. Retry....
                    //wifi_cmd_endpoint_send(tcp_endpoint, (uint8_t) strlen("NOK"), "NOK");
                    wifi_cmd_endpoint_send(tcp_endpoint, sizeof(uint32_t), &nSwapFrame);

                    if (Wifi_Command_Response_Data (wifi_rsp_endpoint_send_id, OSTIME_200MSEC) == FALSE)
                    {
                        return eERR_COMM_WIFI_RECEIVE_CONFIG_FILE;
                    }

                    if(pck_resp->rsp_endpoint_send.result != wifi_err_success)
                    {
                        return eERR_COMM_WIFI_RECEIVE_CONFIG_FILE;
                    }
                }

                if(pck_evt[0]->evt_endpoint_data.data.len != 0)
                {
                    /*if(memcmp(pck_evt[0]->evt_endpoint_data.data.data, (uchar_t*)&rcv_id, 2) != 0)
                    {*/
                        //memset(&endpoint_data, 0, MAX_WIFI_DATA_TX_SZ);
                        memcpy(rcv_id, pck_evt[0]->evt_endpoint_data.data.data, 2);
                        id_payload_rcv = (uint16_t)(((uint8_t)rcv_id[0] << 8) | (uint8_t)rcv_id[1]);

                        if(id_payload_rcv == id_payload_cnt)
                        {
                            memcpy(endpoint_data, &pck_evt[0]->evt_endpoint_data.data.data[2], MAX_WIFI_DATA_TX_SZ);

                            // ... write  the settings in file
                            fx_res = (uint8_t) fx_file_write (&wifi_file, &endpoint_data, (ULONG)(pck_evt[0]->evt_endpoint_data.data.len - WIFI_ID_PAYLOAD_SIZE));
                            //fx_res = (uint8_t) fx_file_write (&config_file, (uint8_t *) &pck_evt[0]->evt_endpoint_data.data.data, pck_evt[0]->evt_endpoint_data.data.len);

                            // ... flush data to physical media
                            fx_res = (uint8_t) fx_media_flush (&sd_fx_media);

                            if (fx_res == 0)
                            {
                                crc_16 = Get_Checksum_Add_16((uint8_t*)&endpoint_data, (uint32_t)(pck_evt[0]->evt_endpoint_data.data.len - WIFI_ID_PAYLOAD_SIZE), crc_16);
                            }
                            data_rec = data_rec + (uint32_t)(pck_evt[0]->evt_endpoint_data.data.len - WIFI_ID_PAYLOAD_SIZE);
                            nFrame++;
                            tries_cnt = 0;
                        }
                        else
                        {
                            tries_cnt ++;
                            if(tries_cnt > 20) return eERR_COMM_WIFI_RECEIVE_CONFIG_FILE;
                        }
                }
                id_payload_cnt ++;
                if(id_payload_cnt >= UINT16_MAX) id_payload_cnt = 0;

                if(wifi_header.file_size <= data_rec)
                {
                    if(crc_16 == wifi_header.wifi_cheksum)
                    {
                        // Before closing the session, an OK is received / sent to know that the communication has been correct.
                        read_bytes = BGLIB_RX_BYTES(BGLIB_RESP_3_BYTES, (BGLIB_EVT_ENDP_DATA_6 + BGLIB_EVT_ENDP_DATA_5 + BGLIB_EVT_RESPONSE));
                        wifi_cmd_endpoint_send(tcp_endpoint, (uint8_t) strlen("OK"), "OK");
                        if (Wifi_Command_Response (wifi_rsp_endpoint_send_id, read_bytes, OSTIME_10SEC) == FALSE)
                        {
                            return eERR_COMM_WIFI_RECEIVE_CONFIG_FILE;
                        }
                        if(pck_resp->rsp_endpoint_send.result != wifi_err_success)
                        {
                            return eERR_COMM_WIFI_RECEIVE_CONFIG_FILE;
                        }
                    }
                    else
                    {
                        // Before closing the session, an OK is received / sent to know that the communication has been correct.
                        read_bytes = BGLIB_RX_BYTES(BGLIB_RESP_3_BYTES, (BGLIB_EVT_ENDP_DATA_6 + BGLIB_EVT_ENDP_DATA_5 + BGLIB_EVT_RESPONSE));
                        wifi_cmd_endpoint_send(tcp_endpoint, (uint8_t) strlen("ER"), "ER");
                        if (Wifi_Command_Response (wifi_rsp_endpoint_send_id, read_bytes, OSTIME_10SEC) == FALSE)
                        {
                            return eERR_COMM_WIFI_RECEIVE_CONFIG_FILE;
                        }
                        if(pck_resp->rsp_endpoint_send.result != wifi_err_success)
                        {
                            return eERR_COMM_WIFI_RECEIVE_CONFIG_FILE;
                        }
                    }
                }

                Trace_Arg (TRACE_NO_FLAGS | TRACE_NEWLINE, "CFG data len: %d", data_rec);

                // wait a little time
                (void) tx_thread_sleep(OSTIME_100MSEC);

            }
        }
        else
        {
            // Close TCP connection
            retError = Wifi_Close_TLS_Client();
            return eERR_COMM_WIFI_INIT_OPEN;
        }

        //Trace ((TRACE_TIME_STAMP | TRACE_NEWLINE), "=== End Of Transmission ====");

        // ... close the file
        fx_res = (uint8_t) fx_file_close(&wifi_file);

        // Close TCP connection
        retError = Wifi_Close_TLS_Client();

        /*if(wifi_header.wifi_cheksum != wifi_file_xor_checksum)
        {
            return eERR_COMM_WIFI_CRC;
        }*/
        return retError;
    }
    else
    {
        return eERR_COMM_WIFI_RECEIVE_CONFIG_FILE;
    }

    return eERR_NONE;
}

/******************************************************************************
** Name:    Wifi_Send_Command
*****************************************************************************/
/**
** @brief   Function called when a message needs to be written to the serial port
**
** @param   msg_len Length of the message
** @param   msg_data Message data, including the header
** @param   data_len Optional variable data length
** @param   data Optional variable data
**
** @return  none
******************************************************************************/
static void Wifi_Send_Command(uint8 msg_len, uint8* msg_data, uint16 data_len, uint8* data)
{
    char_t wifi_cmd [MAX_WIFI_CMD_LENGTH];

    // Clear command variable
    memset (wifi_cmd, 0, MAX_WIFI_CMD_LENGTH);
    memcpy (wifi_cmd, msg_data, msg_len);
    Comm_Uart_Wifi_Send(wifi_cmd, msg_len);

    if(data_len && data)
    {
        // Clear command variable
        memset (wifi_cmd, 0, MAX_WIFI_CMD_LENGTH);
        memcpy (wifi_cmd, data, data_len);
        Comm_Uart_Wifi_Send(wifi_cmd, (uint8_t) data_len);
    }
}

/******************************************************************************
** Name:    Wifi_Command_Response
*****************************************************************************/
/**
** @brief   Waits for a response to a given command
**
** @param   ack   expected response number 1
** @param   length   expected response number 2 (NULL if only 1 response expected)
** @param   tout  timeout for the response
**
** @return  bool_t  true if received expected response, false if timeout expired
******************************************************************************/
static bool_t Wifi_Command_Response (uint32_t ack, uint8_t length, uint32_t tout)
{
    char wifi_buffer[1024];
    uint8_t     wifi_resp[20];
    uint32_t    msg_length, msg_id;         // length of message payload
    uint32_t    *msg_header=NULL;         // pointer to message header (ID + length)
    ERROR_ID_e  retError;
    wifi_evt_t  wifi_event[BGLIB_MAX_EVT];
    uint16_t    evt_pos   [BGLIB_MAX_EVT+1];
    uint32_t    i, j;
    uint32_t    aux=0;              // aux variable to copy msg header to msg header pointer

    memset(wifi_buffer, 0, sizeof(wifi_buffer));
    memset(wifi_resp,   0, sizeof(wifi_resp));
    memset(wifi_event,  0, sizeof(wifi_event));
    memset(evt_pos,     0, sizeof(evt_pos));

    // Read enough data from serial port for the BGAPI message header + Event
    retError = Comm_Uart_Wifi_Receive (wifi_buffer, length, tout, ack);
    if(retError != eERR_NONE)
    {
        return FALSE;
    }

    memcpy (wifi_resp, wifi_buffer, BGLIB_MSG_HEADER_LEN);
    memcpy (&aux, wifi_resp, sizeof(uint32_t));
    msg_header = &aux;

    // The buffer now contains the message header
    // See BGAPI protocol definition for details on packet format
    msg_length = BGLIB_MSG_LEN(msg_header);

    // Read the payload data if required and store it after the header.
    if (msg_length)
    {
        memcpy(&wifi_resp  [BGLIB_MSG_HEADER_LEN],
               &wifi_buffer[BGLIB_MSG_HEADER_LEN],
               msg_length);
    }
    msg_id = BGLIB_MSG_ID(msg_header);
    if (msg_id != ack)
    {
       return FALSE;
    }

    pck_resp = BGLIB_MSG(wifi_resp);

    // Check if there is an event
    if (wifi_buffer[BGLIB_MSG_HEADER_LEN + msg_length] == (char)0x88)
    {
        for (i=j=0; i<length; i++)
        {
            if (wifi_buffer[i] == (char)0x88)
            {
                evt_pos[j] = (uint16_t) i;
                j++;
            }
        }

        for (i=0; i < BGLIB_MAX_EVT; i++)
        {
            memcpy (&wifi_event[i], &wifi_buffer[evt_pos[i]], (size_t) (length-(evt_pos[i+1])));
            pck_evt[i] = BGLIB_MSG(&wifi_event [i]);
            if(evt_pos[i+1] == 0)
            {
                break;
            }
        }
    }

    return TRUE;
}

/******************************************************************************
** Name:    Wifi_Command_Response_CFG_FW
*****************************************************************************/
/**
** @brief   Waits for a response to a given command
**
** @param   ack   expected response number 1
** @param   length   expected response number 2 (NULL if only 1 response expected)
** @param   tout  timeout for the response
**
** @return  bool_t  true if received expected response, false if timeout expired
******************************************************************************/
static bool_t Wifi_Command_Response_CFG_FW (uint32_t ack, uint8_t length, uint32_t tout, bool_t wait_length)
{
    char wifi_buffer[1024];
    uint8_t     wifi_resp[20];
    uint32_t    msg_length, msg_id, received_length = 0;         // length of message payload
    uint32_t    *msg_header=NULL;         // pointer to message header (ID + length)
    ERROR_ID_e  retError;
    wifi_evt_t  wifi_event[BGLIB_MAX_EVT];
    uint16_t    evt_pos   [BGLIB_MAX_EVT+1];
    uint32_t    i, j;
    uint32_t    aux=0;              // aux variable to copy msg header to msg header pointer

    memset(wifi_buffer, 0, sizeof(wifi_buffer));
    memset(wifi_resp,   0, sizeof(wifi_resp));
    memset(wifi_event,  0, sizeof(wifi_event));
    memset(evt_pos,     0, sizeof(evt_pos));

    // Read enough data from serial port for the BGAPI message header + Event
    retError = Comm_Uart_Wifi_Receive_CFG_FW (wifi_buffer, length, tout, ack, &received_length, wait_length);
    if(retError != eERR_NONE)
    {
        return FALSE;
    }

    memcpy (wifi_resp, wifi_buffer, BGLIB_MSG_HEADER_LEN);
    memcpy (&aux, wifi_resp, sizeof(uint32_t));
    msg_header = &aux;

    // The buffer now contains the message header
    // See BGAPI protocol definition for details on packet format
    msg_length = BGLIB_MSG_LEN(msg_header);

    // Read the payload data if required and store it after the header.
    if (msg_length)
    {
        memcpy(&wifi_resp  [BGLIB_MSG_HEADER_LEN],
               &wifi_buffer[BGLIB_MSG_HEADER_LEN],
               msg_length);
    }
    msg_id = BGLIB_MSG_ID(msg_header);
    if (msg_id != ack)
    {
       return FALSE;
    }

    pck_resp = BGLIB_MSG(wifi_resp);

    // Check if there is an event
    if (wifi_buffer[BGLIB_MSG_HEADER_LEN + msg_length] == (char)0x88)
    {
        for (i=j=0; i<length; i++)
        {
            if (wifi_buffer[i] == (char)0x88)
            {
                evt_pos[j] = (uint16_t) i;
                j++;
            }
        }

        if(wifi_rsp_endpoint_send_id == ack)
        {
            for (i=0; i < BGLIB_MAX_EVT; i++)
            {
                memcpy (&wifi_event[i], &wifi_buffer[evt_pos[i]], (size_t) (received_length-(evt_pos[i+1])));
                pck_evt[i] = BGLIB_MSG(&wifi_event [i]);
                if(evt_pos[i+1] == 0)
                {
                    break;
                }
            }
        }
        else
        {
            for (i=0; i < BGLIB_MAX_EVT; i++)
            {
                memcpy (&wifi_event[i], &wifi_buffer[evt_pos[i]], (size_t) (length-(evt_pos[i+1])));
                pck_evt[i] = BGLIB_MSG(&wifi_event [i]);
                if(evt_pos[i+1] == 0)
                {
                    break;
                }
            }
        }
    }

    return TRUE;
}

/******************************************************************************
** Name:    Wifi_Command_Response_Data
*****************************************************************************/
/**
** @brief   Waits for a response to a given command
**
** @param   ack   expected response number 1
** @param   tout  timeout for the response
**
** @return  bool_t  true if received expected response, false if timeout expired
******************************************************************************/
static bool_t Wifi_Command_Response_Data (uint32_t ack, uint32_t tout)
{
    char wifi_buffer[1024];
    ERROR_ID_e  retError;
    uint32_t    msg_id = 0;
    UNUSED(msg_id);
    uint8_t     wifi_resp[20];
    uint32_t    aux=0;                    // aux variable to copy msg header to msg header pointer
    uint32_t    *msg_header=NULL;         // pointer to message header (ID + length)

    UNUSED(retError);

    memset(wifi_buffer,0,sizeof(wifi_buffer));
    memset(wifi_resp,   0, sizeof(wifi_resp));

    // Read enough data from serial port for the BGAPI message header + Event
    retError = Comm_Uart_Wifi_Receive (wifi_buffer, 0, tout, 0);
    //memcpy (&msg_id, wifi_buffer, sizeof(uint32_t));
    memcpy (wifi_resp, wifi_buffer, BGLIB_MSG_HEADER_LEN);
    memcpy (&aux, wifi_resp, sizeof(uint32_t));
    msg_header = &aux;

    //msg_id = BGLIB_MSG_ID(msg_header);

    if (BGLIB_MSG_ID(msg_header) != ack)
    {
        msg_id = BGLIB_MSG_ID(pck_resp->header);
        return FALSE;
    }

    pck_resp   = BGLIB_MSG(&wifi_buffer[0]);
    pck_evt[0] = BGLIB_MSG(&wifi_buffer[7]);

    return TRUE;
}

/******************************************************************************
** Name:    Wifi_Save_MAC_Address
*****************************************************************************/
/**
** @brief   Parse WGM110 MAC Address format (HEX) to ASCII and save it
**
** @param   none
**
** @return  none
******************************************************************************/
static void  Wifi_Save_MAC_Address(void)
{
    hw_addr mac_address;

    // get module MAC address
    memset(R100_wifi.mac_address, 0, MAX_MAC_ADDR_LENGTH);

    mac_address = pck_evt[0]->evt_config_mac_address.mac;

    sprintf (R100_wifi.mac_address, "%.2x:%.2x:%.2x:%.2x:%.2x:%.2x",
            mac_address.addr[0],
            mac_address.addr[1],
            mac_address.addr[2],
            mac_address.addr[3],
            mac_address.addr[4],
            mac_address.addr[5]);
}

/******************************************************************************
** Name:    Wifi_Save_IPv4_Address
*****************************************************************************/
/**
** @brief   Parse WGM110 IPv4 Address format (HEX) to ASCII and save it
**
** @param   none
**
** @return  none
******************************************************************************/
/*static void  Wifi_Save_IPv4_Address(void)
{
    ipv4 ip_address;

    // get module IPv4 address
    memset(R100_wifi.ip_address, 0, MAX_IPV4_ADDR_LENGTH);

    ip_address = pck_evt[1]->evt_tcpip_configuration.address; // ip_address = pck_evt[3]->evt_tcpip_configuration.address;

    sprintf (R100_wifi.ip_address, "%u.%u.%u.%u",
            ip_address.a[0],
            ip_address.a[1],
            ip_address.a[2],
            ip_address.a[3]);
}*/
/*lint -restore*/

/******************************************************************************
** Name:    Wifi_Fill_Header
*****************************************************************************/
/**
** @brief   Fill the wifi header to send
**
** @param   id identifier
** @param   file_size size of file
** @param   crc_16 CRC 16 of file
**
** @return  none
******************************************************************************/
static void Wifi_Fill_Header(uint8_t id, uint64_t file_size, uint16_t crc_16)
{
    uint32_t    tag_version = 0;    // TAG Version

    memset ((uint8_t *) &wifi_header, 0, sizeof (WIFI_HEADER_t));

    tag_version = APP_REV_CODE;
    wifi_header.version = WIFI_SWAP_UINT32(tag_version);
    memcpy(wifi_header.s_n, (uint8_t*)&(Get_Device_Info()->sn), 16);
    wifi_header.id = id;
    //Device_pw (access_key, (uint8_t *) Get_Device_Info()->sn, RSC_NAME_SZ);
    memcpy(wifi_header.password, access_key, 8);
    //wifi_header.password[0] = 'a';
    //wifi_header.password[63] = 'b';
    wifi_header.file_size = WIFI_SWAP_UINT64(file_size);
    wifi_header.wifi_cheksum = WIFI_SWAP_UINT16(crc_16);
    //wifi_header.file_size = 0x0807060504030201;
    //wifi_header.wifi_cheksum = 0x0802;
    memcpy(wifi_header.ssid, R100_wifi.wlan_ssid, 32);
    wifi_header.rssid = (uint8_t)(R100_wifi.signal_rssi * -1);

    wifi_header.must_be_0x55 = 0x55;
}
