/******************************************************************************
 * Name      : R100_S3A7                                                      *
 * Copyright : Osatu S. Coop                                                  *
 * Compiler  : GNU ARM Embedded                                               *
 * Target    : Reanibex Serie 100                                             *
 ******************************************************************************/

/*!
 * @file        ipmk_removal.h
 * @brief       Test File header file
 *
 * @version     v1
 * @date        18/02/2021
 * @author      Iaurrekoetxea
 * @warning     author   date    version     .- Edition
 * @bug
 *
 */


#ifndef IPMK_REMOVAL_H
#define IPMK_REMOVAL_H

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

#include "types_basic.h"

// type for the finite state machine for iPMK suppression
typedef enum
{
	st_searching = 0,
	st_onset_neg_detected,
	st_onset_pos_detected,
	st_hold_sample,
	st_refractary_period
} fsm_ipmk_st_t;

extern void ipmk_removal (int16_t* pSamples, uint32_t front_porch_size, uint32_t win_size);


#endif // IPMK_REMOVAL_H
