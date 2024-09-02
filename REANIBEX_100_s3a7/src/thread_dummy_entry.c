/******************************************************************************
 * Name      : R100_S3A7                                                      *
 * Copyright : Osatu S. Coop                                                  *
 * Compiler  : GNU ARM Embedded                                               *
 * Target    : Reanibex Serie 100                                             *
 ******************************************************************************/

/*!
 * @file        thread_dummy_entry.c
 * @brief
 *
 * @version     v1
 * @date        12/12/2019
 * @author      ilazkanoiturburu
 * @warning     author   date    version     .- Edition
 * @bug
 *
 */

/******************************************************************************
 **Includes
 */
#include "thread_dummy.h"

/* Thread Dummy entry function */
void thread_dummy_entry(void)
{
    /* TODO: add your own code here */
    while (1)
    {
        g_ioport.p_api->pinWrite (IOPORT_PORT_06_PIN_09, IOPORT_LEVEL_LOW);
        g_ioport.p_api->pinWrite (IOPORT_PORT_06_PIN_09, IOPORT_LEVEL_HIGH);
//      tx_thread_sleep (1);
    }
}
