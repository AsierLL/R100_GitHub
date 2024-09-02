/******************************************************************************
 * Name      : R100_S3A7                                                      *
 * Copyright : Osatu S. Coop                                                  *
 * Compiler  : GNU ARM Embedded                                               *
 * Target    : Reanibex Serie 100                                             *
 ******************************************************************************/

/*!
 * @file        bsp_wifi_silabs-WGM110.h
 * @brief       BSP Wifi Silicon Labs WGM110 header file
 *
 * @version     v1
 * @date        03/05/2019
 * @author      jarribas
 * @warning     author   date    version     .- Edition
 * @bug
 *
 */

#ifndef BSP_WIFI_SILABS_H_
#define BSP_WIFI_SILABS_H_
/******************************************************************************
 **Includes
 */
#include "R100_Errors.h"
#include "types_basic.h"

/******************************************************************************
 ** Defines
 */
#define TRY_AGAIN       1

// Response data
#define BGLIB_RESP_NO_BYTES  0
#define BGLIB_RESP_1_BYTE    1
#define BGLIB_RESP_2_BYTES   2
#define BGLIB_RESP_3_BYTES   3
#define BGLIB_RESP_4_BYTES   4
#define BGLIB_RESP_8_BYTES   8
#define BGLIB_RESP_9_BYTES   9
#define BGLIB_RESP_16_BYTES  16
#define BGLIB_RESP_18_BYTES  18
#define BGLIB_RESP_19_BYTES  19

// Event Data
#define BGLIB_MAX_EVT  5 ///< Max events to be stored

#define BGLIB_NO_EVT                        0
#define BGLIB_EVT_HW_UART_CONF              13
#define BGLIB_EVT_GET_MAC                   11
#define BGLIB_EVT_WIFI_ON_OFF               6
#define BGLIB_EVT_SCAN_RESULT               16
#define BGLIB_EVT_CONNECTED                 12
#define BGLIB_EVT_TCPIP_CONF                17
#define BGLIB_EVT_TCPIP_ENDP_STS            17
#define BGLIB_EVT_TCPIP_UDP_DATA            13
#define BGLIB_EVT_TCPIP_DNS_HOSTNAME        11
#define BGLIB_EVT_DNS                       9
#define BGLIB_EVT_ENDP_STS                  12
#define BGLIB_EVT_ENDP_DATA                 13
#define BGLIB_EVT_ENDP_DATA_6               6
#define BGLIB_EVT_ENDP_DATA_5               5
#define BGLIB_EVT_ENDP_DATA_2               2
#define BGLIB_EVT_RESPONSE                  8
#define BGLIB_EVT_TLS_VERIFY                7
#define BGLIB_EVT_X509_CERT                 8
#define BGLIB_EVT_X509_ADD_CERT_FINISH      7
#define BGLIB_EVT_X509_CERT_FINGER          16
#define BGLIB_EVT_X509_CERT_SUB             6
#define BGLIB_EVT_X509_CERT_SUB_LEN         54
#define BGLIB_EVT_X509_CERT_LIST            4

#define MAX_WIFI_HEADER_BYTES               129     ///< WIFI header bytes size
#define MAX_WIFI_TEST_FRAME                 153     ///< WIFI test frame size
#define MAX_WIFI_ALERT_GPS                  141     ///< WIFI alerts and GPS data size
#define MAX_WIFI_READ_FILE                  253     ///< WIFI read data size
#define WIFI_ID_PAYLOAD_SIZE                2       ///< WIFI payload size

#define NONE                                0   ///< No authentication method
#define TLS                                 1   ///< TLS authentication method
#define PEAP                                2   ///< PEAP authentication method
#define MSCHAPv2                            3   ///< MSCHAPv2 authentication method

#define DHCP_ON                             1   ///< Activate DHCP
#define DHCP_OFF                            0   ///< Desactivate DHCP

#define PORT_HOST_ALIVE                     20000
#define PORT_DATA_FRAME                     20001
#define PORT_TEST                           20002
#define PORT_EPI                            20003
#define PORT_FW                             20004
#define PORT_CFG                            20005

#define CERT_FOLDER_NAME                    "CERT"      ///< CERT folder
#define CERT_FILENAME                       "cert.dat"  ///< Cert configuration filename

/******************************************************************************
 ** Typedefs
 */
typedef struct
{
    char evt[50];

} wifi_evt_t;

#pragma pack(1)
// Wifi protocol first package to be sended
typedef struct
{
    uint32_t    version;        ///< SW version
    uint8_t     s_n[16];        ///< Device serial number
    uint8_t     id;             ///< Identifier(1-Test frame; 2-Alert frame; 3- GPS frame)
    uint8_t     password[64];   ///< Password
    uint64_t    file_size;      ///< File size
    uint16_t    wifi_cheksum;   ///< Data CRC
    uint8_t     ssid[32];       ///< WIFI SSID
    uint8_t     rssid;          ///< WIFI RSSID
    uint8_t     must_be_0x55;   ///< Must be 0x55

} WIFI_HEADER_t;

COMPILE_ASSERT((sizeof(WIFI_HEADER_t) == MAX_WIFI_HEADER_BYTES));

typedef struct
{
    uint16_t    id_payload;     ///< ID of the frame to be sent

}WIFI_PAYLOAD_t;

#pragma pack()

// structure to know if you have to load certificates
typedef struct
{
    char_t wifi_fingerprint[16];    ///< Certificate fingerprint use to configure
    uint8_t wifi_load_eap;          ///< To know if you have to upload the certificate for EAP (0-Load certificate; 1-Certificate is loaded)
    uint8_t wifi_load_tls;          ///< To know if you have to upload the certificate for TLS (0-Load certificate; 1-Certificate is loaded)

}WIFI_CERT_t;

/******************************************************************************
 ** Globals
 */

/******************************************************************************
 ** Prototypes
 */

void          Wifi_Initialize_BGLib                 (void);
ERROR_ID_e    Wifi_Set_Operating_Mode               (void);
ERROR_ID_e    Wifi_Check_Module_Status              (void);
ERROR_ID_e    Wifi_Configure_Module_Uart            (void);
ERROR_ID_e    Wifi_Get_Module_Status                (void);
ERROR_ID_e    Wifi_Get_Module_MAC_Address           (void);
ERROR_ID_e    Wifi_Get_RSSI                         (void);
ERROR_ID_e    Wifi_Configure_TCPIP_DHCP_Settings    (bool_t dhcp);
ERROR_ID_e    Wifi_Configure_DNS_Settings           (void);

ERROR_ID_e    Wifi_Set_Wifi_Radio_On                (void);
ERROR_ID_e    Wifi_Set_Wifi_Radio_Off               (void);

ERROR_ID_e    Wifi_Scan_Network                     (void);

ERROR_ID_e    Wifi_Load_EAP_TLS_Cert                (void);
ERROR_ID_e    Wifi_Load_Server_TLS_Cacert           (void);
ERROR_ID_e    Wifi_Load_EAP_CA_Cert                 (void);
ERROR_ID_e    Wifi_Load_EAP_PEAP_Cert               (uint8_t peap);
ERROR_ID_e    Wifi_No_Load_EAP_TLS_Cert             (char_t* fingerprint);

ERROR_ID_e    Wifi_Config_Certs                     (bool_t write_read, uint8_t eap, uint8_t tls, char_t* write_fingerprint);
ERROR_ID_e    Wifi_Reset_Certs_Store                (void);

ERROR_ID_e    Wifi_Set_Password                     (uint8_t eap);
ERROR_ID_e    Wifi_Connect_Network                  (void);
ERROR_ID_e    Wifi_Disconnect_Network               (void);

ERROR_ID_e    Wifi_Module_Reset                     (void);

ERROR_ID_e    Wifi_Open_TCP_Client                  (void);
ERROR_ID_e    Wifi_Close_TCP_Client                 (void);
ERROR_ID_e    Wifi_Open_UDP_Client                  (void);
ERROR_ID_e    Wifi_Close_UDP_Client                 (void);
ERROR_ID_e    Wifi_Start_UDP_Server                 (void);
ERROR_ID_e    Wifi_Close_UDP_Server                 (void);

ERROR_ID_e    Wifi_Open_TLS_Client                  (void);
ERROR_ID_e    Wifi_Close_TLS_Client                 (void);

ERROR_ID_e    Wifi_Is_Host_Alive                    (void);
ERROR_ID_e    Wifi_Send_Data_Frame                  (uint8_t* data, uint8_t size_data, uint8_t frame_type);
ERROR_ID_e    Wifi_Send_Test                        (void);
ERROR_ID_e    Wifi_Send_Test_Result                 (void);
ERROR_ID_e    Wifi_Send_Episode                     (void);
ERROR_ID_e    Wifi_Send_Episode_Result              (void);
ERROR_ID_e    Wifi_Receive_Firmware                 (void);
ERROR_ID_e    Wifi_Receive_Firmware_Result          (void);
void          Wifi_Delete_Firmware                  (void);
ERROR_ID_e    Wifi_Receive_Configuration            (void);
ERROR_ID_e    Wifi_Receive_Configuration_Result     (void);

#endif /* BSP_BSP_WIFI_ZENTRI_H_ */
