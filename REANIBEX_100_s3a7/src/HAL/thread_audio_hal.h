/******************************************************************************
 * Name      : R100_S3A7                                                      *
 * Copyright : Osatu S. Coop                                                  *
 * Compiler  : GNU ARM Embedded                                               *
 * Target    : Reanibex Serie 100                                             *
 ******************************************************************************/

/*!
 * @file        thread_audio_hal.h
 * @brief       Header with functions related to the audio BSP service
 *
 * @version     v1
 * @date        16/01/2018
 * @author      ltorres
 * @warning     author   date    version     .- Edition
 * @bug
 *
 */

#ifndef THREAD_AUDIO_HAL_H_
#define THREAD_AUDIO_HAL_H_

/******************************************************************************
 **Includes
 */

#include "types_basic.h"

/******************************************************************************
 ** Defines
 */

#define MAX_AUDIO_VOLUME            63      ///< Maximum Audio volume value
#define MIN_AUDIO_VOLUME            25      ///< Minimum Audio volume value


// I/O ports
#define     AUDIO_PLAYER_SHDN   IOPORT_PORT_05_PIN_01
#define     AUDIO_PLAYER_MUTE   IOPORT_PORT_05_PIN_00


/******************************************************************************
 ** Typedefs
 */

/******************************************************************************
 ** Globals
 */

/******************************************************************************
 ** Prototypes
 */

int        Audio_Init              (void);
int        Audio_Play_Start        (const uint16_t *pSamples, uint16_t nSamples);
void        Audio_Stop              (void);
void        Audio_Mute              (void);
void        Audio_Mute_Off          (void);
void        Audio_Volume_Set        (uint8_t volume);
void        DMA_Shot                (uint16_t *pSamples, uint16_t nSamples);

#endif /* THREAD_AUDIO_HAL_H_ */

/*
 ** $Log$
 **
 ** end of thread_audio_hal.h
 ******************************************************************************/
