/******************************************************************************
 * Copyright : Osatu S. Coop                                                  *
 * Compiler  : GNU ARM Embedded                                               *
 * Target    : Reanibex Serie 100                                             *
 ******************************************************************************/

/**
 * @file        thread_acc_entry.c
 * @brief
 *
 * @version     v1
 * @date        15/07/2021
 * @author      ivicente
 * @warning     author   date    version     .- Edition
 * @bug
 *
 */

/**
 * INCLUDES
 */
#include "device_init.h"
#include "R100_Errors.h"
#include "types_basic.h"
#include "types_app.h"
#include "Trace.h"
#include "Comm.h"
#include "I2C_1.h"
#include "I2C_2.h"
#include "RTC.h"
#include "Keypad.h"
#include "DB_Test.h"
#include "DB_Episode.h"
#include "bsp_acc_LIS3DH.h"

#include "HAL/thread_defibrillator_hal.h"
#include "HAL/thread_patMon_hal.h"
#include "HAL/thread_audio_hal.h"
#include "HAL/thread_comm_hal.h"
#include "thread_comm_entry.h"
#include "thread_patMon_entry.h"
#include "thread_sysMon_entry.h"
#include "thread_sysMon.h"
#include "thread_audio_entry.h"
#include "thread_audio.h"
#include "thread_core_entry.h"
#include "thread_core.h"
#include "thread_acc.h"
#include "thread_acc_entry.h"
#include "sysMon_RF_Comms.h"

#include "thread_comm.h"
#include "thread_api.h"
#include <math.h>


/******************************************************************************
 ** Macros
 */
#define GPS_ZERO                        "\0\0\0\0"
#define acc_wait_time(x)                (x==false ? (OSTIME_60SEC*2) : (OSTIME_60SEC*4))    ///< If not send the first message when 1 minute passes and if it is still moving every 3 minutes
#define acc_send_time(x)                (x==false ? (OSTIME_60SEC) : (OSTIME_60SEC*3))      ///< Send the first message when 1 minute passes and if it is still moving every 3 minutes
#define acc_send_counter                ACC_INT_COUNT_MAX                                   ///< Counter to send message
#define ABS(x)                          (((x) < 0) ? -(x) : (x))                            ///< Convert to asolute value
#define BUF_MAX                          11 ///< Buffer max
#define BUF_FIL_MAX                      10 ///< Filter buffer max


/**
 * EXTERNS
 */
extern gps_config_t R100_gps;
extern acc_config_t R100_acc;

/**
 * LOCALS
 */
static NV_DATA_t        nv_data;                    ///< Non volatile data
static NV_DATA_BLOCK_t  nv_data_block;              ///< Non volatile data

static uint16_t         thread_acc_intp_counter;    ///< Movement counter
static bool_t           acc_send_first_time;        ///< Determine if send message

static uint32_t         time_led;

/******************************************************************************
 ** Prototypes
 */

/**
 * @brief   Increment counter
 *
 */
/*static void Run_Counter(void)
{
    thread_acc_intp_counter++;
    Trace_Arg (TRACE_NO_FLAGS | TRACE_NEWLINE , "Counter : %d", (uint32_t) thread_acc_intp_counter);
}*/

/**
 * @brief   Method 1: Depending on the configured interrupt increments the counter. 
 * 
 */
/*static void Capture_Axis_Data(void)
{
    Refresh_Wdg();
    // Get acceleration data
    Comm_ACC_Get_Acceleration_Data();
    Refresh_Wdg();
    //Trace (TRACE_TIME_STAMP + TRACE_NEWLINE, " capture_axis_data  !!!");
    switch(pos_hvi)
    {
        case 1: 
            if((acc.x_data > 20 || acc.x_data < -20) || (acc.y_data > 20 || acc.y_data < -20))
            {
                Run_Counter();
            }
            break;
        case 2: 
            if(acc.z_data > 25 || acc.z_data < -25)
            {
                Run_Counter();
            }
            break;
        case 3:
            if((acc.x_data < 20 || acc.x_data > 50) || (acc.y_data < 20 || acc.y_data > 50))
            {
                Run_Counter();
            }
            break;

        default:
            break;      // by default, just in case
    }
}*/

/**
 * @brief   Method 2: Increments the counter by calculating if there is movement by doing a simple subtraction.
 * 
 */
/*static void Capture_Axis_Data_Manual(void)
{
    int16_t    x_n[BUF_MAX]={0};
    int16_t    y_n[BUF_MAX]={0};
    int16_t    filter_x[BUF_FIL_MAX] = {0};
    int16_t    filter_y[BUF_FIL_MAX] = {0};
    uint8_t     static_count = 0;

    for(int i=0; i<BUF_MAX; i++)
    {
        Refresh_Wdg ();
        Comm_ACC_Get_Acceleration_Data();

        Refresh_Wdg ();
        x_n[i] = (int16_t)abs(acc.x_data);
        y_n[i] = (int16_t)abs(acc.y_data);
        tx_thread_sleep(OSTIME_100MSEC);
    }
    for(int ii=0; ii<BUF_FIL_MAX; ii++)
    {
        filter_x[ii] = (int16_t)abs( (x_n[ii] - x_n[ii+1]) );
        filter_y[ii] = (int16_t)abs( (y_n[ii] - y_n[ii+1]) );
        if(filter_x[ii] > 8 || filter_y[ii] > 8)
        {
            static_count++;
        }
        tx_thread_sleep(OSTIME_10MSEC);
    }
    if(static_count > 5)
    {
        //acc is moving
        thread_acc_intp_counter++;
        Trace_Arg (TRACE_NO_FLAGS | TRACE_NEWLINE , "Counter : %d", (uint32_t) thread_acc_intp_counter);
    }
}*/

/**
 * @brief   Method 3: Increment the counter if there is movement depending on the result of the axes using the math.h library.
 *          La funcion sqrt da warning y se añade FLT_EVAL_METHOD=3 porque se añade la libreria math.h
 */
static void Capture_Axis_Data_SQRT(void)
{
    double_t    x_n[BUF_MAX]={0};
    double_t    y_n[BUF_MAX]={0};
    double_t    z_n[BUF_MAX]={0};
    double_t    vector[BUF_MAX] = {0};
    int16_t     filter[BUF_FIL_MAX] = {0};
    int16_t     abs_filter[BUF_FIL_MAX] = {0};
    uint8_t     static_count = 0;
    double_t    aux = 0;

    for(int i=0; i<BUF_MAX; i++)
    {
        Refresh_Wdg();
        tx_thread_relinquish();
        Comm_ACC_Get_Acceleration_Data();
        Refresh_Wdg();
        x_n[i] = (double)abs(R100_acc.x_data);
        y_n[i] = (double)abs(R100_acc.y_data);
        z_n[i] = (double)abs(R100_acc.z_data);
        tx_thread_sleep(OSTIME_100MSEC);

        aux = (x_n[i]*x_n[i]) + (y_n[i]*y_n[i]) + (z_n[i]*z_n[i]);
        vector[i] = sqrt(aux);
    }

    for(int ii=0; ii<BUF_FIL_MAX; ii++)
    {
        filter[ii] = (int16_t)(vector[ii] - vector[ii+1]);
        abs_filter[ii] = (int16_t)(abs((int16_t)filter[ii]));
        if(abs_filter[ii] > 6)
        {
            static_count++;
        }
        tx_thread_sleep(OSTIME_10MSEC);
    }
    if(static_count > 4)
    {
        //acc is moving
        thread_acc_intp_counter++;
        Trace_Arg (TRACE_NO_FLAGS | TRACE_NEWLINE , "Counter : %d", (uint32_t) thread_acc_intp_counter);
    }
}

/**
 * @brief This function kill the devide.
 * 
 * @param none
 * 
 * @return none
 */
void ACC_Kill(void)
{
    //EV_ACC_e    rx_event_acc;

    // Initialize accelerometer variable to configure interrupt
    Get_NV_Data_Block()->acc_pos_hvi = 0;

    // wait 500 mili second before powering OFF to see if it has been powering ON for another reason
    /*(void) tx_queue_receive(&queue_acc, &rx_event_acc, OSTIME_1SEC);
    if(rx_event_acc == 1)
    {
        tx_thread_terminate(&thread_acc);
    }*/
    Trace (TRACE_TIME_STAMP | TRACE_NEWLINE, ">>> ACC KILL");

    // Do not detect more Cover close event
    tx_thread_terminate (&thread_drd);
    tx_thread_terminate (&thread_hmi);

    // application tidy closure
    tx_thread_terminate (&thread_audio);

    // suspend the remaining threads
    tx_thread_terminate (&thread_patMon);
    //tx_thread_terminate (&thread_sysMon);
    tx_thread_terminate (&thread_defibrillator);
    tx_thread_terminate (&thread_recorder);
    tx_thread_terminate (&thread_comm);

    // reprogram the auto-test depending on the manual power off
    //R100_Program_Autotest();

    R100_PowerOff ();
}

/**
 * @brief This function will be used to send message through comms.
 * 
 * @param none
 * 
 * @return none
 */
void ACC_Send_Comms(void)
{
    uint32_t    tout = (OSTIME_60SEC*2); // 2 min timeout
    uint32_t    ref_timeout = 0;

    thread_acc_intp_counter = 0;
    acc_send_first_time = true;
    
    // In DEMO mode do not send anything
    if (Is_Battery_Mode_Demo() == FALSE)
    {
        Led_Blink_ACC(true);

        tx_thread_resume (&thread_comm);
        tx_thread_sleep (OSTIME_20MSEC); // wait pointer initialization just in case
        Refresh_Wdg ();

        //Audio_Message (eAUDIO_CMD_CONCAT, eAUDIO_TONE, TRUE);

        // Check if movement alert is enabled
        if (Get_Device_Settings()->misc.glo_movement_alert == TRUE)
        {
            Trace (TRACE_TIME_STAMP + TRACE_NEWLINE, " Movement alert enable !!!");

            if(Is_Sigfox_TX_Enabled() == SIGFOX_ONLY || Is_Sigfox_TX_Enabled() == SIGFOX_PRIOR)
            {
                // Relinquish control to other thread. 
                tx_thread_relinquish();
                
                Send_Sigfox_Alert(MSG_ID_ACCEL_ATIVATE_MOVE_ALERT);     // Generate and Send alert indicating that accelerometer is activate
            }


            if(Is_Wifi_TX_Enabled() == WIFI_ONLY || Is_Wifi_TX_Enabled() == WIFI_PRIOR)
            {
                Send_Wifi_Alert(WIFI_MSG_ID_ACCEL_ATIVATE_MOVE_ALERT);      // Generate and Send Alert report using Wifi
            }

            //tx_thread_sleep (OSTIME_20MSEC);

            if(Is_GPS_TX_Enabled() && (Is_Sigfox_TX_Enabled() != FALSE || Is_Wifi_TX_Enabled() != FALSE))
            {
                Comm_GPS_Open();

                // Relinquish control to other thread. 
                tx_thread_relinquish();

                // refresh the internal watchdog timer
                Refresh_Wdg ();

                ref_timeout = tx_time_get() + tout;
                while (tx_time_get() <= ref_timeout)
                {
                    // Relinquish control to other thread. 
                    tx_thread_relinquish();

                    Refresh_Wdg ();
                    if((memcmp(R100_gps.lat_data, GPS_ZERO, 4) == 0) && 
                        (memcmp(R100_gps.long_data, GPS_ZERO, 4) == 0))
                    {
                        // Relinquish control to other thread. 
                        tx_thread_relinquish();
                        // Set GPS Get Data Command
                        Comm_GPS_Get_Position();
                        Pasatiempos (OSTIME_4SEC);
                    }
                    else if((memcmp(R100_gps.lat_data, Get_NV_Data()->latitude, 12) != 0) &&
                        (memcmp(R100_gps.long_data, Get_NV_Data()->longitude, 12) != 0))
                    {
                        // save gps position to non-volatile memory
                        memcpy (Get_NV_Data()->latitude, R100_gps.lat_data, 12);
                        memcpy (Get_NV_Data()->longitude, R100_gps.long_data, 12);

                        nv_data.lat_long_dir = 0;
                        if (R100_gps.N_S == 'N') Get_NV_Data()->lat_long_dir = 0x2;
                        if (R100_gps.E_W == 'E') Get_NV_Data()->lat_long_dir |= 0x1;

                        // refresh the internal watchdog timer
                        Refresh_Wdg ();

                        // update the non volatile data
                        NV_Data_Write(&nv_data, &nv_data_block);

                        // refresh the internal watchdog timer
                        Refresh_Wdg ();
                    }
                    else
                    {
                        break;
                    }
                    Led_Blink_ACC(false);
                }
                // refresh the internal watchdog timer
                Refresh_Wdg ();

                if(Is_Sigfox_TX_Enabled() == SIGFOX_ONLY || Is_Sigfox_TX_Enabled() == SIGFOX_PRIOR)
                {
                    Send_Sigfox_Position(MSG_ID_ACCEL_POS_GPS);     // Send position if changed
                }
                Refresh_Wdg ();

                if(Is_Wifi_TX_Enabled() == WIFI_ONLY || Is_Wifi_TX_Enabled() == WIFI_PRIOR)
                {
                    Send_Wifi_Position(WIFI_MSG_ID_ACCEL_POS_GPS);      // Generate and Send Alert report using Wifi
                }
            }
        }
        // Wait some time until to send position. ADJUST!! 
        Pasatiempos (OSTIME_10SEC); 
        NFC_Write_Device_Info(true);
    }

    // Check if R100 is moving or not
    if(thread_acc_intp_counter == 0)
    {
        Capture_Axis_Data_SQRT();
        if(thread_acc_intp_counter >= 1)
        {
            Trace (TRACE_TIME_STAMP | TRACE_NEWLINE, "ALREADY MOVING!!!!!!!");
            ACC_Cal_Mov();
        }
        else
        {
            Trace (TRACE_TIME_STAMP | TRACE_NEWLINE, "NO MOVING!!!!!!!");
        }
        
    }
    // Kill the R100
    ACC_Kill();
}

/**
 * @brief This function led blink each 10 seconds as RTC.
 * 
 * @param force Force to blink led
 * 
 * @return none
 */
void Led_Blink_ACC(bool_t force)
{
    if(tx_time_get() > time_led || force == true)
    {
        time_led = tx_time_get() + OSTIME_10SEC;
        if (Get_NV_Data()->status_led_blink == 0x01) Led_On(LED_ONOFF);
        tx_thread_sleep(OSTIME_100MSEC);
        Led_Off(LED_ONOFF);
    }
}

/**
 * @brief This function capture axis data and determines if the device is moving to send by communications, otherwise it kills the thread.
 * 
 * @param none
 * 
 * @return none
 */
void ACC_Cal_Mov(void)
{
    uint8_t     pos_hvi;                    // Determine acc position horizontal/vertical/inclined
    uint32_t    my_time;                    // Time at the moment
    uint32_t    wait_time;                  // Time to wait
    uint32_t    send_time;                  // Time to send

    my_time   = tx_time_get();
    wait_time = tx_time_get() + acc_wait_time(acc_send_first_time);
    send_time = tx_time_get() + acc_send_time(acc_send_first_time);
    time_led  = tx_time_get() + OSTIME_10SEC;

    thread_acc_intp_counter = 0;

    pos_hvi = Get_NV_Data_Block()->acc_pos_hvi;
    Trace_Arg (TRACE_NO_FLAGS | TRACE_NEWLINE , "pos_hvi : %d", (uint32_t) pos_hvi);
    tx_thread_relinquish();

    // Wait a while depending on the number of sendigs
    while(my_time < wait_time)
    {
        Led_Blink_ACC(false);
        Capture_Axis_Data_SQRT();
        tx_thread_relinquish();
        Refresh_Wdg ();

        // If in 1 or 3 minutes (depending on the sending number) the counter reaches 10 (24 seconds) send by sigfox
        if(thread_acc_intp_counter >= acc_send_counter && my_time > send_time)
        {
            ACC_Send_Comms();
        }

        my_time = tx_time_get();
    }
}

/**
 * @brief This function will be executed when the cover has not been opened and if accelerometer interrupt is enabled.
 *        Also, will send an alert and the gps position with Sigfox.
 * 
 * @param none
 * 
 * @return none
 */
void thread_acc_entry(void)
{
    acc_send_first_time = false;

    // If you keep moving the accelerometer keeps sending
    ACC_Cal_Mov();

    // Kill the R100
    ACC_Kill();
}
