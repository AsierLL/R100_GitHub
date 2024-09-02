/******************************************************************************
 * Name      : R100_S3A7                                                      *
 * Copyright : Osatu S. Coop                                                  *
 * Compiler  : GNU ARM Embedded                                               *
 * Target    : Reanibex Serie 100                                             *
 ******************************************************************************/

/*!
 * @file        ipmk_removal.c
 * @brief       All functions related to the iPMK pulses removal
 *
 * @version     v1
 * @date        18/02/2021
 * @author      Iaurrekoetxea
 * @warning     author   date    version     .- Edition
 * @bug
 *
 */

/******************************************************************************
 ** Includes
 */

#include <stdio.h>
#include <string.h>

#include "ipmk_removal.h"



/******************************************************************************
 * * Defines
 */

//////////////////////////////////
// iPMK SUPPRESSION DEFINITIONS
//////////////////////////////////
#define IPMK_REFRACTARY_PERIOD      6       // Period during which do not look for another pacemaker pulse
#define IPMK_PACEMAKER_PERIOD       6       // Maximum length of the peak to be considered a pacemaker pulse

#define IPMK_THRESHOLD_ONSET        14      // Threshold to detect onset  of iPMK. (adcs(40/ECG_FS)-adcs(0))
#define IPMK_THRESHOLD_OFFSET       6       // Threshold to detect offset of iPMK. (adcs(25/ECG_FS)-adcs(0))
#define IPMK_SHADOW_BEFORE          8       // Number of samples we suppress previous to the iPMK detection
#define IPMK_SHADOW_AFTER           20      // Number of samples we suppress after the iPMK detection


static uint32_t counterPMKPeriod  = IPMK_PACEMAKER_PERIOD;      // Peaks max duration to be detected as a iPMK pulse
static uint32_t counterHold       = IPMK_SHADOW_AFTER;          // Num of samples supressed after iPMK pulse detection
static uint32_t counterRefractary = IPMK_REFRACTARY_PERIOD;     // Num of samples where iPMK is not searched after iPMK is detected

static fsm_ipmk_st_t     fsm_ipmk_state = st_searching;         // Finite state machine

//////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////
// pSamples --> pointer to the signal buffer
// front_porch_size --> size of the front porch of the signal buffer
// size --> size of the signal window including the front porch

void ipmk_removal (int16_t* pSamples, uint32_t front_porch_size, uint32_t win_size)
{
    uint32_t cS, i;         // Indexes
    int16_t  s;             // ECG slope
    int16_t  val;           // Reference value to copy values where iPMK is detected
    int16_t  prev_sample;   // Stores last value, to compare it with the next value

    // be sure that the front porch has enought samples to process the shadow
    if (front_porch_size < IPMK_SHADOW_BEFORE) return;

    // Process each new sample
    prev_sample = pSamples[front_porch_size - 1];  // Windows last samples stored in prev_sample
    for (cS = front_porch_size; cS<win_size; cS++) // ECG analysis
    {
        s = (int16_t) (pSamples[cS] - prev_sample); // Slope calculated
        prev_sample = pSamples[cS];                 // Store prev_sample as the actual sample, to analyze it with the next sample
        switch(fsm_ipmk_state)
        {
            case st_searching: //State "searching"
                // update the state in function of the slope
                if (s < -IPMK_THRESHOLD_ONSET) { fsm_ipmk_state = st_onset_neg_detected; }
                if (s >  IPMK_THRESHOLD_ONSET) { fsm_ipmk_state = st_onset_pos_detected; }
                break;

            case st_onset_neg_detected  : //State "Negative onset detected"
                // If period in which iPMK could be detected has expired and iPMK has not be detected; back to state "Searching"
                if (counterPMKPeriod == 0)
                {
                    counterPMKPeriod = IPMK_PACEMAKER_PERIOD;
                    fsm_ipmk_state = st_searching;
                }
                else
                {
                    counterPMKPeriod--;

                    // If offset is detected (s > offset threshold)--> iPMK pulse has been detected
                    if (s > IPMK_THRESHOLD_OFFSET)
                    {
                        val = pSamples[cS-IPMK_SHADOW_BEFORE];          //val reference value of the sample previous to iPMK.
                        for (i=cS-IPMK_SHADOW_BEFORE+1; i<=cS; i++)     //val overwritten where iPMK has been detected
                        {
                            pSamples[i] = val;
                        }
                        counterPMKPeriod = IPMK_PACEMAKER_PERIOD;       // State changed to "Hold sample"
                        fsm_ipmk_state = st_hold_sample;
                    }
                }
                break;

            case st_onset_pos_detected  : //If positive onset has been detected
                if (counterPMKPeriod == 0) //if counterPMKPeriod == 0, onset pos period has finished and iPMK has not been detected; back to state "Searching".
                {
                    counterPMKPeriod = IPMK_PACEMAKER_PERIOD;
                    fsm_ipmk_state = st_searching;
                }
                else
                {
                    //Decrease counter
                    counterPMKPeriod--;

                    // If offset detected, (s < negative of offset threshold)-->iPMK pulse detected
                    if (s < -IPMK_THRESHOLD_OFFSET)
                    {
                        val = pSamples[cS-IPMK_SHADOW_BEFORE]; //val reference value of the sample previous to iPMK.
                        for (i=cS-IPMK_SHADOW_BEFORE+1; i<=cS; i++)
                        {
                            pSamples[i] = val; //val overwritten where iPMK has been detected
                        }

                        // State changed to "Hold sample"
                        counterPMKPeriod = IPMK_PACEMAKER_PERIOD;
                        fsm_ipmk_state = st_hold_sample;
                    }
                }
                break;

            case st_hold_sample  : //After iPMK has been detected, processing of the next samples
                if (counterHold ==0)
                {
                    fsm_ipmk_state = st_refractary_period;          // Refractary state where another iPMK pulse cannot be detected
                    counterHold = IPMK_SHADOW_AFTER;                // Reset counter
                }
                else
                {
                    // val reference value overwritten in the samples after iPMK detection.
                    counterHold--;
                    pSamples[cS] = pSamples[cS -1];
                }
                break;

            case st_refractary_period    : //"Refractary state" where another iPMK pulse cannot be detected
                if (counterRefractary == 0)
                {
                    fsm_ipmk_state = st_searching;                  // Go back to searching
                    counterRefractary = IPMK_REFRACTARY_PERIOD;     // Reset counter
                }
                else //Still in refractary period
                {
                    counterRefractary--;
                }
                break;
            default :
                break;
        }
    }
}
