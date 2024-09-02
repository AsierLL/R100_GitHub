/******************************************************************************
 * Name      : R100_S3A7                                                      *
 * Copyright : Osatu S. Coop                                                  *
 * Compiler  : GNU ARM Embedded                                               *
 * Target    : Reanibex Serie 100                                             *
 ******************************************************************************/

/*!
 * @file        FSM_R100_AED.c
 * @brief       Header with functions related to the FSM service
 *              Automatic generated C-code by the application: HFSM Editor v1.4r3
 *              Methodology: Hierarchical Switch-case state. [v1.5r1]
 *              Exported function: void FSM_R100_AED(EVENT_ID_e aed_event);
 *
 * @version     v1
 * @date        23/05/2018
 * @author      ilazkanoiturburu
 * @warning     author   date    version     .- Edition
 * @bug
 *
 */

/******************************************************************************
 ** Includes
*/
#include "types_basic.h"
#include "types_app.h"
#include "event_ids.h"

#include "device_init.h"

#include "frcp.h"
#include "drd.h"
#include "thread_drd.h"
#include "thread_drd_entry.h"
#include "thread_audio_entry.h"
#include "thread_comm_entry.h"
#include "thread_core_entry.h"
#include "thread_defibrillator_entry.h"
#include "DB_Episode.h"
#include "thread_recorder.h"
#include "thread_defibrillator.h"
#include "thread_patMon_hal.h"
#include "thread_patMon_entry.h"
#include "thread_comm_hal.h"
#include "sysMon_RF_Comms.h"

#include "Trace.h"
#include "Keypad.h"

/******************************************************************************
 ** Macros
*/

#define T_bit   bool_t
#define T_u8    uint8_t


/******************************************************************************
 ** Defines
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

static uint32_t         zp;                         ///< ZP value in ohms
static bool_t           zp_good;                    ///< ZP good flag
static ZP_SEGMENT_e     zp_segment;                 ///< identifies the ZP segment
static uint16_t         FSM_patient_adult;          ///< local flag to identify the patient type

static uint32_t         audio_parrot_counter;       ///< counter to parrot some messages

static bool_t           first_connection = TRUE;    ///< identifies the first connection of the patient
static uint32_t         metronome_cnt_max;          ///< metronome max count
static EL_SIGN_e        electrodes_signature;       ///< electrodes signature

static RCP_BREAK_t      break_condition;            ///< condition to break an audio sequence


/******************************************************************************
 ** Locals
*/


// [Child:  FSM_Zp_Good]

/******************************************************************************
 ** Includes
    */

// already included in the main FSM
//#include "thread_drd_entry.h"         // DRD entry
//#include "thread_core_entry.h"        // CORE entry

#include "thread_defibrillator_hal.h"   // Defibrillator controller
#include "thread_patMon_hal.h"          // Patient monitor controller

/******************************************************************************
 ** Macros
    */



/******************************************************************************
 ** Defines
*/

#define RCP_WINSIZE     200                         ///< Defines the size of a ZP window used in the RCP

/******************************************************************************
 ** Constants
    */

/******************************************************************************
 ** Externals
    */

/******************************************************************************
 ** Globals
    */
uint16_t                FSM_RCP_time_secs;          ///< Current RCP time in seconds
uint16_t                FSM_shock_energy;           ///< Energy to apply in the shock
bool_t                  RCP_running;

/******************************************************************************
 ** Locals
    */

static DRD_DIAG_e       drd_diagosis;               ///< DRD diagnosis result

static uint32_t         RCP_number;                 ///< Number of RCP in course
static bool_t           RCP_timeout;                ///< Flag to indicate that the RCP has finished
static bool_t           RCP_new;                    ///< Flag to indicate that a new RCP must be run
static uint32_t         RCP_timeout_counter;        ///< Counter for RCP timeout
static uint16_t         RCP_rate_bpm;               ///< Rate to apply during the RCP
static uint16_t         RCP_rate_bpm_feedback;      ///< Compression frequency during RCP

static uint32_t         RCP_zp_adcs;                ///< ZP value used to initialize the FRCP filters

static uint32_t         RCP_zp_idx;
static uint32_t         RCP_zp_signal [RCP_WINSIZE];

static uint8_t          nCompressions;              ///< Number of compressions to apply during the CPR
static uint8_t          nCompressions_counter;      ///< Count the number of compressions during the CPR

static RYTHM_TYPE_e     rythm_type;                 ///< Rythm type identifier

static uint32_t         asystole_timeout_counter;   ///< Counter for asystole detection

static uint32_t         unknown_timeout_counter;    ///< Counter for unknown rithms

static uint32_t         autoshock_timeout_counter;  ///< Counter for automated shock

static uint32_t         shock_timeout_counter;      ///< Counter for shock timeout
static bool_t           shock_timeout;              ///< Flag for shock timeout
static bool_t           shock_done;                 ///< Flag to identify shock done event

static uint16_t         nConsecutive_shocks;        ///< Count the number of consecutive shocks to force an RCP
static uint16_t         nShocks;                    ///< Count the number of current consecutive shocks
static uint16_t         nShocks_same_patient;       ///< Count the number of shocks over the same patient

static REG_EVENTS_e     event_id;                   ///< Event identifier to register

static DEFIB_STATE_e    aedef_state;                ///< Defibrillator state
static bool_t           aedef_in_error;             ///< Flag to indicate that the defibrillator is in error
static bool_t           vc_charged;                 ///< Flag to determine if the capacitor is charged or not

static uint32_t         endurance_number_items = 0; ///< Counter used in the endurance test

static REG_EVENTS_e     reg_event;
static AUDIO_ID_e       prev_audio_msg = eAUDIO_MAX_ID;
static AUDIO_ID_e       audio_msg = eAUDIO_MAX_ID;

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// Initialization proposed by the user (End).
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// Entry flags (Start).
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
typedef struct {
    T_bit flg            :1;
    T_bit flg_1          :1;
} T_EntryFsmFlg;
static T_EntryFsmFlg entryFsm;
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// Entry flags (End).
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// Automatic initialization (Start).
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// States index.
#define  i_FSM_ZP_GOOD              ((T_u8)0)
#define  i_FSM_INSTRUCT_USER        ((T_u8)1)
#define  i_AED_MODE                 ((T_u8)2)
#define  i_FSM_CHANGE_PATIENT_TYPE  ((T_u8)3)

// Index variable.
static T_u8 ind_FSM_R100_AED = i_AED_MODE;

// Macros & functions.
// Force the FSM to the initial state.
#define FSM_R100_AED_Init() { ind_FSM_R100_AED = i_AED_MODE; entryFsm.flg = 0U; }

// This is not a history FSM.
#define _FSM_R100_AED_Init()  {FSM_R100_AED_Init(); /* Children: */ _FSM_Zp_Good_Init();}


// Definition of Exit_functions.
static void FSM_Zp_Good_OnExit(void);


// [Child:  FSM_Zp_Good]

// States index.
#define  i_1_FSM_ANALYZING      ((T_u8)0)
#define  i_1_FSM_RCP            ((T_u8)1)
#define  i_1_FSM_PREPARE_SHOCK  ((T_u8)2)
#define  i_1_FSM_DEMAND_SHOCK   ((T_u8)3)
#define  i_1_LOCK_ON_ERROR      ((T_u8)4)

// Index variable.
static T_u8 ind_1_FSM_Zp_Good = i_1_FSM_RCP;

// Macros & functions.
// Force the FSM to the initial state.
#define FSM_Zp_Good_Init() { ind_1_FSM_Zp_Good = i_1_FSM_RCP; entryFsm.flg_1 = 0U; }

// This is not a history FSM.
#define _FSM_Zp_Good_Init()  {FSM_Zp_Good_Init(); /* Children: */ }

// Definition of Exit_functions.
static void FSM_Analyzing_OnExit(void);
static void FSM_Demand_Shock_OnExit(void);

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// Automatic initialization (End).
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// Local child functions  (Start).
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
static void FSM_Zp_Good(EVENT_ID_e aed_event);
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// Local child functions  (End).
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// FSM dispatcher function to be called by the O.S.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
void FSM_R100_AED(EVENT_ID_e aed_event)
{
    // Next state analyser.
    switch(ind_FSM_R100_AED)
    {
        case i_FSM_ZP_GOOD:
            // Entry check
            if(!((T_bit)entryFsm.flg))
            {
                entryFsm.flg = (T_bit)1;
                // Entry actions
                if (first_connection)
                {
                    first_connection = FALSE;

                    // register used electrodes event
                    Electrodes_Register_Event();

                    // assign the initial CPR times in function of the patient type
                    if (pMisc_Settings->glo_patient_adult)
                    {
                        FSM_RCP_time_secs = pAed_Settings->aed_aCPR_init;
                        metronome_cnt_max = pAed_Settings->aed_metronome_ratio_a;
                    }
                    else {
                        FSM_RCP_time_secs = pAed_Settings->aed_pCPR_init;
                        metronome_cnt_max = pAed_Settings->aed_metronome_ratio_p;
                    }
                }

                // record patient good connection event
                DB_Episode_Set_Event (eREG_ECG_PAT_GOOD_CONN);

                // command the defibrillator to precharge the main capacitor !!!
                // Force the charge to the PRE_CHARGE_VOLTAGE value
                Defib_Cmd_Precharge (FALSE);
                
                // Pointing to the next state.
                if (ind_1_FSM_Zp_Good != i_1_FSM_RCP)
                {
                    ind_1_FSM_Zp_Good = i_1_FSM_ANALYZING;
                    // Reset entry flag
                    entryFsm.flg_1 = 0U;
                }
                else
                {
                    // Stop prevoius audio
                    Audio_Message (eAUDIO_CMD_STOP, eAUDIO_ASYSTOLE, TRUE);
                }

            }

            // State actions (Moore).
            FSM_Zp_Good (aed_event);
            // Next state selection.
            if((zp_good == FALSE))// [1].
            {
                // If electrodes disconnected while demanding shock
                if(ind_1_FSM_Zp_Good == i_1_FSM_DEMAND_SHOCK)
                {
                    // In DEMO mode do not send anything related to alerts
                    if ((!Is_Battery_Mode_Demo()) && (Is_Sigfox_TX_Enabled() == SIGFOX_ONLY || Is_Sigfox_TX_Enabled() == SIGFOX_PRIOR) && (Get_Device_Settings()->misc.glo_patient_alert == TRUE))
                    {
                        Send_Sigfox_Alert(MSG_ID_SHOCK_DISC_ALERT);   // Generate and Send Alert using Sigfox
                    }
                    if((!Is_Battery_Mode_Demo()) && (Is_Wifi_TX_Enabled() == WIFI_ONLY || Is_Wifi_TX_Enabled() == WIFI_PRIOR) && (Get_Device_Settings()->misc.glo_patient_alert == TRUE)) //WIFI_WITH_PATIENT
                    {
                        Send_Wifi_Alert(WIFI_MSG_ID_SHOCK_DISC_ALERT);    // Generate and Send Alert report using Wifi
                    }
                }
                // Child Initialization.
                _FSM_Zp_Good_Init();
                // Exit actions.
                FSM_Zp_Good_OnExit();
                // Pointing to the next state.
                ind_FSM_R100_AED = i_FSM_INSTRUCT_USER;
                // Reset entry flag
                entryFsm.flg = 0U;
            }
            else if((Is_Patient_Type_Change() == TRUE))// [2].
            {
                // Child Initialization.
                _FSM_Zp_Good_Init();
                // Exit actions.
                FSM_Zp_Good_OnExit();
                // Pointing to the next state.
                ind_FSM_R100_AED = i_FSM_CHANGE_PATIENT_TYPE;
                // Reset entry flag
                entryFsm.flg = 0U;
            }
        break;
        case i_FSM_INSTRUCT_USER:
            // Entry check
            if(!((T_bit)entryFsm.flg))
            {
                entryFsm.flg = (T_bit)1;
                // Entry actions
                // initialize the parrot counter
                audio_parrot_counter = tx_time_get();
                
                // sleep a little bit time to be sure that the electrodes presence flag has been read (in the System monitor)
                tx_thread_sleep (OSTIME_400MSEC);
                tx_thread_sleep (OSTIME_400MSEC);
            }

            // State actions (Moore).
            // update the ZP value and segment ...
            zp = patMon_Get_Zp_Ohms();
            zp_segment = patMon_Get_Zp_Segment ();

            // check if the Zp value has a good connection
            zp_good = (zp_segment == eZP_SEGMENT_GOOD_CONN);

            // Next state selection.
            if((zp_good == TRUE))
            {
                // Child Initialization.
                _FSM_Zp_Good_Init();
                // Exit actions.
                // check the presence of valid electrodes
                electrodes_signature = Electrodes_Get_Signature();

                if (electrodes_signature == eEL_SIGN_BEXEN)
                {
                    // record electrodes connected event
                    DB_Episode_Set_Event (eREG_ECG_EL_BEXEN);
                }
                if (electrodes_signature == eEL_SIGN_BEXEN_EXPIRED)
                {
                    // record electrodes connected event
                    DB_Episode_Set_Event (eREG_ECG_EL_EXPIRED);
                }
                else if (electrodes_signature == eEL_SIGN_UNKNOWN)
                {
                    // record electrodes unknown
                    DB_Episode_Set_Event (eREG_ECG_EL_UNKNOWN);
                }
                // Pointing to the next state.
                ind_FSM_R100_AED = i_FSM_ZP_GOOD;
                // Reset entry flag
                entryFsm.flg = 0U;
            }
            else
            { // Else actions.
                if (Is_Patient_Type_Change())
                {
                    do
                    {
                        if (pMisc_Settings->glo_patient_adult)
                        {
                            Audio_Message (eAUDIO_CMD_PLAY, eAUDIO_ADULT_MODE, TRUE);
                        }
                        else {
                            Audio_Message (eAUDIO_CMD_PLAY, eAUDIO_PEDIATRIC_MODE, TRUE);
                        }
                        break_condition = Pastime_While_Audio_Playing_with_Break_PT(OSTIME_NO_DELAY);
                    } while (break_condition == RCP_BREAK_PATIENT_TYPE);
                    
                    ////////////////////////////////////////////////////////////////////    
                    // consider a new patient and initialize some internal variables
                    ////////////////////////////////////////////////////////////////////    
                    // force to first connection to assign new CPR_init times
                    first_connection = TRUE;

                    // allow a new RCP
                    RCP_new = TRUE;

                    // restart the number of shocks over the same patient
                    nShocks_same_patient = 0;
                    // Restart the number of consecutive shocks when RCP
                    nShocks = 0;
                }

                // Check if audio message has changed
                electrodes_signature = Electrodes_Get_Signature();  // check electrodes again

                // assume that zp is not GOOD
                if (electrodes_signature == eEL_SIGN_MUST_DEMO)
                {
                    // record electrodes disconnected event
                    reg_event = eREG_ECG_EL_NONE;

                    // parrot the audio message demanding to connect electrodes to the device
                    audio_msg = eAUDIO_REPLACE_ELECTRODES;
                }
                else
                {
                    if (electrodes_signature == eEL_SIGN_NONE)
                    {
                        // record electrodes disconnected event
                        reg_event = eREG_ECG_EL_NONE;
                        
                        // parrot the audio message demanding to connect electrodes to the device
                        audio_msg = eAUDIO_CONNECT_ELECTRODES_DEVICE;
                    }
                    else 
                    {
                        if (zp_segment == eZP_SEGMENT_OPEN_CIRC)
                        {
                            // record patient disconnected event
                            reg_event = eREG_ECG_PAT_OPEN_CIRC;
                    
                            // play the audio message demanding to connect electrodes to the patient
                            audio_msg = eAUDIO_APPLY_ELECTRODES_CHEST;
                        }
                        else if (zp_segment == eZP_SEGMENT_BAD_CONN)
                        {
                            // record patient bad connection event
                            reg_event = eREG_ECG_PAT_BAD_CONN;
                    
                            // play the audio message demanding to press electrodes
                            audio_msg = eAUDIO_PRESS_ELECTRODES;
                        }
                        else if (zp_segment == eZP_SEGMENT_SHORT_CIRC)
                        {
                            // record patient short circuit event
                            reg_event = eREG_ECG_PAT_SHORT_CIRC;
                    
                            // play the audio message demanding to check electrodes
                            audio_msg = eAUDIO_CHECK_ELECTRODES;
                        }
                        else
                        {
                            // zp_good is TRUE --> execute the FSM_Zp_Good
                            audio_msg = eAUDIO_MAX_ID;
                        }
                    }
                }

                // If new audio must be played or replay last one because time has expired...
                if ((prev_audio_msg != audio_msg) || (tx_time_get() >= audio_parrot_counter))
                {
                    // initialize the parrot counter
                    audio_parrot_counter = tx_time_get() + OSTIME_10SEC;
                    Audio_Message (eAUDIO_CMD_PLAY, audio_msg, TRUE);
                    if (electrodes_signature == eEL_SIGN_MUST_DEMO && (Is_Battery_Mode_Demo()))
                    {
                        Audio_Message (eAUDIO_CMD_CONCAT, eAUDIO_DEMO_MODE, TRUE);
                    }
                    prev_audio_msg = audio_msg;
                    DB_Episode_Set_Event (reg_event);
                }
            }
        break;
        case i_AED_MODE:
            // Entry check
            if(!((T_bit)entryFsm.flg))
            {
                entryFsm.flg = (T_bit)1;
                // Entry actions
                if (pMisc_Settings->glo_patient_adult)
                {
                    Audio_Message (eAUDIO_CMD_CONCAT, eAUDIO_ADULT_MODE, TRUE);
                }
                else
                {
                    Audio_Message (eAUDIO_CMD_CONCAT, eAUDIO_PEDIATRIC_MODE, TRUE);
                }
                Pastime_While_Audio_Playing();
                
                // play the initial audio messages with the possibility to break the sequence
                Audio_Message (eAUDIO_CMD_CONCAT, eAUDIO_KEEP_CALM, TRUE);
                Audio_Message (eAUDIO_CMD_CONCAT, eAUDIO_FOLLOW_INSTRUCTIONS, TRUE);
                Audio_Message (eAUDIO_CMD_CONCAT, eAUDIO_ASK_FOR_HELP, TRUE);
                break_condition = Pastime_While_Audio_Playing_with_Break_PT(OSTIME_NO_DELAY);
                if (break_condition == RCP_BREAK_PATIENT_TYPE)
                {
                    if (pMisc_Settings->glo_patient_adult)
                    {
                        Audio_Message (eAUDIO_CMD_PLAY, eAUDIO_ADULT_MODE, TRUE);
                    }
                    else
                    {
                        Audio_Message (eAUDIO_CMD_PLAY, eAUDIO_PEDIATRIC_MODE, TRUE);
                    }
                    Pastime_While_Audio_Playing ();
                }

                event_id = pMisc_Settings->glo_patient_adult ? eREG_SET_PATIENT_ADULT : eREG_SET_PATIENT_PEDIATRIC;
                DB_Episode_Set_Event (event_id);
                
                // check the presence of valid electrodes and the presence of a patient
                electrodes_signature = Electrodes_Get_Signature();
                if ((patMon_Get_Zp_Segment () != eZP_SEGMENT_GOOD_CONN) && ((electrodes_signature == eEL_SIGN_BEXEN) || (electrodes_signature == eEL_SIGN_UNKNOWN)))
                {
                    Audio_Message (eAUDIO_CMD_CONCAT, eAUDIO_REMOVE_CHEST_CLOTHES, TRUE);
                    Audio_Message (eAUDIO_CMD_CONCAT, eAUDIO_OPEN_ELECTRODE_PACKAGE, TRUE);
                
                    break_condition = Pastime_While_Audio_Playing_with_Break_PT_CC();
                    if (break_condition == RCP_BREAK_PATIENT_TYPE)
                    {
                        if (pMisc_Settings->glo_patient_adult)
                        {
                            Audio_Message (eAUDIO_CMD_PLAY, eAUDIO_ADULT_MODE, TRUE);
                        }
                        else
                        {
                            Audio_Message (eAUDIO_CMD_PLAY, eAUDIO_PEDIATRIC_MODE, TRUE);
                        }
                        Pastime_While_Audio_Playing ();
                    }
                }
            }
            // Next state selection.
            {
                // Pointing to the next state.
                ind_FSM_R100_AED = i_FSM_INSTRUCT_USER;
                // Reset entry flag
                entryFsm.flg = 0U;
            }
        break;
        case i_FSM_CHANGE_PATIENT_TYPE:
            // Entry check
            if(!((T_bit)entryFsm.flg))
            {
                entryFsm.flg = (T_bit)1;
                // Entry actions
                do {
                    // memorize the patient type ...
                    FSM_patient_adult = pMisc_Settings->glo_patient_adult;

                    // advice that the patient has been changed !!!
                    if (FSM_patient_adult)
                    {
                        Audio_Message (eAUDIO_CMD_PLAY, eAUDIO_ADULT_MODE, TRUE);
                    }
                    else {
                        Audio_Message (eAUDIO_CMD_PLAY, eAUDIO_PEDIATRIC_MODE, TRUE);
                    }
                    Pastime_While_Audio_Playing_with_Break_PT_OC(OSTIME_NO_DELAY);
                    entryFsm.flg = 0U; // IÑIGO
                
                } while (FSM_patient_adult != pMisc_Settings->glo_patient_adult);
                
                // force to first connection to assign new CPR_init times
                first_connection = TRUE;
                    
                // allow a new RCP
                RCP_new = TRUE;
                
                // restart the number of shocks over the same patient
                nShocks_same_patient = 0;
                
                // Restart the number of consecutive shocks when RCP
                nShocks = 0;

            }
            // Next state selection.
            if((Is_Patient_Type_Change() == FALSE))
            {
                // Pointing to the next state.
                ind_FSM_R100_AED = i_FSM_ZP_GOOD;
                // Reset entry flag
                entryFsm.flg = 0U;
            }
        break;
    }
}


// Implementation of Exit_functions.
static void FSM_Zp_Good_OnExit(void)
{
    // just in case, stops the analysis
    DRD_Stop_Analysis();

    // Stop Shock LEDs blink if it was in shock demand state
    Led_Off (LED_SHOCK);

    // command the defibrillator to precharge the main capacitor !!!
    // Force the charge to the PRE_CHARGE_VOLTAGE value
    Defib_Cmd_Precharge (FALSE);
}


// [Child:  FSM_Zp_Good]

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// FSM dispatcher function to be called by the O.S.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
static void FSM_Zp_Good(EVENT_ID_e aed_event)
{
    // User actions to be executed before analyse the state.
    // initialize the number of consecutive shocks before force a RCP
    nConsecutive_shocks = pAed_Settings->aed_consec_shocks;

    // update the ZP value and segment ...
    zp = patMon_Get_Zp_Ohms();
    zp_segment = patMon_Get_Zp_Segment ();
    
    // check if the Zp value has a good connection
    zp_good = (zp_segment == eZP_SEGMENT_GOOD_CONN);

    // Next state analyser.
    switch(ind_1_FSM_Zp_Good)
    {
        case i_1_FSM_ANALYZING:
            // Entry check
            if(!((T_bit)entryFsm.flg_1))
            {
                entryFsm.flg_1 = (T_bit)1;
                
                Audio_Message (eAUDIO_CMD_CONCAT, eAUDIO_DO_NOT_TOUCH_PATIENT, TRUE);
                tx_thread_sleep(OSTIME_800MSEC);

                // Entry actions
                // starts the analysis
                DRD_Start_Analysis();
                rythm_type = eRYTHM_UNKNOWN;
                
                // register the analisis start
                DB_Episode_Set_Event (eREG_DRD_START);
                
                // play the audio message to do not touch the patient ...
                //Audio_Message (eAUDIO_CMD_CONCAT, eAUDIO_DO_NOT_TOUCH_PATIENT, TRUE);
                Audio_Message (eAUDIO_CMD_CONCAT, eAUDIO_ANALYZING_HEART_RATE, TRUE);
                
                // wait to finish playing audio messages (allow process of patient impedance) ...
                if (Pastime_While_Audio_Playing_with_Break_PT_OC(OSTIME_NO_DELAY) == RCP_BREAK_PATIENT_TYPE)
                {
                    // restart the number of shocks over the same patient
                    nShocks_same_patient = 0;
                    // Restart the number of consecutive shocks when RCP
                    nShocks = 0;
                
                    // and set as a new RCP ...
                    RCP_new = TRUE;
                
                    // Child Initialization.
                    _FSM_Zp_Good_Init();
                    // Exit actions.
                    FSM_Zp_Good_OnExit();
                    // Pointing to the next state.
                    ind_FSM_R100_AED = i_FSM_CHANGE_PATIENT_TYPE;
                    // Reset entry flag
                    entryFsm.flg = 0U;
                }
            }
            // State actions (Moore).
            if (aed_event == eEV_DRD_DIAG_RDY)
            {
                // State actions (Moore).
                drd_diagosis = DRD_Get_Diagnosis(NULL);
                
                Trace_Arg (TRACE_NEWLINE, " drd_diagosis: = %4d", drd_diagosis);
                // Proceed to update the process flags ...
                switch (drd_diagosis)
                {
                    case eDRD_DIAG_NON_SHOCK:
                        rythm_type = eRYTHM_NON_SHOCKABLE;
                        asystole_timeout_counter = 0;
                        unknown_timeout_counter = 0;

                        // In DEMO mode do not send anything related to alerts
                        if ((!Is_Battery_Mode_Demo()) && (Is_Sigfox_TX_Enabled() == SIGFOX_ONLY || Is_Sigfox_TX_Enabled() == SIGFOX_PRIOR) && (Get_Device_Settings()->misc.glo_patient_alert == TRUE))
                        {
                            Send_Sigfox_Alert(MSG_ID_NO_SHOCK_ALERT);   // Generate and Send Alert using Sigfox
                            Send_Sigfox_Alert(MSG_ID_NO_SHOCK_ALERT);   // Repeat send to ensure it arrives.
                        }
                        if((!Is_Battery_Mode_Demo()) && (Is_Wifi_TX_Enabled() == WIFI_ONLY || Is_Wifi_TX_Enabled() == WIFI_PRIOR) && (Get_Device_Settings()->misc.glo_patient_alert == TRUE)) //WIFI_WITH_PATIENT
                        {
                            Send_Wifi_Alert(WIFI_MSG_ID_NO_SHOCK_ALERT);    // Generate and Send Alert report using Wifi
                        }
                        break;
                    case eDRD_DIAG_NON_SHOCK_ASYSTOLE:
                        rythm_type = eRYTHM_NON_SHOCKABLE;
                        unknown_timeout_counter = 0;
                        if (pAed_Settings->aed_asys_detect && (asystole_timeout_counter == 0))
                        {
                            // assign the asystole timeout ...
                            asystole_timeout_counter = (uint32_t) tx_time_get() + ((uint32_t) pAed_Settings->aed_asys_time * 60 * OSTIME_1SEC);
                        }
                        break;
                    case eDRD_DIAG_SHOCK_SYNC:
                        rythm_type = eRYTHM_SHOCKABLE_SYNC;
                        asystole_timeout_counter = 0;
                        unknown_timeout_counter = 0;
                        break;
                    case eDRD_DIAG_SHOCK_ASYNC:
                        rythm_type = eRYTHM_SHOCKABLE_ASYNC;
                        asystole_timeout_counter = 0;
                        unknown_timeout_counter = 0;
                        break;
                    case eDRD_DIAG_UNKNOWN:

                        drd_diagosis = eDRD_DIAG_NOT_READY;
                        unknown_timeout_counter++;

                        if (unknown_timeout_counter == 6)
                        {
                            rythm_type = eRYTHM_UNKNOWN;
                            Audio_Message (eAUDIO_CMD_CONCAT, eAUDIO_STOP_PATIENT_MOVEMENT, TRUE);
                            unknown_timeout_counter = 0;
                        }
                        else if ((unknown_timeout_counter % 2) == 0)
                        {
                            Audio_Message (eAUDIO_CMD_CONCAT, eAUDIO_DO_NOT_TOUCH_PATIENT, TRUE);
                        }
                        break;
                        default:
                        break;
                }
            }
            // Next state selection.
            if((rythm_type == eRYTHM_NON_SHOCKABLE))// [1].
            {
                // Exit actions.
                FSM_Analyzing_OnExit();
                // Pointing to the next state.
                ind_1_FSM_Zp_Good = i_1_FSM_RCP;
                // Reset entry flag
                entryFsm.flg_1 = 0U;
            }
            else if(((rythm_type == eRYTHM_SHOCKABLE_SYNC) ||
                    (rythm_type == eRYTHM_SHOCKABLE_ASYNC)) )// [2].
            {
                // Exit actions.
                FSM_Analyzing_OnExit();
                // Pointing to the next state.
                ind_1_FSM_Zp_Good = i_1_FSM_PREPARE_SHOCK;
                // Reset entry flag
                entryFsm.flg_1 = 0U;
            }
            break;
        case i_1_FSM_RCP:
            // Entry check
            if(!((T_bit)entryFsm.flg_1))
            {
                entryFsm.flg_1 = (T_bit)1;
                // Entry actions
                // play the CPR message ONLY if proceed ...
                if (FSM_RCP_time_secs && RCP_new)
                {
                    RCP_new = FALSE;

                    // register the CPR start
                    DB_Episode_Set_Event (eREG_CPR_START);

                    // assign the RCP rate (100, 100 or 120)
                    switch (Get_RCP_Rate ())
                    {
                        case 0 : RCP_rate_bpm = 100;    break;
                        case 1 : RCP_rate_bpm = 110;    break;
                        default: RCP_rate_bpm = 120;    break;
                    }

                    // play the audio message to start the CPR
                    Audio_Message (eAUDIO_CMD_PLAY, eAUDIO_CPR_START, TRUE);
                    if (RCP_number == 0)
                    {
                        // play once to explain RCP instructions
                        Audio_Message (eAUDIO_CMD_CONCAT, eAUDIO_CPR_COMPRESSIONS, TRUE);
                    }

                    // wait to finish playing audio messages (allow process of patient impedance) ...
                    if (Pastime_While_Audio_Playing_with_Break_PT_OC(OSTIME_NO_DELAY) == RCP_BREAK_PATIENT_TYPE)
                    {
                        // restart the number of shocks over the same patient
                        nShocks_same_patient = 0;
                        // Restart the number of consecutive shocks when RCP
                        nShocks = 0;

                        // and set as a new RCP ...
                        RCP_new = TRUE;

                        // Child Initialization.
                        _FSM_Zp_Good_Init();
                        // Exit actions.
                        FSM_Zp_Good_OnExit();
                        // Pointing to the next state.
                        ind_FSM_R100_AED = i_FSM_CHANGE_PATIENT_TYPE;
                        // Reset entry flag
                        entryFsm.flg = 0U;

                    }

                    // start rcp counter
                    RCP_timeout_counter = (uint32_t) tx_time_get() + ((uint32_t) FSM_RCP_time_secs * OSTIME_1SEC);

                    // assign the number of compressions (15:2 or 30:2)
                    // Only compressions option is controlled in other function
                    if (pMisc_Settings->glo_patient_adult)
                    {
                        nCompressions = (pAed_Settings->aed_metronome_ratio_a == 0) ? 15/5 : 30/5;
                    }
                    else
                    {
                        nCompressions = (pAed_Settings->aed_metronome_ratio_p == 0) ? 15/5 : 30/5;
                    }
                    nCompressions_counter = 0;

                    // if FRCP is enabled, initialize the compression frequency algorithm
                    if (pAed_Settings->aed_feedback_en)
                    {
                        RCP_zp_idx  = patMon_Get_ECG_nSample ();
                        RCP_zp_adcs = patMon_Get_Zp_ADCs ();
                        FRCP_Init (RCP_zp_adcs);
                    }

                    // increment the number of RCP
                    RCP_number++;
                }

            }
            // State actions (Moore).
            if (RCP_new)
            {
                RCP_timeout = TRUE;
            }
            else
            {
                RCP_timeout = (tx_time_get() >= RCP_timeout_counter);
            }

            // Next state selection.
            if(RCP_timeout)
            {
                // Pointing to the next state.
                ind_1_FSM_Zp_Good = i_1_FSM_ANALYZING;
                // Reset entry flag
                entryFsm.flg_1 = 0U;
                if(FSM_RCP_time_secs != 0)
                {
                    Audio_Message (eAUDIO_CMD_CONCAT, eAUDIO_COMPRESSIONS_STOP, TRUE);
                    Pastime_While_Audio_Playing();
                }
            }
            else
            { // Else actions.
                /////////////////////////////////////////////////
                /////////////////////////////////////////////////
                /////////////////////////////////////////////////
                // Play the metronome audio

                if (pAed_Settings->aed_metronome_en && FSM_RCP_time_secs)
                {
                    if (/*!Is_Audio_Playing() ||*/ Is_Audio_Playing_Last())
                    {
                        RCP_running = true;
                        nCompressions_counter++;
                        if ((nCompressions_counter <= nCompressions)      ||
                            ((pMisc_Settings->glo_patient_adult == 1) && pAed_Settings->aed_metronome_ratio_a == 2)   ||
                            ((pMisc_Settings->glo_patient_adult == 0) && pAed_Settings->aed_metronome_ratio_p == 2))
                        {
                            // if FRCP is enabled, get the compressions frequency
                            if (pAed_Settings->aed_feedback_en)
                            {
                                RCP_rate_bpm_feedback = FRCP_Get_Frequency ();
                            }
                            else
                            {
                                RCP_rate_bpm_feedback = RCP_rate_bpm;
                            }

                            // if the FRCP is out of range when is going to play the 3rd audio tone advice the user !!!
                            if ((((RCP_rate_bpm_feedback > RCP_rate_bpm) && ((RCP_rate_bpm_feedback - RCP_rate_bpm) > 5)) ||
                                ((RCP_rate_bpm_feedback < RCP_rate_bpm) && ((RCP_rate_bpm - RCP_rate_bpm_feedback) > 5))) &&
                                ((nCompressions_counter != 0) && ((nCompressions_counter % 2) == 0)))
                            {
                                if (RCP_rate_bpm == 100) { Audio_Message (eAUDIO_CMD_CONCAT, eAUDIO_CPR_KEEP_UP_100, TRUE); }
                                if (RCP_rate_bpm == 110) { Audio_Message (eAUDIO_CMD_CONCAT, eAUDIO_CPR_KEEP_UP_110, TRUE); }
                                if (RCP_rate_bpm == 120) { Audio_Message (eAUDIO_CMD_CONCAT, eAUDIO_CPR_KEEP_UP_120, TRUE); }
                                nCompressions--;
                            }
                            else
                            {
                                if (RCP_rate_bpm == 100) { Audio_Message (eAUDIO_CMD_CONCAT, eAUDIO_TONE_100, TRUE); }
                                if (RCP_rate_bpm == 110) { Audio_Message (eAUDIO_CMD_CONCAT, eAUDIO_TONE_110, TRUE); }
                                if (RCP_rate_bpm == 120) { Audio_Message (eAUDIO_CMD_CONCAT, eAUDIO_TONE_120, TRUE); }
                            }
                        }
                        else
                        {
                            if(!RCP_timeout)
                            {
                                Audio_Message (eAUDIO_CMD_CONCAT, eAUDIO_COMPRESSIONS_STOP, TRUE);
                                Audio_Message (eAUDIO_CMD_CONCAT, eAUDIO_BREATH_2, TRUE);
                                Audio_Message (eAUDIO_CMD_CONCAT, eAUDIO_COMPRESSIONS_CONTINUE, TRUE);
                            }
                            // assign the number of compressions (15:2 or 30:2)
                            // Only compressions option is controlled in other function
                            if (pMisc_Settings->glo_patient_adult)
                            {
                                nCompressions = (pAed_Settings->aed_metronome_ratio_a == 0) ? 15/5 : 30/5;
                            }
                            else
                            {
                                nCompressions = (pAed_Settings->aed_metronome_ratio_p == 0) ? 15/5 : 30/5;
                            }

                            nCompressions_counter = 0;
                        }
                    }
                }

                // call to the FRCP compressions calculator
                if (patMon_Get_ZP_Window (RCP_zp_idx, RCP_WINSIZE, RCP_zp_signal) == TRUE)
                {
                    FRCP_Process_Zpwin (RCP_zp_signal);
                    RCP_zp_idx += RCP_WINSIZE;
                }

            }
            break;
        case i_1_FSM_PREPARE_SHOCK:
            // Entry check
            if(!((T_bit)entryFsm.flg_1))
            {
                entryFsm.flg_1 = (T_bit)1;
                // Entry actions
                // determine the energy to store in the main capacitor to use during the shock ...
                if (pMisc_Settings->glo_patient_adult)
                {
                    if (nShocks_same_patient == 0)
                    {
                        FSM_shock_energy = pAed_Settings->aed_energy_shock1_a;
                    }
                    else if (nShocks_same_patient == 1)
                    {
                        FSM_shock_energy = pAed_Settings->aed_energy_shock2_a;
                    }
                    else
                    {
                        FSM_shock_energy = pAed_Settings->aed_energy_shock3_a;
                    }
                }
                else
                {
                    if (nShocks_same_patient == 0)
                    {
                        FSM_shock_energy = pAed_Settings->aed_energy_shock1_p;
                    }
                    else if (nShocks_same_patient == 1)
                    {
                        FSM_shock_energy = pAed_Settings->aed_energy_shock2_p;
                    }
                    else
                    {
                        FSM_shock_energy = pAed_Settings->aed_energy_shock3_p;
                    }
                }

                // command the defibrillator to charge the main capacitor !!!
                // register the capacitor charge start
                DB_Episode_Set_Event (eREG_DEF_CHARGE);
                Defib_Cmd_Charge (FSM_shock_energy);

                // play the audio message demanding the shock
                Audio_Message (eAUDIO_CMD_PLAY, eAUDIO_SHOCK_ADVISED, TRUE);
                Audio_Message (eAUDIO_CMD_CONCAT, eAUDIO_DO_NOT_TOUCH_PATIENT, TRUE);
                if (Get_Device_Info()->enable_b.aed_fully_auto)
                {
                    // play the audio message demanding to press shock key
                    if (Pastime_While_Audio_Playing_with_Break_PT(OSTIME_500MSEC) == RCP_BREAK_PATIENT_TYPE)
                    {
                        // Child Initialization.
                        _FSM_Zp_Good_Init();
                        // Exit actions.
                        FSM_Zp_Good_OnExit();
                        // Pointing to the next state.
                        ind_FSM_R100_AED = i_FSM_CHANGE_PATIENT_TYPE;
                        // Reset entry flag
                        entryFsm.flg = 0U;
                        break;
                    }

                    // assign the automated shock timeout ...
                    autoshock_timeout_counter = tx_time_get() + (OSTIME_5SEC);
                    Audio_Message (eAUDIO_CMD_PLAY, eAUDIO_PREPARING_SHOCK, TRUE);
                    break_condition = Pastime_While_Audio_Playing_with_Break_PT_OC(OSTIME_1SEC);
                    if (break_condition == RCP_BREAK_PATIENT_TYPE)
                    {
                        // Child Initialization.
                        _FSM_Zp_Good_Init();
                        // Exit actions.
                        FSM_Zp_Good_OnExit();
                        // Pointing to the next state.
                        ind_FSM_R100_AED = i_FSM_CHANGE_PATIENT_TYPE;
                        // Reset entry flag
                        entryFsm.flg = 0U;
                        break;
                    }
                    if(break_condition == RCP_BREAK_ZP)
                    {
                        Audio_Message (eAUDIO_CMD_STOP, eAUDIO_PREPARING_SHOCK, TRUE);
                        // Child Initialization.
                        _FSM_Zp_Good_Init();
                        // Exit actions.
                        FSM_Zp_Good_OnExit();
                        // Pointing to the next state.
                        ind_FSM_R100_AED = i_FSM_INSTRUCT_USER;
                        // Reset entry flag
                        entryFsm.flg = 0U;
                        break;
                    }
                }

            }
            // State actions (Moore).
            // monitor the charging process
            aedef_state = Defib_Get_State();
            aedef_in_error = (aedef_state == eDEFIB_STATE_IN_ERROR);
            if (aedef_in_error)
            {
                // register the error condition
                DB_Episode_Set_Event (eREG_DEF_ERROR_CHARGING);
            }

            vc_charged = (aedef_state == eDEFIB_STATE_CHARGED);

            // Next state selection.
            if(vc_charged)// [1].
            {
                // Pointing to the next state.
                ind_1_FSM_Zp_Good = i_1_FSM_DEMAND_SHOCK;
                // Reset entry flag
                entryFsm.flg_1 = 0U;
            }
            else if(aedef_in_error)// [2].
            {
                // Pointing to the next state.
                ind_1_FSM_Zp_Good = i_1_LOCK_ON_ERROR;
                // Reset entry flag
                entryFsm.flg_1 = 0U;
            }
            break;
        case i_1_FSM_DEMAND_SHOCK:
            // Entry check
            if(!((T_bit)entryFsm.flg_1))
            {
                entryFsm.flg_1 = (T_bit)1;
                // Entry actions
                // wait to finish playing messages ...
                // blink the Shock LEDs
                if (Get_Device_Info()->enable_b.aed_fully_auto) Led_On (LED_SHOCK);
                if (Get_Device_Info()->enable_b.aed_fully_auto) Audio_Message (eAUDIO_CMD_PLAY, eAUDIO_SHOCK_3_2_1, TRUE);
                break_condition = Pastime_While_Audio_Playing_with_Break_PT_OC(OSTIME_NO_DELAY);
                if (break_condition == RCP_BREAK_PATIENT_TYPE)
                {
                    // Child Initialization.
                    _FSM_Zp_Good_Init();
                    // Exit actions.
                    FSM_Zp_Good_OnExit();
                    // Pointing to the next state.
                    ind_FSM_R100_AED = i_FSM_CHANGE_PATIENT_TYPE;
                    // Reset entry flag
                    entryFsm.flg = 0U;
                    break;
                }
                if(break_condition == RCP_BREAK_ZP)
                {
                    Led_Off (LED_SHOCK);
                    Audio_Message (eAUDIO_CMD_STOP, eAUDIO_SHOCK_3_2_1, TRUE);
                    // Child Initialization.
                    _FSM_Zp_Good_Init();
                    // Exit actions.
                    FSM_Zp_Good_OnExit();
                    // Pointing to the next state.
                    ind_FSM_R100_AED = i_FSM_INSTRUCT_USER;
                    // Reset entry flag
                    entryFsm.flg = 0U;
                    break;
                }

                if (! Get_Device_Info()->enable_b.aed_fully_auto)
                {
                    // play the audio message demanding to press shock key
                    Audio_Message (eAUDIO_CMD_PLAY, eAUDIO_PUSH_TO_SHOCK, TRUE);

                    // blink the Shock LEDs
                    Led_Blink (LED_SHOCK);

                    // restart the automated shock timeout ...
                    autoshock_timeout_counter = 0;
                }
                else
                {   
                    if (Pastime_While_Audio_Playing_with_Break_PT(OSTIME_NO_DELAY) == RCP_BREAK_PATIENT_TYPE)
                    {
                        // Child Initialization.
                        _FSM_Zp_Good_Init();
                        // Exit actions.
                        FSM_Zp_Good_OnExit();
                        // Pointing to the next state.
                        ind_FSM_R100_AED = i_FSM_CHANGE_PATIENT_TYPE;
                        // Reset entry flag
                        entryFsm.flg = 0U;
                        break;
                    }
                    // ON the Shock LEDs
                    if(break_condition != RCP_BREAK_ZP) Led_On (LED_SHOCK);
                }

                // restart the shock timeout
                shock_timeout_counter = tx_time_get() + OSTIME_10SEC;
            }
            // State actions (Moore).
            shock_timeout = (tx_time_get() >= shock_timeout_counter);

            shock_done = FALSE;

            // if fully automated option then shock automatically
            if (Get_Device_Info()->enable_b.aed_fully_auto)
            {
                // process autoshock condition
                if (tx_time_get() >= autoshock_timeout_counter)
                {
                    aed_event = eEV_KEY_SHOCK;
                }
            }

            // Shock the patient
            if (aed_event == eEV_KEY_SHOCK)
            {
                // command the defibrillator to shock the patient !!!
                Defib_Cmd_Shock (rythm_type);

                // monitor the shock process
                aedef_state = Defib_Get_State();
                aedef_in_error = (aedef_state == eDEFIB_STATE_IN_ERROR);
                if (aedef_in_error)
                {
                    // register the error condition
                    DB_Episode_Set_Event (eREG_DEF_ERROR_SHOCK);
                }
                else
                {
                    // set the shock flag !!!
                    shock_done = TRUE;
                }
            }

            // register the shock event ...
            if (shock_done)
            {
                // register in the electrodes
                Electrodes_Register_Shock ();

                // register in the episode
                event_id = eREG_NULL;
                if (nShocks_same_patient == 0)
                {
                    event_id = pMisc_Settings->glo_patient_adult ? eREG_SHOCK_1_ADULT : eREG_SHOCK_1_PEDIATRIC;
                }
                else if (nShocks_same_patient == 1)
                {
                    event_id = pMisc_Settings->glo_patient_adult ? eREG_SHOCK_2_ADULT : eREG_SHOCK_2_PEDIATRIC;
                }
                else
                {
                    event_id = pMisc_Settings->glo_patient_adult ? eREG_SHOCK_3_ADULT : eREG_SHOCK_3_PEDIATRIC;
                }
                DB_Episode_Set_Event (event_id);

                // increment the number of current consecutive shocks
                nShocks++;
                nShocks_same_patient++;

                // inform that the endurance test has finished ...
                if (Get_Device_Info()->develop_mode == DEVELOP_TEST_2500_eSHOCK)
                {
                    endurance_number_items++;
                    Trace_Arg (TRACE_NEWLINE, " n_shocks: = %4d", endurance_number_items);
                    if(endurance_number_items >= 2500)
                    {
                        Trace (TRACE_TIME_STAMP | TRACE_NEWLINE,"ENDURANCE_2500_TEST FINISH!!!!!!");

                        // play the audio message advertising Device ready
                        Audio_Message (eAUDIO_CMD_PLAY, eAUDIO_DEVICE_READY, TRUE);

                        // wait to finish playing messages ...
                        Pastime_While_Audio_Playing();
                    }
                }
            }

            // Next state selection.
            if(shock_timeout)// [1].
            {
                // Exit actions.
                FSM_Demand_Shock_OnExit();
                // Pointing to the next state.
                ind_1_FSM_Zp_Good = i_1_FSM_ANALYZING;
                // Reset entry flag
                entryFsm.flg_1 = 0U;
            }
            else if((shock_done &&
                    (nShocks >= nConsecutive_shocks)))// [2].
            {
                // Exit actions.
                FSM_Demand_Shock_OnExit();
                // Pointing to the next state.
                ind_1_FSM_Zp_Good = i_1_FSM_RCP;
                // Reset entry flag
                entryFsm.flg_1 = 0U;
            }
            else if((shock_done &&
                    (nShocks < nConsecutive_shocks)))// [3].
            {
                // Exit actions.
                FSM_Demand_Shock_OnExit();
                // Pointing to the next state.
                ind_1_FSM_Zp_Good = i_1_FSM_ANALYZING;
                // Reset entry flag
                entryFsm.flg_1 = 0U;
            }
            else if(aedef_in_error)// [4].
            {
                // Exit actions.
                FSM_Demand_Shock_OnExit();
                // Pointing to the next state.
                ind_1_FSM_Zp_Good = i_1_LOCK_ON_ERROR;
                // Reset entry flag
                entryFsm.flg_1 = 0U;
            }
            break;
        case i_1_LOCK_ON_ERROR:
            // Entry check
            if(!((T_bit)entryFsm.flg_1))
            {
                entryFsm.flg_1 = (T_bit)1;
                // Entry actions
            }
            // State actions (Moore).
            break;
    }
}


// Implementation of Exit_functions.
static void FSM_Analyzing_OnExit(void)
{
    // stops the analysis
    DRD_Stop_Analysis();

    // register the result
    DB_Episode_Set_Event (eREG_DRD_DIAG_RDY);

    // assign the RCP timeout
    if (rythm_type == eRYTHM_NON_SHOCKABLE)
    {
        // assign the CPR time
        if (pMisc_Settings->glo_patient_adult)
        {
            FSM_RCP_time_secs = pAed_Settings->aed_aCPR_2;
            metronome_cnt_max = pAed_Settings->aed_metronome_ratio_a;
        }
        else
        {
            FSM_RCP_time_secs = pAed_Settings->aed_pCPR_2;
            metronome_cnt_max = pAed_Settings->aed_metronome_ratio_p;
        }

        // check if capacitor voltage has lowered too much
        if ( (PRE_CHARGE_VOLTAGE - 100) > Defib_Get_Vc())
        {
            Defib_Cmd_Precharge (FALSE);
        }

        // play the audio message with NO-SHOCK advised
        Audio_Message (eAUDIO_CMD_CONCAT, eAUDIO_NO_SHOCK_ADVISED, TRUE);

        // wait to finish playing messages ...
        Pastime_While_Audio_Playing();

        // Restart the number of consecutive shocks when RCP
        nShocks = 0;

        // to allow a new RCP
        RCP_new = TRUE;
    }

    // process asystole condition
    if (asystole_timeout_counter && (tx_time_get() >= asystole_timeout_counter))
    {
        // play the audio message advertising ASYSTOLE
        Audio_Message (eAUDIO_CMD_CONCAT, eAUDIO_ASYSTOLE, TRUE);

        // wait to finish playing messages ...
        Pastime_While_Audio_Playing();
    }

}


static void FSM_Demand_Shock_OnExit(void)
{
    // OFF the Shock LEDs
    Led_Off (LED_SHOCK);

    if (shock_done)
    {
        if (!aedef_in_error)
        {
            // play the audio message announcing that the shock is done
            Audio_Message (eAUDIO_CMD_PLAY, eAUDIO_SHOCK_DELIVERED, TRUE);

            // wait to finish to play the message ...
            Pastime_While_Audio_Playing();

            // command the defibrillator to precharge the main capacitor !!!
            // Do not force the precharge and maintain the remaining voltage after the Shock
            Defib_Cmd_Precharge (FALSE);
        }
    }
    else 
    {
        // register shock not delivered event
        DB_Episode_Set_Event (eREG_SHOCK_NOT_DELIVERED);

        // play the audio message announcing that the shock is not delivered
        Audio_Message (eAUDIO_CMD_PLAY, eAUDIO_NO_SHOCK_DELIVERED, TRUE);

        // wait to finish to play the message ...
        Pastime_While_Audio_Playing();
    }

    // Restart the number of consecutive shocks when timeout
    if (shock_timeout)
    {
        // Restart the number of consecutive shocks when RCP
        //nShocks = 0;

        // In DEMO mode do not send anything related to alerts
        if ((!Is_Battery_Mode_Demo()) && (Is_Sigfox_TX_Enabled() == SIGFOX_ONLY || Is_Sigfox_TX_Enabled() == SIGFOX_PRIOR) && (Get_Device_Settings()->misc.glo_patient_alert == TRUE))
        {
            Send_Sigfox_Alert(MSG_ID_SHOCK_TIMEOUT_ALERT);   // Generate and Send Alert using Sigfox
        }
        if((!Is_Battery_Mode_Demo()) && (Is_Wifi_TX_Enabled() == WIFI_ONLY || Is_Wifi_TX_Enabled() == WIFI_PRIOR) && (Get_Device_Settings()->misc.glo_patient_alert == TRUE)) //WIFI_WITH_PATIENT
        {
            Send_Wifi_Alert(WIFI_MSG_ID_SHOCK_TIMEOUT_ALERT);   // Generate and Send Alert report using Wifi
        }

        // endurance 500 internal discharge: force discharge through the security relay
        // else --> must maintain the stored energy in the main capacitor
        if (Get_Device_Info()->develop_mode == DEVELOP_TEST_500_iSHOCK)
        {
            Defib_Cmd_Disarm ();

            endurance_number_items++;

            Trace_Arg (TRACE_NEWLINE, " n_int_discharge: = %4d", endurance_number_items);
            if(endurance_number_items >= 500)
            {
                Trace (TRACE_TIME_STAMP | TRACE_NEWLINE,"ENDURANCE_500_TEST FINISH!!!!!!");

                // play the audio message advertising Device ready
                Audio_Message (eAUDIO_CMD_PLAY, eAUDIO_DEVICE_READY, TRUE);

                // wait to finish playing messages ...
                Pastime_While_Audio_Playing();
            }
        }
    }

    // When goto RCP ...
    if (nShocks >= nConsecutive_shocks)
    {
        // assign the CPR time
        if (pMisc_Settings->glo_patient_adult)
        {
            FSM_RCP_time_secs = pAed_Settings->aed_aCPR_1;
            metronome_cnt_max = pAed_Settings->aed_metronome_ratio_a;
        }
        else
        {
            FSM_RCP_time_secs = pAed_Settings->aed_pCPR_1;
            metronome_cnt_max = pAed_Settings->aed_metronome_ratio_p;
        }

        // Restart the number of consecutive shocks when RCP
        nShocks = 0;

        // to allow a new RCP
        RCP_new = TRUE;
    }

}


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// Ending code proposed by the user (Start).
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// Ending code proposed by the user (End).
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  End of the automatic generated C-code.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
