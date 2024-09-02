/******************************************************************************
 * Name      : R100_S3A7                                                      *
 * Copyright : Osatu S. Coop                                                  *
 * Compiler  : GNU ARM Embedded                                               *
 * Target    : Reanibex Serie 100                                             *
 ******************************************************************************/

/*!
 * @file        DB_Test.h
 * @brief       Header with functions related to the test services
 *
 * @version     v1
 * @date        20/07/2020
 * @author      lsanz
 * @warning     author   date    version     .- Edition
 * @bug
 *
 */

#ifndef DB_TEST_H     // Entry, use file only if it's not already included.
#define DB_TEST_H

/******************************************************************************
**Includes
*/
#include <device_init.h>
#include "types_basic.h"

/******************************************************************************
** Defines
*/

#define NUM_BASIC_TEST      30                          ///< Number of basic tests to perform a full test
#define TEST_FULL_ID        (NUM_BASIC_TEST - 1)        ///< Identifier of the full test
#define TEST_MANUAL_ID      40                          ///< Identifier of the manual test
#define TEST_INITIAL_ID     50                          ///< Identifier of the initial test
#define TEST_FUNC_ID        60                          ///< Identifier of the functional test



/******************************************************************************
** Typedefs
*/



/// Structure to report the voltages test result
typedef struct
{
    uint16_t    dc_main;                ///< Voltage in the DC_MAIN (mVolts)
    uint8_t     dc_main_error;          ///< Error identifier
    uint16_t    dc_18v;                 ///< Voltage in the DC_18V (mVolts)
    uint8_t     dc_18v_error;           ///< Error identifier
} TEST_VOLTAGE_t;


/// Structure to report the battery test result
typedef struct
{
    char_t      sn  [20];               ///< Battery serial number -- format AAAA/21100001
    uint8_t     name[20];               ///< Company name -- (default: BexenCardio)
    uint32_t    manufacture_date;       ///< Manufacture date (YYYY.MM.DD)
    uint32_t    expiration_date;        ///< Expiration  date (YYYY.MM.DD)
    uint32_t    nominal_capacity;       ///< Battery nominal capacity (default: 4200 mAh)
    uint16_t    version_major;          ///< Battery version (major)
    uint16_t    version_minor;          ///< Battery version (minor)
    uint32_t    runTime_total;          ///< Accumulated run time (in minutes)
    uint32_t    battery_state;          ///< Battery state
    uint16_t    nFull_charges;          ///< Number of full charges
    uint16_t    rem_charge;             ///< Remaining charge percent
    int16_t     bat_temperature;        ///< Battery temperature
    uint32_t    error_code;             ///< Battery error code
} TEST_BATTERY_t;

/// Structure to report the electrodes test result
typedef struct
{
    uint64_t    sn;                     ///< Electrode serial number
    uint32_t    expiration_date;        ///< Electrode expiration date (YYYY.MM.DD)
    uint32_t    event_date;             ///< Electrode event date (YYYY.MM.DD)
    uint32_t    event_time;             ///< Electrode event time
    uint32_t    event_id;               ///< Electrode event id bitset
    uint32_t    error_code;             ///< electrode error code
} TEST_ELECTRODES_t;

/// Structure to report the patient monitor test result
typedef struct
{
    uint32_t    ADS_cal;                ///< ADS ZP calibration resistance value
    uint32_t    ADS_pat;                ///< ADS ZP patient resistance value
    uint32_t    ADS_comms;              ///< ADS SPI comms available or not
    int32_t     ADS_temperature;        ///< Internal temperature (�C)
} TEST_PATMON_t;

/// Structure to report the external comms test result
typedef struct
{
    uint32_t    wifi;                   ///< WiFi module comms check
    uint32_t    sigfox;                 ///< Sigfox module comms check
    uint32_t    gps;                    ///< GPS module comms check
    uint32_t    accelerometer;          ///< Accelerometer check
} TEST_COMMS_t;

/// Structure to report the miscellaneous test result
typedef struct
{
    uint32_t    nfc;                    ///< NFC comms available or not
    uint32_t    boot;                   ///< Boot processor comms available or not
    uint32_t    audio;                  ///< Audio signal check
    uint32_t    leds_keys;              ///< Led & keys check
} TEST_MISC_t;

/// Structure to report the defibrillator test result
typedef struct
{
    uint16_t    full_charging_time;                 ///< Capacitor charging time in milliseconds
    uint16_t    full_charging_voltage;              ///< Voltage of main capacitor after charging
    uint32_t    full_discharg_H_current;            ///< Current in the H-Bridge
    uint16_t    full_discharg_H_time;               ///< Discharging time of H bridge discharge
    uint16_t    full_discharg_H_voltage;            ///< Voltage after H bridge discharge
    uint16_t    full_discharg_R_time;               ///< Discharging time of R discharge
    uint16_t    full_discharg_R_voltage;            ///< Voltage after R discharge
    uint32_t    error_code;                         ///< Defibrillation circuit error code
} TEST_DEFIB_t;

/// Enum for test result status
typedef enum
{
    TEST_OK = 0,                            ///< Test with no critical error
    TEST_NOT_OK,                            ///< Test with critical error
    TEST_ABORTED                            ///< Test aborted
} TEST_STATUS_t;

// Structure to report the certificates load test result
typedef struct
{
    uint16_t tls_cert_exp;                       ///< TLS certificate check
    uint16_t wpa_eap_cert_exp;                   ///< WPA_EAP certificate check
}TEST_CERT_t;

/// Structure to hold data before writing to file in multiples of cluster size
typedef struct
{
    char_t              device_sn[RSC_NAME_SZ];     ///< Device serial number
    uint8_t             test_id;                    ///< Test identifier (used internally to identify the test number)
    TEST_STATUS_t       test_status;                ///< Test OK, NOT OK or ABORT
    uint32_t            error_code;                 ///< Error code

    TEST_VOLTAGE_t      voltage;                    ///< Voltages test results
    TEST_MISC_t         misc;                       ///< Miscellaneous test results
    TEST_PATMON_t       patmon;                     ///< Patient monitor test results
    TEST_ELECTRODES_t   electrodes;                 ///< Electrodes test results
    TEST_BATTERY_t      battery;                    ///< Battery test results
    TEST_COMMS_t        comms;                      ///< Comms test results


    // Full test related variables ...
    //char_t              gps_position [64];        ///< GPS position
    TEST_DEFIB_t        defib;                      ///< Defibrillator test results

    TEST_CERT_t         certificate;                ///< Certificate test results

} DB_TEST_RESULT_t;


/******************************************************************************
** Globals
*/

extern DB_TEST_RESULT_t    R100_test_result;           ///< Test result !!!



/******************************************************************************
** Prototypes
*/

extern  void    Fill_Generic_Date (char_t *pStr, uint32_t my_date);
extern  void    Fill_Generic_Time (char_t *pStr, uint32_t my_time);

extern  bool_t  DB_Test_Generate_Report (DB_TEST_RESULT_t *pResult, bool_t file_report);
extern  char_t* DB_Test_Get_Filename (void);
extern  void    DB_Sigfox_Store_Info (char_t * device_id, char_t * pac_id);

extern  int    PowerOff_Write_Batt_Elec(void);

#endif  /*DB_TEST_H*/
