/******************************************************************************
 * Name      : R100_S3A7                                                      *
 * Copyright : Osatu S. Coop                                                  *
 * Compiler  : GNU ARM Embedded                                               *
 * Target    : Reanibex Serie 100                                             *
 ******************************************************************************/

/*!
 * @file        RTC.h
 * @brief       Header with functions related to the RTC
 *
 * @version     v1
 * @date        16/01/2018
 * @author      ltorres
 * @warning     author   date    version     .- Edition
 * @bug
 *
 */

#ifndef RTC_H_
#define RTC_H_

/******************************************************************************
 **Includes
 */

/******************************************************************************
 ** Defines
 */

/* Set values for set calendar time function */
#define START_TIME_HR                   (0)     ///< 0
#define START_TIME_MIN                  (0)     ///< 0
#define START_TIME_SEC                  (0)     ///< 0
#define START_TIME_ISDST                (0)     ///< Daylight Saving Time Flag (set to zero for standard time)
#define START_TIME_MDAY                 (1)     ///< Value must be between 1 to  31 inclusive
#define START_TIME_MON                  (0)     ///< Value must be between 0 to  11 inclusive
#define START_TIME_WDAY                 (0)     ///< Value must be between 0 to   6 inclusive
#define START_TIME_YDAY                 (0)     ///< Value must be between 0 to 365 inclusive
#define START_TIME_YEAR                 (100)   ///< Years since 1900

#define RTCIO_PIN                       IOPORT_PORT_04_PIN_02

/******************************************************************************
 ** Typedefs
 */


/******************************************************************************
 ** Globals
 */

/******************************************************************************
 ** Prototypes
 */

void    Rtc_Init            (void);

bool_t  Rtc_Test_Trigger    (void);

void    Rtc_Program_Wakeup  (uint32_t msec);
void    Rtc_Program_Kill    (uint32_t msec);


#endif /* RTC_H_ */

/*
 ** $Log$
 **
 ** end of RTC.h
 ******************************************************************************/

