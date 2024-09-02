/******************************************************************************
 * Name      : R100_S3A7                                                      *
 * Copyright : Osatu S. Coop                                                  *
 * Compiler  : GNU ARM Embedded                                               *
 * Target    : Reanibex Serie 100                                             *
 ******************************************************************************/

/*!
 * @file        Keypad.c
 * @brief       All functions related to the keyboard and LEDs manage
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

#include "Keypad.h"
#include "types_app.h"
#include "types_basic.h"
#include "hal_data.h"

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

const uint16_t led_io_port [] = {
    IOPORT_PORT_07_PIN_08,          ///< LED_ONOFF
    IOPORT_PORT_04_PIN_14,          ///< LED_SHOCK
    IOPORT_PORT_03_PIN_06           ///< LED_PAT_TYPE
};

/******************************************************************************
 ** Externals
 */

/******************************************************************************
 ** Globals
 */

/******************************************************************************
 ** Locals
 */

static uint32_t blinker[LED_MAX];

/******************************************************************************
 ** Prototypes
 */

/******************************************************************************
 ** Name:    Led_On
 *****************************************************************************/
/**
 ** @brief   Turn on the led indicated by index
 ** @param   led_idx       Index of the led to turn on
 **
 ******************************************************************************/
void Led_On(LED_IDENT_e led_idx)
{
    g_ioport.p_api->pinWrite (led_io_port[led_idx], LED_VALUE_ON);
    blinker [led_idx] = 0;
}

/******************************************************************************
 ** Name:    Led_Off
 *****************************************************************************/
/**
 ** @brief   Turn off the led indicated by index
 ** @param   led_idx       Index of the led to turn off
 **
 ******************************************************************************/
void Led_Off(LED_IDENT_e led_idx)
{
    g_ioport.p_api->pinWrite (led_io_port[led_idx], LED_VALUE_OFF);
    blinker [led_idx] = 0;
}

/******************************************************************************
 ** Name:    Led_Blink
 *****************************************************************************/
/**
 ** @brief   Blink the led indicated by index
 ** @param   led_idx       Index of the led to blink
 **
 ******************************************************************************/
void Led_Blink(LED_IDENT_e led_idx)
{
    blinker [led_idx] = led_io_port[led_idx];
}

/******************************************************************************
 ** Name:    Led_Toggle
 *****************************************************************************/
/**
 ** @brief   Toggle the led indicated by index
 ** @param   led_idx       Index of the led to toggle
 **
 ******************************************************************************/
void Led_Toggle(LED_IDENT_e led_idx)
{
    ioport_level_t led_level;

    g_ioport.p_api->pinRead (led_io_port[led_idx], &led_level);
    led_level = (led_level == LED_VALUE_ON) ? LED_VALUE_OFF: LED_VALUE_ON;
    g_ioport.p_api->pinWrite (led_io_port[led_idx], led_level);
    blinker [led_idx] = 0;
}

/******************************************************************************
 ** Name:    Led_Ongoing_Transmission
 *****************************************************************************/
/**
 ** @brief   LED animation for transmission
 **
 ******************************************************************************/
void Led_Ongoing_Transmission (void)
{
    static uint8_t idx = 0;

    switch (idx)
    {
        case 0: Led_Toggle(LED_ONOFF);  break;
        case 1: Led_Toggle(LED_PATYPE); break;
        case 2: Led_Toggle(LED_SHOCK);  break;
        default: break;
    }
    (idx >=2 ) ? idx = 0 : idx++;
}

/******************************************************************************
 ** Name:    Blink_Refresh
 *****************************************************************************/
/**
 ** @brief   Refresh the blinking leds
 ** @param   none
 **
 ******************************************************************************/
void Blink_Refresh (void)
{
    static uint8_t free_counter;
    uint8_t        i;
    ioport_level_t led_level;

    free_counter++;
    led_level = (free_counter & 0x01) ? LED_VALUE_ON: LED_VALUE_OFF;

    // set/reset the blinking LEDs
    for (i=0; i<LED_MAX; i++)
    {
        if (blinker[i]) g_ioport.p_api->pinWrite (blinker[i], led_level);
    }
}

/******************************************************************************
 ** Name:    Key_Read
 *****************************************************************************/
/**
 ** @brief   Read from Key-board
 ** @param   key_idx       Key to check
 **
 ** @return  KEY_STATUS_e
 ******************************************************************************/
KEY_STATUS_e Key_Read(uint32_t key_idx)
{
    KEY_STATUS_e status;
    ioport_level_t key_level;

    g_ioport.p_api->pinRead (key_idx, &key_level);
    status = (key_level) ? KEY_STATUS_OFF : KEY_STATUS_ON;
    return status;
}
