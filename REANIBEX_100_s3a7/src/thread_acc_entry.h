/******************************************************************************
 * Name      : R100_S3A7                                                      *
 * Copyright : Osatu S. Coop                                                  *
 * Compiler  : GNU ARM Embedded                                               *
 * Target    : Reanibex Serie 100                                             *
 ******************************************************************************/

/*!
 * @file        thread_acc_entry.h
 * @brief       Header with functions related to the acc task
 *
 * @version     v1
 * @date        27/07/2021
 * @author      ivicente
 * @warning     author   date    version     .- Edition
 * @bug
 *
 */

#ifndef THREAD_ACC_ENTRY_H_
#define THREAD_ACC_ENTRY_H_

/******************************************************************************
 * INCLUDES
 */
#include "tx_api.h"
#include "tx_trace.h"
#include "tx_timer.h"

/******************************************************************************
 * EXTERNS
 */

/******************************************************************************
 * GLOBALS
 */

/******************************************************************************
 ** Macros
 */
#define ACC_INT_COUNT_MAX               10          ///< ACC counter to send message

/******************************************************************************
 ** Typedefs
 */
// Enum to comunicate between SysMon and ACC threads
typedef enum
{
    ACC_STATUS_OK = 0,      ///< OK state for the ACC thread to do a POWER_OFF
    ACC_STATUS_PON = 1,     ///< PON state so that the ACC thread does not make a POWER_OFF

}EV_ACC_e;


/******************************************************************************
 * PROTOTYPES
 */

void            ACC_Cal_Mov     (void);
void            ACC_Send_Comms  (void);
void            ACC_Kill        (void);
void            Led_Blink_ACC   (bool_t force);

#endif /* THREAD_ACC_ENTRY_H_ */

/*
 ** $Log$
 **
 ** end of thread_acc_entry.h
 ******************************************************************************/