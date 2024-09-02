/******************************************************************************
 * Name      : R100_S3A7                                                      *
 * Copyright : Osatu S. Coop                                                  *
 * Compiler  : GNU ARM Embedded                                               *
 * Target    : Reanibex Serie 100                                             *
 ******************************************************************************/

/*!
 * @file        sysMon_Battery.h
 * @brief       Header with functions related to the battery services
 *
 * @version     v1
 * @date        24/11/2021
 * @author      ilazkanoiturburu
 * @warning     author   date    version     .- Edition
 * @bug
 *
 */

#ifndef SYSMON_BATTERY_H_
#define SYSMON_BATTERY_H_

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

#define     VB_25C_SAFE_BATTERY         10000           ///< Security voltage
#define     VB_00C_SAFE_BATTERY         9500            ///< Security voltage

#define     VB_SAFE_BATTERY             9300            ///< Security voltage
#define     SMON_DCMAIN_MAX             15500L          ///< Maximun DC_main voltage (milivolts)
#define     SMON_DCMAIN_MIN             8500L           ///< Minimun DC_main voltage (milivolts)

#define     SMON_18V_MAX                19000L          ///< Maximun DC_main voltage (milivolts)
#define     SMON_18V_MIN                16000L          ///< Minimun DC_main voltage (milivolts)

#define     BATTERY_LOW_CHARGE          15              ///< Battery charge percent to advice low battery
#define     BATTERY_REPLACE_CHARGE      10              ///< Battery charge percent to demand the battery replacement
#define     BATTERY_VERY_LOW_TEMP       (-25)           ///< Battery minimum storage temperature limit
#define     BATTERY_VERY_HIGH_TEMP      (70)            ///< Battery maximum storage temperature limit
#define     BATTERY_LOW_TEMP            (0)             ///< Battery minimum operation temperature limit
#define     BATTERY_HIGH_TEMP           (50)            ///< Battery maximum operation temperature limit



/******************************************************************************
 ** Typedefs
 */

/// Battery info (stored in the battery pack)
typedef struct {
    uint8_t     sn  [16];               ///< Battery serial number -- format AAAA/21100001
    uint8_t     name[16];               ///< Company name -- (default: BEXENCARDIO)
    uint32_t    manufacture_date;       ///< Manufacture date (YYYY.MM.DD)
    uint32_t    expiration_date;        ///< Expiration  date (YYYY.MM.DD)
    uint32_t    nominal_capacity;       ///< Battery nominal capacity (default: 4200 mAh)
    uint8_t     version_major;          ///< Battery version major
    uint8_t     version_minor;          ///< Battery version minor
    uint8_t     must_be_0xAA;           ///< Force to 0xAA
    uint8_t     checksum_add;           ///< Data checksum
} BATTERY_INFO_t;

/// Battery statistics (stored in the battery pack)
typedef struct {
    uint32_t    runTime_total;          ///< Accumulated run time (in minutes)
    uint32_t    battery_state;          ///< Battery state depend on the safe voltage
    uint16_t    nFull_charges;          ///< Number of full charges with patient
    uint16_t    nTicks_open_cover;      ///< Number of ticks with open cover
    uint16_t    nTestManual;            ///< Number of manual/monthly test
    uint8_t     must_be_0x55;           ///< Reserved (fixed to 0x55)
    uint8_t     checksum_xor;           ///< Data checksum
} BATTERY_STATISTICS_t;

/// Comms Battery statistics (stored in the battery pack)
typedef struct {
    uint16_t    daily_test;                     ///< Number of daily test
    uint16_t    nTest_charges;                  ///< Number of charges in manual test
    uint16_t    rtc_warning;                    ///< Number of alerts that have been said by audio in RTC(Close cover, Call SAT, BEEP-BEEP)
    uint16_t    sigfox_open_close_cover;        ///< Number of alerts that have been sent with the open and close cover with Sigfox
    uint16_t    sigfox_daily_test;              ///< Number of daily test that have been sent with Sigfox
    uint16_t    sigfox_manual_test;             ///< Number of monthly test that have been sent with Sigfox
    uint16_t    sigfox_manual_test_gps;         ///< Number of monthly test that have been sent with Sigfox + GPS
    uint16_t    wifi_test_connect;              ///< Number of times that WiFi has been connected successfully
    uint16_t    wifi_test_no_connect;           ///< Number of times that WiFi has not been connected successfully
    uint16_t    wifi_test_connect_gps;          ///< Number of fully test that have been sent with Wifi + GPS
    uint16_t    wifi_cover_connect;             ///< Number of alerts that have been sent with the open and close cover with Wifi
    uint16_t    wifi_cover_no_connect;          ///< Number of alerts that have no been sent with the open and close cover with Wifi
    uint32_t    wifi_runTime_total;             ///< Accumulated run time (in minutes)
    uint8_t     enable;                         ///< Bit to know if the battery has been used
    uint8_t     reserved;                       ///< Reserved for future use
    uint8_t     must_be_0x55;                   ///< Reserved (fixed to 0x55)
    uint8_t     checksum_xor;                   ///< Data checksum
} BATTERY_STATISTICS_COMMS_t;


/// Battery date/time (read from the battery pack)
typedef struct {
    uint8_t     sec;                    ///< seconds (0:59)
    uint8_t     min;                    ///< minutes (0:59)
    uint8_t     hour;                   ///< hour    (0:23)
    uint8_t     weekday;                ///< weekday (1:7)
    uint8_t     date;                   ///< date    (1:31)
    uint8_t     month;                  ///< month   (1:12)
    uint8_t     year;                   ///< year    (0:99)
     int8_t     utc_time;               ///< UTC time (-11:12)
} BATTERY_RTC_t;



/******************************************************************************
 ** Globals
 */

extern BATTERY_INFO_t       battery_info;                   ///< Battery information
extern BATTERY_STATISTICS_t battery_statistics;             ///< Battery statistics
extern BATTERY_STATISTICS_COMMS_t battery_statistics_comms; ///< battery statistics
extern BATTERY_RTC_t        battery_RTC;                    ///< Battery RTC


/******************************************************************************
 ** Prototypes
 */

extern void         Inc_OpenCover              (void);
extern void         Inc_TestManual             (void);
extern void         Inc_FullCharges            (void);
extern void         Inc_TestCharges            (void);
extern void         Inc_RunTime                (void);
extern void         Inc_RunTime_Update         (void);
extern void         Inc_DailyTest              (void);
extern void         Inc_RTCWarning             (void);

extern void         Inc_SigfoxCover            (void);
extern void         Inc_SigfoxDailyTest        (void);
extern void         Inc_SigfoxManualTest       (void);
extern void         Inc_SigfoxManualTestGPS    (void);
extern void         Inc_WifiCoverConnect       (void);
extern void         Inc_WifiCoverNoConnect     (void);
extern void         Inc_WifiTestConnect        (WIFI_TEST_RESULT_t *pwifi_test_result);
extern void         Inc_WifiTestNoConnect      (WIFI_TEST_RESULT_t *pwifi_test_result);
extern void         Inc_WifiTestConnectGPS     (void);
extern void         Inc_WifiRunTime            (void);

extern void         Battery_Get_Info                (BATTERY_INFO_t *pData);
extern void         Battery_Get_Statistics          (BATTERY_STATISTICS_t *pData);
extern void         Battery_Get_Statistics_Comms    (BATTERY_STATISTICS_COMMS_t* pData);
extern uint16_t     Battery_Get_Charge              (void);
extern int8_t       Battery_Get_Temperature         (void);

extern ssp_err_t    Battery_Write_Statistics        (BATTERY_STATISTICS_t *pStatistics);
extern ssp_err_t    Battery_Write_Statistics_Comms  (BATTERY_STATISTICS_COMMS_t *pStatistics_comms);
extern ssp_err_t    Battery_Read_Statistics         (BATTERY_STATISTICS_t *pStatistics);
extern ssp_err_t    Battery_Read_Statistics_Comms   (BATTERY_STATISTICS_COMMS_t *pStatistics_comms);
extern ssp_err_t    Battery_Read_Temperature        (int8_t *pTemp);
extern ssp_err_t    Battery_Read_RTC                (BATTERY_RTC_t *pRTC);

extern bool_t       Is_Battery_Mode_Demo            (void);

extern void         Battery_I2C_Init                (uint32_t *ponDate, uint32_t *ponTime);
extern ssp_err_t    Battery_I2C_Init_RTC            (uint32_t *ponDate, uint32_t *ponTime);

extern uint32_t     diff_time_days                  (uint32_t date_1, uint32_t date_2);
extern uint32_t     diff_time_seconds               (uint32_t time_1, uint32_t time_2);

extern void         Get_Date                        (uint8_t *pYear, uint8_t *pMonth, uint8_t *pDate);
extern void         Get_Time                        (uint8_t *pHour, uint8_t *pMin,   uint8_t *pSec);
extern uint8_t      Get_Rem_Charge                  (void);
extern uint8_t      Get_RTC_Month                   (void);
extern uint8_t      Get_RTC_Year                    (void);


#endif /* SYSMON_BATTERY_H_ */

/*
 ** $Log$
 **
 ** end of sysMon_Battery.h
 ******************************************************************************/
