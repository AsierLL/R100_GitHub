/******************************************************************************
 * Name      : R100_S3A7                                                      *
 * Copyright : Osatu S. Coop                                                  *
 * Compiler  : GNU ARM Embedded                                               *
 * Target    : Reanibex Serie 100                                             *
 ******************************************************************************/

/*!
 * @file        device_init.h
 * @brief       Header file that manages the settings of the system
 *
 * @version     v1
 * @date        04/09/2018 
 * @author      ltorres
 * @warning     author   date    version     .- Edition
 * @bug
 */

#ifndef DEVICE_INIT_H_
#define DEVICE_INIT_H_

/******************************************************************************
 ** Includes
 */
#include "types_basic.h"

/******************************************************************************
 ** Defines
 */
#define RSC_NAME_SZ                 16          ///< Size for generic names used in resources

// Application revision code
#define APP_REV_CODE                0x30323130  ///< 02.10
                                                ///< MM.nn
                                                ///< MM -> Tag mayor version
                                                ///< mm -> Tag minor version
#define NFC_REV_CODE                0x30323031  ///< 02.01
                                                ///< MM.nn
                                                ///< MM -> NFC mayor version
                                                ///< mm -> NFC minor version

#define     FNAME_VERSION           "version.info"      ///< SW version info
#define     UPGRADE_FILENAME        "firmware.srec"     ///< Upgrade filename
#define     GOLDEN_FILENAME         "golden.srec"       ///< Golden filename
#define     CFG_FILENAME            "R100.cfg"          ///< Configuration filename
#define     USB_FILENAME            "USB.cfg"           ///< Configuration filename
#define     WIFI_FILENAME           "wifi.dat"          ///< Encrypted Wifi configuration filename
#define     WIFI_FILENAME_PLAIN     "wifi.plain"        ///< Aux wifi configuration filename
#define     FNAME_DATE              "fw_date.info"      ///< Firmware update date
#define     UPDATE_REG              "update_reg.log"    ///< Update registers log

///< device options bitset
#define ENABLE_OPT_ACCEL            0x08        ///< Enables the Accel
#define ENABLE_OPT_WIFI             0x04        ///< Enables the Wifi
#define ENABLE_OPT_SIGFOX           0x02        ///< Enables the Sigfox
#define ENABLE_OPT_GPS              0x01        ///< Enables the GPS

#define NFC_BYTES_PER_PAGE          16          ///< Number of bytes in a single page
#define NFC_LOCKING_BLOCK            0          ///< 1st block to store the locking bytes
#define NFC_DEVICE_ID_BLOCK          1          ///< 1st block to store the device identifier
#define NFC_DEVICE_INFO_BLOCK        4          ///< 1st block to store the device info
#define NFC_DEVICE_SETTINGS_BLOCK   11          ///< 1st block to store the device settings
#define NFC_DEVICE_COMM_BLOCK       15          ///< 1st block to store the device communication settings
#define NFC_DEVICE_COMM_KEY         31          ///< 1st block to store the device communication key

#define MAX_RCP_TIME                180         ///< Maximum RCP time (in seconds)
#define MAX_ENERGY_A                200         ///< Maximum energy for patient adult (joules)
#define MAX_ENERGY_P                100          ///< Maximum energy for patient pediatric (joules)
#define MAX_CONSEC_SHOCKS           4           ///< Maximum number of consecutive shocks
#define MAX_ASYSTOLE_TIME           60          ///< Maximum asystole time (in minutes)

#define MIN_ENERGY_A                150         ///< Minimum energy for patient adult (joules)
#define MIN_ENERGY_P                50          ///< Minimum energy for patient pediatric (joules)

#define MAX_AUD_VOLUME              8           ///< Maximum audio volume setting

/******************************************************************************
 ** Typedefs
 */

typedef enum {
    DEVELOP_DISABLE = 0,                        ///< Development operations disabled
    DEVELOP_TEST_2500_eSHOCK,                   ///< Test a 2500 shocks series over an external resistor
    DEVELOP_TEST_500_iSHOCK,                    ///< Test a  500 shocks series over an internal resistor
    DEVELOP_FORCE_SHOCKABLE,                    ///< Force a shockable rythm
    DEVELOP_TEST_RELAY,                         ///< Test the security relay
    DEVELOP_ZP_CALIBRATION,                     ///< DEVELOP_ZP_CALIBRATION
    DEVELOP_SAVE_GPS_POSITION,                  ///< DEVELOP_SAVE_GPS_POSITION
    DEVELOP_MANUFACTURE_CONTROL,                ///< Special SW for Osatu production
    DEVELOP_DELETE_ERRORS,                      ///< Special SW for Osatu production, delete ther error in internal memory
    DEVELOP_ATECC,                              ///< Configure and save keys in secure element ATECC608B
} DEVELOP_OPERATIONS_t;

/// Device Date
typedef struct
{
    uint8_t     year;          ///< year  (starting from 2000)
    uint8_t     month;         ///< month (1:12)
    uint8_t     date;          ///< date  (1:31)
} DEVICE_DATE_t;

/// Device Time
typedef struct
{
    uint8_t     hour;          ///< hour (0:23)
    uint8_t     min;           ///< minutes (0:59)
    uint8_t     sec;           ///< seconds (0:59)
} DEVICE_TIME_t;

/// Device Information
typedef struct
{
    char_t          model            [RSC_NAME_SZ];     ///< Device model
    char_t          sn               [RSC_NAME_SZ];     ///< Device serial number
    char_t          manufacture_date [RSC_NAME_SZ];     ///< Manufacturing date

    union
    {
        uint16_t    device_options;                     ///< bitset for device options
        struct
        {
            uint16_t gps            :1;                 ///< GPS
            uint16_t sigfox         :1;                 ///< Sigfox platform
            uint16_t wifi           :1;                 ///< Wifi remote access
            uint16_t accelerometer  :1;                 ///< Accelerometer for wake-up
            uint16_t reserved       :3;                 ///< reserved for future use
            uint16_t aed_fully_auto :1;                 ///< Fully auto AED
        } enable_b; /*!< BitSize */
    };

    uint32_t        zp_adcs_short;                      ///< Zp with electrodes in short-circuit (in ADCs)
    uint32_t        zp_adcs_calibration;                ///< Zp input connected to calibration (in ADCs)
    uint16_t        develop_mode;                       ///< Development mode
    uint8_t         align_2;
    uint8_t         checksum;                           ///< Checksum of data integrity
} DEVICE_INFO_t;

/// AED settings
typedef struct
{
    // Configuration - AED
    uint16_t    aed_aCPR_init;              ///< Adult -> Initial CPR time in seconds (can be 0)
    uint16_t    aed_aCPR_1;                 ///< Adult -> After Shock Series CPR time in seconds (can be OFF)
    uint16_t    aed_aCPR_2;                 ///< Adult -> No Shock Advised CPR time in seconds (can be OFF)
    uint16_t    aed_pCPR_init;              ///< Pediatric -> Initial CPR time in seconds (can be 0)
    uint16_t    aed_pCPR_1;                 ///< Pediatric -> After Shock Series CPR time in seconds (can be OFF)
    uint16_t    aed_pCPR_2;                 ///< Pediatric -> No Shock Advised CPR time in seconds (can be OFF)
    uint16_t    aed_cpr_msg_long;           ///< Long/Short audio messages for CPR (0-Short; 1-Long)
    uint16_t    aed_analysis_cpr;           ///< Enables start an analysis cycle in CPR (0-No; 1-Yes)
    uint16_t    aed_asys_detect;            ///< Enables Asystole warning message (0-No; 1-Yes)
    uint16_t    aed_asys_time;              ///< Time on asystole before emitting asystole warning message (minutes)
    uint16_t    aed_metronome_en;           ///< Enables metronome for marking compression pace (0-No; 1-Yes)
    uint16_t    aed_feedback_en;            ///< Enables cpr feedback for marking compression pace (0-No; 1-Yes)
    uint16_t    aed_consec_shocks;          ///< Max Shocks in a Shock Series (1:4)
    uint16_t    aed_energy_shock1_a;        ///< Energy for 1st shock in shock series for adult (150J, 175J, 200J)
    uint16_t    aed_energy_shock2_a;        ///< Energy for 2nd shock in shock series for adult (150J, 175J, 200J)
    uint16_t    aed_energy_shock3_a;        ///< Energy for 3rd shock in shock series for adult (150J, 175J, 200J)
    uint16_t    aed_metronome_ratio_a;      ///< Metronome pace for adult (0-15:2; 1-30:2; 2-Only compressions)
    uint16_t    aed_energy_shock1_p;        ///< Energy for 1st shock in shock series for pediatric (50J, 65J, 75J, 90J)
    uint16_t    aed_energy_shock2_p;        ///< Energy for 2nd shock in shock series for pediatric (50J, 65J, 75J, 90J)
    uint16_t    aed_energy_shock3_p;        ///< Energy for 3rd shock in shock series for pediatric (50J, 65J, 75J, 90J)
    uint16_t    aed_metronome_ratio_p;      ///< Metronome pace for pediatric (0-15:2; 1-30:2; 2-Only compressions)
    uint16_t    aed_metronome_rate;         ///< Metronome rate in compressions per minute (100; 110; 120;)
} AED_SETTINGS_t;

/// Miscellaneous settings
typedef struct
{
    uint16_t    glo_filter;                 ///< Type of filter
    uint8_t     glo_auto_test_hour;         ///< Auto test hour (hours)
    uint8_t     glo_auto_test_min;          ///< Auto test hour (minutes)
    uint16_t    glo_patient_adult;          ///< Patient type (0-pediatric 1-adult)
    uint16_t    glo_language;               ///< Audio language
    uint16_t    glo_volume;                 ///< Audio volume from a 8 level scale (0 to 8)
     int16_t    glo_utc_time;               ///< UTC time
    uint16_t    glo_ecg_format;             ///< ECG format standard
    uint16_t    glo_demo_mode;              ///< DEMO mode (0-disabled; 1-enabled)
    uint16_t    glo_patient_alert;          ///< Send alerts (0-disabled; 1-enabled)
    uint16_t    glo_movement_alert;         ///< Send alerts (0-disabled; 1-enabled)
    union
    {
        uint16_t    glo_transmis_mode;                     ///< Bitset for device options
        struct
        {
            uint16_t sigfox_only    :1;                 ///< Send only by Sigfox
            uint16_t sigfox_pri     :1;                 ///< Prioritize Sigfox
            uint16_t wifi_only      :1;                 ///< Sed only by WIFI
            uint16_t wifi_pri       :1;                 ///< Prioritize WIFI
            uint16_t gps            :1;                 ///< GPS enable
            uint16_t wifi_1_day     :1;                 ///< Send by wifi every 1 days
            uint16_t wifi_7_day     :1;                 ///< Send by wifi every 7 days
            uint16_t wifi_30_day    :1;                 ///< Send by wifi every 30 days
            uint16_t reserve        :8;                 ///< Reserve
        } alert_options; /*!< BitSize */
    };
    uint16_t    glo_warning_alert;          ///< Send alerts (0-disabled; 1-enabled)
} MISC_SETTINGS_t;

/// Communication settings
typedef struct
{
    uint16_t    wpa_eap_enabled;            ///< Use to define connection mode (0-Open; 1-Password;2-TLS;3-PEAP)
    uint16_t    advs;                       ///< Advanced settings (0-No;1-Only IP;2-Only DNS;3-IP + DNS)
    uint8_t     wlan_ssid [32];             ///< Wifi WLAN SSID (Net name) MAX: 32 characters
    uint8_t     wlan_pass [32];             ///< Wifi WLAN Passkey MAX: 32 characters
    uint8_t     router_pass_key [32];       ///< Wifi WLAN Router Passkey MAX: 32 characters
    uint8_t     eap_cnf [32];               ///< Wifi WLAN EAP Identity Configuration MAX: 32 characters
    uint8_t     eap_pass [32];              ///< Wifi WLAN EAP Password Configuration MAX: 32 characters
    uint8_t     ip[16];                     ///< Estatic IP address
    uint8_t     mask[16];                   ///< Subnet mask
    uint8_t     gateway[16];                ///< Gateway
    uint8_t     pdns[16];                   ///< Primary DNS
    uint8_t     sdns[16];                   ///< Secondary DNS
} COMM_SETTINGS_t;

/// Device settings
typedef struct
{
    AED_SETTINGS_t  aed;                    ///< AED settings
    uint8_t         align_1[2];             ///< Data align
    MISC_SETTINGS_t misc;                   ///< Miscellaneous  settings
    COMM_SETTINGS_t comm;                   ///< Communications settings (NOT USED!!!!! ONLY FOR ALIGNEMENT PURPOSES)
    uint8_t         align_2;                ///< Data align
    uint8_t         checksum;               ///< Checksum of data integrity
} DEVICE_SETTINGS_t;

/// Device settings
typedef struct
{
    uint16_t    aCPR_init;                  ///< Adult --> Initial CPR time in seconds (can be 0)
    uint16_t    aCPR_1;                     ///< Adult --> After Shock Series CPR time in seconds (can be OFF)
    uint16_t    aCPR_2;                     ///< Adult --> No Shock Advised CPR time in seconds (can be OFF)
    uint16_t    pCPR_init;                  ///< Pediatric --> Initial CPR time in seconds (can be 0)
    uint16_t    pCPR_1;                     ///< Pediatric --> After Shock Series CPR time in seconds (can be OFF)
    uint16_t    pCPR_2;                     ///< Pediatric --> No Shock Advised CPR time in seconds (can be OFF)
    uint16_t    cpr_msg_long;               ///< Long/Short audio messages for CPR
    uint16_t    analysis_cpr;               ///< Enables start an analysis cycle in CPR

    uint16_t    asys_detect;                ///< Enables Asystole warning message
    uint16_t    asys_time;                  ///< Time on asystole before emitting asystole warning message
    uint16_t    metronome_en;               ///< Enables metronome for marking compression pace
    uint16_t    metronome_ratio_a;          ///< Metronome pace for adult
    uint16_t    metronome_ratio_p;          ///< Metronome pace for pediatric
    uint16_t    metronome_rate;             ///< Metronome rate for both pediatric and adult
    uint16_t    feedback_en;                ///< Enables cpr feedback for marking compression pace
    uint16_t    consec_shocks;              ///< Max Shocks in a Shock Series

    uint16_t    energy_shock1_a;            ///< Energy for 1st shock in shock series for adult
    uint16_t    energy_shock2_a;            ///< Energy for 2nd shock in shock series for adult
    uint16_t    energy_shock3_a;            ///< Energy for 3rd shock in shock series for adult
    uint16_t    energy_shock1_p;            ///< Energy for 1st shock in shock series for pediatric
    uint16_t    energy_shock2_p;            ///< Energy for 2nd shock in shock series for pediatric
    uint16_t    energy_shock3_p;            ///< Energy for 3rd shock in shock series for pediatric
    uint16_t    patient_type;               ///< Patient type (0-Pediatric 1-Adult)
    uint16_t    mains_filter;               ///< Mains Filter (0-None, 1-50 Hz, 2-60 Hz)

    uint16_t    language;                   ///< Language to use in audio messages
     int16_t    utc_time;                   ///< UTC time (-11 to +12)
    uint16_t    audio_volume;               ///< Audio volume (1:8)
    uint16_t    demo_mode;                  ///< Demo_mode (0-Disabled; 1-Enabled)
    uint16_t    patient_alert;              ///< Send a alert in patient mode (0-Disabled; 1-Enabled)
    uint16_t    movement_alert;             ///< Send a alert in movement detection (0-Disabled; 1-Enabled)
    uint16_t    transmis_mode;              ///< Transmission mode (0-None;1-Sigfox;2-PriSigfox;4-Wifi;8-PriWifi;16-GPS;32-Diario;64-4 veces;128-1 vez)
    uint16_t    warning_alert;              ///< Emit an alert if warning is detected (0-Disabled; 1-Enabled)
} NFC_SETTINGS_t;

/// Device communication credentials
typedef struct
{
    uint16_t    wpa_eap_enabled;            ///< Use to define connection mode (0-Open; 1-Password;2-TLS;3-PEAP)
    uint16_t    advs;                       ///< Advanced settings (0-No;1-Only IP;2-Only DNS;3-IP + DNS)
    uint16_t    reserved_2 [6];             ///< ***** Reserved for future use
    uint8_t     wlan_ssid [32];             ///< Wifi WLAN SSID (Net name) MAX: 32 characters
    uint8_t     wlan_pass [32];             ///< Wifi WLAN Passkey MAX: 32 characters
    uint8_t     router_pass_key [32];       ///< Wifi WLAN Router Passkey MAX: 32 characters
    uint8_t     eap_cnf [32];               ///< Wifi WLAN EAP Identity Configuration MAX: 32 characters
    uint8_t     eap_pass [32];              ///< Wifi WLAN EAP Password Configuration MAX: 32 characters
    uint8_t     ip[16];                     ///< Estatic IP address
    uint8_t     mask[16];                   ///< Subnet mask
    uint8_t     gateway[16];                ///< Gateway
    uint8_t     pdns[16];                   ///< Primary DNS
    uint8_t     sdns[16];                   ///< Secondary DNS
    uint8_t     wifi_dat_key[32];           ///< Wifi data validation key
} NFC_DEVICE_COM_t;

/// Device info
typedef struct
{
    uint32_t    structure_version;                      ///< Structure version code
    uint16_t    develop_mode;                           ///< Develop mode (0 - Max Uint16_t)
    uint16_t    device_options;                         ///< Bitset for device options (b7: full_auto b2:Wifi; b1:SIGFOX; b0:GPS)
    uint32_t    zp_adcs_short;                          ///< Zp with electrodes in short-circuit (in ADCs)
    uint32_t    zp_adcs_calibration;                    ///< Zp input connected to calibration (in ADCs)
    uint8_t     device_sn               [RSC_NAME_SZ];  ///< Device serial number
    uint8_t     device_manufacture_date [RSC_NAME_SZ];  ///< Device manufacture date
} NFC_DEVICE_ID_t;

/// Peripheral info
typedef struct
{
    uint32_t    free1;                                  ///< Free1
    uint32_t    free2;                                  ///< Free2
    uint32_t    internal_memory_cap;                    ///< Internal memory capacity  (in MBytes)
    uint32_t    internal_memory_rem;                    ///< Remaining internal memory (in MBytes)

    uint32_t    electrodes_sn1;                         ///< Electrodes serial number 1 (from 1-Wire memory address)
    uint32_t    electrodes_sn2;                         ///< Electrodes serial number 2 (from 1-Wire memory address)
    uint32_t    electrodes_manufacture_date;            ///< Electrodes manufacture date (YYYY.MM.DD)
    uint32_t    electrodes_expiration_date;             ///< Electrodes expiration  date (YYYY.MM.DD)
    uint32_t    electrodes_event_date;                  ///< Last event date (YYYY.MM.DD)
    uint32_t    electrodes_event_time;                  ///< Last event time (hh:mm:ss)
    uint32_t    electrodes_event_id;                    ///< Last event identifier
    uint32_t    reserved_electrodes;                    ///< ***** Reserved for future use

    uint8_t     battery_sn [RSC_NAME_SZ];               ///< Battery serial number
    uint32_t    battery_manufacture_date;               ///< Battery manufacture date (YYYY.MM.DD)
    uint32_t    battery_expiration_date;                ///< Battery expiration  date (YYYY.MM.DD)
    uint16_t    battery_cap;                            ///< Battery nominal capacity (mAh)
    uint16_t    battery_rem;                            ///< Battery remaining capacity (mAh)
    uint32_t    reserved_battery;                       ///< ***** reserved for future use

    uint32_t    app_sw_version;                         ///< Device SW TAG version
    uint16_t    error_code;                             ///< Last recorded error code
    uint16_t    compiler_date;                          ///< Compiler date(MM.DD)
    uint16_t    compiler_time;                          ///< Compiler time(HH.MM)
    union
    {
        uint16_t    pcb_hw_options;                     ///< Device PCB HW options
        struct
        {
            uint16_t    pcb_hw_version:     8;          ///< Device PCB HW version to apply new consumptions with limitations
            uint16_t    pcb_hw_extended:    8;          ///< Device PCB HW version to apply new consumptions
        } pcb_hw;
    };
    uint16_t    reserved_0[2];                          ///< ***** Reserved for future use

} NFC_DEVICE_INFO_t;

/// Device info
typedef struct
{
    NFC_DEVICE_ID_t     dev_id;
    NFC_DEVICE_INFO_t   dev_info;
} NFC_MAIN_INFO_t;


/******************************************************************************
 ** Globals
 */
extern NFC_DEVICE_ID_t write_nfc_device_id; // Global struct for comparing NFC reading and writing
extern NFC_DEVICE_ID_t read_nfc_device_id;  // Global struct for comparing NFC reading and writing

/******************************************************************************
 ** Prototypes
 */

extern  void                Device_Init     (char_t *serial_num);   ///< Device initialization

// Device Info
extern  NFC_DEVICE_INFO_t*  Get_NFC_Device_Info (void);
extern  NFC_DEVICE_ID_t*    Get_NFC_Device_ID (void);
extern  DEVICE_INFO_t*      Get_Device_Info (void);
extern  DEVICE_DATE_t*      Get_Device_Date (void);
extern  DEVICE_TIME_t*      Get_Device_Time (void);
extern  void                Set_Device_Date_time   (void);
extern  void                Get_APP_SW_Version  (char_t* sw_version);
extern NFC_SETTINGS_t* CU_Get_NFC_Settings();

// Device Settings
extern  void                Settings_Open_From_uSD (void);      ///< Open the default settings from the uSD
extern  void                Settings_Save_To_uSD   (void);      ///< Open the default settings from the uSD
extern  void                Settings_Open_From_NFC (void);      ///< Open the default settings from the NFC
extern  AED_SETTINGS_t*     Get_AED_Settings       (void);      ///< Get AED Settings handler
extern  MISC_SETTINGS_t*    Get_Misc_Settings      (void);      ///< Get Miscellaneous Settings handler
extern  DEVICE_SETTINGS_t*  Get_Device_Settings    (void);      ///< Get Device Settings handler
extern  void                NFC_Write_Settings     (void);      ///< Write the device settings
extern  void                NFC_Write_Settings_Montador     (void);      ///< Write the device settings
extern  void                NFC_Write_Device_ID    (bool_t flag_write, bool_t flag_cal_write);      ///< Write the device identifier
extern  void                NFC_Write_Device_Info   (bool_t flag_write); ///< Write the device & accessories info
extern  void                Wifi_Save_To_uSD       (void);      ///< Save wifi settings to uSD

void                        Get_Compile_Date_Time   (void);

#endif /* DEVICE_INIT_H_ */

/*
 ** $Log$
 **
 ** end of device.h
 ******************************************************************************/
