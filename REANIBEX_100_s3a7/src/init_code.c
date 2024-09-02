/******************************************************************************
 * Name      : R100_S3A7                                                      *
 * Copyright : Osatu S. Coop                                                  *
 * Compiler  : GNU ARM Embedded                                               *
 * Target    : Reanibex Serie 100                                             *
 ******************************************************************************/

/*!
 * @file        init_code.c
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

/* This include file is need only for the 'temporary' fix to insure that the Ioport reference counter is zeroed before it
 * gets referenced. Ioport init is currently called before the C Runtime initialization takes place.
 * It will be removed when a more complete solution for this problem is added.
 */
#include "../../../src/driver/r_ioport/hw/hw_ioport_private.h"

/* BSP Pin Configuration. Allocate BSP pin configuration table in this module. */
#include "bsp_pin_cfg.h"

#include "hal_data.h"
#include "types_basic.h"
#include "init_code.h"
#include "Keypad.h"

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

/***********************************************************************************************************************
 * Function Name: R_BSP_WarmStart
 * Description  : This function is called at various points during the startup process. This function is declared as a
 *                weak symbol higher up in this file because it is meant to be overridden by a user implemented version.
 *                One of the main uses for this function is to call functional safety code during the startup process.
 *                To use this function just copy this function into your own code and modify it to meet your needs.
 * Arguments    : event -
 *                    Where at in the start up process the code is currently at
 * Return Value : none
 ***********************************************************************************************************************/
void R_BSP_WarmStart(bsp_warm_start_event_t event)
{
    if (BSP_WARM_START_PRE_C == event)
    {
        // C runtime environment has not been setup so you cannot use globals. System clocks and pins are not setup
    }
    else if (BSP_WARM_START_POST_C == event)
    {
        // C runtime environment, system clocks, and pins are all setup

        // premature turn on the life-beat (ASAP)
        // Led_On (LED_ONOFF);

        // Used initialize SCE Crypto API
        // Identify the power-up source
        /*poweron_event = Identify_PowerOn_Event();

        if(poweron_event != PON_RTC)
        {
            // Initialize SCE Crypto API
            g_sce.p_api->open(g_sce.p_ctrl, g_sce.p_cfg);
        }*/
    }
    else
    {
        /* Unhandled case. */
    }
}
