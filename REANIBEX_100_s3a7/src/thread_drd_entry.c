/******************************************************************************
 * Name      : S3A7_REANIBEX_100                                              *
 * Copyright : Osatu S. Coop                                                  *
 * Compiler  : MinGW32                                                        *
 * Target    : Reanibex Series                                                *
 ******************************************************************************/

/*!
 * @file        thread_drd_entry.c
 * @brief
 *
 * @version     v1
 * @date        16/01/2018
 * @author      ltorres
 * @warning     author   date    version     .- Edition
 * @bug
 *
 */

/******************************************************************************
 ** Includes
 */

#include <device_init.h>
#include "types_basic.h"
#include "types_app.h"
#include "event_ids.h"

#include "Keypad.h"
#include "DB_Episode.h"
#include "thread_drd_entry.h"
#include "thread_core_entry.h"
#include "thread_audio_entry.h"

#include "thread_core.h"
#include "thread_defibrillator.h"
#include "thread_patMon_hal.h"
#include "thread_sysMon_entry.h"
#include "thread_comm_hal.h"
#include "thread_comm_entry.h"
#include "ipmk_removal.h"

/******************************************************************************
 ** Macros
 */

/******************************************************************************
 ** Defines
 */

#define ECG_WIN_SIZE_RAW    ((ECG_WIN_SIZE_DRD * 1000) / 150)       ///< ECG captured at 1 Ksps, processed at 150 sps
#define iPMK_FRONT_PORCH    8                                       ///< Front porch used in the iPMK removal (must be bigger than the shadow defined in the iPMK_removal function)

#define ECG_WIN_SIZE_EXT    (iPMK_FRONT_PORCH + ECG_WIN_SIZE_RAW)   ///< ECG buffer to store an extended raw (includes iPMK removed)


/******************************************************************************
 ** Typedefs
 */

typedef int16_t  ecg_win_raw_t [ECG_WIN_SIZE_EXT];              ///< ECG captured samples
typedef int16_t  ecg_win_drd_t [ECG_WIN_SIZE_DRD];              ///< ECG samples to be processed

/******************************************************************************
 ** Constants
 */

/******************************************************************************
 ** Externals
 */

/******************************************************************************
 ** Globals
 */

/******************************************************************************
 ** Locals
 */

static int16_t          ecg_raw [ECG_WIN_SIZE_EXT]; ///< Raw ecg, as captured

static bool_t           drd_init        = 0;        ///< Used for initializing the DRD
static bool_t           drd_running     = false;    ///< Flag to indicate that the DRD is executing the algorythm
static uint32_t         win_id          = 0;        ///< Window number or identifier
static uint32_t         firstSample_Wnd = 0;        ///< 1st sample in the analyzing window
static uint32_t         firstSample_Drd = 0;        ///< 1st sample in the diagnosed DRD

static DRD_DIAG_e       drd_diag;

/******************************************************************************
 ** Prototypes
 */
static bool_t       DRD_Analyze    (bool_t init, RHYTHM_e *pRhythm, bool_t adult);
static void         Downsample_ECG (int16_t *pSrc, int16_t *pDst);
static DRD_DIAG_e   Diagnose       (RHYTHM_e rPrev, RHYTHM_e rCurr);
static void         Execute_DRD    (void);

/******************************************************************************
** Name:    Send_DRD_Queue
*****************************************************************************/
/**
** @brief   Process events related to DRD
**
** @param   ev event identifier
**
******************************************************************************/
static void Send_DRD_Queue(EVENT_ID_e ev)
{
//    Trace_Arg (TRACE_NEWLINE, "  *****************QUEUE DRD AVAILABLE = %5d", (uint32_t) (queue_drd.tx_queue_available_storage));
    if (queue_drd.tx_queue_available_storage == 0) return;
    tx_queue_send(&queue_drd, &ev, TX_WAIT_FOREVER);

}

/******************************************************************************
** Name:    Downsample_ECG
*****************************************************************************/
/**
** @brief   downsamples from 1000Hz to 150Hz. As division is not exact
**          7,6,7,7,6,7... src samples are used to generate a dst sample.
**
** @param   pSrc [in] pointer to adquired ecg buffer
** @param   pDtn [in] pointer to downsampled ecg buffer
**
** @return  None
**
******************************************************************************/
static void Downsample_ECG (int16_t *pSrc, int16_t *pDst)
{
    int32_t  avg;

    // 6.6 samples at 1000Hz ---> 1 sample at 150Hz (1000/150) ---> Not exact division: 6.666
    // 6.66666... x 3 = 20 (first exact multiplication) samples in 3 frames -> 7 , 6 , 7 on each consecutive frame = 20
    // downsample ECG using window average
    for (uint32_t frame = 0; frame < (ECG_WIN_SIZE_DRD/3); frame++)
    {
        // from 7 to 1
        avg = pSrc[0] + pSrc[1] + pSrc[2] + pSrc[3] + pSrc[4] + pSrc[5] + pSrc[6];
        pDst[0] = (int16_t) (avg / 7);

        // from 6 to 1
        avg = pSrc[7] + pSrc[8] + pSrc[9] + pSrc[10] + pSrc[11] + pSrc[12];
        pDst[1] = (int16_t) (avg / 6);

        // from 7 to 1
        avg = pSrc[13] + pSrc[14] + pSrc[15] + pSrc[16] + pSrc[17] + pSrc[18] + pSrc[19];
        pDst[2] = (int16_t) (avg / 7);

        // update pointers
        pDst += 3;
        pSrc += 20;
    }
}

/******************************************************************************
** Name:    DRD_Analyze
*****************************************************************************/
/**
** @brief  gets the ecg, adapts it and performs the analysis
**
** @param  init    [in]  DRD filters are initialized before window processed
** @param  pRhythm [out] returns the rhythm
** @param  adult   [in]  flag to set the patient type (adult vs. pediatric)
**
** @return  TRUE if the window has been analyzed
**
******************************************************************************/
static bool_t DRD_Analyze (bool_t init, RHYTHM_e *pRhythm, bool_t adult)
{
    bool_t      ret;                        // operation result
    RHYTHM_e    my_rhythm;                  // identified rhythm
    static      ecg_win_drd_t   ecg_drd;    // Downsampled ECG, ready to be processed in the DRD

    Downsample_ECG (&ecg_raw[iPMK_FRONT_PORCH], ecg_drd);

    ret = DRD_Process_Ecg (ecg_drd, init, &my_rhythm, !adult);
    if (ret)
    {
        *pRhythm = my_rhythm;
    }


    if ((Get_Device_Info()->develop_mode == DEVELOP_TEST_2500_eSHOCK) ||     // endurance 2500 shocks
        (Get_Device_Info()->develop_mode == DEVELOP_TEST_500_iSHOCK) ||      // endurance 500 internal discharge
        (Get_Device_Info()->develop_mode == DEVELOP_FORCE_SHOCKABLE))        // always shockable rhythm
    {
        // force a shockable rhythm
        *pRhythm = eRHYTHM_VF_SHOCK;
    }

    return ret;
}

/******************************************************************************
** Name:    Diagnose
*****************************************************************************/
/**
** @brief   gives a diagnosis based on the last two rythm classification
**
** @param   rPrev [in] rhythm of previous classification
** @param   rCurr [in] rhythm of current  classification
**
** @return  next state
**
******************************************************************************/
static DRD_DIAG_e Diagnose (RHYTHM_e rPrev, RHYTHM_e rCurr)
{
    DRD_DIAG_e my_diag;             // diagnose result ...

    if ((rPrev & eRHYTHM_VF_SHOCK) && (rCurr & eRHYTHM_VF_SHOCK))
    {
        my_diag = eDRD_DIAG_SHOCK_ASYNC;
    }
    else if ((rPrev & (eRHYTHM_VT_SHOCK | eRHYTHM_VF_SHOCK)) &&
             (rCurr & (eRHYTHM_VT_SHOCK | eRHYTHM_VF_SHOCK)))
    {
        //syncronized shock will be removed from all devices
        my_diag = eDRD_DIAG_SHOCK_ASYNC; //eDRD_DIAG_SHOCK_SYNC
    }
    else if ((rPrev & (eRHYTHM_VT_NON_SHOCK | eRHYTHM_NORMAL | eRHYTHM_ASYSTOLE)) &&
             (rCurr & (eRHYTHM_VT_NON_SHOCK | eRHYTHM_NORMAL | eRHYTHM_ASYSTOLE)))
    {
        // my_diag = eDRD_DIAG_SHOCK_ASYNC;         // force asynchronous shock ...
        my_diag = eDRD_DIAG_NON_SHOCK;
    }
    else
    {
        my_diag = eDRD_DIAG_UNKNOWN;
    }

    return my_diag;
}

/******************************************************************************
 ** Name:    Execute_DRD
 *****************************************************************************/
/**
 ** @brief   Execute and monitor the DRD execution
 ** @param   void
 **
 ** @return  void
 ** @todo    pending things to be done
 ******************************************************************************/
static void Execute_DRD(void)
{
    static RHYTHM_e rhythm_curr;
    static RHYTHM_e rhythm_prev;

    // initialize the rhythm types for the previous and the current windows ...
    if (drd_init)
    {
        rhythm_curr = eRHYTHM_UNKNOWN;
        rhythm_prev = eRHYTHM_UNKNOWN;
    }

    // Check if a correct window is available (3.2s window = ECG WINDOW SIZE_RAW = 3200 sample)
    if (patMon_Get_ECG_Window(firstSample_Wnd, ECG_WIN_SIZE_RAW, &ecg_raw[iPMK_FRONT_PORCH]))
    {
        // filter the ECG window to remove the iPMK pulses !!!
        ipmk_removal (ecg_raw, iPMK_FRONT_PORCH, ECG_WIN_SIZE_EXT);
        memcpy (ecg_raw, &ecg_raw[ECG_WIN_SIZE_EXT - iPMK_FRONT_PORCH], sizeof (int16_t) * iPMK_FRONT_PORCH);

        // Check if rhythm is valid
        if (DRD_Analyze (drd_init, &rhythm_curr, (bool_t) pMisc_Settings->glo_patient_adult))
        {
            win_id++;
            firstSample_Wnd += ECG_WIN_SIZE_RAW;
            if (win_id >= 2)
            {
                drd_diag = Diagnose (rhythm_prev, rhythm_curr);

                // Update the DRD start point
                firstSample_Drd += ECG_WIN_SIZE_RAW;

                // Diagnostic is ready
                DRD_Diag_Ready ();
            }
            rhythm_prev = rhythm_curr;
        }
        drd_init = 0;
    }
}

/******************************************************************************
** Name:    DRD_Start_Analysis
*****************************************************************************/
/**
** @brief   Start DRD analysis algorithm
**
** @param   void
** @return  void
**
******************************************************************************/
void DRD_Start_Analysis()
{
    drd_diag = eDRD_DIAG_NOT_READY;                   // restart the diagnose result
    Send_DRD_Queue (eEV_DRD_START);
}

/******************************************************************************
** Name:    DRD_Stop_Analysis
*****************************************************************************/
/**
** @brief   Stop DRD Analysis
**
** @param   void
** @return  void
**
******************************************************************************/
void DRD_Stop_Analysis()
{
    Send_DRD_Queue (eEV_DRD_STOP);
}

/******************************************************************************
** Name:    DRD_Diag_Ready
*****************************************************************************/
/**
** @brief   DRD_Diag_Ready
**
** @param   void
** @return  void
**
******************************************************************************/
void DRD_Diag_Ready()
{
    Send_Core_Queue (eEV_DRD_DIAG_RDY);
}

/******************************************************************************
** Name:    DRD_Is_Running
*****************************************************************************/
/**
** @brief   Check if DRD is running
**
** @param   void
** @return  bool_t indicating if DRD is running
**
******************************************************************************/
bool_t DRD_Is_Running()
{
    return drd_running;
}

/******************************************************************************
** Name:    DRD_Get_Diagnosis
*****************************************************************************/
/**
** @brief   return the diagnosis
** @param   p_drd_data pointer to struct where to store data
**
** @return  the diagnosis
**
******************************************************************************/
DRD_DIAG_e DRD_Get_Diagnosis (DRD_RESULT_t *p_DRD_Result)
{
    // get the result and bye-bye
    if (p_DRD_Result)
    {
        (void) DRD_Get_Result (p_DRD_Result);
    }

    return drd_diag;
}

/******************************************************************************
** Name:    DRD_Get_Diag_nSample
*****************************************************************************/
/**
** @brief   return the 1st sample of the diagnosed window
** @param   none
**
** @return  1st sample of the diagnosed window
**
******************************************************************************/
uint32_t DRD_Get_Diag_nSample(void)
{
    return firstSample_Drd;
}

/******************************************************************************
 ** Name:    thread_drd_entry
 *****************************************************************************/
/**
 ** @brief   A description of the function
 ** @param   void
 **
 ** @return  void
 ** @todo    pending things to be done
 ******************************************************************************/
void thread_drd_entry(void)
{
    uint32_t    status;
    EVENT_ID_e  ev;

    // TODO [thread_drd_entry] Add required initializations for equipment

    while(1)
    {
        status = tx_queue_receive(&queue_drd, &ev, (OSTIME_3SEC + OSTIME_200MSEC + OSTIME_10MSEC)); // The window's time must be greater than 3,2seconds
        if ((status == TX_QUEUE_EMPTY) && drd_running)
        {
            Execute_DRD();
        }
        if ((status == TX_SUCCESS) && (ev == eEV_DRD_START) && !drd_running)
        {
            win_id = 0;                                     // restart the window identifier
            drd_running = true;                             // start the DRD
            drd_init = 1;                                   // initialize the analysis
            drd_diag = eDRD_DIAG_NOT_READY;                   // restart the diagnose result
            firstSample_Wnd = patMon_Get_ECG_nSample();     // initialize the first sample identifier
            firstSample_Drd = firstSample_Wnd;              // assign the DRD start point
        }
        if ((status == TX_SUCCESS) && (ev == eEV_DRD_STOP))
        {
            drd_running = false;                            // stop the DRD
            drd_diag = eDRD_DIAG_NOT_READY;                   // restart the diagnose result
        }
    }
}
