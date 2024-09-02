/******************************************************************************
 * Name      : R100_S3A7                                                      *
 * Copyright : Osatu S. Coop                                                  *
 * Compiler  : GNU ARM Embedded                                               *
 * Target    : Reanibex Serie 100                                             *
 ******************************************************************************/

/*!
 * @file        thread_audio_hal.c
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

#include "HAL/thread_audio_hal.h"
#include "thread_audio.h"
//#include "thread_audio_entry.h"

#include "types_basic.h"
#include "Trace.h"
#include "I2C_2.h"

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
 ** Name:    DMA_Shot
 *****************************************************************************/
/**
 ** @brief   program the DMA to transfer audio data samples
 **
 ** @param   pSamples pointer to audio samples
 ** @param   nSamples number of samples to transfer
 **
 ******************************************************************************/
void DMA_Shot (uint16_t *pSamples, uint16_t nSamples)
{
    dma_audio.p_api->close (dma_audio.p_ctrl);

    dma_audio.p_cfg->p_info->p_src = pSamples;
    dma_audio.p_cfg->p_info->length = nSamples;
    dma_audio.p_api->open (dma_audio.p_ctrl, dma_audio.p_cfg);
    dma_audio.p_api->enable (dma_audio.p_ctrl);
}

/******************************************************************************
 ** Name:    Audio_init
 *****************************************************************************/
/**
 ** @brief   initializes the audio engine. Assume that all messages are sampled
 **          at 11,025 Ksps with 12 bits resolution
 **
 ******************************************************************************/
int Audio_Init(void)
{
    ssp_err_t error, result;

    error = dac_audio.p_api->open (dac_audio.p_ctrl, dac_audio.p_cfg);

    result = error;

    error = timer_audio.p_api->open (timer_audio.p_ctrl, timer_audio.p_cfg);
    if(error != SSP_SUCCESS && result == SSP_SUCCESS)result = error;

    tx_semaphore_ceiling_put (&audio_semaphore, 1);
    tx_mutex_put(&usd_mutex);

    // set the audio player mute and energized ...
    error = g_ioport.p_api->pinWrite (AUDIO_PLAYER_MUTE, IOPORT_LEVEL_HIGH);
    if(error != SSP_SUCCESS && result == SSP_SUCCESS)result = error;

    error = g_ioport.p_api->pinWrite (AUDIO_PLAYER_SHDN, IOPORT_LEVEL_HIGH);
    if(error != SSP_SUCCESS && result == SSP_SUCCESS)result = error;

    return result;
}

/******************************************************************************
 ** Name:    Audio_Play
 *****************************************************************************/
/**
 ** @brief   plays a single audio message
 **
 ** @param   pMsg  pointer to the message data samples
 ** @param   size  number of samples to transfer
 **
 ******************************************************************************/
int Audio_Play_Start (const uint16_t *pSamples, uint16_t nSamples)
{
    ssp_err_t err, result;

    if (pSamples && nSamples)
    {
        // set the audio player running
        Audio_Mute_Off();

        // program the DMA, the DAC and the timer to play the audio message
        DMA_Shot ((uint16_t *) pSamples, nSamples);
        result = dac_audio.p_api->start (dac_audio.p_ctrl);

        err = timer_audio.p_api->start (timer_audio.p_ctrl);
        if(result == SSP_SUCCESS && err != SSP_SUCCESS){
            result = err;
        }
    }
    return result;
}

/******************************************************************************
 ** Name:    Audio_Stop
 *****************************************************************************/
/**
 ** @brief   stops the audio reproduction
 **
 ** @param   pMsg  pointer to the message data
 ** @param   size  number of samples of the audio message
 **
 ******************************************************************************/
void Audio_Stop(void)
{
    transfer_properties_t dma_info;

    // disable the DMA used to play audio messages
    dma_audio.p_api->stop (dma_audio.p_ctrl);
    dma_audio.p_api->disable (dma_audio.p_ctrl);

    // WARNING: The current transfer can not be aborted
    // Real stop is effective after completion of the current transfer !!!
    do {
        dma_audio.p_api->infoGet (dma_audio.p_ctrl, &dma_info);
        tx_thread_sleep (OSTIME_20MSEC);
    } while (dma_info.in_progress);

    // write the half scale into the audio port !!!
    dac_audio.p_api->write (dac_audio.p_ctrl, 0x4000);
}

/******************************************************************************
 ** Name:    Audio_Mute
 *****************************************************************************/
/**
 ** @brief   mute the audio engine
 **
 ******************************************************************************/
void Audio_Mute(void)
{
    g_ioport.p_api->pinWrite (AUDIO_PLAYER_MUTE, IOPORT_LEVEL_HIGH);
    dac_audio.p_api->close (dac_audio.p_ctrl);
}

/******************************************************************************
 ** Name:    Audio_Mute_Off
 *****************************************************************************/
/**
 ** @brief   un-mute the audio engine
 **
 ******************************************************************************/
void Audio_Mute_Off(void)
{

    dac_audio.p_api->open (dac_audio.p_ctrl, dac_audio.p_cfg);
    dac_audio.p_api->start (dac_audio.p_ctrl);
    g_ioport.p_api->pinWrite (AUDIO_PLAYER_MUTE, IOPORT_LEVEL_LOW);
}

/******************************************************************************
 ** Name:    Audio_Volume_Set
 *****************************************************************************/
/**
 ** @brief   set playing audio volume
 **
 ******************************************************************************/
void Audio_Volume_Set(uint8_t volume)
{
    if (volume > MAX_AUDIO_VOLUME)
    {
        volume = MAX_AUDIO_VOLUME;
    }

    if (volume < MIN_AUDIO_VOLUME)
    {
        volume = MIN_AUDIO_VOLUME;
    }

    // set the audio volume to the requested value ...
    I2C2_WriteByte (i2C_ADD_AUDIO, volume);
}
