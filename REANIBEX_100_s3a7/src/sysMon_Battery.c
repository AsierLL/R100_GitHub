/******************************************************************************
 * Copyright : Osatu S. Coop                                                  *
 * Compiler  : GNU ARM Embedded                                               *
 * Target    : Reanibex Serie 100                                             *
 ******************************************************************************/

/*!
 * @file        sysMon_Battery.c
 * @brief       Battery functions executed in the context of the SysMon thread
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

#include "R100_Errors.h"
#include "types_basic.h"
#include "types_app.h"
#include "Trace.h"
#include "Comm.h"
#include "I2C_1.h"
#include "I2C_2.h"
#include "RTC.h"
#include "Keypad.h"
#include "DB_Test.h"
#include "DB_Episode.h"
#include "bsp_acc_LIS3DH.h"

#include "HAL/thread_defibrillator_hal.h"
#include "HAL/thread_patMon_hal.h"
#include "HAL/thread_audio_hal.h"
#include "thread_comm_entry.h"
#include "thread_patMon_entry.h"
#include "thread_sysMon_entry.h"
#include "thread_sysMon.h"
#include "thread_audio_entry.h"
#include "thread_audio.h"
#include "thread_core_entry.h"
#include "thread_core.h"

#include "thread_comm.h"
#include "thread_api.h"
#include "sysMon_Battery.h"
#include "sysMon_RF_Comms.h"

#ifdef UNIT_TESTS
#include "unit_tests.h"
#endif

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

BATTERY_INFO_t       battery_info;                      ///< Battery information
BATTERY_STATISTICS_t battery_statistics;                ///< Battery statistics
BATTERY_STATISTICS_COMMS_t battery_statistics_comms;    ///< Battery statistics
BATTERY_RTC_t        battery_RTC;                       ///< Battery RTC


/******************************************************************************
 ** Locals
 */

static uint16_t      battery_charge;             ///< Battery charge percent
static int8_t        battery_temperature;        ///< Battery temperature
static DEVICE_DATE_t save_sw_date;

static  FX_MEDIA   *sd_media = (FX_MEDIA *)NULL;    ///< Handler of the SD media card

/******************************************************************************
 ** Prototypes
 */


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
static uint8_t Get_Checksum_Add (uint8_t *pBuffer, uint32_t size)
{
    uint32_t    i;          // global use index
    uint32_t    add = 0;    // add count

    // calculate the data checksum
    for (i=0; i<size; i++)
    {
        add += (uint32_t) pBuffer [i];
    }
    return ((uint8_t) add);
}

/******************************************************************************
** Name:    Get_Checksum_Xor
*****************************************************************************/
/**
** @brief   Calculates the data array checksum (XOR)
**
** @param   pBuffer   pointer to the structure to fill
** @param   size      number of bytes to process
**
** @return  checksum result
******************************************************************************/
static uint8_t Get_Checksum_Xor (uint8_t *pBuffer, uint32_t size)
{
    uint32_t    i;          // global use index
    uint8_t     xor = 0;    // xor result

    // calculate the data checksum
    for (i=0; i<size; i++)
    {
        xor ^= pBuffer [i];
    }
    return xor;
}

/******************************************************************************
** Name:    Get_RTC_Month
*****************************************************************************/
/**
** @brief   Get RTC month
**
** @param   none
**
** @return  month number
******************************************************************************/
uint8_t Get_RTC_Month (void)
{
    return battery_RTC.month;
}

/******************************************************************************
** Name:    Get_RTC_Year
*****************************************************************************/
/**
** @brief   Get RTC year
**
** @param   none
**
** @return  year number
******************************************************************************/
uint8_t Get_RTC_Year (void)
{
    return battery_RTC.year;
}

/******************************************************************************
 ** Name:    diff_time_days
 *****************************************************************************/
/**
 ** @brief   Computes the difference time between two dates (in days)
 **          (date_1 - date_2)
 **
 ** @param   date_1     date 1 in (YYYY:MM:DD) format
 ** @param   date_2     date 2 in (YYYY:MM:DD) format
 **
 ** @return  difference in number of days
 ******************************************************************************/
uint32_t diff_time_days (uint32_t date_1, uint32_t date_2)
{
    uint32_t    nDays_1, nDays_2;

    nDays_1 = ((date_1 >> 16) * 365) + (((date_1 >> 8) & 0xFF) * 30) + (date_1  & 0xFF);
    nDays_2 = ((date_2 >> 16) * 365) + (((date_2 >> 8) & 0xFF) * 30) + (date_2  & 0xFF);

    // return the number of days between the two dates
    return (nDays_1 > nDays_2) ? (nDays_1 - nDays_2) : 0;
}

/******************************************************************************
 ** Name:    diff_time_seconds
 *****************************************************************************/
/**
 ** @brief   Computes the difference time between two hours (in seconds)
 **          (time_1 - time_2)
 **
 ** @param   time_1     time 1 in (HH:MM:SS) format
 ** @param   time_2     time 2 in (HH:MM:SS) format
 **
 ** @return  difference in seconds
 ******************************************************************************/
uint32_t diff_time_seconds (uint32_t time_1, uint32_t time_2)
{
    uint32_t    sec_1, sec_2;

    // if time overflow has been detected, add 24 hours to the first operator
    if (time_2 > time_1) time_1 += (24 << 16);

    sec_1 = ((time_1 >> 16) * 60 * 60) + (((time_1 >> 8) & 0xFF) * 60) + (time_1  & 0xFF);
    sec_2 = ((time_2 >> 16) * 60 * 60) + (((time_2 >> 8) & 0xFF) * 60) + (time_2  & 0xFF);

    // return the number of seconds between the two times
    return (sec_1 - sec_2);
}

/******************************************************************************
 ** Name:    Is_Battery_Mode_Demo
 *****************************************************************************/
/**
 ** @brief   Function that report if the battery is DEMO or not
 **
 ** @param   none
 **
 ** @return  battery DEMO mode
 *****************************************************************************/
bool_t Is_Battery_Mode_Demo()
{
    return ((memcmp (battery_info.name, "DEMO", sizeof("DEMO"))==0) ? true : false);
}

/******************************************************************************
 ** Name:    Battery_Read_Info
 *****************************************************************************/
/**
 ** @brief   Read the battery information
 **
 ** @param   pInfo     pointer to the structure to fill
 **
 ** @return  operation result (error code)
 ******************************************************************************/
static ssp_err_t Battery_Read_Info (BATTERY_INFO_t *pInfo)
{
    ssp_err_t   ssp_error;      // ssp error code
    uint8_t     checksum;       // data structure checksum

    ssp_error = SSP_ERR_INVALID_POINTER;        // initialize the error code
    if (pInfo)
    {
        ssp_error = SSP_SUCCESS;

        // read the battery information from the battery pack (address 0x000)
        I2C1_Read_Eeprom (0x00, (uint8_t *) pInfo, sizeof(BATTERY_INFO_t));

        // check the data integrity ...
        checksum = Get_Checksum_Add ((uint8_t *) pInfo, sizeof(BATTERY_INFO_t)-1);
        if ((checksum != pInfo->checksum_add) || (pInfo->must_be_0xAA != 0xAA))
        {
            memset ((uint8_t *) pInfo, 0, sizeof(BATTERY_INFO_t));
            ssp_error = SSP_ERR_ABORTED;
        }
    }

    // return the operation result
    return ssp_error;
}

/******************************************************************************
 ** Name:    Battery_Get_Statistics
 *****************************************************************************/
/**
 ** @brief   Get the battery statistics
 **
 ** @param   pData    pointer to battery statistics structure
 **
 ** @return  none
 ******************************************************************************/
void Battery_Get_Statistics (BATTERY_STATISTICS_t* pData)
{
    memcpy ((uint8_t *) pData, (uint8_t *) &battery_statistics, sizeof (BATTERY_STATISTICS_t));
}

/******************************************************************************
 ** Name:    Battery_Get_Statistics_Comms
 *****************************************************************************/
/**
 ** @brief   Get the battery statistics
 **
 ** @param   pData    pointer to comms battery statistics structure
 **
 ** @return  none
 ******************************************************************************/
void Battery_Get_Statistics_Comms (BATTERY_STATISTICS_COMMS_t* pData)
{
    memcpy ((uint8_t *) pData, (uint8_t *) &battery_statistics_comms, sizeof (BATTERY_STATISTICS_COMMS_t));
}

/******************************************************************************
 ** Name:    Battery_Get_Info
 *****************************************************************************/
/**
 ** @brief   Get the battery information
 **
 ** @param   pInfo     pointer to battery info structure
 **
 ** @return  none
 ******************************************************************************/
void Battery_Get_Info (BATTERY_INFO_t* pData)
{
    memcpy ((uint8_t *) pData, (uint8_t *) &battery_info, sizeof (BATTERY_INFO_t));
}

/******************************************************************************
 ** Name:    Battery_Read_Statistics
 *****************************************************************************/
/**
 ** @brief   Read the battery statistics
 **
 ** @param   pStatistics     pointer to the structure to fill
 **
 ** @return  operation result (error code)
 ******************************************************************************/
ssp_err_t Battery_Read_Statistics (BATTERY_STATISTICS_t *pStatistics)
{
    ssp_err_t   ssp_error = SSP_SUCCESS;      // ssp error code
    uint8_t     checksum = 0, error_code = 0;       // data structure checksum

    ssp_error = SSP_ERR_INVALID_POINTER;        // initialize the error code
    if (pStatistics)
    {
        ssp_error = SSP_SUCCESS;

        tx_thread_sleep (OSTIME_10MSEC);        // This time is necessary!!

        // read the battery information from the battery pack (address 0x300)
        if(error_code = I2C1_Read_Eeprom (0x300, (uint8_t *) pStatistics, sizeof(BATTERY_STATISTICS_t)), error_code != eERR_NONE)
        {
            I2C1_Read_Eeprom (0x300, (uint8_t *) pStatistics, sizeof(BATTERY_STATISTICS_t));
        }

        // check the data integrity ...
        checksum = Get_Checksum_Xor ((uint8_t *) pStatistics, sizeof(BATTERY_STATISTICS_t)-1);
        if ((checksum != pStatistics->checksum_xor) || (pStatistics->must_be_0x55 != 0x55))
        {
            memset ((uint8_t *) pStatistics, 0, sizeof(BATTERY_STATISTICS_t));
            ssp_error = SSP_ERR_ABORTED;
        }
    }

    // return the operation result
    return ssp_error;
}

/******************************************************************************
 ** Name:    Battery_Read_Statistics_Comms
 *****************************************************************************/
/**
 ** @brief   Read the battery statistics
 **
 ** @param   pStatistics     pointer to the structure to fill
 **
 ** @return  operation result (error code)
 ******************************************************************************/
ssp_err_t Battery_Read_Statistics_Comms (BATTERY_STATISTICS_COMMS_t *pStatistics_comms)
{
    ssp_err_t   ssp_error = SSP_SUCCESS;      // ssp error code
    uint8_t     checksum = 0, error_code = 0;       // data structure checksum
    uint8_t     zero_array[14] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0};
    uint32_t    nDays = 0, nTest_D = 0;
    uint32_t    my_date = 0, sw_date = 0;               // Current date, SW update date

    //memset(batt_array, 0xFF, sizeof(batt_array));

    ssp_error = SSP_ERR_INVALID_POINTER;        // initialize the error code
    if (pStatistics_comms)
    {
        ssp_error = SSP_SUCCESS;

        tx_thread_sleep (OSTIME_10MSEC);        // This time is necessary!!

        // read the battery information from the battery pack (address 0x200)
        if(error_code = I2C1_Read_Eeprom (0x200, (uint8_t *) pStatistics_comms, sizeof(BATTERY_STATISTICS_COMMS_t)), error_code != eERR_NONE)
        {
            I2C1_Read_Eeprom (0x200, (uint8_t *) pStatistics_comms, sizeof(BATTERY_STATISTICS_COMMS_t));
        }
        /*I2C1_Read_Eeprom (0x200, (uint8_t *) pStatistics_comms, 14);
        tx_thread_sleep (OSTIME_10MSEC);        // This time is necessary!!

        I2C1_Read_Eeprom (0x20E, ((uint8_t *) pStatistics_comms)+14, 2);
        tx_thread_sleep (OSTIME_10MSEC);        // This time is necessary!!

        I2C1_Read_Eeprom (0x210, ((uint8_t *) pStatistics_comms)+16, 16);
        tx_thread_sleep (OSTIME_10MSEC);        // This time is necessary!!*/

        // check the data integrity ...
        checksum = Get_Checksum_Xor ((uint8_t *) pStatistics_comms, sizeof(BATTERY_STATISTICS_COMMS_t)-1);
        if ((checksum != pStatistics_comms->checksum_xor) || (pStatistics_comms->must_be_0x55 != 0x55))
        {
            memset ((uint8_t *) pStatistics_comms, 0, sizeof(BATTERY_STATISTICS_COMMS_t));
            ssp_error = SSP_ERR_ABORTED;
        }

        if(pStatistics_comms->enable == 0xFF || (pStatistics_comms->enable == 0x00 && memcmp(&battery_statistics, zero_array, sizeof(zero_array)) != 0))
        {
            ssp_error = SSP_SUCCESS;

            // Format the current date ...
            my_date  = (uint32_t) (battery_RTC.year + 2000) << 16;
            my_date += (uint32_t)  battery_RTC.month <<  8;
            my_date += (uint32_t)  battery_RTC.date;

            if (save_sw_date.year == 0)
            {
                sw_date = my_date;
            }
            else
            {
                sw_date  = (uint32_t) (save_sw_date.year + 2000) << 16;
                sw_date += (uint32_t)  save_sw_date.month <<  8;
                sw_date += (uint32_t)  save_sw_date.date;
            }

            // Software update date must be older or equal than current day
            if (diff_time_days(my_date, sw_date) == 0) sw_date = my_date;

            // Calculate time to apply old consumptions
            nDays   = diff_time_days (sw_date, battery_info.manufacture_date);
            // Calculate time to apply new consumptions
            if (nDays != 0)
            {
                //Battery is older than software update date
                nDays   = diff_time_days (my_date, sw_date);
                nTest_D =  nDays - (nDays / 30);
            }
            else
            {
                // Battery is newer than software update, so count the number of days since battery manufacture date, not since sw_date
                nDays   = diff_time_days (my_date, battery_info.manufacture_date);
                nTest_D =  nDays - (nDays / 30);
            }

            memset(pStatistics_comms, 0, sizeof(BATTERY_STATISTICS_COMMS_t));

            if (Get_Device_Info()->enable_b.sigfox == 0)
            {
                // If the device is a stand alone device
                pStatistics_comms->daily_test = (uint16_t)nTest_D;
            }
            else
            {
                // If the device has Sigfox, adjust the counters to the new version
                pStatistics_comms->sigfox_daily_test = (uint16_t)nTest_D;
                pStatistics_comms->sigfox_manual_test_gps = battery_statistics.nTestManual;
            }

            if(battery_statistics.nTestManual > battery_statistics.nFull_charges)
            {
                battery_statistics.nTestManual = battery_statistics.nFull_charges;
            }
            
            pStatistics_comms->enable = 1;
            pStatistics_comms->must_be_0x55 = 0x55;
            //pStatistics_comms->checksum_xor = Get_Checksum_Xor ((uint8_t *) pStatistics_comms, sizeof(BATTERY_STATISTICS_COMMS_t)-1);
        }
        if(pStatistics_comms->enable == 0x00) pStatistics_comms->enable = 1;
    }

    // return the operation result
    return ssp_error;
}

/******************************************************************************
 ** Name:    Battery_Write_Statistics
 *****************************************************************************/
/**
 ** @brief   Write the battery statistics
 **
 ** @param   pStatistics     pointer to the structure to fill
 **
 ** @return  operation result (error code)
 ******************************************************************************/
ssp_err_t Battery_Write_Statistics (BATTERY_STATISTICS_t *pStatistics)
{
    BATTERY_STATISTICS_t read_stat;
            ssp_err_t   ssp_error = SSP_SUCCESS;      // ssp error code
    static  uint8_t     my_buffer[32];  // buffer to write a single page into the memory

    ssp_error = SSP_ERR_INVALID_POINTER;        // initialize the error code
    if (pStatistics)
    {
        // fill the integrity markers ...
        pStatistics->checksum_xor = Get_Checksum_Xor ((uint8_t *) pStatistics, sizeof(BATTERY_STATISTICS_t)-1);

        // fill the buffer to send ...
        my_buffer[0] = 0;
        memcpy (&my_buffer[1], (uint8_t *) pStatistics, sizeof(BATTERY_STATISTICS_t));

        // write the battery information to the battery pack (address 0x300)
        I2C1_Write_Eeprom (0x300, my_buffer, sizeof(BATTERY_STATISTICS_t)+1);
        tx_thread_sleep (OSTIME_10MSEC);        // This time is necessary!!

        // Check write process with a read operation
        ssp_error = Battery_Read_Statistics (&read_stat);
        if ((ssp_error != SSP_SUCCESS) || (memcmp(pStatistics, &read_stat, sizeof (BATTERY_STATISTICS_t)) != 0))
        {
            return SSP_ERR_INVALID_POINTER;
        }
    }

    // return the operation result
    return ssp_error;
}

/******************************************************************************
 ** Name:    Battery_Write_Statistics_Comms
 *****************************************************************************/
/**
 ** @brief   Write the battery statistics
 **
 ** @param   pStatistics     pointer to the structure to fill
 **
 ** @return  operation result (error code)
 ******************************************************************************/
ssp_err_t Battery_Write_Statistics_Comms (BATTERY_STATISTICS_COMMS_t *pStatistics_comms)
{
    BATTERY_STATISTICS_COMMS_t  read_stat_comms;
            ssp_err_t           ssp_error = SSP_SUCCESS;      // ssp error code
    static  uint8_t             my_buffer_comms[32];   // buffer to write a single page into the memory

    ssp_error = SSP_ERR_INVALID_POINTER;        // initialize the error code
    if (pStatistics_comms)
    {
        // fill the integrity markers ...
        pStatistics_comms->checksum_xor = Get_Checksum_Xor ((uint8_t *) pStatistics_comms, sizeof(BATTERY_STATISTICS_COMMS_t)-1);

        // fill the buffer to send ...
        my_buffer_comms[0] = 0x00;
        memcpy (&my_buffer_comms[1], (uint8_t *) pStatistics_comms, 16);

        // write the battery information to the battery pack (address 0x300)
        I2C1_Write_Eeprom (0x200, my_buffer_comms, 16+1);
        tx_thread_sleep (OSTIME_10MSEC);        // This time is necessary!!

        my_buffer_comms[0] = 0x10; // LSB address of page 2
        memcpy (&my_buffer_comms[1], ((uint8_t *) pStatistics_comms)+16, 16);
        // write the battery information to the battery pack (address 0x210)
        I2C1_Write_Eeprom (0x200, my_buffer_comms, 16+1); // MSB address of page 2
        tx_thread_sleep (OSTIME_10MSEC);        // This time is necessary!!

        // fill the buffer to send ...
        /*my_buffer_comms[0] = 0x0;
        memcpy (&my_buffer_comms[1], (uint8_t *) pStatistics_comms, 14);
        // write the battery information to the battery pack (address 0x200)
        I2C1_Write_Eeprom (0x200, my_buffer_comms, 14+1);
        tx_thread_sleep (OSTIME_10MSEC);        // This time is necessary!!

        my_buffer_comms[0] = 0x0E;
        memcpy (&my_buffer_comms[1], ((uint8_t *) pStatistics_comms)+14, 2);
        // write the battery information to the battery pack (address 0x20E)
        I2C1_Write_Eeprom (0x200, my_buffer_comms, 2+1);
        tx_thread_sleep (OSTIME_10MSEC);        // This time is necessary!!

        // fill the buffer to send ...
        my_buffer_comms[0] = 0x10; // LSB address of page 2
        memcpy (&my_buffer_comms[1], ((uint8_t *) pStatistics_comms)+16, 16);
        // write the battery information to the battery pack (address 0x210)
        I2C1_Write_Eeprom (0x200, my_buffer_comms, 16+1); // MSB address of page 2
        tx_thread_sleep (OSTIME_10MSEC);        // This time is necessary!!*/

        // Check write process with a read operation
        ssp_error = Battery_Read_Statistics_Comms (&read_stat_comms);
        if ((ssp_error != SSP_SUCCESS) || (memcmp(pStatistics_comms, &read_stat_comms, sizeof (BATTERY_STATISTICS_COMMS_t)) != 0))
        {
            return SSP_ERR_INVALID_POINTER;
        }
    }

    // return the operation result
    return ssp_error;
}

/***********************************************************************************************************************
 * Function Name: Battery_Read_RTC
 * Description  : read the battery RTC
 *
 * Arguments    : pRTC     pointer to the structure to fill
 * Return Value : operation result (error code)
 ***********************************************************************************************************************/
ssp_err_t Battery_Read_RTC (BATTERY_RTC_t *pRTC)
{
    ssp_err_t       ssp_error;      // ssp error code
    BATTERY_RTC_t   rtc_bcd;        // battery RTC in BCD format

    ssp_error = SSP_ERR_INVALID_POINTER;        // initialize the error code
    if (pRTC)
    {
        ssp_error = SSP_SUCCESS;

        // read the battery information from the battery pack
        I2C1_Read_RTC ((uint8_t *) &rtc_bcd, sizeof(BATTERY_RTC_t));

        // convert from BCD to binary ...
        pRTC->sec      = (uint8_t) (((rtc_bcd.sec     >> 4) * 10) + (rtc_bcd.sec     & 0x0f));
        pRTC->min      = (uint8_t) (((rtc_bcd.min     >> 4) * 10) + (rtc_bcd.min     & 0x0f));
        pRTC->hour     = (uint8_t) (((rtc_bcd.hour    >> 4) * 10) + (rtc_bcd.hour    & 0x0f));
        pRTC->weekday  = (uint8_t) (((rtc_bcd.weekday >> 4) * 10) + (rtc_bcd.weekday & 0x0f));
        pRTC->date     = (uint8_t) (((rtc_bcd.date    >> 4) * 10) + (rtc_bcd.date    & 0x0f));
        pRTC->month    = (uint8_t) (((rtc_bcd.month   >> 4) * 10) + (rtc_bcd.month   & 0x0f));
        pRTC->year     = (uint8_t) (((rtc_bcd.year    >> 4) * 10) + (rtc_bcd.year    & 0x0f));
        pRTC->utc_time = (int8_t) Get_Device_Settings()->misc.glo_utc_time;

        // verify the date/time
        if ((pRTC->sec   > 59) || (pRTC->min   > 59) || (pRTC->hour  > 23) ||
            (pRTC->date  > 31) || (pRTC->month > 12) || (pRTC->year  > 99) ||
            (pRTC->date  == 0) || (pRTC->month == 0))
        {
            ssp_error = SSP_ERR_CLAMPED;

            // set a default value:  1-Jan 2020 at 00:00:00
            pRTC->sec     = 0;
            pRTC->min     = 0;
            pRTC->hour    = 0;
            pRTC->weekday = 1;
            pRTC->date    = 1;
            pRTC->month   = 1;
            pRTC->year    = 20;
        }

              // compensate the UTC time ...
      //RTC_Normalize_Time (pRTC);
    }

    // return the operation result
    return ssp_error;
}

/***********************************************************************************************************************
 * Function Name: Battery_Read_Temperature
 * Description  : read the battery temperature sensor value
 *
 * Arguments    : pTemp     pointer to the temperature variable
 * Return Value : operation result (error code)
 ***********************************************************************************************************************/
ssp_err_t Battery_Read_Temperature (int8_t *pTemp)
{
    int8_t pcb_temp;
    uint8_t error;

    // read from the temperature sensor mounted in the battery pack
    error = I2C1_Read_Temperature (pTemp);
    pcb_temp = (int8_t)patMon_Get_Temperature();

    if (error && (pcb_temp >=15)) *pTemp = pcb_temp;

    // update the registered temperature
    battery_temperature = *pTemp;

    // return the operation result
    return SSP_SUCCESS;
}

/******************************************************************************
 ** Name:    Battery_Get_Temperature
 *****************************************************************************/
/**
 ** @brief   Get temperature value in battery
 **
 ** @param   none
 **
 ** @return  temperature in celsius degrees
 ******************************************************************************/
int8_t Battery_Get_Temperature (void)
{
    return battery_temperature;
}

/***********************************************************************************************************************
 * Function Name: Battery_Read_Update_Date
 * Description  : read the FW update date
 *
 * Arguments    : pDate     pointer to the date
 * Return Value : operation result (error code)
 ***********************************************************************************************************************/
static ssp_err_t Battery_Read_Update_Date (DEVICE_DATE_t *pDate)
{
    uint8_t err_open = 0;
    FX_FILE my_version_file;
    uint32_t nBytes;                    // read size

    sd_media = &sd_fx_media;    // initialize the SD media handler (just in case)

    // read from usd
    // get the mutex to access to the uSD
    tx_mutex_get(&usd_mutex, TX_WAIT_FOREVER);
    err_open = (uint8_t) fx_file_open(sd_media, &my_version_file, FNAME_DATE, FX_OPEN_FOR_READ);
    if (err_open == FX_SUCCESS)
    {
        err_open = (uint8_t) fx_file_read(&my_version_file, (uint8_t *) pDate, sizeof(DEVICE_DATE_t), &nBytes);
        err_open = (uint8_t) fx_file_close (&my_version_file);
        err_open = (uint8_t) fx_media_flush (sd_media);
    }

    // release the uSD mutex
    tx_mutex_put(&usd_mutex);

    if ((err_open) || (nBytes < (sizeof(DEVICE_DATE_t))))
    {
        pDate->date = 0;
        pDate->month = 0;
        pDate->year = 0;
    }

    // return the operation result
    return SSP_SUCCESS;
}

/******************************************************************************
 ** Name:    Battery_Get_Charge
 *****************************************************************************/
/**
 ** @brief   Computes the remaining capacity in the battery
 **
 ** @param   none
 **
 ** @return  remaining capacity (in %)
 ******************************************************************************/
uint16_t Battery_Get_Charge (void)
{
    // NOTE: See "Reanibex_Life_Calculator.xlsx" file

    #define KTE 1.03333333333       // Constant to calculate the standby sum of daily + monthly days (1 + 1/30)

    #define CURRENT_SELF_DISCHARGE          9    // 1% self discharge + battery's PCB current    --> 0.009mA/h

    #define CURRENT_WHEN_STANDBY_ALL_NEW            (1+37)  // DJS 0013 A version and more new...device average current in standby  (in uA)
                                                            // standby (TLV3401) and LED every 10seconds, LED time 33msec    --> (0.001 + 0.037)mA/h

    #define CURRENT_WHEN_STANDBY_OLD_PCB_NEW_SW    (31+37)  // DJS 0013 A version and more new...device average current in standby  (in uA)
                                                            // standby (LMC7221) and LED every 10seconds, LED time 33msec    --> (0.031 + 0.037)mA/h

    #define CURRENT_WHEN_STANDBY_OLD_PCB_OLD_SW    (31+74)  // device average current in standby  (in uA)
                                                            // standby (LMC7221) and LED every 10seconds, LED time 66msec    --> (0.031 + 0.074)mA/h

    // PCB SW limitations to 5 years(Stand alone), 4 years Sigfox, 3,5 years Wifi when pcb_hw_extended == 1
    #define LIMIT_CURRENT_TEST_DAILY                        770       // Device average current in daily test (in uA/h)
    #define LIMIT_SIGFOX_GPS_CURRENT_TEST_MONTHLY           18500     // Device average current in daily test with Sigfox + GPS (in uA/h)
    #define LIMIT_WIFI_GPS_CURRENT_TEST_MONTHLY_CONNECT     43000     // Device average current in test(daily/monthly) with Wifi + GPS when connect and send/no send (in uA/h)


    // STAND_ALONE CURRENT
    #define CURRENT_RTC_WARNING             64          // Number of alerts that have been said by audio in RTC(Close cover, Call SAT, BEEP-BEEP) (in uA/h)
    #define CURRENT_WHEN_OPEN_COVER         630         // 630uA/hour during 20min (in uA/h)
    #define CURRENT_TEST_DAILY              83          // Device average current in daily test (in uA/h)
    #define CURRENT_TEST_MONTHLY            270         // Device average current in monthly test (in uA/h)
    #define CURRENT_WHEN_RUNNING            86          // Device average current when running  (in mA)
    #define CURRENT_WHEN_CHARGING           4000        // Device average current when charging (in mA)

    // STAND_ALONE TIME
    #define AVERAGE_TIME_OPEN_COVER         20          // Tick open cover (20 minutes)
    #define AVERAGE_TIME_TO_FULL_CHARGE     8           // Average time to full charge the main capacitor (in seconds)
    #define AVERAGE_TIME_TO_TEST_CHARGE     3           // Average time to test charge the main capacitor (in seconds)

    // DEMO and Test Manual
    #define DEMO_AVERAGE_TIME_TO_CHARGE     2           // Average time to full charge the main capacitor in DEMO/Manual mode (in seconds)
    #define DEMO_CURRENT_WHEN_CHARGING      1000        // Device average current when charging in DEMO/Manual mode (in mA)

    // SIGFOX CURRENT
    #define SIGFOX_CURRENT_COVER                323       // Device average current when cover is open and close (in uA/h)
    #define SIGFOX_CURRENT_TEST_DAILY           650       // Device average current in daily test with Sigfox (in uA/h)
    #define SIGFOX_CURRENT_TEST_MONTHLY         987       // Device average current in monthly test with Sigfox (in uA/h)
    #define SIGFOX_GPS_CURRENT_TEST_MONTHLY     4784      // Device average current in monthly test with Sigfox and 3 minutes of GPS (in uA/h)
                                                          // Device average current when running  (in mA) --> Same as stand alone

    // WIFI CURRENT
    #define WIFI_CURRENT_COVER_CONNECT              1011     // Device average current when cover is open/close and connect (in uA/h)
    #define WIFI_CURRENT_COVER_NO_CONNECT           2333     // Device average current when cover is open/close and no connect (in uA/h)
    #define WIFI_CURRENT_TEST_CONNECT               1597     // Device average current in test(daily/monthly) with Wifi when connect and send/no send (in uA/h)
    #define WIFI_CURRENT_TEST_NO_CONNECT            3107     // Device average current in test(daily/monthly) with Wifi when no connect and no send (in uA/h)
    #define WIFI_GPS_CURRENT_TEST_MONTHLY_CONNECT   9848     // Device average current in monthly test with Wifi when connect and send/no send and 3 minutes of GPS (in uA/h)
    #define WIFI_CURRENT_WHEN_RUNNING               160      // Device average current when running  (in mA)


    #define NOMINAL_CAPACITY(a)             ((a * 95) / 100)
    #define BATTERY_CHARGE(a,b,c,d,e)       (uint16_t) (100 - (a + b + c + d + e))
    #define DISCHARGE_STANDBY(a,b,c)        (float_t) (((float_t)a * 24 * (float_t)b) / ((float_t)c * 10))  //(a*(b[uAh]/1000)*100/(c[mAh])) * 24 h
    #define DISCHARGE_TEST(a,b,c)           (float_t) (((float_t)a * (float_t)b) / ((float_t)c * 10))       //(a*(b[uAh]/1000)*100/(c[mAh]))
    #define DISCHARGE_CHARGES(a,b,c,d)      (float_t) (((float_t)a * (float_t)b * (float_t)c) / ((float_t)d * 36))   //(a*(b[seg]/3600)*c[mA]*100)/(d[mAh])
    #define DISCHARGE_RTIME(a,b,c)          (float_t) (((float_t)a * (float_t)b * 10) / ((float_t)c * 6))   //(a*(b[min]/60)*c[mA]*100)/(d[mAh])
    #define DISCHARGE_OPENCOVER(a,b,c,d)    (float_t) (((float_t)a * (float_t)b * (float_t)c) / ((float_t)d * 600))  //(a*(b 20[min]/60)*c[mA]/1000*100)/(d[mAh])

    uint32_t    nDays = 0, nDays_mydate_battdate = 0;   // Number of days to compute the standby discharge
    //uint32_t    nTest_D = 0, nTest_M = 0;               // Number of daily test and monthly test
    float_t    discharge_standby_old = 0;              // Discharge in standby with old SW
    float_t    discharge_standby_new = 0;              // Discharge in standby with new SW
    float_t    discharge_test = 0;                     // Discharge in standby for doing daily Test
    float_t    discharge_standby = 0;                  // Discharge in standby
    float_t    discharge_running = 0;                  // Discharge in running
    float_t    discharge_run_HV = 0;                   // Discharge while running the HV circuits
    float_t    discharge_open_cover = 0;               // Discharge in standby with open cover
    uint32_t   my_date = 0, sw_date = 0;               // Current date, SW update date
    uint32_t   nominal_capacity = 0;                   // Compensated nominal capacity for the battery

    // Check if the battery is a controlled battery (MUST contain valid information in the info and statistics structures)
    if ((battery_info.must_be_0xAA != 0xAA) || (battery_statistics.must_be_0x55 != 0x55))
    {
        return 0;
    }

    // Apply a safety margin ...
    nominal_capacity = NOMINAL_CAPACITY(battery_info.nominal_capacity);
    if (nominal_capacity == 0) return 0;

    // Format the current date ...
    my_date  = (uint32_t) (battery_RTC.year + 2000) << 16;
    my_date += (uint32_t)  battery_RTC.month <<  8;
    my_date += (uint32_t)  battery_RTC.date;

    // Check if RTC is ok)
    if (my_date == 132382977)        // This value correspond do default date in case of RTC error
    {
        battery_charge = 0;
        return 0;
    }

    // Format SW update date
    if (save_sw_date.year == 0)
    {
        sw_date = my_date;
    }
    else
    {
        sw_date  = (uint32_t) (save_sw_date.year + 2000) << 16;
        sw_date += (uint32_t)  save_sw_date.month <<  8;
        sw_date += (uint32_t)  save_sw_date.date;
    }
    //Trace_Arg (TRACE_NEWLINE, "sw_date = %d", (uint32_t)sw_date);
    //Trace_Arg (TRACE_NEWLINE, "my_date = %d", (uint32_t)my_date);

    // Add test consumptions
    nDays_mydate_battdate = diff_time_days (my_date, battery_info.manufacture_date);

    discharge_standby = DISCHARGE_STANDBY(nDays_mydate_battdate, CURRENT_SELF_DISCHARGE, nominal_capacity);

    // Software update date must be older or equal than current day
    if (diff_time_days(my_date, sw_date) == 0) sw_date = my_date;

    // Calculate time to apply old consumptions
    nDays   = diff_time_days (sw_date, battery_info.manufacture_date);
    //Trace_Arg (TRACE_NEWLINE, "nDays_old = %d", (uint32_t)nDays);
    if(Is_Battery_Mode_Demo())
    {
        discharge_standby_old = 0;
    }
    else discharge_standby_old = DISCHARGE_STANDBY(nDays, CURRENT_WHEN_STANDBY_OLD_PCB_OLD_SW, nominal_capacity);

    //////////////////////////////////////////////////////////////////////////////////////////////////////////
    // Standby consumptions
    //////////////////////////////////////////////////////////////////////////////////////////////////////////
    if(Get_NFC_Device_Info()->pcb_hw.pcb_hw_version >= 1)
    {
        discharge_standby_new = DISCHARGE_STANDBY((battery_statistics_comms.daily_test*KTE), CURRENT_WHEN_STANDBY_ALL_NEW, nominal_capacity);
    }
    else
    {
        discharge_standby_new = DISCHARGE_STANDBY((battery_statistics_comms.daily_test*KTE), CURRENT_WHEN_STANDBY_OLD_PCB_NEW_SW, nominal_capacity);
    }

    discharge_standby += discharge_standby_old + discharge_standby_new;

    discharge_standby += DISCHARGE_TEST(battery_statistics_comms.rtc_warning, CURRENT_RTC_WARNING, nominal_capacity);

    // Comsuption in stand alone mode
    if(Get_NFC_Device_Info()->pcb_hw.pcb_hw_extended == 1)
    {
        discharge_test = DISCHARGE_TEST(battery_statistics_comms.daily_test, CURRENT_TEST_DAILY, nominal_capacity);
    }
    else discharge_test = DISCHARGE_TEST(battery_statistics_comms.daily_test, LIMIT_CURRENT_TEST_DAILY, nominal_capacity);
    discharge_test += DISCHARGE_TEST(battery_statistics.nTestManual, CURRENT_TEST_MONTHLY, nominal_capacity);

    // Comsuption with Sigfox
    discharge_test += DISCHARGE_TEST(battery_statistics_comms.sigfox_daily_test, SIGFOX_CURRENT_TEST_DAILY, nominal_capacity);
    discharge_test += DISCHARGE_TEST(battery_statistics_comms.sigfox_manual_test, SIGFOX_CURRENT_TEST_MONTHLY, nominal_capacity);
    if(Get_NFC_Device_Info()->pcb_hw.pcb_hw_extended == 1)
    {
        discharge_test += DISCHARGE_TEST(battery_statistics_comms.sigfox_manual_test_gps, SIGFOX_GPS_CURRENT_TEST_MONTHLY, nominal_capacity);
    }
    else discharge_test += DISCHARGE_TEST(battery_statistics_comms.sigfox_manual_test_gps, LIMIT_SIGFOX_GPS_CURRENT_TEST_MONTHLY, nominal_capacity);

    // Comsuption with Wifi
    discharge_test += DISCHARGE_TEST(battery_statistics_comms.wifi_test_connect, WIFI_CURRENT_TEST_CONNECT, nominal_capacity);
    discharge_test += DISCHARGE_TEST(battery_statistics_comms.wifi_test_no_connect, WIFI_CURRENT_TEST_NO_CONNECT, nominal_capacity);
    if(Get_NFC_Device_Info()->pcb_hw.pcb_hw_extended == 1)
    {
        discharge_test += DISCHARGE_TEST(battery_statistics_comms.wifi_test_connect_gps, WIFI_GPS_CURRENT_TEST_MONTHLY_CONNECT, nominal_capacity);
    }
    else discharge_test += DISCHARGE_TEST(battery_statistics_comms.wifi_test_connect_gps, LIMIT_WIFI_GPS_CURRENT_TEST_MONTHLY_CONNECT, nominal_capacity);

    //////////////////////////////////////////////////////////////////////////////////////////////////////////
    // Monitoring patient consumptions
    //////////////////////////////////////////////////////////////////////////////////////////////////////////
    // Get the battery discharge percent when running (monitoring the patient, maintenance, autotest, etc...)  ...
    discharge_running = DISCHARGE_RTIME(battery_statistics.runTime_total, CURRENT_WHEN_RUNNING, nominal_capacity);
    discharge_running += DISCHARGE_RTIME(battery_statistics_comms.wifi_runTime_total, WIFI_CURRENT_WHEN_RUNNING, nominal_capacity);

    //////////////////////////////////////////////////////////////////////////////////////////////////////////
    // Capacitor chargers consumptions
    //////////////////////////////////////////////////////////////////////////////////////////////////////////
    // Get the battery discharge percent when running different HV circuits (capacitor precharge, full charge, shock, etc...)
    if(Is_Battery_Mode_Demo())
    {
        discharge_run_HV = DISCHARGE_CHARGES(battery_statistics.nFull_charges, DEMO_AVERAGE_TIME_TO_CHARGE, DEMO_CURRENT_WHEN_CHARGING, nominal_capacity);
        discharge_run_HV += DISCHARGE_CHARGES(battery_statistics_comms.nTest_charges, AVERAGE_TIME_TO_TEST_CHARGE/2, CURRENT_WHEN_CHARGING, nominal_capacity);
    }
    else
    {
        discharge_run_HV = DISCHARGE_CHARGES(battery_statistics.nFull_charges, AVERAGE_TIME_TO_FULL_CHARGE, CURRENT_WHEN_CHARGING, nominal_capacity);
        discharge_run_HV += DISCHARGE_CHARGES(battery_statistics_comms.nTest_charges, AVERAGE_TIME_TO_TEST_CHARGE/2, CURRENT_WHEN_CHARGING, nominal_capacity);
    }

    //////////////////////////////////////////////////////////////////////////////////////////////////////////
    // Open cover consumptions
    //////////////////////////////////////////////////////////////////////////////////////////////////////////
    // Get the battery discharge percent when device cover is open
    discharge_open_cover =  DISCHARGE_OPENCOVER(battery_statistics.nTicks_open_cover, AVERAGE_TIME_OPEN_COVER, CURRENT_WHEN_OPEN_COVER, nominal_capacity);
    discharge_open_cover +=  DISCHARGE_TEST(battery_statistics_comms.sigfox_open_close_cover, SIGFOX_CURRENT_COVER, nominal_capacity);
    discharge_open_cover +=  DISCHARGE_TEST(battery_statistics_comms.wifi_cover_connect, WIFI_CURRENT_COVER_CONNECT, nominal_capacity);
    discharge_open_cover +=  DISCHARGE_TEST(battery_statistics_comms.wifi_cover_no_connect, WIFI_CURRENT_COVER_NO_CONNECT, nominal_capacity);

    // If the remaining capacity is bigger than 100% return 1
    if ((discharge_standby + discharge_test + discharge_running + discharge_run_HV + discharge_open_cover) > 100)
    {
        battery_charge = 1;
        return 1;
    }

    battery_charge = BATTERY_CHARGE(discharge_standby, discharge_test, discharge_running, discharge_run_HV, discharge_open_cover);

    return battery_charge;
}

void set_open_cover(uint16_t value){
    battery_statistics.nTicks_open_cover = value;
    battery_statistics_comms.sigfox_open_close_cover = value;
    battery_statistics_comms.wifi_cover_connect = value;
    battery_statistics_comms.wifi_cover_no_connect = value;
}

void set_charges(uint16_t value){
    battery_statistics.nFull_charges = value;
    battery_statistics_comms.nTest_charges = value;
}

void set_runTime(uint32_t value){
    battery_statistics.runTime_total = value;
    battery_statistics_comms.wifi_runTime_total = value;
}

void set_daily_test(uint16_t value){
    battery_statistics_comms.daily_test = value;
    battery_statistics.nTestManual = value;
    battery_statistics_comms.sigfox_daily_test = value;
    battery_statistics_comms.sigfox_manual_test_gps = value;
    battery_statistics_comms.wifi_test_connect = value;
    battery_statistics_comms.wifi_test_no_connect = value;
    battery_statistics_comms.wifi_test_connect_gps = value;
}

void set_battery_manufacture_date(uint8_t year, uint8_t month, uint8_t day){
    uint32_t CU_date;
    CU_date  = (uint32_t) (year + 2000) << 16;
    CU_date += (uint32_t)  month <<  8;
    CU_date += (uint32_t)  day;

    battery_info.manufacture_date = CU_date;
}
void set_battery_info_integrity(uint8_t value){
    battery_info.must_be_0xAA = value;
}

void set_battery_statistics_integrity(uint8_t value){
    battery_statistics.must_be_0x55 = value;
}

void set_battery_RTC_date(uint8_t year, uint8_t month, uint8_t day){
    battery_RTC.year = year;
    battery_RTC.month = month;
    battery_RTC.date = day;
}

void set_nominal_capacity(uint32_t value){
    battery_info.nominal_capacity = value;
}

void set_update_year(uint8_t value){
    save_sw_date.year = value;
}

/******************************************************************************
 ** Name:    Battery_I2C_Init
 *****************************************************************************/
/**
 ** @brief   Initializes the battery i2c port to communicate with
 **          internals RTC and EEprom
 **
 ** @param   ponDate        pointer to formatted date
 ** @param   ponTime        pointer to formatted time
 **
 ** @return  none
 ******************************************************************************/
void Battery_I2C_Init (uint32_t *ponDate, uint32_t *ponTime)
{
    ssp_err_t   ssp_error;      // ssp error code

    // Save upgrade date
    ssp_error = Battery_Read_Update_Date (&save_sw_date);

    // Read the battery information from the battery pack (if any error is detected, set battery name as "Unknown")
    ssp_error = Battery_Read_Info (&battery_info);
    if (ssp_error != SSP_SUCCESS) strcpy ((char_t *) battery_info.name, "Unknown");

    // Read the battery statistics from the battery pack (if any error is detected, set battery name as "Unknown")
    ssp_error = Battery_Read_Statistics (&battery_statistics);
    if (ssp_error != SSP_SUCCESS) strcpy ((char_t *) battery_info.name, "Unknown");

    // Read the battery statistics from the battery pack (if any error is detected, set battery name as "Unknown")
    ssp_error = Battery_Read_Statistics_Comms (&battery_statistics_comms);
    if (ssp_error != SSP_SUCCESS) strcpy ((char_t *) battery_info.name, "Unknown");

    // Read the battery current date & time from the battery pack
    ssp_error = Battery_Read_RTC (&battery_RTC);

    ssp_error = Battery_Read_Temperature (&battery_temperature);

    // Check if must accelerate the time count
#if (ACCELERATOR_TIME == 1)
    battery_RTC.hour = battery_RTC.min % 24;
#endif

    // Register the power on date
    *ponDate  = (uint32_t) (battery_RTC.year + 2000) << 16;
    *ponDate += (uint32_t)  battery_RTC.month <<  8;
    *ponDate += (uint32_t)  battery_RTC.date;

    // register the power on time
    *ponTime  = (uint32_t) (battery_RTC.hour << 16);
    *ponTime += (uint32_t) (battery_RTC.min  <<  8);
    *ponTime += (uint32_t) (battery_RTC.sec);
}

/******************************************************************************
** Name:    Battery_I2C_Init_RTC
 *****************************************************************************/
/**
 ** @brief   Initializes the battery i2c port to communicate with
 **          internals RTC and EEprom
 **
 ** @param   ponDate        pointer to formatted date
 ** @param   ponTime        pointer to formatted time
 **
 ** @return  none
 ******************************************************************************/
ssp_err_t Battery_I2C_Init_RTC (uint32_t *ponDate, uint32_t *ponTime)
{
    ssp_err_t       ssp_error;      // ssp error code

    // read the battery current date & time from the battery pack
    ssp_error = Battery_Read_RTC (&battery_RTC);

    // register the power on date
    *ponDate  = (uint32_t) (battery_RTC.year + 2000) << 16;
    *ponDate += (uint32_t)  battery_RTC.month <<  8;
    *ponDate += (uint32_t)  battery_RTC.date;

    // register the power on time
    *ponTime  = (uint32_t) (battery_RTC.hour << 16);
    *ponTime += (uint32_t) (battery_RTC.min  <<  8);
    *ponTime += (uint32_t) (battery_RTC.sec);

    return ssp_error;
}

/******************************************************************************
** Name:    Get_Date
*****************************************************************************/
/**
** @brief   Report the date information
**
** @param   pYear     pointer to year
** @param   pMonth    pointer to month
** @param   pDate     pointer to date
**
** @return  none
******************************************************************************/
void Get_Date (uint8_t *pYear, uint8_t *pMonth, uint8_t *pDate)
{
    if (pYear)  *pYear  = battery_RTC.year;
    if (pMonth) *pMonth = battery_RTC.month;
    if (pDate)  *pDate  = battery_RTC.date;
}

/******************************************************************************
** Name:    Get_Time
*****************************************************************************/
/**
** @brief   Report the time information
**
** @param   pHour    pointer to hour
** @param   pMin     pointer to minutes
** @param   pSec     pointer to seconds
**
** @return  none
******************************************************************************/
void Get_Time (uint8_t *pHour, uint8_t *pMin, uint8_t *pSec)
{
    if (pHour) *pHour = battery_RTC.hour;
    if (pMin)  *pMin  = battery_RTC.min;
    if (pSec)  *pSec  = battery_RTC.sec;
}

/******************************************************************************
** Name:    Get_Rem_Charge
*****************************************************************************/
/**
** @brief   Report the remaining battery charge
**
** @param   none
**
** @return  remaining charge (%)
******************************************************************************/
uint8_t Get_Rem_Charge (void)
{
    return ((uint8_t) battery_charge);
}

/******************************************************************************
 ** Name:    Inc_OpenCover
 *****************************************************************************/
/**
 ** @brief   Increment the number of ticks with open cover
 **
 ** @param   none
 **
 ** @return  none
 *****************************************************************************/
inline void Inc_OpenCover (void)
{
    battery_statistics.nTicks_open_cover++;
    //Trace_Arg (TRACE_NEWLINE, "  battery_statistics.nTicks_open_cover = %d", battery_statistics.nTicks_open_cover);
}

/******************************************************************************
 ** Name:    Inc_TestManual
 *****************************************************************************/
/**
 ** @brief   Increment the number of test manual
 **
 ** @param   none
 **
 ** @return  none
 ******************************************************************************/
inline void Inc_TestManual (void)
{
    battery_statistics.nTestManual++;
    //Trace_Arg (TRACE_NEWLINE, "  battery_statistics.nTestManual = %d", battery_statistics.nTestManual);
}

/******************************************************************************
 ** Name:    Inc_FullCharges
 *****************************************************************************/
/**
 ** @brief   Increment the number of Vc full charges
 **
 ** @param   none
 **
 ** @return  none
 *****************************************************************************/
inline void Inc_FullCharges (void)
{
    battery_statistics.nFull_charges++;
    //Trace_Arg (TRACE_NEWLINE, "  battery_statistics.nFull_charges = %d", battery_statistics.nFull_charges);
}

/******************************************************************************
 ** Name:    Inc_RunTime
 *****************************************************************************/
/**
 ** @brief   Increment the number of running time
 **
 ** @param   none
 **
 ** @return  none
 *****************************************************************************/
inline void Inc_RunTime (void)
{
    battery_statistics.runTime_total++;
    //Trace_Arg (TRACE_NEWLINE, "  battery_statistics.runTime_total = %d", battery_statistics.runTime_total);
}

/******************************************************************************
 ** Name:    Inc_RunTime_Update
 *****************************************************************************/
/**
 ** @brief   Increment the number of running time
 **
 ** @param   none
 **
 ** @return  none
 *****************************************************************************/
inline void Inc_RunTime_Update (void)
{
    battery_statistics.runTime_total += 10;
    //Trace_Arg (TRACE_NEWLINE, "  battery_statistics.runTime_total = %d", battery_statistics.runTime_total);
}

/******************************************************************************
 ** Name:    Inc_TestCharges
 *****************************************************************************/
/**
 ** @brief   Increment the number of Vc test charges
 **
 ** @param   none
 **
 ** @return  none
 *****************************************************************************/
inline void Inc_TestCharges (void)
{
    battery_statistics_comms.nTest_charges++;
    //Trace_Arg (TRACE_NEWLINE, "  battery_statistics_comms.nTest_charges = %d", battery_statistics_comms.nTest_charges);
}

/******************************************************************************
 ** Name:    Inc_DailyTest
 *****************************************************************************/
/**
 ** @brief   Increment the number of daily test
 **
 ** @param   none
 **
 ** @return  none
 *****************************************************************************/
inline void Inc_DailyTest (void)
{
    battery_statistics_comms.daily_test++;
    //Trace_Arg (TRACE_NEWLINE, "  battery_statistics_comms.daily_test = %d", battery_statistics_comms.daily_test);
}

/******************************************************************************
 ** Name:    Inc_RTCWarning
 *****************************************************************************/
/**
 ** @brief   Increment the number of alerts that have been said by audio in RTC(Close cover, Call SAT, BEEP-BEEP)
 **
 ** @param   none
 **
 ** @return  none
 *****************************************************************************/
inline void Inc_RTCWarning (void)
{
    battery_statistics_comms.rtc_warning++;
    //Trace_Arg (TRACE_NEWLINE, "  battery_statistics_comms.rtc_warning = %d", battery_statistics_comms.rtc_warning);
}

/******************************************************************************
 ** Name:    Inc_SigfoxCover
 *****************************************************************************/
/**
 ** @brief   Increment the number of alerts that have been sent with the open and close cover with Sigfox
 **
 ** @param   none
 **
 ** @return  none
 *****************************************************************************/
inline void Inc_SigfoxCover (void)
{
    battery_statistics_comms.sigfox_open_close_cover++;
    //Trace_Arg (TRACE_NEWLINE, "  battery_statistics_comms.sigfox_open_close_cover = %d", battery_statistics_comms.sigfox_open_close_cover);
}

/******************************************************************************
 ** Name:    Inc_SigfoxDailyTest
 *****************************************************************************/
/**
 ** @brief   Increment the number of daily test that have been sent with Sigfox
 **
 ** @param   none
 **
 ** @return  none
 *****************************************************************************/
inline void Inc_SigfoxDailyTest (void)
{
    battery_statistics_comms.sigfox_daily_test++;
    if(battery_statistics_comms.daily_test != 0) battery_statistics_comms.daily_test--;
    //Trace_Arg (TRACE_NEWLINE, "  battery_statistics_comms.sigfox_daily_test = %d", battery_statistics_comms.sigfox_daily_test);
    //Trace_Arg (TRACE_NEWLINE, "  battery_statistics_comms.daily_test = %d", battery_statistics_comms.daily_test);
}

/******************************************************************************
 ** Name:    Inc_SigfoxManualTest
 *****************************************************************************/
/**
 ** @brief   Increment the number of monthly test that have been sent with Sigfox
 **
 ** @param   none
 **
 ** @return  none
 *****************************************************************************/
inline void Inc_SigfoxManualTest (void)
{
    battery_statistics_comms.sigfox_manual_test++;
    if(battery_statistics.nTestManual != 0) battery_statistics.nTestManual--;
    //Trace_Arg (TRACE_NEWLINE, "  battery_statistics_comms.sigfox_manual_test = %d", battery_statistics_comms.sigfox_manual_test);
    //Trace_Arg (TRACE_NEWLINE, "  battery_statistics.nTestManual = %d", battery_statistics.nTestManual);
}

/******************************************************************************
 ** Name:    Inc_SigfoxManualTestGPS
 *****************************************************************************/
/**
 ** @brief   Increment the number of monthly test that have been sent with Sigfox + GPS
 **
 ** @param   none
 **
 ** @return  none
 *****************************************************************************/
inline void Inc_SigfoxManualTestGPS (void)
{
    battery_statistics_comms.sigfox_manual_test_gps++;
    if(battery_statistics_comms.sigfox_manual_test != 0) battery_statistics_comms.sigfox_manual_test--;
    if(battery_statistics.nTestManual != 0) battery_statistics.nTestManual--;
    //Trace_Arg (TRACE_NEWLINE, "  battery_statistics_comms.sigfox_manual_test_gps = %d", battery_statistics_comms.sigfox_manual_test_gps);
    //Trace_Arg (TRACE_NEWLINE, "  battery_statistics_comms.sigfox_manual_test = %d", battery_statistics_comms.sigfox_manual_test);
    //Trace_Arg (TRACE_NEWLINE, "  battery_statistics.nTestManual = %d", battery_statistics.nTestManual);
}

/******************************************************************************
 ** Name:    Inc_WifiCoverConnect
 *****************************************************************************/
/**
 ** @brief   Increment the number of alerts that have been sent with the open and close cover with Wifi
 **
 ** @param   none
 **
 ** @return  none
 *****************************************************************************/
inline void Inc_WifiCoverConnect (void)
{
    battery_statistics_comms.wifi_cover_connect++;
    //Trace_Arg (TRACE_NEWLINE, "  battery_statistics_comms.wifi_cover_connect = %d", battery_statistics_comms.wifi_cover_connect);
}

/******************************************************************************
 ** Name:    Inc_WifiCoverNoConnect
 *****************************************************************************/
/**
 ** @brief   Increment the number of of alerts that have no been sent with the open and close cover with Wifi
 **
 ** @param   none
 **
 ** @return  none
 *****************************************************************************/
inline void Inc_WifiCoverNoConnect (void)
{
    battery_statistics_comms.wifi_cover_no_connect++;
    //Trace_Arg (TRACE_NEWLINE, "  battery_statistics_comms.wifi_cover_no_connect = %d", battery_statistics_comms.wifi_cover_no_connect);
}

/******************************************************************************
 ** Name:    Inc_WifiTestConnect
 *****************************************************************************/
/**
 ** @brief   Increment the number of times that WiFi has been connected successfully
 **
 ** @param   none
 **
 ** @return  none
 *****************************************************************************/
inline void Inc_WifiTestConnect (WIFI_TEST_RESULT_t *pwifi_test_result)
{
    battery_statistics_comms.wifi_test_connect++;
    if(battery_statistics_comms.daily_test != 0 && pwifi_test_result->info_type == WIFI_MSG_ID_DIARY_TEST) battery_statistics_comms.daily_test--;
    if(battery_statistics.nTestManual != 0 && 
       (pwifi_test_result->info_type == WIFI_MSG_ID_MONTHLY_TEST || 
       pwifi_test_result->info_type == WIFI_MSG_ID_MANUAL_TEST)) battery_statistics.nTestManual--;
    
    //Trace_Arg (TRACE_NEWLINE, "  battery_statistics_comms.wifi_test_connect = %d", battery_statistics_comms.wifi_test_connect);
}

/******************************************************************************
 ** Name:    Inc_WifiTestNoConnect
 *****************************************************************************/
/**
 ** @brief   Increment the number of times that WiFi has not been connected successfully
 **
 ** @param   none
 **
 ** @return  none
 *****************************************************************************/
inline void Inc_WifiTestNoConnect (WIFI_TEST_RESULT_t *pwifi_test_result)
{
    battery_statistics_comms.wifi_test_no_connect++;
    if(battery_statistics_comms.daily_test != 0 && pwifi_test_result->info_type == WIFI_MSG_ID_DIARY_TEST) battery_statistics_comms.daily_test--;
    if(battery_statistics.nTestManual != 0 && 
       (pwifi_test_result->info_type == WIFI_MSG_ID_MONTHLY_TEST || 
       pwifi_test_result->info_type == WIFI_MSG_ID_MANUAL_TEST)) battery_statistics.nTestManual--;

    //Trace_Arg (TRACE_NEWLINE, "  battery_statistics_comms.wifi_test_no_connect = %d", battery_statistics_comms.wifi_test_no_connect);
}

/******************************************************************************
 ** Name:    Inc_WifiTestConnectGPS
 *****************************************************************************/
/**
 ** @brief   Increment the number of fully test that have been sent with Wifi + GPS
 **
 ** @param   none
 **
 ** @return  none
 *****************************************************************************/
inline void Inc_WifiTestConnectGPS (void)
{
    battery_statistics_comms.wifi_test_connect_gps++;
    if(battery_statistics_comms.wifi_test_connect != 0)battery_statistics_comms.wifi_test_connect--;
    if(battery_statistics.nTestManual != 0)battery_statistics.nTestManual--;

    //Trace_Arg (TRACE_NEWLINE, "  battery_statistics_comms.wifi_test_connect_gps = %d", battery_statistics_comms.wifi_test_connect_gps);
    //Trace_Arg (TRACE_NEWLINE, "  battery_statistics_comms.wifi_test_connect = %d", battery_statistics_comms.wifi_test_connect);
    //Trace_Arg (TRACE_NEWLINE, "  battery_statistics.nTestManual = %d", battery_statistics.nTestManual);
}

/******************************************************************************
 ** Name:    Inc_WifiRunTime
 *****************************************************************************/
/**
 ** @brief   Increment the number of run time
 **
 ** @param   none
 **
 ** @return  none
 *****************************************************************************/
inline void Inc_WifiRunTime (void)
{
    battery_statistics_comms.wifi_runTime_total++;
    //Trace_Arg (TRACE_NEWLINE, "  battery_statistics_comms.wifi_runTime_total = %d", battery_statistics_comms.wifi_runTime_total);
}
