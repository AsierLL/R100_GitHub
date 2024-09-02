/******************************************************************************
 * Name      : R100_S3A7                                                      *
 * Copyright : Osatu S. Coop                                                  *
 * Compiler  : GNU ARM Embedded                                               *
 * Target    : Reanibex Serie 100                                             *
 ******************************************************************************/

/*!
 * @file        frcp.h
 * @brief       Header with functions related to the FRCP services
 *
 * @version     v1
 * @date        11/01/2020
 * @author      lsanz
 * @warning     author   date    version     .- Edition
 * @bug
 *
 */

#ifndef FRCP_H     // Entry, use file only if it's not already included.
#define FRCP_H

/******************************************************************************
**Includes
*/

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
 extern void     FRCP_Init           (uint32_t first_sample);
 extern void     FRCP_Process_Zpwin  (uint32_t *signal);
 extern uint16_t FRCP_Get_Frequency  (void);

#endif  /*FRCP_H*/
