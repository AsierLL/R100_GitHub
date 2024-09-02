/******************************************************************************
 * Name      : R100_S3A7                                                      *
 * Copyright : Osatu S. Coop                                                  *
 * Compiler  : GNU ARM Embedded                                               *
 * Target    : Reanibex Serie 100                                             *
 ******************************************************************************/

/*!
 * @file        thread_drd_entry.h
 * @brief       Header with functions related to the drd task
 *
 * @version     v1
 * @date        16/01/2018
 * @author      ltorres
 * @warning     author   date    version     .- Edition
 * @bug
 *
 */

#ifndef THREAD_DRD_ENTRY_H_
#define THREAD_DRD_ENTRY_H_

/******************************************************************************
 **Includes
 */
#include "drd.h"
#include "thread_drd.h"

/******************************************************************************
 ** Defines
 */
#define DRD_DIAG_FLAG ((ULONG)0x0001)

/******************************************************************************
 ** Typedefs
 */
typedef enum{
     eDRD_DIAG_NOT_READY = 0,       ///< Not ready
     eDRD_DIAG_UNKNOWN,             ///< Not known
     eDRD_DIAG_NON_SHOCK,           ///< Non shockable
     eDRD_DIAG_NON_SHOCK_ASYSTOLE,  ///< Asystole (no shockable)
     eDRD_DIAG_SHOCK_ASYNC,         ///< Shockable with asynchronous shock
     eDRD_DIAG_SHOCK_SYNC           ///< Shockable with synchronous shock
}DRD_DIAG_e;

typedef enum {
    eRYTHM_UNKNOWN = 0,             ///< Unknown rythm type
    eRYTHM_NON_SHOCKABLE,           ///< Non shockable rythm type
    eRYTHM_SHOCKABLE_SYNC,          ///< Shockable rythm type (synchronous)
    eRYTHM_SHOCKABLE_ASYNC,         ///< Shockable rythm type (asynchronous)
} RYTHM_TYPE_e;

/******************************************************************************
 ** Globals
 */

/******************************************************************************
 ** Prototypes
 */
extern  void        DRD_Start_Analysis  (void);
extern  void        DRD_Stop_Analysis   (void);
extern  void        DRD_Diag_Ready      (void);
extern  bool_t      DRD_Is_Running      (void);
extern  DRD_DIAG_e  DRD_Get_Diagnosis   (DRD_RESULT_t *p_DRD_Result);
extern  uint32_t    DRD_Get_Diag_nSample(void);

#endif /* THREAD_DRD_ENTRY_H_ */

/*
 ** $Log$
 **
 ** end of thread_drd_entry.h
 ******************************************************************************/
