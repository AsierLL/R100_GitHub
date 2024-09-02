/******************************************************************************
 * Name      : R100_S3A7                                                      *
 * Copyright : Osatu S. Coop                                                  *
 * Compiler  : GNU ARM Embedded                                               *
 * Target    : Reanibex Serie 100                                             *
 ******************************************************************************/

/*!
 * @file        event_ids.h
 * @brief       Event identifiers header file
 *
 * @version     v1
 * @date        21/05/2018
 * @author      ltorres
 * @warning     author   date    version     .- Edition
 * @bug
 *
 */

#ifndef EVENT_IDS_H_
#define EVENT_IDS_H_

/******************************************************************************
 **Includes
 */

/******************************************************************************
 ** Defines
 */

/******************************************************************************

 ** Typedefs
 */

// machine events
typedef enum
{
    eEV_NULL = 0,
    eEV_AED_START,              ///< Command the AED_FSM
    eEV_ERROR,                  ///< ----
    eEV_POWER_OFF,              ///< ----

    eEV_EL_NONE,                ///< ----
    eEV_PAT_BAD_CONN,           ///< Command the AED_FSM
    eEV_PAT_GOOD_CONN,          ///< Command the AED_FSM
    eEV_PAT_OPEN_CIRC,          ///< Command the AED_FSM
    eEV_PAT_SHORT_CIRC,         ///< Command the AED_FSM
    eEV_PAT_UNKNOWN,            ///< Command the AED_FSM

    eEV_DRD_START,              ///< Command the DRD task
    eEV_DRD_STOP,               ///< Command the DRD task
    eEV_DRD_ANALYZE,            ///< Command the DRD task
    eEV_DRD_DIAG_RDY,           ///< Command the DRD task

    eEV_DEF_PRECHARGE,          ///< Command the defibrillator task to precharge
    eEV_DEF_CHARGE,             ///< Command the defibrillator task to charge
    eEV_DEF_SHOCK_SYNC,         ///< Command the defibrillator task to shock
    eEV_DEF_SHOCK_ASYNC,        ///< Command the defibrillator task to shock
    eEV_DEF_DISARM,             ///< Command the defibrillator tast to disarm

    eEV_KEY_ONOFF,              ///< When Key ON/OFF is pressed
    eEV_KEY_SHOCK,              ///< When Key SHOCK  is pressed
    eEV_KEY_PATYPE,             ///< When Key patient type is pressed
    eEV_KEY_COVER,              ///< When the cover is closed
    eEV_LOCK_CORE,              ///< Command the lock the Core task
    eEV_USB                     ///< When USB is detected in patient mode

} EVENT_ID_e;

// Registered events
typedef enum
{
    eREG_NULL = 0,

    eREG_CPR_START,
    eREG_DRD_START,
    eREG_DRD_DIAG_RDY,

    eREG_DEF_CHARGE,

    eREG_SHOCK_1_ADULT,
    eREG_SHOCK_2_ADULT,
    eREG_SHOCK_3_ADULT,
    eREG_SHOCK_1_PEDIATRIC,
    eREG_SHOCK_2_PEDIATRIC,
    eREG_SHOCK_3_PEDIATRIC,
    eREG_SHOCK_NOT_DELIVERED,

    eREG_SET_PATIENT_ADULT,
    eREG_SET_PATIENT_PEDIATRIC,

    eREG_ECG_PAT_OPEN_CIRC,
    eREG_ECG_PAT_BAD_CONN,
    eREG_ECG_PAT_SHORT_CIRC,
    eREG_ECG_PAT_GOOD_CONN,
    eREG_ECG_EL_NONE,
    eREG_ECG_EL_UNKNOWN,
    eREG_ECG_EL_BEXEN,
    eREG_ECG_EL_EXPIRED,
    eREG_ECG_EL_USED,

    eREG_DEF_ERROR_CHARGING,
    eREG_DEF_ERROR_SHOCK,

    eREG_BAT_UNKNOWN,                    ///< Battery not recognized
    eREG_BAT_LOW_BATTERY,                ///< Low charge in battery
    eREG_BAT_VERY_LOW_BATTERY,           ///< Very low voltage in battery voltage
    eREG_BAT_LOW_TEMPERATURE,            ///< Low temperature
    eREG_BAT_HIGH_TEMPERATURE,           ///< High temperature
    eREG_BAT_VERY_LOW_TEMPERATURE,       ///< Very low temperature
    eREG_BAT_VERY_HIGH_TEMPERATURE,      ///< Very high temperature

    eREG_SAT_ERROR,
    eREG_ERROR,

    eREG_POWER_OFF,
    eREG_WIFI_POWER_OFF

} REG_EVENTS_e;



/******************************************************************************
 ** Globals
 */

/******************************************************************************
 ** Prototypes
 */

#endif /* EVENT_IDS_H_ */

/*
 ** $Log$
 **
 ** end of event_ids.h
 ******************************************************************************/
