/******************************************************************************
 * Name      : R100_S3A7                                                      *
 * Copyright : Osatu S. Coop                                                  *
 * Compiler  : GNU ARM Embedded                                               *
 * Target    : Reanibex Serie 100                                             *
 ******************************************************************************/

/*!
 * @file        thread_sysMon_entry.h
 * @brief       Header with functions related to the system monitor task
 *
 * @version     v1
 * @date        06/06/2018
 * @author      ltorres
 * @warning     author   date    version     .- Edition
 * @bug
 *
 */

#ifndef THREAD_SYSMON_ENTRY_H_
#define THREAD_SYSMON_ENTRY_H_

/******************************************************************************
 **Includes
 */
#include "R100_Errors.h"
#include "HAL/thread_patMon_hal.h"
#include "HAL/thread_comm_hal.h"
#include "DB_Test.h"
#include "secure_crypto.h"
#include "BSP/bsp_wifi_silabs-WGM110.h"
#include "BSP/bsp_gps_quectel-L96.h"

/******************************************************************************
 ** Defines
 */
#define THREAD_ID_sMON       0x00000001
#define THREAD_ID_CORE       0x00000002
#define THREAD_ID_COMM       0x00000004
#define THREAD_ID_DRD        0x00000008
#define THREAD_ID_HMI        0x00000010
#define THREAD_ID_DEFIB      0x00000020
#define THREAD_ID_AUDIO      0x00000040
#define THREAD_ID_PATMON     0x00000080
#define THREAD_ID_RECORDER   0x00000100

//#define LIFE_BEAT_PERIOD     3000                     ///< Life beat period in msecs.
#define LIFE_BEAT_PERIOD     10000                      ///< Life beat period in msecs. (10 seconds)
#define LIFE_BEAT_PERIOD_DEMO_MODE     1200000          ///< Life beat period in msecs. (20 minutes)
#define WAKEUP_POWERON       10                         ///< Force the GPIO to ON to maintain the power


#define UTC_MIN             -12
#define UTC_MAX              14

#define ADS_CAL_MIN         5501000        ///< Max ADS calibration value for an internal resistor 6K04 ohms
#define ADS_CAL_MAX         5605000        ///< Min ADS calibration value for an internal resistor 6K04 ohms
#define ADS_ZP_0_MIN        4020000        ///< Max ADS 0 Ohms value for an external resistor
#define ADS_ZP_0_MAX        4056000        ///< Min ADS 0 Ohms value for an external resistor

/******************************************************************************
 ** Typedefs
 */

/// Patmon test result structure
typedef struct
{
 //   uint32_t    ZP;               ///< Internal impedance (ohms)
    ERROR_ID_e  sts;                ///< Test result
} TEST_RESULT_ECGMON_t;

/// Data collected during the defibrillation shock
typedef struct
{
    uint32_t    vc;             ///< Voltage in the main capacitor               (volts)
    uint32_t    time;           ///< Time in curse or programed maximun time     (usecs)
    uint32_t    energy;         ///< Energy computed or programed maximun energy (embedded Units)
} DEFIB_PULSE_DATA_t;

/// Summary of defibrillation pulse data for the phase-1
typedef struct
{
    uint32_t            shock_energy;   ///< Energy to discharge on the patient (Jules)
    uint32_t            shock_zp;       ///< Patient impedance                  (ohms)
    uint32_t            shock_alarms;   ///< Alarms detected during the shock

    DEFIB_PULSE_DATA_t  start_p0;       ///< Data programed at p0 start (pulse start)
    DEFIB_PULSE_DATA_t  start_p1;       ///< Data programed at p1 start
    DEFIB_PULSE_DATA_t  end_p0;         ///< Data collected at p0 end   (4 msecs)
    DEFIB_PULSE_DATA_t  end_p1;         ///< Data collected at p1 end   (phase-1 end)
    DEFIB_PULSE_DATA_t  end_p2;         ///< Data collected at p2 end   (phase-2 end)
} DEFIB_PULSE_BLACK_BOX_t;

/// Summary of defibrillation pulse data for the phase-1
typedef struct
{
    DEFIB_PULSE_BLACK_BOX_t pulse_data; ///< Pulse data
    ERROR_ID_e              sts;        ///< Test result
} TEST_RESULT_DEFIB_t;

/// Electrodes info header (stored in the electrodes internal memory)
typedef struct {
    uint8_t     lot_number[8];          ///< Lot number
    uint32_t    manufacture_date;       ///< Manufacture date (YYYY.MM.DD)
    uint32_t    expiration_date;        ///< Expiration  date (YYYY.MM.DD)
    uint8_t     name[12];               ///< Company name -- (default: BexenCardio)
    uint8_t     version;                ///< Version code / electrode type
    uint8_t     revision_code;          ///< Revision code
    uint8_t     reserved[1];            ///< Reserved
    uint8_t     checksum_add;           ///< Data checksum
} ELECTRODES_INFO_t;

/// Electrodes event (stored in the electrodes internal memory)
typedef struct {
    uint32_t    date;                   ///< Formatted date
    uint32_t    time;                   ///< Formatted time
    uint32_t    id;                     ///< Event identifier (number of shocks)
    uint32_t    checksum_xor;           ///< Data checksum
} ELECTRODES_EVENT_t;

/// Electrodes info structure
typedef struct {
    uint64_t            sn;             ///< Electrodes serial number -- 1Wire memory address -- 0 if no electrodes are connected
    ELECTRODES_INFO_t   info;           ///< Electrodes information header
    ELECTRODES_EVENT_t  event;          ///< Electrodes event register
} ELECTRODES_DATA_t;

/// Non volatile data (implemented in the internal data flash)
typedef struct {
    uint32_t    time_warning;           ///< Formatted time (HH:MM:SS) to generate a warning
    uint8_t     time_test;              ///< Time to execute the test (hour identifier)
    ERROR_ID_e  error_code;             ///< Active error code (only codes that requires SAT)
    uint8_t     test_id;                ///< Test identifier (test executed in the last session)
    bool_t      test_pending;           ///< Flag to indicate that a test is pending to be executed
    char        latitude[12];           ///< Device gps latitude
    char        longitude[12];          ///< Device gps longitude
    uint8_t     lat_long_dir;           ///< Bit1 N or S;Bit0 E or W
    uint32_t    open_cover;             ///< Counter to detect open cover state during extended periods
    uint8_t     open_cover_tx_flag;     ///< Open cover transmission flag
    bool_t      open_cover_first;       ///< Open cover first flag
    bool_t      prev_cover_status;      ///< Previous cover status
    //uint8_t     check_sat_tx_flag;      ///< Check sat transmission flag
    uint8_t     update_flag;            ///< Update firmware flag
    uint8_t     status_led_blink;       ///< Force the blink of the status LED (do not blink with expired electrodes or battery)
    NFC_SETTINGS_t  default_settings;   ///< Default settings
    uint8_t     must_be_0x55;           ///< Reserved (fixed to 0x55)
    uint8_t     checksum_xor;           ///< Data checksum
} NV_DATA_t;

// NV data alignment in 1 byte. You can use pack or the attribute "__attribute__((__packed__))" in the structure instance
#pragma pack(1)
// Second block of Non Volatile Data (implemented in the internal data flash)
typedef struct {
    uint32_t    zp_adcs_short;          ///< Zp with electrodes in short-circuit (in ADCs)
    uint32_t    zp_adcs_calibration;    ///< Zp input connected to calibration (in ADCs)
    uint8_t     acc_pos_hvi;            ///< Determine accelerometer position (horizontal/vertical/inclined)
    uint8_t     must_be_0x55;           ///< Reserved (fixed to 0x55)
    uint8_t     checksum_xor;           ///< Data checksum
} NV_DATA_BLOCK_t;
#pragma pack()

/******************************************************************************
 ** Globals
 */
extern  POWER_ON_SOURCE_e   poweron_event;      ///< Power event
extern  uint32_t    thread_supervisor;          ///< Thread supervisor
extern  bool_t      write_elec_last_event;      ///< Write las electrodes registered event in test
extern  bool_t      global_poweron_event;       ///< Save Power On event

/******************************************************************************
 ** Prototypes
 */

extern void                 R100_Program_Autotest               (void);
extern void                 R100_PowerOff                       (void);
extern void                 R100_PowerOff_RTC                   (void);

extern EL_SIGN_e            Electrodes_Get_Signature            (void);
extern void                 Electrodes_Get_Data                 (ELECTRODES_DATA_t* pData);
extern void                 Electrodes_Register_Event           (void);
extern void                 Electrodes_Register_Shock           (void);

extern NV_DATA_t*           Get_NV_Data                         (void);
extern NV_DATA_BLOCK_t*     Get_NV_Data_Block                   (void);
extern void                 Set_NV_Data_Error_IF_NOT_SAT        (ERROR_ID_e error);
extern void                 Set_NV_Data_Error_IF_NOT_SAT_Comms  (ERROR_ID_e error);
extern ERROR_ID_e           Save_Comms_Error                    (void);
extern bool_t               Is_Test_Mode                        (void);
extern bool_t               Is_Test_Mode_Montador               (void);
//extern bool_t               Is_Mode_Demo                        (void);
extern void                 Set_Mode_Demo                       (bool_t aux);

extern void                 Device_pw                           (uint8_t *pPassword, uint8_t *pDevice_sn, uint32_t size);

extern bool_t               Is_Sysmon_Task_Initialized          (void);

//extern void                 NV_Data_Write                       (NV_DATA_t *pNV_data, NV_DATA_BLOCK_t *pNV_data_block);
extern int                 NV_Data_Write                       (NV_DATA_t *pNV_data, NV_DATA_BLOCK_t *pNV_data_block);

extern int                 NV_Data_Read                        (NV_DATA_t *pNV_data, NV_DATA_BLOCK_t *pNV_data_block);

extern void                 Lock_on_Panic                       (uint8_t error, uint8_t num);
extern int                 Check_Device_Led_Status             (void);
extern bool_t               Check_Test_Abort_User               (DB_TEST_RESULT_t *pResult);

extern void                 Refresh_Wdg                         (void);
extern void                 Pasatiempos                         (uint32_t nTicks);
extern void                 Pasatiempos_Listening               (void);

int                        bsp_clock_init_Reconfigure          (void);

NV_DATA_t CU_getNV_Data();
extern void CU_setBatteryRTC_hour(uint8_t value);
extern void CU_setNV_Data_time_test(uint8_t value);
extern uint8_t CU_getNV_Data_time_test();
uint32_t CU_getPon_date();

#endif /* THREAD_SYSMON_ENTRY_H_ */

/*
 ** $Log$
 **
 ** end of thread_sysMon_entry.h
 ******************************************************************************/
