/******************************************************************************
 * Name      : S3A7_REANIBEX_100                                              *
 * Copyright : Osatu S. Coop                                                  *
 * Compiler  : MinGW32                                                        *
 * Target    : Reanibex Series                                                *
 ******************************************************************************/

/*!
 * @file        thread_patMon_entry.c
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
#include "Trace.h"
#include "types_app.h"
#include "types_basic.h"
#include "R100_Errors.h"
#include "HAL/thread_patMon_hal.h"
#include "thread_patMon.h"
#include "thread_patMon_entry.h"

#include "thread_audio_entry.h"
#include "thread_core.h"
#include "thread_core_entry.h"

#include "event_ids.h"

/******************************************************************************
 ** Macros
 */

/******************************************************************************
 ** Defines
 */

/******************************************************************************
 ** Typedefs
 */

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

/******************************************************************************
 ** Prototypes
 */



/******************************************************************************
** Name:    Check_Impedancec
*****************************************************************************/
/**
** @brief   check the impedance segment to convert to an Event (this event is
**          used in the AED state machine)
**
** @return  The impedance event ...
**
******************************************************************************/
EVENT_ID_e Check_Impedance (void)
{
    ZP_SEGMENT_e     zp_segment;            // Measured impedance value

    // Check ZP range
    zp_segment = patMon_Get_Zp_Segment();
    switch (zp_segment)
    {
       case eZP_SEGMENT_GOOD_CONN:  return eEV_PAT_GOOD_CONN;
       case eZP_SEGMENT_BAD_CONN:   return eEV_PAT_BAD_CONN;
       case eZP_SEGMENT_OPEN_CIRC:  return eEV_PAT_OPEN_CIRC;
       case eZP_SEGMENT_SHORT_CIRC: return eEV_PAT_SHORT_CIRC;
       default:                     return eEV_PAT_UNKNOWN;
    }
}

/******************************************************************************
** Name:    Wait_For_Sync
*****************************************************************************/
/**
** @brief   waits for a QRS-SYNC pulse. The ECG signal is downsampled to 500 Hz
**          the maximum considered heart-rate is 300 ppm = 1 QRS in 200 msecs
**          the analyzed windows are 50 samples wide (corresponding to 100 ms)
**          every 20 ms (10 samples). The detection must be made in Real Time
**          According to regulations, the delay from the QRS pulse to the shock
**          MUST be less than 60 msecs.
**
** @param   timeout     timeout for the wait process in system ticks
**
** @return  True if QRS pulse is detected ...
**
******************************************************************************/
bool_t  Wait_For_Sync (uint32_t timeout)
{
#define WIN_FOR_QRS_SIZE        100     // 100 samples or 100 msecs to analyze
#define WIN_FOR_QRS_INTERVAL    20      // 20 msecs of window accurate
#define WIN_FOR_QRS_TIME        40      // window's last 40 msecs to find a QRS pulse

    static int16_t my_win[WIN_FOR_QRS_SIZE];
    static int16_t my_drv[WIN_FOR_QRS_SIZE];
    uint32_t first_sample;                  // first sample in the ECG series
     int16_t sample, min, max, avg, vpp;    // window parameters
     int32_t i, add, maxD;                  // global use counters

    // get the time reference
    first_sample = patMon_Get_ECG_nSample() - WIN_FOR_QRS_SIZE;

    // loop trying to find the QRS pulse ...
    timeout += tx_time_get ();
    while (tx_time_get () < timeout)
    {
        // update the time reference
        first_sample += WIN_FOR_QRS_INTERVAL;

        // read the ECG window to process ...
        while (patMon_Get_ECG_Window(first_sample, WIN_FOR_QRS_SIZE, my_win) == FALSE) ;

        // pre-process the whole window
        min = max = my_win[0];
        add = (int32_t) my_win[0];
        for (i=1; i<WIN_FOR_QRS_SIZE; i++)
        {
            sample = my_win[i];
            if (sample > max) max = sample;
            if (sample < min) min = sample;
            add += sample;

            // calculate the 1st derivation
            my_drv[i] = (int16_t) (sample - my_win[i-1]);
        }
        avg = (int16_t) (add / WIN_FOR_QRS_SIZE);
        max = (int16_t) (max - avg);
        min = (int16_t) (avg - min);
        vpp = (max > min) ? max : min;

        // analyze the window's last 60 msecs. to check if contains a QRS pulse ...
        // the detector analyzes 20 msecs. windows in the 1st derivation array
        for (maxD=0, i=WIN_FOR_QRS_TIME; i<(WIN_FOR_QRS_TIME+20); i++)
        {
            maxD += abs(my_drv[i]);
        }

        for ( ; i<WIN_FOR_QRS_SIZE; i++)
        {
            maxD -= abs(my_drv[i-20]);
            maxD += abs(my_drv[i]);

            // final check of calculated parameters
            // maximum derivation is bigger than the 25% of the whole window peak to peak value (Vpp)
            // see "ADS_Init" function --> 0,2885 uV/bit
            // see "ECG_Offset_Removal" function --> 4,88 uV/bit
            // trigger set to --> 58 * 4,88 = 283 uV
            if ((vpp > 58) && (maxD > (vpp / 5)))
            {
                Trace (TRACE_NEWLINE,"QRS FOUND!!!");
                return TRUE;
            }
        }
    }
    return FALSE;
}

/******************************************************************************
 ** Name:    thread_patMon_entry
 *****************************************************************************/
/**
 ** @brief   A description of the function
 ** @param   void
 **
 ** @return  void
 ** @todo    pending things to be done
 ******************************************************************************/
void thread_patMon_entry(void)
{
    uint32_t    my_flags;       // flag to identify a new frame ready in the AFE

    //  patMon_HAL_Init (NOTCH_NO);
    patMon_HAL_Init ((NOTCH_t) pMisc_Settings->glo_filter);

    // Forever...
    while (1)
    {
        // Read ECG & ZP
        tx_event_flags_get (&g_events_PatMon, IRQ_ADS_DRDY, TX_OR_CLEAR, &my_flags, TX_WAIT_FOREVER);
        ADS_Read_Frame ();
    }
}
