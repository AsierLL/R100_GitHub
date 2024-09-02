/******************************************************************************
 * Name      : S3A7_REANIBEX_100                                              *
 * Copyright : Osatu S. Coop                                                  *
 * Compiler  : MinGW32                                                        *
 * Target    : Reanibex Series                                                *
 ******************************************************************************/

/*!
 * @file        thread_comm_hal.h
 * @brief       Header with functions related to the Comm BSP service
 *
 * @version     v1
 * @date        03/05/2019
 * @author      jarribas
 * @warning     author   date    version     .- Edition
 * @bug
 *
 */
#ifndef THREAD_COMM_HAL_H_
#define THREAD_COMM_HAL_H_

/******************************************************************************
 ** Includes
 */
/*lint -save -e537 Spurious warning ignored due to include guards*/
#include "R100_Errors.h"
#include "types_basic.h"
/*lint -restore*/
#include "thread_patMon_hal.h"
#include "DB_Test.h"
#include "I2C_1.h"
#include "bsp_acc_LIS3DH.h"

/******************************************************************************
 ** Defines
 */
// I/O ports
#define     MIKROBUS_RST   IOPORT_PORT_06_PIN_09
#define     MIKROBUS_AN    IOPORT_PORT_00_PIN_06
#define     MIKROBUS_PWM   IOPORT_PORT_01_PIN_12

//#define     I2C_UART_CMD(x , y)       ((x << 2) | y)
//#define     I2C_RESET_CMD(x , y)      ((y << 5) & x)

#define     I2C_PCF8574_DATA(x , y)       (x & y)

/*lint -save -e146 compiler supports binary constants*/
#define     I2C_DEFAULT_DATA  0b11111111        ///< PCF8574 P0 ... P7 = 1

#define     I2C_RESET_GPS     0b11111110        ///< PCF8574 P0 = 0 ; P1 = 1 ; P2 = 1
#define     I2C_RESET_WIFI    0b11111101        ///< PCF8574 P0 = 1 ; P1 = 0 ; P2 = 1
#define     I2C_RESET_SIGFOX  0b11111011        ///< PCF8574 P0 = 1 ; P1 = 1 ; P2 = 0

#define     I2C_UART_GPS      0b01111111        ///< PCF8574 P6 = 1 ; P7 = 0   SN74CBT3253 S1 = 0; S0 = 1
#define     I2C_UART_WIFI     0b10111111        ///< PCF8574 P6 = 0 ; P7 = 1   SN74CBT3253 S1 = 1; S0 = 0
#define     I2C_UART_SIGFOX   0b11111111        ///< PCF8574 P6 = 1 ; P7 = 1   SN74CBT3253 S1 = 1; S0 = 1
#define     I2C_UART_FREE     0b00111111        ///< PCF8574 P6 = 0 ; P7 = 0   SN74CBT3253 S1 = 0; S0 = 0
/*lint -restore*/

#define MAX_WLAN_SSID_LENGTH        32
#define MAX_WLAN_PASS_LENGTH        32
#define MAX_IPV4_ADDR_LENGTH        16
#define MAX_MAC_ADDR_LENGTH         18
#define MAX_HOST_NAME_LENGTH        64
#define MAX_WIFI_DATA_SZ            12
#define MAX_WIFI_DATA_TEST_SZ       24

#define WIFI_FILE_FRAME             0x30
#define WIFI_TEST_FRAME             0x31
#define WIFI_ALERT_FRAME            0x32
#define WIFI_GPS_FRAME              0x33

#define WIFI_CERT_READ               0
#define WIFI_CERT_WRITE              1
#define WIFI_EAP_LOADED              1
#define WIFI_FP_LOADED               2
#define WIFI_TLS_LOADED              1

#define MAX_SIGFOX_DATA_SZ          12
#define MAX_SIGFOX_DEV_ID_LENGTH    10
#define MAX_SIGFOX_DEV_PAC_LENGTH   18
#define MAX_SIGFOX_FREQ_LENGTH      11

#define SIGFOX_TEST_TX_FREQ "868130000"
#define SIGFOX_TEST_RX_FREQ "869525000"

#define SIGFOX_DOWNLINK_OFF         0
#define SIGFOX_DOWNLINK_ON          1

#define MAX_GPS_DEV_ID_LENGTH       8

#define MAX_ACC_DEV_ID_LENGTH       4
#define MAX_ACC_XYZ_CONF_LENGTH     9

#define ACC_ENABLE_XYZ                  0x07        ///< Enable X Y Z axis
///< Low power mode: 0x01(1 Hz), 0x02(10 Hz), 0x03(25 Hz), 0x04(50 Hz), 0x05(100 Hz), 0x06(200 Hz), 0x07(400 Hz), 0x08(1.60 kHz)
#define ACC_SAMPLE_RATE_PD              0x00        ///< Power-down mode
#define ACC_SAMPLE_RATE_25              0x03        ///< Low power mode: 0x03(25 Hz)
#define ACC_SAMPLE_RATE_50              0x04        ///< Low power mode: 0x04(50 Hz)

#define ACC_2G_ACCEL_RANGE              0x00        ///< 2g

/// COMM Modules
typedef enum
{
    eMOD_NONE = 0,      ///< No select Module
    eMOD_WIFI,          ///< Wifi Module
    eMOD_SIGFOX,        ///< Sigfox Module
    eMOD_GPS            ///< GPS Module
} COMM_MOD_e;

/// Wifi Security
typedef enum
{
    eWIFI_OPEN_SEC = 0,         ///< Open Network without password
    eWIFI_WPA,                  ///< WPA-Personal Security
    eWIFI_EAP_TLS_SEC,          ///< WPA-Enterprise Security With EAP TLS
    eWIFI_EAP_PEAP_SEC          ///< WPA-Enterprise Security With EAP PEAP 
} WIFI_SEC_e;

/// Wifi Advance Settings
typedef enum
{
    eWIFI_NO = 0,           ///< No configure
    eWIFI_ONLY_IP,          ///< Only configure IP
    eWIFI_ONLY_DNS,         ///< Only configure DNS
    eWIFI_IP_DNS            ///< Configure IP & DNS
} WIFI_ADVS_e;

/******************************************************************************
 ** Typedefs
 */
typedef struct {
    WIFI_SEC_e wlan_eap_security;
    char_t wlan_ssid[MAX_WLAN_SSID_LENGTH];
    char_t wlan_pass[MAX_WLAN_PASS_LENGTH];
    char_t router_pass_key[MAX_WLAN_SSID_LENGTH];
    char_t eap_cnf[MAX_WLAN_PASS_LENGTH];
    char_t eap_pass[MAX_WLAN_PASS_LENGTH];
    WIFI_ADVS_e wlan_advs;
    char_t ip[MAX_IPV4_ADDR_LENGTH];
    char_t mask[MAX_IPV4_ADDR_LENGTH];
    char_t gateway[MAX_IPV4_ADDR_LENGTH];
    char_t pdns[MAX_IPV4_ADDR_LENGTH];
    char_t sdns[MAX_IPV4_ADDR_LENGTH];
    char_t ip_address[MAX_IPV4_ADDR_LENGTH];
    char_t mac_address[MAX_MAC_ADDR_LENGTH];
    char_t host_name[MAX_HOST_NAME_LENGTH];
    int8_t signal_rssi;
} wifi_config_t;

typedef struct {
    char_t device_id[MAX_SIGFOX_DEV_ID_LENGTH + 1];
    char_t device_pac[MAX_SIGFOX_DEV_PAC_LENGTH + 1];
    char_t device_tx_freq[MAX_SIGFOX_FREQ_LENGTH + 1];
    char_t device_rx_freq[MAX_SIGFOX_FREQ_LENGTH + 1];
} sigfox_config_t;

typedef struct {
    char_t device_id[MAX_GPS_DEV_ID_LENGTH];
    char lat_data[12];
    char N_S;
    char long_data[12];
    char E_W;
} gps_config_t;

typedef struct {
    char_t  device_id[MAX_ACC_DEV_ID_LENGTH];
    uint8_t xyz_config;
    uint8_t hp_filter;
    uint8_t interrupt;
    int16_t x_data;
    int16_t y_data;
    int16_t z_data;
} acc_config_t;

//////////////////////////////// WIFI DATA STRUCTURES ///////////////////////////////

/// Wifi Message IDs (same as SIGFOX_MSG_ID_e)
typedef enum
{
    WIFI_MSG_ID_POWER_ON_TEST  = 0,  ///< message identifier to report the power on test result
    WIFI_MSG_ID_DIARY_TEST,          ///< message identifier to report the diary autotest result
    WIFI_MSG_ID_MONTHLY_TEST,        ///< message identifier to report the monthly autotest result
    WIFI_MSG_ID_MANUAL_TEST,         ///< message identifier to report the manual test result
    WIFI_MSG_ID_FUNC_TEST,           ///< message identifier to report the functional test error
    WIFI_MSG_ID_COVER_OPEN_ALERT,    ///< message identifier to report the cover broken alert
    WIFI_MSG_ID_COVER_CLOSE_ALERT,   ///< message identifier to report the cover broken alert
    WIFI_MSG_ID_NO_SHOCK_ALERT,      ///< message identifier to report the no shock alert
    WIFI_MSG_ID_SHOCK_DISC_ALERT,    ///< message identifier to report the shock and elec. disconnected alert
    WIFI_MSG_ID_SHOCK_TIMEOUT_ALERT, ///< message identifier to report the shock timeout alert
    WIFI_MSG_ID_SHOCK_DONE_ALERT,    ///< message identifier to report the shock performed alert
    WIFI_MSG_ID_SAVED_POS_GPS,       ///< message identifier to report the gps position saved in the device
    WIFI_MSG_ID_MON_TEST_GPS,        ///< message identifier to report the gps position when monthly test performed
    WIFI_MSG_ID_MAN_TEST_GPS,        ///< message identifier to report the gps position when manual test performed
    WIFI_MSG_ID_CHANGED_POS_GPS,     ///< message identifier to report the gps position if changed during func
    WIFI_MSG_ID_ACCEL_POS_GPS,       ///< message identifier to report the gps position if accelerometer activated
    WIFI_MSG_ID_BATT_ELECT_EXP,      ///< message identifier to report the battery and electrodes expiration date
    WIFI_MSG_ID_ACCEL_ATIVATE_MOVE_ALERT,    ///< message identifier to report the accelerometer movement alert when the device is moving
    WIFI_MSG_ID_MAX
} WIFI_MSG_ID_e;

// Wifi data frame alignment in 1 byte. You can use pack or the attribute "__attribute__((__packed__))" in the structure instance
#pragma pack(1)

/// Wifi structure to report the test result (same as SIGFOX_TEST_RESULT_t)
typedef struct {
    WIFI_MSG_ID_e       info_type;              ///< identifies the information to report
    uint8_t             error_code_defib;       ///< error code in defibrillator circuits
    uint8_t             error_code_ADS;         ///< error code in ADS circuits
    uint8_t             error_code_dc_18v;      ///< error code in the 18V power supply
    uint8_t             error_code_dc_main;     ///< error code in the main power supply
    uint8_t             error_code_audio;       ///< error code in the audio circuits
    uint8_t             error_electrodes;       ///< error code in the pre-connected electrodes
    uint8_t             battery_soc;            ///< State of charge (%)
    uint8_t             battery_volt;           ///< battery voltage (volts * 10)
     int8_t             battery_temp;           ///< battery temperature (degrees)
    uint8_t             error_code;             ///< error code summary
    uint8_t             must_be_0x55;           ///< Fixed value = 0x55
    WIFI_MSG_ID_e       info_type_exp;          ///< identifies the information to report
    uint32_t            batt_expiration;        ///< battery expiration date
    uint32_t            elec_expiration;        ///< electrodes expiration date
    uint8_t             demo_mode;              ///< if device is configured in DEMO mode or not
    uint8_t             open_cover;             ///< open cover flag (0 - closed; 1 - opened)
    uint8_t             must_be_0x55_exp;       ///< Fixed value = 0x55
} WIFI_TEST_RESULT_t;

COMPILE_ASSERT((sizeof(WIFI_TEST_RESULT_t) == MAX_WIFI_DATA_TEST_SZ));

/// Wifi structure to report the functional alert (same as SIGFOX_FUNC_ALERT_t)
typedef struct {
    WIFI_MSG_ID_e       info_type;              ///< identifies the information to report
    EL_SIGN_e           electrodes_sign;        ///< electrodes signature
    uint8_t             battery_soc;            ///< State of charge (%)
    uint8_t             patient_type;           ///< patient selected on shock
    uint32_t            zp;                     ///< impedance on shock (ohms)
    uint16_t            shock_energy;           ///< energy on shock
    uint8_t             drd_diag;               ///< analisys diagnosis
    uint8_t             must_be_0x55;           ///< Fixed value = 0x55
} WIFI_FUNC_ALERT_t;

COMPILE_ASSERT((sizeof(WIFI_FUNC_ALERT_t) == MAX_WIFI_DATA_SZ));

/// Wifi structure to report the gps position (same as SIGFOX_GPS_POSITION_t)
typedef struct {
    WIFI_MSG_ID_e   info_type;          ///< identifies the information to report
    uint8_t         lat_grados;         ///< device latitude    MM
    uint8_t         lat_minutos_1;      ///< device latitude    mm
    uint16_t        lat_minutos_2;      ///< device latitude    mmmm
    uint16_t        long_grados;        ///< device longitude   MMM
    uint8_t         long_minutos_1;     ///< device longitude   mm
    uint16_t        long_minutos_2;     ///< device longitude   mmmm
    uint8_t         lat_long_dir;       ///< bit1: N or S; bit 0: E or W
    uint8_t         must_be_0x55;       ///< Fixed value = 0x55
} WIFI_GPS_POSITION_t;

COMPILE_ASSERT((sizeof(WIFI_GPS_POSITION_t) == MAX_WIFI_DATA_SZ));

// Wifi data frame alignment
#pragma pack()

/// Wifi structure to get the host server
typedef struct {
    uint8_t             must_be_0xAA;           ///< Fixed value = 0xAA
    uint32_t            timestamp;              ///< timestamp
    uint8_t             cfg_available;          ///< .cfg file available
    uint8_t             frmw_available;         ///< firmware file available
    uint8_t             must_be_0xBB;           ///< Fixed value = 0xBB
} WIFI_HOST_RESPONSE_t;

/// Wifi structure to get the host server
typedef struct {
    uint32_t            version;                ///< Version
    char_t              s_n[16];                ///< Serial number
    char_t              pass[64];               ///< Password or data hash
    uint64_t            file_size;              ///< File size
    uint16_t            crc_file;               ///< CRC file
} WIFI_SERVER_RESPONSE_t;


//////////////////////////////// SIGFOX DATA STRUCTURES ///////////////////////////////

/// Sigfox Message IDs
typedef enum
{
    MSG_ID_POWER_ON_TEST  = 0,  ///< message identifier to report the power on test result
    MSG_ID_DIARY_TEST,          ///< message identifier to report the diary autotest result
    MSG_ID_MONTHLY_TEST,        ///< message identifier to report the monthly autotest result
    MSG_ID_MANUAL_TEST,         ///< message identifier to report the manual test result
    MSG_ID_FUNC_TEST,           ///< message identifier to report the functional test error
    MSG_ID_COVER_OPEN_ALERT,    ///< message identifier to report the cover broken alert
    MSG_ID_COVER_CLOSE_ALERT,   ///< message identifier to report the cover broken alert
    MSG_ID_NO_SHOCK_ALERT,      ///< message identifier to report the no shock alert
    MSG_ID_SHOCK_DISC_ALERT,    ///< message identifier to report the shock and elec. disconnected alert
    MSG_ID_SHOCK_TIMEOUT_ALERT, ///< message identifier to report the shock timeout alert
    MSG_ID_SHOCK_DONE_ALERT,    ///< message identifier to report the shock performed alert
    MSG_ID_SAVED_POS_GPS,       ///< message identifier to report the gps position saved in the device
    MSG_ID_MON_TEST_GPS,        ///< message identifier to report the gps position when monthly test performed
    MSG_ID_MAN_TEST_GPS,        ///< message identifier to report the gps position when manual test performed
    MSG_ID_CHANGED_POS_GPS,     ///< message identifier to report the gps position if changed during func
    MSG_ID_ACCEL_POS_GPS,       ///< message identifier to report the gps position if accelerometer activated
    MSG_ID_BATT_ELECT_EXP,      ///< message identifier to report the battery and electrodes expiration date
    MSG_ID_ACCEL_ATIVATE_MOVE_ALERT,    ///< message identifier to report the accelerometer movement alert when the device is moving
    MSG_ID_MAX
} SIGFOX_MSG_ID_e;

// sigfox data frame alignment
#pragma pack(1)

/// Sigfox structure to report the test result
typedef struct {
    SIGFOX_MSG_ID_e     info_type;              ///< identifies the information to report
    uint8_t             error_code_defib;       ///< error code in defibrillator circuits
    uint8_t             error_code_ADS;         ///< error code in ADS circuits
    uint8_t             error_code_dc_18v;      ///< error code in the 18V power supply
    uint8_t             error_code_dc_main;     ///< error code in the main power supply
    uint8_t             error_code_audio;       ///< error code in the audio circuits
    uint8_t             error_electrodes;       ///< error code in the pre-connected electrodes
    uint8_t             battery_soc;            ///< State of charge (%)
    uint8_t             battery_volt;           ///< battery voltage (volts * 10)
     int8_t             battery_temp;           ///< battery temperature (degrees)
    uint8_t             error_code;             ///< error code summary
    uint8_t             must_be_0x55;           ///< Fixed value = 0x55
} SIGFOX_TEST_RESULT_t;

COMPILE_ASSERT((sizeof(SIGFOX_TEST_RESULT_t) == MAX_SIGFOX_DATA_SZ));

/// Sigfox structure to report the test result
typedef struct {
    SIGFOX_MSG_ID_e     info_type;              ///< identifies the information to report
    uint32_t            batt_expiration;        ///< battery expiration date
    uint32_t            elec_expiration;        ///< electrodes expiration date
    uint8_t             demo_mode;           ///< if device is configured in DEMO mode or not
    uint8_t             open_cover;             ///< open cover flag (0 - closed; 1 - opened)
    uint8_t             must_be_0x55;           ///< Fixed value = 0x55
} SIGFOX_ELEC_BATT_EXPIRATION_t;

COMPILE_ASSERT((sizeof(SIGFOX_ELEC_BATT_EXPIRATION_t) == MAX_SIGFOX_DATA_SZ));


/// Sigfox structure to report the functional alert
typedef struct {
    SIGFOX_MSG_ID_e     info_type;              ///< identifies the information to report
    EL_SIGN_e           electrodes_sign;        ///< electrodes signature
    uint8_t             battery_soc;            ///< State of charge (%)
    uint8_t             patient_type;           ///< patient selected on shock
    uint32_t            zp;                     ///< impedance on shock (ohms)
    uint16_t            shock_energy;           ///< energy on shock
    uint8_t             drd_diag;               ///< analisys diagnosis
    uint8_t             must_be_0x55;           ///< Fixed value = 0x55
} SIGFOX_FUNC_ALERT_t;

COMPILE_ASSERT((sizeof(SIGFOX_FUNC_ALERT_t) == MAX_SIGFOX_DATA_SZ));

/// Sigfox structure to report the gps position
typedef struct {
    SIGFOX_MSG_ID_e     info_type;          ///< identifies the information to report
    uint8_t             lat_grados;         ///< device latitude    MM
    uint8_t             lat_minutos_1;      ///< device latitude    mm
    uint16_t            lat_minutos_2;      ///< device latitude    mmmm
    uint16_t            long_grados;        ///< device longitude   MMM
    uint8_t             long_minutos_1;     ///< device longitude   mm
    uint16_t            long_minutos_2;     ///< device longitude   mmmm
    uint8_t             lat_long_dir;       ///< bit1: N or S; bit 0: E or W
    uint8_t             must_be_0x55;       ///< Fixed value = 0x55
} SIGFOX_GPS_POSITION_t;

COMPILE_ASSERT((sizeof(SIGFOX_GPS_POSITION_t) == MAX_SIGFOX_DATA_SZ));

#pragma pack()

/******************************************************************************
 ** Globals
 */

/******************************************************************************
 ** Prototypes
 */
void          Comm_Modules_Initialize             (void);
void          Comm_Hardware_Reset                 (COMM_MOD_e comm_mod);
void          Comm_Hardware_Off                   (COMM_MOD_e comm_mod);
void          Comm_Hardware_On                    (COMM_MOD_e comm_mod);
ERROR_ID_e    Comm_Select_Uart                    (COMM_MOD_e comm_uart);
COMM_MOD_e    Comm_Get_Selected_Uart              (void);
bool_t        Comm_Is_Sigfox_Free                 (void);
void          Comm_Modules_Print_Configuration    (COMM_MOD_e comm_mod);

ERROR_ID_e    Comm_Sigfox_Initialize              (void);
ERROR_ID_e    Comm_Sigfox_Execute_Test            (void);
ERROR_ID_e    Comm_Sigfox_Send_Test               (void);
ERROR_ID_e    Comm_Sigfox_Send_Alert              (void);
ERROR_ID_e    Comm_Sigfox_Send_Position           (void);
ERROR_ID_e    Comm_Sigfox_Send_Exp                (void);
ERROR_ID_e    Comm_Sigfox_Start_Uplink_Test       (void);
ERROR_ID_e    Comm_Sigfox_Stop_Uplink_Test        (void);
ERROR_ID_e    Comm_Sigfox_Start_Downlink_Test     (void);
ERROR_ID_e    Comm_Sigfox_Device_Info_Check       (void);

ERROR_ID_e    Comm_Wifi_Initialize                (void);
ERROR_ID_e    Comm_Wifi_Initialize_Fast           (void);
ERROR_ID_e    Comm_Wifi_Radio_Off                 (void);
ERROR_ID_e    Comm_Wifi_Reset                     (void);
ERROR_ID_e    Comm_Wifi_Execute_Test              (void);
ERROR_ID_e    Comm_Wifi_Is_Host_Server_Alive      (void);
ERROR_ID_e    Comm_Wifi_Send_Frame_Test           (void);
ERROR_ID_e    Comm_Wifi_Send_Frame_Alert          (void);
ERROR_ID_e    Comm_Wifi_Send_Frame_Alert_Aux      (void);
ERROR_ID_e    Comm_Wifi_Send_Frame_Gps            (void);
ERROR_ID_e    Comm_Wifi_Send_Last_Test            (void);
ERROR_ID_e    Comm_Wifi_Send_Last_Episode         (void);
ERROR_ID_e    Comm_Wifi_Get_Upgrade_Firmware      (void);
ERROR_ID_e    Comm_Wifi_Get_Configuration_File    (void);

ERROR_ID_e    Comm_Wifi_Send_R100_Info_Data       (void);
ERROR_ID_e    Comm_Wifi_Send_R100_TCP_Data        (void);
ERROR_ID_e    Comm_Wifi_Send_R100_UDP_Data        (void);
ERROR_ID_e    Comm_Wifi_Send_R100_TLS_Data        (void);

ERROR_ID_e    Comm_GPS_Initialize                 (void);
ERROR_ID_e    Comm_GPS_Execute_Test               (void);
ERROR_ID_e    Comm_GPS_Get_Position_Data          (void);
ERROR_ID_e    Comm_GPS_Send_Command               (void);

ERROR_ID_e    Comm_ACC_Initialize                 (void);
ERROR_ID_e    Comm_ACC_Execute_Test               (void);
ERROR_ID_e    Comm_ACC_Get_Acceleration_Data      (void);
uint8_t       Comm_ACC_Get_INT2                   (void);

bool_t        Comm_Wifi_Is_MAC_Valid              (char_t *s);
bool_t        Comm_Wifi_Is_IPv4_Valid             (char_t *s);

void          Comm_Wifi_Save_Host_Response        (char_t *resp);
uint8_t       Comm_Wifi_Is_Cfg_Available          (void);
uint8_t       Comm_Wifi_Is_Frmw_Available         (void);
void          Comm_Wifi_Save_Server_Response      (char_t *resp);
uint64_t      Comm_Wifi_SZ                        (void);
uint16_t      Comm_Wifi_CRC                       (void);
void          Comm_Wifi_PASS                      (uint8_t *password);

#endif /* HAL_THREAD_COMM_HAL_H_ */
