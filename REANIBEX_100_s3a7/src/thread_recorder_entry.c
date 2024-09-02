/******************************************************************************
 * Name      : R100_S3A7                                                      *
 * Copyright : Osatu S. Coop                                                  *
 * Compiler  : GNU ARM Embedded                                               *
 * Target    : Reanibex Serie 100                                             *
 ******************************************************************************/

/*!
 * @file        thread_recorder_entry.c
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
#include "thread_recorder.h"
#include <stdio.h>
#include <inttypes.h>

#include "Trace.h"
#include "event_ids.h"
#include "types_basic.h"
#include "DB_Episode.h"
#include "thread_drd_entry.h"

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
 ** Name:    str_to_int
 *****************************************************************************/
/**
 ** @brief   Converts a string in decimal system into an integer
 ** @param   p_result pointer to result
 ** @param   p_str pointer to string to convert
 ** @param   p_str_size pointer to convert string size
 **
 ******************************************************************************/
void str_to_int(uint32_t *p_result, CHAR const *p_str, uint8_t p_str_size);

void str_to_int(uint32_t *p_result, CHAR const *p_str, uint8_t p_str_size)
{
    uint8_t digit;
    *p_result = 0;

    /* Convert the string */
    while (p_str_size--)
    {
        digit = (uint8_t) (*p_str - '0');
        *p_result *= 10u;
        *p_result += digit;
        ++p_str;
    }
}


/******************************************************************************
 ** Name:    thread_recorder_entry
 *****************************************************************************/
/**
 ** @brief   A description of the function
 ** @param   void
 **
 ******************************************************************************/
void thread_recorder_entry(void)
{
    bool_t  episode_ready;      // flag to indicate that the episode is being registered

    while (Is_Sysmon_Task_Initialized() == false)
    {
        tx_thread_sleep (OSTIME_100MSEC);
    }

    if (Is_Battery_Mode_Demo()) return;    // Do not create episode in DEMO mode
    // Create an episode or overwrite if exist
    episode_ready = DB_Episode_Create();

    while (1)
    {
        if (episode_ready)
        {
            episode_ready = DB_Episode_Write();
        }
        else {
            // thread Loop
            tx_thread_sleep (OSTIME_100MSEC);
        }
    }
}
