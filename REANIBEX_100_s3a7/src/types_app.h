/******************************************************************************
 * Name      : R100_S3A7                                                      *
 * Copyright : Osatu S. Coop                                                  *
 * Compiler  : GNU ARM Embedded                                               *
 * Target    : Reanibex Serie 100                                             *
 ******************************************************************************/

/*!
* @file        types_app.h
* @brief       Common Types' definition.
* @version     0.0
* @date        2018-02-01
* @author      ltorres
* @warning     EG           2007-10-11      V0.0      .- First edition
*
*
*/

#ifndef TYPES_APP_H
#define TYPES_APP_H
/******************************************************************************
**Includes
*/

/******************************************************************************
** Defines
*/

/******************************************************************************
 ** Typedefs
 */
typedef enum {
    eWMODE_NOMODE = 0,          // Only in initialization
    eWMODE_CFG,                 // Configuration mode
    eWMODE_AED,                 // AED mode
    eWMODE_N                    // Number of modes
}WORKING_MODE_e;

typedef enum {
    ePATIENT_TYPE_ADULT = 0,    // pediatric patient
    ePATIENT_TYPE_PEDIATRIC,    // adult patient
    ePATIENT_TYPE_MAX
} PATIENT_TYPE_e;

typedef enum {
    PON_RTC = 0,                // automatic power-on due to RTC
    PON_COVER,                  // user open the enclosure cover
    PON_BUTTON,                 // user press the button
    PON_TEST,                   // user press ALL buttons to force a Test
    PON_USB,                    // user connects USB cable to computer
    PON_TEST_MONTADOR,          // PCB test
    PON_ACC                     // Acelerometer interrupt activation
} POWER_ON_SOURCE_e;


/******************************************************************************
 ** Globals
 */

/******************************************************************************
** Prototypes
*/

#endif  /*TYPES_APP_H*/

/*
** $Log$
**
** end of types_app.h
******************************************************************************/
