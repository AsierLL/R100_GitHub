/******************************************************************************
 * Name      : R100_S3A7                                                      *
 * Copyright : Osatu S. Coop                                                  *
 * Compiler  : GNU ARM Embedded                                               *
 * Target    : Reanibex Serie 100                                             *
 ******************************************************************************/

/*!
 * @file        thread_audio_entry.c
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
#include "types_app.h"
#include "HAL/thread_defibrillator_hal.h"
#include "HAL/thread_patMon_hal.h"
#include "HAL/thread_audio_hal.h"
#include "thread_audio.h"
#include "thread_sysMon_entry.h"
#include "thread_audio_entry.h"
#include "thread_core_entry.h"

#include "Trace.h"
#include "thread_api.h"

/******************************************************************************
 ** Macros
 */

/******************************************************************************
 ** Defines
 */
#define AUDIO_FOLDER_NAME               "AUDIO\\lang"       ///< Folder name for audio messages
#define AUDIO_FOLDER                    "AUDIO"             ///< Folder name for audio messages
#define AUDIO_VOLUME_DEFAULT            32                  ///< Default volume
#define AUDIO_VOLUME_TEST               45                  ///< Volume for test
#define AUDIO_VOLUME_DEFAULT_MONTADOR   MAX_AUDIO_VOLUME    ///< Default volume
//#define AUDIO_NOISE_DEFAULT           1450                ///< Default noise (empirical value)
#define AUDIO_NOISE_DEFAULT             1640                ///< Default noise (empirical value)

#define WAV_HEADER_OFFSET               40
#define WAV_DATA_SIZE_BYTES             4

#define AUDIO_FRAME_NSAMPLES            2048                ///< Size of the buffer for audio operations (limited by DMA)
#define AUDIO_PLAY_QUEUE_SZ             6                   ///< Number of message to play concatenated

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
extern bool_t                  RCP_running;

/******************************************************************************
 ** Locals
 */

static FX_FILE  aFile[MAX_AUDIO_MSG];

static uint8_t  concat_queue [AUDIO_PLAY_QUEUE_SZ];         ///< Queue for concatenated messages

static int32_t  remaining_nSamples_to_play;                 ///< Number of remaining bytes to play the audio
static int32_t  remaining_nSamples_to_read;                 ///< Number of remaining bytes to read from the audio file

static uint32_t audio_dma_free = 0;                         ///< Synchronize the DMA with audio frames ...
static uint8_t  audio_playing_msg_id = eAUDIO_MAX_ID;
static uint16_t audio_buffer[AUDIO_FRAME_NSAMPLES * 4];     ///< Triple buffer used to store the audio samples

static uint16_t *pAudio_to_fill;                            ///< Pointer to audio frame for storage
static uint16_t *pAudio_to_play;                            ///< Pointer to audio frame for playing
static uint32_t sequence_fill = 0;                          ///< Control the audio sequence to fill the audio buffer
static uint32_t sequence_play = 0;                          ///< Control the audio sequence to play the audio buffer

static uint8_t  audio_volume = AUDIO_VOLUME_DEFAULT;

static bool_t   flag_audio_init = false;

static bool_t   flag_error_in_audio = false;

/******************************************************************************
 ** Prototypes
 */


/******************************************************************************
 ** Name:    dma_audio_callback
 *****************************************************************************/
/**
 ** @brief   DMA audio callback function
 **
 ******************************************************************************/
void dma_audio_callback(transfer_callback_args_t *p_args)
{
    uint16_t    nSamples;
    uint32_t    seq;            // sequencer value to update

    SSP_PARAMETER_NOT_USED(p_args);

    // subtract the number of transferred samples
    if (remaining_nSamples_to_play > 0)
    {
        remaining_nSamples_to_play -= (int32_t) dma_audio.p_cfg->p_info->length;
    }

    // if there are more samples to play, reprogram a new DMA shot !!!
    if (remaining_nSamples_to_play > 0)
    {
        // update the number of samples to play in the next frame
        nSamples = (remaining_nSamples_to_play > AUDIO_FRAME_NSAMPLES) ? AUDIO_FRAME_NSAMPLES : (uint16_t) remaining_nSamples_to_play;

        // update the audio sequencer and the audio frame
        switch (sequence_play)
        {
            case 0 : seq = 1;   pAudio_to_play = &audio_buffer[AUDIO_FRAME_NSAMPLES * 1];  break;
            case 1 : seq = 2;   pAudio_to_play = &audio_buffer[AUDIO_FRAME_NSAMPLES * 2];  break;
            case 2 : seq = 3;   pAudio_to_play = &audio_buffer[AUDIO_FRAME_NSAMPLES * 3];  break;
            default: seq = 0;   pAudio_to_play = &audio_buffer[AUDIO_FRAME_NSAMPLES * 0];  break;
        }
        sequence_play = seq;

        // play the next audio frame
        DMA_Shot (pAudio_to_play, nSamples);
    }
    else
    {
        remaining_nSamples_to_play = 0;
        Audio_Mute ();
    }
    audio_dma_free++;
}


/******************************************************************************
 ** Name:    Is_Task_Audio_Enabled
 *****************************************************************************/
/**
 ** @brief   Is audio task initialized?
 **
 ** @param   none
 **
 ** @return  true or false
 ** @todo    none
 ******************************************************************************/
bool_t Is_Task_Audio_Enabled (void)
{
    return flag_audio_init;
}

/******************************************************************************
 ** Name:    Adjust_Audio_Volume
 *****************************************************************************/
/**
 ** @brief   Adaptive volume adjust in function of the ambient noise
 **
 ** @param   volume_set     volume to set as center value
 **
 ** @return  void
 ** @todo    none
 ******************************************************************************/
static void Adjust_Audio_Volume (uint16_t volume_set)
{
    static const uint16_t  audio_adjust_table [] = {
        AUDIO_NOISE_DEFAULT +  (20 *  1),       //
        AUDIO_NOISE_DEFAULT +  (20 *  2),       //
        AUDIO_NOISE_DEFAULT +  (20 *  3),       //
        AUDIO_NOISE_DEFAULT +  (20 *  4),       //
        AUDIO_NOISE_DEFAULT +  (20 *  5),       //
        AUDIO_NOISE_DEFAULT +  (20 *  6),       //
        AUDIO_NOISE_DEFAULT +  (20 *  7),       //
        AUDIO_NOISE_DEFAULT +  (20 *  8),       //
        AUDIO_NOISE_DEFAULT +  (20 *  9),       //
        AUDIO_NOISE_DEFAULT +  (20 * 10),       //
        AUDIO_NOISE_DEFAULT +  (20 * 11),       //
        AUDIO_NOISE_DEFAULT +  (20 * 12),       //
        AUDIO_NOISE_DEFAULT +  (20 * 13),       //
        AUDIO_NOISE_DEFAULT +  (20 * 14),       //
        AUDIO_NOISE_DEFAULT +  (20 * 15),       //
        AUDIO_NOISE_DEFAULT +  (20 * 16),       //
        AUDIO_NOISE_DEFAULT +  (20 * 17),       //
        AUDIO_NOISE_DEFAULT +  (20 * 18),       //
        AUDIO_NOISE_DEFAULT +  (20 * 19),       //
        AUDIO_NOISE_DEFAULT +  (20 * 20),       //
        AUDIO_NOISE_DEFAULT +  (20 * 21),       //
        AUDIO_NOISE_DEFAULT +  (20 * 22),       //
        AUDIO_NOISE_DEFAULT +  (20 * 23),       //
        AUDIO_NOISE_DEFAULT +  (20 * 24),       //
        AUDIO_NOISE_DEFAULT +  (20 * 25),       //
        AUDIO_NOISE_DEFAULT +  (20 * 26),       //
        AUDIO_NOISE_DEFAULT +  (20 * 27),       //
        AUDIO_NOISE_DEFAULT +  (20 * 28),       //
        AUDIO_NOISE_DEFAULT +  (20 * 29),       //
        AUDIO_NOISE_DEFAULT +  (20 * 30),       //
        AUDIO_NOISE_DEFAULT +  (20 * 31),       //
        AUDIO_NOISE_DEFAULT +  (20 * 32),       // MAX_AUDIO_VOLUME = 32
        AUDIO_NOISE_DEFAULT +  (20 * 33),       //
        AUDIO_NOISE_DEFAULT +  (20 * 34),       //
        AUDIO_NOISE_DEFAULT +  (20 * 35),       //
        AUDIO_NOISE_DEFAULT +  (20 * 36),       //
        AUDIO_NOISE_DEFAULT +  (20 * 37),       //
        AUDIO_NOISE_DEFAULT +  (20 * 38),       //
        AUDIO_NOISE_DEFAULT +  (20 * 39),       //
        AUDIO_NOISE_DEFAULT +  (20 * 40),       //
        AUDIO_NOISE_DEFAULT +  (20 * 41),       //
        AUDIO_NOISE_DEFAULT +  (20 * 42),       //
        AUDIO_NOISE_DEFAULT +  (20 * 43),       //
        AUDIO_NOISE_DEFAULT +  (20 * 44),       //
        AUDIO_NOISE_DEFAULT +  (20 * 45),       //
        AUDIO_NOISE_DEFAULT +  (20 * 46),       //
        AUDIO_NOISE_DEFAULT +  (20 * 47),       // AUDIO_VOLUME_DEFAULT
        AUDIO_NOISE_DEFAULT +  (20 * 48),       //
        AUDIO_NOISE_DEFAULT +  (20 * 49),       //
        AUDIO_NOISE_DEFAULT +  (20 * 50),       //
        AUDIO_NOISE_DEFAULT +  (20 * 51),       //
        AUDIO_NOISE_DEFAULT +  (20 * 52),       //
        AUDIO_NOISE_DEFAULT +  (20 * 53),       //
        AUDIO_NOISE_DEFAULT +  (20 * 54),       //
        AUDIO_NOISE_DEFAULT +  (20 * 55),       //
        AUDIO_NOISE_DEFAULT +  (20 * 56),       //
        AUDIO_NOISE_DEFAULT +  (20 * 57),       //
        AUDIO_NOISE_DEFAULT +  (20 * 58),       //
        AUDIO_NOISE_DEFAULT +  (20 * 59),       //
        AUDIO_NOISE_DEFAULT +  (20 * 60),       //
        AUDIO_NOISE_DEFAULT +  (20 * 61),       //
        AUDIO_NOISE_DEFAULT +  (20 * 62),       //
        AUDIO_NOISE_DEFAULT +  (20 * 63),       //
        AUDIO_NOISE_DEFAULT +  (20 * 64),       // MAX_AUDIO_VOLUME = 64
        0                                       // Zero termination table
    };

    static uint32_t audio_down_rate, audio_up_rate;
    static bool_t   audio_flag;             // flag to identify if the player has been in use
    static uint8_t  adapt_vol = AUDIO_VOLUME_DEFAULT;              // adaptive volume transition

    uint8_t         volume_min;             // minimum volume value
    uint8_t         volume_max;             // maximum volume value
    uint16_t        audio_energy;
    uint32_t        i;                      // global use counter

    audio_down_rate++;                      // counter to set the volume down dynamic
    audio_up_rate++;                        // counter to set the volume up dynamic

    // assign a max volume
    volume_min = (uint8_t) (volume_set);
    volume_max = (uint8_t) (volume_set + 10);

    if (volume_max > MAX_AUDIO_VOLUME) volume_max = MAX_AUDIO_VOLUME;
    if (volume_max < MIN_AUDIO_VOLUME) volume_max = MIN_AUDIO_VOLUME +10;
    if (volume_min > MAX_AUDIO_VOLUME) volume_min = MAX_AUDIO_VOLUME;
    if (volume_min < MIN_AUDIO_VOLUME) volume_min = MIN_AUDIO_VOLUME;

    volume_set = volume_min;

    // get the audio energy and process the read value ...
    audio_energy = (uint16_t) Get_Audio_Energy ();
    if (audio_energy && !audio_flag && !Is_Audio_Playing())
    {
        for (i=0; audio_adjust_table[i]; i++)
        {
            if (audio_energy > audio_adjust_table[i])
            {
                volume_set++;
            }
            else {
                break;
            }
        }

        if (!audio_adjust_table[i]) volume_set = MAX_AUDIO_VOLUME;
        if (volume_set > volume_max) volume_set = volume_max;
        adapt_vol = (uint8_t)((volume_set + adapt_vol)/2);

    }
    if ((adapt_vol > audio_volume) && ((audio_up_rate % 4) == 0))
    {
        audio_down_rate = 0;
        audio_volume++;
        Audio_Volume_Set(audio_volume);
    }
    if ((adapt_vol < audio_volume) && ((audio_down_rate % 8) == 0))
    {
        audio_up_rate = 0;
        audio_volume--;
        Audio_Volume_Set(audio_volume);
    }

    // update the audio flag ...
    audio_flag = Is_Audio_Playing();
}


/******************************************************************************
 ** Name:    Load_Audio_Resources
 *****************************************************************************/
/**
 ** @brief   load all the audio resources
 **
 ** @param   audio_num number of audios that have been opened
 **
 ** @return  error id
 **
 ******************************************************************************/
ERROR_ID_e Load_Audio_Resources(uint8_t* audio_num)
{
    static   uint32_t   n_entries = 0;
    uint32_t fx_res = 0;
    uint32_t attributes = 0;
    char_t   curr_path[FX_MAX_LONG_NAME_LEN/2];
    char_t   audio_file_name[10];
    uint8_t  audio_file_num = 0;

    if (n_entries > 1) return eERR_NONE;
    n_entries++;

    strcpy (curr_path, AUDIO_FOLDER_NAME);
    fx_res = strlen(curr_path);
    curr_path[fx_res  ] = (char_t) (Get_Misc_Settings()->glo_language + '0');
    curr_path[fx_res+1] = 0;

    // Hidden folder
    fx_res = (uint8_t) fx_directory_attributes_set(&sd_fx_media, AUDIO_FOLDER, FX_HIDDEN);
    fx_res = (uint8_t) fx_directory_default_set(&sd_fx_media, curr_path);         //Set the default directory to current language folder

    if ((uint8_t) FX_SUCCESS == fx_res)
    {
        fx_res = (uint8_t) fx_directory_first_full_entry_find(&sd_fx_media, curr_path, (UINT *)&attributes, 0, 0, 0, 0, 0, 0, 0);
        while ((uint8_t) FX_SUCCESS == fx_res)
        {
           // First pass: only files are added to the list
           if (!(attributes & FX_DIRECTORY))
           {
               fx_directory_long_name_get(&sd_fx_media, curr_path, audio_file_name);
               audio_file_num = (uint8_t) (((audio_file_name[4] - '0') * 10) + ((audio_file_name[5] - '0')) - 1);
               fx_res = (uint8_t) fx_file_open(&sd_fx_media, &aFile[audio_file_num], audio_file_name, FX_OPEN_FOR_READ);
               (*audio_num)++;
           }

           fx_res = (uint8_t) fx_directory_next_full_entry_find(&sd_fx_media, curr_path, (UINT *)&attributes, 0, 0, 0, 0, 0, 0, 0);
        }

        fx_res = (uint8_t) fx_directory_default_set(&sd_fx_media, FX_NULL);           //Reset the default directory to root
    }
    else {
        // can not open the requested language items. Restart the language identifier and try again !!!
        return eERR_AUDIO_MAX;
    }

    // restart the audio messages queue
    memset (concat_queue, eAUDIO_MAX_ID, AUDIO_PLAY_QUEUE_SZ);

    // Check number of audios
    if (*audio_num == MAX_AUDIO_MSG)
    {
        return eERR_NONE;
    }

    return eERR_AUDIO_MAX;
}

/******************************************************************************
 ** Name:    Get_Next_Audio_Samples
 *****************************************************************************/
/**
 ** @brief   Get an audio frame from the audio file
 **
 ** @param   none
 **
 ** @return  void
 ******************************************************************************/
static void Get_Next_Audio_Samples(void)
{
    uint32_t nBytes_to_read;    // number of read bytes from the file
    uint32_t nBytes;            // number of read bytes from the file
    uint32_t seq;               // sequencer value to update
    uint8_t  fx_res;            // Stores the fileX operations result

    if (remaining_nSamples_to_read > 0)
    {
        // get the mutex to access to the uSD
        tx_mutex_get(&usd_mutex, TX_WAIT_FOREVER);

        // read the next frame
        nBytes_to_read = (remaining_nSamples_to_read > AUDIO_FRAME_NSAMPLES) ? AUDIO_FRAME_NSAMPLES : (uint32_t) remaining_nSamples_to_read;
        fx_res = (uint8_t) fx_file_read (&aFile[audio_playing_msg_id], pAudio_to_fill, nBytes_to_read * sizeof(uint16_t), &nBytes);
        remaining_nSamples_to_read -= (int32_t) (nBytes / sizeof(uint16_t));

        // update the audio sequencer and the audio frame
        switch (sequence_fill)
        {
            case 0 : seq = 1;   pAudio_to_fill = &audio_buffer[AUDIO_FRAME_NSAMPLES * 1];  break;
            case 1 : seq = 2;   pAudio_to_fill = &audio_buffer[AUDIO_FRAME_NSAMPLES * 2];  break;
            case 2 : seq = 3;   pAudio_to_fill = &audio_buffer[AUDIO_FRAME_NSAMPLES * 3];  break;
            default: seq = 0;   pAudio_to_fill = &audio_buffer[AUDIO_FRAME_NSAMPLES * 0];  break;
        }
        sequence_fill = seq;

        // release the uSD mutex
        tx_mutex_put(&usd_mutex);

        // check the operation result
        if ((uint8_t) FX_END_OF_FILE == fx_res)
        {
            // premature end of file --> stop playing the current message
            remaining_nSamples_to_read = remaining_nSamples_to_play = 0;
        }
        else if ((uint8_t) FX_SUCCESS != fx_res)
        {
            // an error is detected in the file --> stop playing the current message
            remaining_nSamples_to_read = remaining_nSamples_to_play = 0;
        }
    }
}

/******************************************************************************
 ** Name:    Audio_Play_Message
 *****************************************************************************/
/**
 ** @brief   Function that loads and plays a message
 **
 ** @param   msg_id      Message identifier
 **
 ** @return  void
 ******************************************************************************/
static void Audio_Play_Message (AUDIO_ID_e msg_id)
{
    uint8_t  size_bytes[WAV_DATA_SIZE_BYTES];   // Audio Data Size Read buffer
    uint32_t nBytes;                            // number of read bytes from the file
    uint8_t  fx_res;                            // Stores the fileX operations result

    audio_playing_msg_id = msg_id;

    // get the mutex to access to the uSD
    tx_mutex_get(&usd_mutex, TX_WAIT_FOREVER);

    // Set the file offset to ignore all the wave file headers, and point directly to the data size
    fx_file_seek (&aFile[audio_playing_msg_id], WAV_HEADER_OFFSET);

    // Read audio raw data length
    fx_res = (uint8_t) fx_file_read (&aFile[audio_playing_msg_id], size_bytes, WAV_DATA_SIZE_BYTES, &nBytes);

    // release the uSD mutex
    tx_mutex_put(&usd_mutex);

    if ((uint8_t) FX_SUCCESS == fx_res)
    {
        // Convert little endian to big endian 4 byte integer, to know the number of bytes in the audio message
        nBytes = ((uint32_t) size_bytes[0])       + ((uint32_t) size_bytes[1] << 8) +
                 ((uint32_t) size_bytes[2] << 16) + ((uint32_t) size_bytes[3] << 24);
        remaining_nSamples_to_read = remaining_nSamples_to_play = (int32_t) (nBytes / sizeof(uint16_t));

        // restart the fill controllers/counters before read the first 4 audio frames
        sequence_fill = 0;
        pAudio_to_fill = audio_buffer;
        Get_Next_Audio_Samples ();
        Get_Next_Audio_Samples ();
        Get_Next_Audio_Samples ();
        Get_Next_Audio_Samples ();

        fx_res = (uint8_t) tx_event_flags_set (&g_events_Audio, SYNC_AUDIO_PLAY_EVENT, TX_OR);
    }
}

/******************************************************************************
 ** Name:    Stop_Audio_Engine
 *****************************************************************************/
/**
 ** @brief   Stops the audio engine in a predefined sequence
 **
 ******************************************************************************/
static void Stop_Audio_Engine (void)
{
    // WARNING: The transfer will stop after completion of the current transfer !!!
    Audio_Stop();

    // initialize some internal variables
    remaining_nSamples_to_play = 0;
    audio_dma_free = 0;
    sequence_play  = 0;
    pAudio_to_play = audio_buffer;

    // restart the message identifiers !!!
    memset (concat_queue, eAUDIO_MAX_ID, AUDIO_PLAY_QUEUE_SZ);
    audio_playing_msg_id = eAUDIO_MAX_ID;
}

/******************************************************************************
 ** Name:    Manage_Audio_Queue
 *****************************************************************************/
/**
 ** @brief   Manages the audio queue
 **
 ******************************************************************************/
static void Manage_Audio_Queue (void)
{
    if (remaining_nSamples_to_play <= 0)
    {
        // release the semaphore
        tx_semaphore_put (&audio_semaphore);

        // Update the queue FOR NEXT MESSAGE
        memcpy (&concat_queue[0], &concat_queue[1], AUDIO_PLAY_QUEUE_SZ - 1);
        concat_queue [AUDIO_PLAY_QUEUE_SZ - 1] = eAUDIO_MAX_ID;
        if (concat_queue [0] == eAUDIO_MAX_ID)
        {
            // WARNING: No transfers are programmed !!!
            Audio_Stop();
        }
        else
        {
            Audio_Play_Message((AUDIO_ID_e) concat_queue[0]);
        }
    }
}

/******************************************************************************
** Name:    Audio_Message
*****************************************************************************/
/**
** @brief   Function that performs an operation with the message player
**
** @param   cmd         Command to execute
** @param   msg_id      Message identifier
** @param   continue    player must be continued
**
** @return  Operation result (OK or error code)
**
******************************************************************************/
ERROR_ID_e Audio_Message (AUDIO_CMD_e cmd, AUDIO_ID_e msg_id, bool_t continue_player)
{
    ERROR_ID_e retCode = OK;
    static bool_t no_message = false;

    if (flag_error_in_audio) return 6;

    // If we are trying to play an invalid message ignore it and return as OK
    if (msg_id >= eAUDIO_MAX_ID)
    {
        return 7;
    }

    // if REPLACE MESSAGE is playing do not admit more messages
    if(no_message == true)
    {
        return 8;
    }

    if (continue_player == false) no_message = true;

    // process the message to play attending to the selected format (long vs. short)
    if (pAed_Settings->aed_cpr_msg_long == 0)
    {
        // when short messages are selected, some of them must be ignored and return OK !!!
        if( (msg_id == eAUDIO_KEEP_CALM) ||
            (msg_id == eAUDIO_FOLLOW_INSTRUCTIONS) ||
            (msg_id == eAUDIO_ASK_FOR_HELP) ||
            (msg_id == eAUDIO_REMOVE_CHEST_CLOTHES) ||
            (msg_id == eAUDIO_OPEN_ELECTRODE_PACKAGE) ||
            (msg_id == eAUDIO_CPR_COMPRESSIONS) ||
            (msg_id == eAUDIO_CPR_BREATH))
        {
            return 9;
        }

        // when short messages are selected, some of them must be changed to the short format !!!
        if(msg_id == eAUDIO_CPR_START)
        {
            msg_id = eAUDIO_CPR_START_SHORT;
        }
    }

    // some messages can not be interrupted. Then, convert the PLAY command to CONCATENATE !!!
    if (((AUDIO_ID_e) audio_playing_msg_id == eAUDIO_LOW_BATTERY) ||
        ((AUDIO_ID_e) audio_playing_msg_id == eAUDIO_REPLACE_BATTERY) ||
        ((AUDIO_ID_e) audio_playing_msg_id == eAUDIO_CALL_SAT))
    {
        if (cmd == eAUDIO_CMD_PLAY) cmd = eAUDIO_CMD_CONCAT;
    }

    if(RCP_running == true && msg_id == eAUDIO_APPLY_ELECTRODES_CHEST)
    {
        cmd = eAUDIO_CMD_PLAY;
        RCP_running = false;
    }

    // process the command
    switch (cmd)
    {
        case eAUDIO_CMD_PLAY:
            Stop_Audio_Engine ();
            concat_queue [0] = msg_id;
            Audio_Play_Message(msg_id);
            break;
        case eAUDIO_CMD_CONCAT:
            // concatenate the new message
                 if (concat_queue[0] == eAUDIO_MAX_ID) concat_queue[0] = msg_id;
            else if (concat_queue[1] == eAUDIO_MAX_ID) concat_queue[1] = msg_id;
            else if (concat_queue[2] == eAUDIO_MAX_ID) concat_queue[2] = msg_id;
            else if (concat_queue[3] == eAUDIO_MAX_ID) concat_queue[3] = msg_id;
            else if (concat_queue[4] == eAUDIO_MAX_ID) concat_queue[4] = msg_id;
            else if (concat_queue[5] == eAUDIO_MAX_ID) concat_queue[5] = msg_id;
            else retCode = eERR_AUDIO_QUEUE_FULL;

            // if the new message is the ONLY in the queue, play the message immediately
            if (concat_queue[1] == eAUDIO_MAX_ID)
            {
                Stop_Audio_Engine ();
                concat_queue[0] = msg_id;
                Audio_Play_Message(msg_id);
            }
            break;
        case eAUDIO_CMD_STOP:
            Stop_Audio_Engine ();
            break;
        default:
            break;
    }

    return retCode;
}

extern void setFlag_error_in_audio(bool_t value){
    flag_error_in_audio = value;
}



/******************************************************************************
** Name:    Audio_Test
*****************************************************************************/
/**
** @brief   Function that executes the audio test
**
** @param   msg_id      audio message to play during the test
**
** @return  test result
**
******************************************************************************/
ERROR_ID_e Audio_Test (const AUDIO_ID_e msg_id, bool_t msg)
{
    uint32_t        audio_energy_1 = 0; // energy when not playing
    uint32_t        audio_energy_2 = 0; // energy when playing
    uint32_t        i = 0;

    tx_thread_sleep (OSTIME_40MSEC);
    // get the audio energy to get the ambient noise ...
    audio_energy_1 = Get_Audio_Energy ();
    // Trace_Arg (TRACE_NEWLINE, "audio_energy_1: = %10d", audio_energy_1);

    // play a message and check that audio energy is detected !!!
    Audio_Message (eAUDIO_CMD_PLAY, msg_id, TRUE);
    do
    {
        i++;
        audio_energy_2 += Get_Audio_Energy ();
        tx_thread_sleep (OSTIME_100MSEC);
    }while (Is_Audio_Playing());
    if (queue_core.tx_queue_available_storage == 4)tx_queue_flush(&queue_core);

    // get the audio energy when playing the audio message ...
    audio_energy_2 /= i;
//Trace_Arg (TRACE_NEWLINE, "audio_energy_1: = %10d", audio_energy_1);
//Trace_Arg (TRACE_NEWLINE, "audio_energy_2: = %10d", audio_energy_2);
//Trace_Arg (TRACE_NEWLINE, "i: = %10d", i);

    if (Is_Test_Mode() && (msg == TRUE))
    {
        Audio_Message (eAUDIO_CMD_CONCAT, eAUDIO_MODE_TEST, TRUE);
    }

    // verify that the player increases the ambient noise
    if (audio_energy_1 < 2000)      // do not perform test if back noise is high
    {
        if((audio_energy_2 < 2000) || (audio_energy_2 < audio_energy_1))
        {
 //it is not an error           return eERR_AUDIO_PLAYER;
        }
    }

    return eERR_NONE;
}

/******************************************************************************
 ** Name:    Is_Audio_Playing
 *****************************************************************************/
/**
 ** @brief   ask if the audio player is running
 **
 ** @return  true (playing) or false (stop)
 **
 ******************************************************************************/
bool_t Is_Audio_Playing(void)
{
    if (flag_error_in_audio) return false;
    // the audio is playing if the semaphore is in use ...
//    return ((audio_semaphore.tx_semaphore_count == 0) ? true : false);
    return (((audio_semaphore.tx_semaphore_count == 0) || (concat_queue [0] != eAUDIO_MAX_ID)) ? true : false);
}

/******************************************************************************
 ** Name:    Is_Audio_Playing_Last
 *****************************************************************************/
/**
 ** @brief   ask if the audio player is running playing the last audio message
 **          verifies that there are not more messages in the queue
 **
 ** @return  true (playing last) or false (stop or more messages queued)
 **
 ******************************************************************************/
bool_t Is_Audio_Playing_Last (void)
{
    // check if it is playing the last message, don not check the current message
    return ((/*(concat_queue [0] != eAUDIO_MAX_ID) && */(concat_queue [1] == eAUDIO_MAX_ID)) ? true : false);
}

/******************************************************************************
 ** Name:    Set_Audio_Player_in_error
 *****************************************************************************/
/**
 ** @brief   set in error
 **
 ** @return  none
 **
 ******************************************************************************/
void Set_Audio_Player_in_error (void)
{
    flag_error_in_audio = true;
}

/******************************************************************************
 ** Name:    thread_audio_entry
 *****************************************************************************/
/**
 ** @brief   A description of the function
 ** @param   void
 **
 ** @return  void
 ** @todo    pending things to be done
 ******************************************************************************/
void thread_audio_entry(void)
{
#define PERIOD_ADJUST_AUDIO_VOLUME      10                  // 10 * 20 msecs

    uint32_t    my_counter = 0;         // counter to wait the volume adjust
    uint32_t    my_flags;
    uint8_t     my_volume;

#if (AUDIO_OUT_SHOW_BIOMETRICS == 1)

    // no audio in the machine !!!
    while (1)
    {
        tx_thread_sleep (OSTIME_100MSEC);
    }

#else

    //Wait till volume setting is initialized
    while (Get_Misc_Settings()->glo_volume == 0)
    {
        tx_thread_sleep (OSTIME_100MSEC);
    }
    tx_thread_sleep (OSTIME_1SEC);

    // Initialize the audio engine
    Audio_Init();

    // assign the volume audio at thread start
 /*   if (Is_Test_Mode())
    {
        my_volume = AUDIO_VOLUME_DEFAULT;
    }
    else*/ if (Is_Test_Mode_Montador())
    {
        my_volume = AUDIO_VOLUME_DEFAULT_MONTADOR;
    }
    else
    {
        // empirical adjust to get a nice audio volume
        my_volume = (uint8_t) ((Get_Misc_Settings()->glo_volume * 3) + 25);
    }

    // set at configured volume !!!
    audio_volume = my_volume;
    Audio_Volume_Set(my_volume);

    flag_audio_init = true;

#endif
/*
    while (1)
    {
        my_counter++;
        tx_event_flags_get (&g_events_Audio, SYNC_AUDIO_ANY_EVENT, TX_OR_CLEAR, &my_flags, OSTIME_20MSEC);

        if (my_flags & SYNC_AUDIO_PLAY_EVENT)
        {
            // start playing a new message
            // --> the audio engine is stopped
            // --> the audio buffer is filled or preloaded with 3 audio frames !!!
            tx_semaphore_get (&audio_semaphore, TX_NO_WAIT);

            my_flags &= (uint32_t) (~SYNC_AUDIO_PLAY_EVENT);
            audio_dma_free = 0;
            sequence_play  = 0;
            pAudio_to_play = &audio_buffer[0];
            Audio_Play_Start (pAudio_to_play, AUDIO_FRAME_NSAMPLES);
        }

        if (audio_dma_free)
        {
           audio_dma_free--;
           Get_Next_Audio_Samples ();
           Manage_Audio_Queue ();
        }

        // adapt the audio volume to the ambient noise ...
        if ( // (Is_Test_Mode() == FALSE)      &&
            (Is_Test_Mode_Montador() == FALSE) &&
            (Get_Device_Info()->develop_mode != DEVELOP_MANUFACTURE_CONTROL) &&
            ((my_counter % PERIOD_ADJUST_AUDIO_VOLUME) == 0))
        {
            Adjust_Audio_Volume (my_volume);
        }
    }
*/
}
