/******************************************************************************
 * Name      : R100_S3A7                                                      *
 * Copyright : Osatu S. Coop                                                  *
 * Compiler  : GNU ARM Embedded                                               *
 * Target    : Reanibex Serie 100                                             *
 ******************************************************************************/

/*!
 * @file        Trace.h
 * @brief       Header with functions related to the output trace through serial
 *              port
 *
 * @version     v1
 * @date        16/01/2018
 * @author      ltorres
 * @warning     author   date    version     .- Edition
 * @bug
 *
 */

#ifndef TRACE_H_
#define TRACE_H_

/******************************************************************************
 **Includes
 */

#include "types_basic.h"

/******************************************************************************
 ** Defines
 */

// FLAGS
#define TRACE_NO_FLAGS              0x00
#define TRACE_TIME_STAMP            0x01
#define TRACE_NEWLINE               0x02
#define TRACE_DISABLED              0x04

/******************************************************************************
 ** Typedefs
 */

/******************************************************************************
 ** Globals
 */

/******************************************************************************
 ** Prototypes
 */

extern void     Trace_Init (void);
extern void     Trace      (uint8_t flags, const char *pString);
extern void     Trace_Arg  (uint8_t flags, const char *pString, const uint32_t aux_code);
extern void     Trace_Char (const char_t data_to_send);
extern uint8_t  Boot_Send_Message (const uint8_t *pMsg, uint32_t size);
extern uint8_t  Boot_Receive_Message (void);
extern void     Boot_Sync (uint8_t nBeacons);

#ifdef UNIT_TESTS
void Trace_Test (const char_t data_to_send);
#endif

#endif /* TRACE_H_ */

/*
 ** $Log$
 **
 ** end of Comm_Trace.h
 ******************************************************************************/
