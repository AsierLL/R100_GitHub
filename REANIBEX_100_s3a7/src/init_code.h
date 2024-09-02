/******************************************************************************
 * Name      : R100_S3A7                                                      *
 * Copyright : Osatu S. Coop                                                  *
 * Compiler  : GNU ARM Embedded                                               *
 * Target    : Reanibex Serie 100                                             *
 ******************************************************************************/

/*!
 * @file        init_code.h
 * @brief       Header with functions related to the code initialization
 *
 * @version     v1
 * @date        16/01/2018
 * @author      ltorres
 * @warning     author   date    version     .- Edition
 * @bug
 *
 */

#ifndef INIT_CODE_H_
#define INIT_CODE_H_

/******************************************************************************
 **Includes
 */

#include "types_app.h"

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

extern void R_BSP_WarmStart (bsp_warm_start_event_t event);

#endif /* INIT_CODE_H_ */

/*
 ** $Log$
 **
 ** end of init_code.h
 ******************************************************************************/
