/******************************************************************************
 * Name      : R100_S3A7                                                      *
 * Copyright : Osatu S. Coop                                                  *
 * Compiler  : GNU ARM Embedded                                               *
 * Target    : Reanibex Serie 100                                             *
 ******************************************************************************/

/*!
 * @file        thread_audio_entry.h
 * @brief       Header with functions related to the audio task
 *
 * @version     v1
 * @date        16/01/2018
 * @author      ltorres
 * @warning     author   date    version     .- Edition
 * @bug
 *
 */

#ifndef THREAD_AUDIO_ENTRY_H_
#define THREAD_AUDIO_ENTRY_H_

/******************************************************************************
 **Includes
 */
#include "R100_Errors.h"
#include "types_basic.h"

/******************************************************************************
 ** Defines
 */

#define SYNC_AUDIO_ANY_EVENT        (0xFFFFFFFF)
#define SYNC_AUDIO_PLAY_EVENT       (0x00000001 << 0)
//#define SYNC_AUDIO_CAT_EVENT        (0x00000001 << 1)
//#define SYNC_AUDIO_MUTE_EVENT       (0x00000001 << 2)
//#define SYNC_AUDIO_STOP_EVENT       (0x00000001 << 3)
//#define SYNC_AUDIO_BEEP_EVENT       (0x00000001 << 4)
//#define SYNC_AUDIO_MUTE_OFF_EVENT   (0x00000001 << 5)
//#define SYNC_AUDIO_VOL_LV1_EVENT    (0x00000001 << 6)
//#define SYNC_AUDIO_VOL_LV2_EVENT    (0x00000001 << 7)
//#define SYNC_AUDIO_VOL_LV3_EVENT    (0x00000001 << 8)
//#define SYNC_AUDIO_VOL_LV4_EVENT    (0x00000001 << 9)
//#define SYNC_AUDIO_VOL_LV5_EVENT    (0x00000001 << 10)


/******************************************************************************
 ** Typedefs
 */
/// Audio commands for message player
typedef enum {
    eAUDIO_CMD_STOP = 0,                    ///< Stop the message player
    eAUDIO_CMD_PLAY,                        ///< Plays a message anyway (if the player is running, the new message
                                            ///< Is played instead the current)
    eAUDIO_CMD_CONCAT,                      ///< Places the message in a queue so that it is played after the current
                                            ///< One ends. If there is no current, it plays the message
    eAUDIO_CMD_STOP_MESSAGE,                ///< Stops current message and continue with next on queue if there are more
    eAUDIO_CMD_MAX
} AUDIO_CMD_e;

/// Audio message identifiers
typedef enum {
    eAUDIO_MODE_TEST = 0,
    eAUDIO_CALL_SAT,
    eAUDIO_KEEP_CALM,
    eAUDIO_FOLLOW_INSTRUCTIONS,
    eAUDIO_ASK_FOR_HELP,
    eAUDIO_REMOVE_CHEST_CLOTHES,
    eAUDIO_OPEN_ELECTRODE_PACKAGE,
    eAUDIO_APPLY_ELECTRODES_CHEST,
    eAUDIO_PRESS_ELECTRODES,
    eAUDIO_CHECK_ELECTRODES,
    eAUDIO_CONNECT_ELECTRODES_DEVICE = 10,
    eAUDIO_DO_NOT_TOUCH_PATIENT,
    eAUDIO_ANALYZING_HEART_RATE,
    eAUDIO_NO_SHOCK_ADVISED,
    eAUDIO_CPR_START,
    eAUDIO_CPR_COMPRESSIONS,
    eAUDIO_CPR_KEEP_UP_100,
    eAUDIO_CPR_KEEP_UP_110,
    eAUDIO_CPR_KEEP_UP_120,
    eAUDIO_CPR_BREATH,
    eAUDIO_BREATH_2 = 20,
    eAUDIO_CPR_START_SHORT,
    eAUDIO_COMPRESSIONS_CONTINUE,
    eAUDIO_COMPRESSIONS_STOP,
    eAUDIO_SHOCK_ADVISED,
    eAUDIO_PREPARING_SHOCK,
    eAUDIO_PUSH_TO_SHOCK,
    eAUDIO_SHOCK_DELIVERED,
    eAUDIO_NO_SHOCK_DELIVERED,
    eAUDIO_DEMO_MODE,
    eAUDIO_CALL_EMERGENCIES = 30,
    eAUDIO_LOW_BATTERY,
    eAUDIO_REPLACE_BATTERY,
    eAUDIO_STOP_PATIENT_MOVEMENT,
    eAUDIO_ADULT_MODE,
    eAUDIO_PEDIATRIC_MODE,
    eAUDIO_DISCONNECT_PATIENT,
    eAUDIO_GETTING_LOCATION,
    eAUDIO_CHECK_PATIENT,
    eAUDIO_ASYSTOLE,
    eAUDIO_DEVICE_READY = 40,
    eAUDIO_REPLACE_ELECTRODES,
    eAUDIO_CONN_ERROR,
    eAUDIO_TRANSMISSION_ONGOING,
    eAUDIO_TRANSMISSION_DONE,
    eAUDIO_TONE_100,
    eAUDIO_TONE_110,
    eAUDIO_TONE_120,
    eAUDIO_OPEN_COVER,
    eAUDIO_TONE,                    // CPR tone  // <------------------------------
    eAUDIO_UPGRADING = 50,
    eAUDIO_UPGRADE_ERROR,
    eAUDIO_SHOCK_3_2_1,
    eAUDIO_TAKE_SEVERAL_MINUTES,
    eAUDIO_TRAINER_MODE,
    eAUDIO_MAX_ID
} AUDIO_ID_e;



#define MAX_LANGUAGES       6                       ///< Number of languages in a single device
#define MAX_AUDIO_MSG       eAUDIO_MAX_ID           ///< Max number of audio messages



/******************************************************************************
 ** Externals
 */

/******************************************************************************
 ** Globals
 */

volatile uint32_t   sync_audio;

/******************************************************************************
 ** Prototypes
 */

extern  ERROR_ID_e  Load_Audio_Resources (uint8_t* audio_num);

extern  ERROR_ID_e  Audio_Message (AUDIO_CMD_e cmd, AUDIO_ID_e msg_id, const bool_t force);
extern  ERROR_ID_e  Audio_Test (const AUDIO_ID_e msg_id, bool_t auto_test);

extern  bool_t      Is_Audio_Playing      (void);
extern  bool_t      Is_Audio_Playing_Last (void);

extern  bool_t      Is_Task_Audio_Enabled (void);

extern  void        Set_Audio_Player_in_error (void);

#endif /* THREAD_AUDIO_ENTRY_H_ */

/*
 ** $Log$
 **
 ** end of thread_audio_entry.h
 ******************************************************************************/
