/******************************************************************************
 * Name      : R100_S3A7                                                      *
 * Copyright : Osatu S. Coop                                                  *
 * Compiler  : GNU ARM Embedded                                               *
 * Target    : Reanibex Serie 100                                             *
 ******************************************************************************/

/*!
 * @file        thread_core_entry.h
 * @brief       Header with functions related to the core task
 *
 * @version     v1
 * @date        16/01/2018
 * @author      ltorres
 * @warning     author   date    version     .- Edition
 * @bug
 *
 */

#ifndef THREAD_CORE_HAL_H_
#define THREAD_CORE_HAL_H_

/******************************************************************************
 **Includes
 */
#include <device_init.h>
#include "types_app.h"
#include "event_ids.h"

#include "thread_sysMon_entry.h"


/******************************************************************************
 ** Defines
 */

#define     LED_VALUE_OFF   IOPORT_LEVEL_LOW
#define     LED_VALUE_ON    IOPORT_LEVEL_HIGH

/******************************************************************************
 ** Typedefs
 */

/******************************************************************************
 ** Globals
 */
extern  WORKING_MODE_e       curr_working_mode;

extern  DEVICE_SETTINGS_t*   pDevice_Settings;
extern  AED_SETTINGS_t*      pAed_Settings;
extern  MISC_SETTINGS_t*     pMisc_Settings;


/******************************************************************************
 ** Prototypes
 */


extern  void        FSM_R100_AED (EVENT_ID_e aed_event);

extern  void        Pastime_While_Audio_Playing                  (void);
extern  RCP_BREAK_t Pastime_While_Audio_Playing_with_Break_PT    (uint32_t delay);
extern  RCP_BREAK_t Pastime_While_Audio_Playing_with_Break_PT_OC (uint32_t delay);
extern  RCP_BREAK_t Pastime_While_Audio_Playing_with_Break_PT_CC (void);

extern  bool_t      Is_Patient_Type_Change (void);
extern  uint8_t     Get_RCP_Rate (void);
extern  void        Send_Core_Queue(EVENT_ID_e ev);

#endif /* THREAD_CORE_HAL_H_ */

/*
 ** $Log$
 **
 ** end of thread_core_hal.h
 ******************************************************************************/
