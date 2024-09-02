/******************************************************************************
 * Name      : S3A7_REANIBEX_100                                              *
 * Copyright : Osatu S. Coop                                                  *
 * Compiler  : MinGW32                                                        *
 * Target    : Reanibex Series                                                *
 ******************************************************************************/

/*!
 * @file        thread_defibrillator_entry.h
 * @brief
 *
 * @version     v1
 * @date        06/06/2018
 * @author      ltorres
 * @warning     author   date    version     .- Edition
 * @bug
 *
 */

#ifndef THREAD_DEFIBRILLATOR_ENTRY_H_
#define THREAD_DEFIBRILLATOR_ENTRY_H_

/******************************************************************************
 **Includes
 */
#include "thread_drd_entry.h"

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
extern void Defib_Cmd_Precharge (bool_t force);
extern void Defib_Cmd_Disarm (void);
extern void Defib_Cmd_Charge (uint16_t energy);
extern void Defib_Cmd_Shock  (RYTHM_TYPE_e rythm_type);


#endif /* THREAD_DEFIBRILLATOR_ENTRY_H_ */
