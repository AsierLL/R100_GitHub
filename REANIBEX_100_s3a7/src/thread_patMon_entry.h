/******************************************************************************
 * Name      : R100_S3A7                                                      *
 * Copyright : Osatu S. Coop                                                  *
 * Compiler  : GNU ARM Embedded                                               *
 * Target    : Reanibex Serie 100                                             *
 ******************************************************************************/

/*!
 * @file        thread_patMon_entry.h
 * @brief       Header with functions related to the patient monitor task
 *
 * @version     v1
 * @date        16/01/2018
 * @author      ltorres
 * @warning     author   date    version     .- Edition
 * @bug
 *
 */

#ifndef THREAD_CORE_ENTRY_H_
#define THREAD_CORE_ENTRY_H_

/******************************************************************************
 ** Includes
 */
#include "event_ids.h"

/******************************************************************************
 ** Defines
 */

/******************************************************************************
 ** Typedefs
 */

/******************************************************************************
 ** Globals
 */

/******************************************************************************
 ** Prototypes
 */

extern EVENT_ID_e   Check_Impedance     (void);
extern bool_t       Wait_For_Sync       (uint32_t timeout);

#endif /* THREAD_CORE_ENTRY_H_ */

/*
 ** $Log$
 **
 ** end of thread_core_entry.h
 ******************************************************************************/
