/******************************************************************************
 * Name      : R100_S3A7                                                      *
 * Copyright : Osatu S. Coop                                                  *
 * Compiler  : GNU ARM Embedded                                               *
 * Target    : Reanibex Serie 100                                             *
 ******************************************************************************/

/*!
 * @file        thread_api.h
 * @brief       Header with functions related to thread APIs
 *
 * @version     v1
 * @date        16/01/2018
 * @author      ltorres
 * @warning     author   date    version     .- Edition
 * @bug
 *
 */

#ifndef THREAD_API_H
#define THREAD_API_H
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

extern TX_THREAD thread_audio;          ///< Thread descriptor
extern TX_THREAD thread_core;           ///< Thread descriptor
extern TX_THREAD thread_drd;            ///< Thread descriptor
extern TX_THREAD thread_hmi;            ///< Thread descriptor
extern TX_THREAD thread_patMon;         ///< Thread descriptor
extern TX_THREAD thread_sysMon;         ///< Thread descriptor
extern TX_THREAD thread_defibrillator;  ///< Thread descriptor
extern TX_THREAD thread_recorder;       ///< Thread descriptor
extern TX_THREAD thread_comm;           ///< Thread descriptor
extern TX_THREAD thread_acc;            ///< Thread descriptor

#endif  /*THREAD_API_H*/

/*
** $Log$
**
** end of thread_api.h
******************************************************************************/
