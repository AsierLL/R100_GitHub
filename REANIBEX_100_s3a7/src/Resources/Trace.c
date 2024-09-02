/******************************************************************************
 * Name      : R100_S3A7                                                      *
 * Copyright : Osatu S. Coop                                                  *
 * Compiler  : GNU ARM Embedded                                               *
 * Target    : Reanibex Serie 100                                             *
 ******************************************************************************/

/*!
 * @file        Trace.c
 * @brief       All functions related to the output trace through serial port
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
#include <tx_api.h>
#include <time.h>
#include <stdio.h>
#include <string.h>

#include "Trace.h"
#include "RTC.h"
#include "types_basic.h"
#include "hal_data.h"

/******************************************************************************
 ** Macros
 */

/******************************************************************************
 ** Defines
 */

#define TRACE_ON

/******************************************************************************
 ** Typedefs
 */

/******************************************************************************
 ** Constants
 */

/******************************************************************************
 ** Externals
 */

extern TX_SEMAPHORE trace_semaphore;

/******************************************************************************
 ** Globals
 */

/******************************************************************************
 ** Locals
 */

static char trace_buffer[256];              ///< Buffer used to write traces
static bool rx_data_ready;                  ///< Flag to indicate that a character has arrived in the RX port

/******************************************************************************
 ** Prototypes
 */

/******************************************************************************
 ** Name:    user_uart_callback
 *****************************************************************************/
/**
 ** @brief   Implements the callback of the UART
 ** @param   p_args       UART callback arguments
 **
 ** @return  void
 ** @todo    pending things to be done
 ******************************************************************************/
void trace_uart_callback(uart_callback_args_t *p_args)
{
    // release the semaphore when the current frame has been printed
    if (p_args->event == UART_EVENT_TX_COMPLETE)
    {
//        if (trace_semaphore.tx_semaphore_count == 0) tx_semaphore_put (&trace_semaphore);
    }
    else if (p_args->event == UART_EVENT_RX_COMPLETE)
    {
        rx_data_ready = true;
    }
    else if (p_args->event == UART_EVENT_RX_CHAR)
    {
        rx_data_ready = true;
    }
}

/******************************************************************************
 ** Name:    Trace_init
 *****************************************************************************/
/**
 ** @brief   Open connection SCI1 (UART). Initialize semaphore
 ** @param   void
 **
 ** @return  void
 ** @todo    pending things to be done
 ******************************************************************************/
void Trace_Init(void)
{
    if (traceUART.p_api->open (traceUART.p_ctrl, traceUART.p_cfg) == SSP_SUCCESS)
    {
        tx_semaphore_ceiling_put (&trace_semaphore, 1);
    }
}

/******************************************************************************
 ** Name:    Trace
 *****************************************************************************/
/**
 ** @brief   Print a trace string inserting the time stamp
 **
 ** @param   flags         Flags to print more information (Time or CRLF)
 ** @param   pString       Pointer to trace string
 **
 ** @return  void
 ** @todo    pending things to be done
 ******************************************************************************/
void Trace(uint8_t flags, const char *pString)
{
    rtc_time_t  current_time;            // Current time
    uint32_t    sz_str;                    // String size
    uint8_t     my_buffer[4];           // local buffer

#ifndef TRACE_ON
    return;
#endif

    if (flags & TRACE_DISABLED) return;

    // get the trace port
    tx_semaphore_get (&trace_semaphore, TX_WAIT_FOREVER);

    // open the trace engine in the Boot processor
    my_buffer[0] = 'T';
    my_buffer[1] = '#';
    traceUART.p_api->write (traceUART.p_ctrl, my_buffer, 2);

    if (flags & TRACE_TIME_STAMP)
    {
        //insert the time stamp ...
        iRTC.p_api->calendarTimeGet (iRTC.p_ctrl, &current_time);
        sprintf (trace_buffer, "%02d:%02d:%02d --> ", current_time.tm_hour, current_time.tm_min, current_time.tm_sec);
    }
    else
    {
        //clean buffer
        trace_buffer[0] = '\0';
    }

    // Copy the string to send to the terminal
    strcat (trace_buffer, pString);
    sz_str = strlen (trace_buffer);

    if (flags & TRACE_NEWLINE)
    {
        //Insert the CR+LF if flag is defined ...
        trace_buffer[sz_str++] = '\n';
        trace_buffer[sz_str++] = '\r';
        trace_buffer[sz_str  ] = 0x00;
    }

    //Transmit trace buffer through UART
    traceUART.p_api->write (traceUART.p_ctrl, (uint8_t *) trace_buffer, sz_str+1);
    tx_thread_sleep ((uint16_t)OSTIME_10MSEC);

    tx_semaphore_put (&trace_semaphore);
}

/******************************************************************************
 ** Name:    Trace_Arg
 *****************************************************************************/
/**
 ** @brief   Print a trace string inserting the time stamp
 **
 ** @param   flags         Flags to print more information (Time or CRLF)
 ** @param   pString       Pointer to trace string
 ** @param   aux_code      Additional parameter or code
 **
 ** @return  void
 ** @todo    pending things to be done
 ******************************************************************************/
void Trace_Arg(uint8_t flags, const char *pString, const uint32_t aux_code)
{
    rtc_time_t  current_time;           // Current time
    uint32_t    sz_str;                 // String size
    uint8_t     my_buffer[4] = {0};           // local buffer

#ifndef TRACE_ON
    return;
#endif

    if (flags & TRACE_DISABLED) return;

    // get the trace port
    tx_semaphore_get (&trace_semaphore, TX_WAIT_FOREVER);

    // open the trace engine in the Boot processor
    my_buffer[0] = 'T';
    my_buffer[1] = '#';
    traceUART.p_api->write (traceUART.p_ctrl, my_buffer, 2);

    if (flags & TRACE_TIME_STAMP)
    {
        //insert the time stamp ...
        iRTC.p_api->calendarTimeGet (iRTC.p_ctrl, &current_time);
        sprintf (trace_buffer, "%02d:%02d:%02d --> ", current_time.tm_hour, current_time.tm_min, current_time.tm_sec);
    }
    else
    {
        //clean buffer
        trace_buffer[0] = '\0';
    }

    // Copy the code at the end of the buffer
    sprintf (trace_buffer, pString, aux_code);
    sz_str = strlen (trace_buffer);

    if (flags & TRACE_NEWLINE)
    {
        // Insert the CR+LF if flag is defined ...
        trace_buffer[sz_str++] = '\n';
        trace_buffer[sz_str++] = '\r';
        trace_buffer[sz_str  ] = 0x00;
    }

    //Transmit trace buffer through UART
    traceUART.p_api->write (traceUART.p_ctrl, (uint8_t *) trace_buffer, sz_str+1);
    tx_thread_sleep ((uint16_t)OSTIME_10MSEC);

    tx_semaphore_put (&trace_semaphore);
}

/******************************************************************************
 ** Name:    Trace_Char
 *****************************************************************************/
/**
 ** @brief   Print a char without inserting the time stamp
 **
 ** @param   data_to_send    byte to be printed
 **
 ** @return  void
 ** @todo    pending things to be done
 ******************************************************************************/
void Trace_Char(const char_t data_to_send)
{
    //uint32_t sz_str;                    // String size
#ifndef TRACE_ON
     return;
#endif

     // get the trace port
     tx_semaphore_get (&trace_semaphore, TX_WAIT_FOREVER);

     //Transmit a single character through UART
     traceUART.p_api->write (traceUART.p_ctrl, (uint8_t *) &data_to_send, 1);
     tx_thread_sleep ((uint16_t)OSTIME_10MSEC);

     tx_semaphore_put (&trace_semaphore);
}

/******************************************************************************
 ** Name:    Boot_Send_Message
 *****************************************************************************/
/**
 ** @brief   Send a message to the Boot processor
 **
 ** @param   pMsg       Pointer to message
 ** @param   size       message size or number of bytes to transfer
 **
 ** @return  received ACK character (NULL if does not receive nothing
 ******************************************************************************/
uint8_t Boot_Send_Message(const uint8_t *pMsg, uint32_t size)
{
          uint8_t   ack_byte;      // received ACK character
          uint32_t  i;             // general use index
    const uint8_t  *my_ptr;        // copy pointer to local var, directly passing pMsg triggers BSP ERROR

    // copy pointer
    my_ptr = pMsg;

    // initialize some internal control variables
    rx_data_ready = false;

    // get the trace port
    tx_semaphore_get (&trace_semaphore, TX_WAIT_FOREVER);

    // Transmit trace buffer through UART
    traceUART.p_api->write (traceUART.p_ctrl, my_ptr, size);

    for (i=0; i<200; i++)
    {
        // 2 second timeout to receive response
        tx_thread_sleep ((uint16_t)OSTIME_10MSEC);

        // Receive the acknowledge character
        if (rx_data_ready)
        {
            traceUART.p_api->read (traceUART.p_ctrl, &ack_byte, 1);

            tx_semaphore_put (&trace_semaphore);
            return ack_byte;
        }
    }

    tx_semaphore_put (&trace_semaphore);
    return 0;
}


/******************************************************************************
 ** Name:    Boot_Receive_Message
 *****************************************************************************/
/**
 ** @brief   Receive a message to the Boot processor
 **
 ** @param   none
 **
 ** @return  received ACK character (NULL if does not receive nothing
 ******************************************************************************/
uint8_t Boot_Receive_Message(void)
{
    uint8_t   ack_byte;      // received ACK character
    uint32_t  i;             // general use index

    // get the trace port
    tx_semaphore_get (&trace_semaphore, TX_WAIT_FOREVER);

    for (i=0; i<200; i++)
    {
        // 2 second timeout to receive response
        tx_thread_sleep ((uint16_t)OSTIME_10MSEC);

        // Receive the acknowledge character
        if (rx_data_ready)
        {
            traceUART.p_api->read (traceUART.p_ctrl, &ack_byte, 1);

            tx_semaphore_put (&trace_semaphore);
            return ack_byte;
        }
    }

    tx_semaphore_put (&trace_semaphore);
    return 0;
}


/******************************************************************************
 ** Name:    Boot_Sync
 *****************************************************************************/
/**
 ** @brief   synchronize the Boot processor's FRC frequency sending a beacon signal
 **          (MUST guarantee a 250Kbps baud rate with a minimum error)
 **
 ** @param   nBeacons   number of beacons used to synchronize
 **
 ** @return  none
 ******************************************************************************/
void Boot_Sync (uint8_t nBeacons)
{
    uint8_t     my_buffer[4];   // local buffer

    // if no beacons are used, bye-bye
    if (!nBeacons) return;

    // prepare the SYNC message
    my_buffer[0] = 'S';
    my_buffer[1] = '#';
    my_buffer[2] = 0x00;

    // get the trace port
    tx_semaphore_get (&trace_semaphore, TX_WAIT_FOREVER);

    // send the SYNC message
    traceUART.p_api->write (traceUART.p_ctrl, my_buffer, 3);

    // loop sending many times the BEACON message
    my_buffer[0] = 'U';
    while (nBeacons)
    {
        tx_thread_sleep (OSTIME_1SEC);
        traceUART.p_api->write (traceUART.p_ctrl, my_buffer, 3);
        nBeacons--;
     }

    tx_semaphore_put (&trace_semaphore);
    tx_thread_sleep (OSTIME_100MSEC);
}


#ifdef UNIT_TESTS
/******************************************************************************
 ** Name:    Trace_Test
 *****************************************************************************/
/**
 ** @brief   Print unit test traces
 **
 ** @param   data_to_send    byte to be printed
 **
 ** @return  void
 ** @todo    pending things to be done
 ******************************************************************************/
void Trace_Test(const char_t data_to_send)
{
    static uint32_t index = 0; // String index
    uint32_t sz_str;           // String size

 #ifndef TRACE_ON
     return;
 #endif

     trace_buffer[index] = data_to_send;
     index++;

     if((data_to_send == '\n') || (index >= 254))
     {
         // get the length
         sz_str = strlen (trace_buffer);

         // get the trace port
         tx_semaphore_get (&trace_semaphore, TX_WAIT_FOREVER);

         //Transmit trace buffer through UART
         traceUART.p_api->write (traceUART.p_ctrl, (uint8_t *) trace_buffer, sz_str);
         tx_thread_sleep (1);
         //clear buffer
         memset(trace_buffer,0,256);
         index = 0;
     }
}
#endif
