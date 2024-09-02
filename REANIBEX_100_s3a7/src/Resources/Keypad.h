/******************************************************************************
 * Name      : R100_S3A7                                                      *
 * Copyright : Osatu S. Coop                                                  *
 * Compiler  : GNU ARM Embedded                                               *
 * Target    : Reanibex Serie 100                                             *
 ******************************************************************************/

/*!
 * @file        Keypad.h
 * @brief       Header with functions related to the keyboard and LEDs manage
 *
 * @version     v1
 * @date        01/02/2018
 * @author      ltorres
 * @warning     author   date    version     .- Edition
 * @bug
 *
 */

#ifndef KEYPAD_H_
#define KEYPAD_H_

/******************************************************************************
 **Includes
 */

#include "types_basic.h"
#include "types_app.h"

/******************************************************************************
 ** Defines
 */

#define     LED_VALUE_OFF       IOPORT_LEVEL_LOW
#define     LED_VALUE_ON        IOPORT_LEVEL_HIGH

#define     KEY_SHOCK           IOPORT_PORT_02_PIN_00
#define     KEY_PATYPE          IOPORT_PORT_03_PIN_07
#define     KEY_ONOFF           IOPORT_PORT_04_PIN_15
#define     KEY_COVER           IOPORT_PORT_04_PIN_04

#define     VBUS_DET            IOPORT_PORT_04_PIN_07

/******************************************************************************
 ** Typedefs
 */

typedef enum
{
    KEY_STATUS_OFF = 0,
    KEY_STATUS_ON
} KEY_STATUS_e;

typedef enum
{
    LED_ONOFF = 0,
    LED_SHOCK,
    LED_PATYPE,
//  LED_LIFE_BEAT,          // use the same led as ON-OFF and as Life beat
    LED_MAX
} LED_IDENT_e;

/******************************************************************************
 ** Globals
 */

/******************************************************************************
 ** Prototypes
 */

void            Led_On          (LED_IDENT_e led_idx);
void            Led_Off         (LED_IDENT_e led_idx);
void            Led_Blink       (LED_IDENT_e led_idx);
void            Led_Toggle      (LED_IDENT_e led_idx);
void            Led_Ongoing_Transmission    (void);
void            Blink_Refresh   (void);

KEY_STATUS_e    Key_Read        (uint32_t key_idx);

#endif /* KEYPAD_H_ */

/*
 ** $Log$
 **
 ** end of Keypad.h
 ******************************************************************************/
