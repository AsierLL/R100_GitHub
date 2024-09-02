/******************************************************************************
 * Name      : S3A7_REANIBEX_100                                              *
 * Copyright : Osatu S. Coop                                                  *
 * Compiler  : MinGW32                                                        *
 * Target    : Reanibex Series                                                *
 ******************************************************************************/

/*!
 * @file        bsp_gps_quectel-L96.c
 * @brief       BSP for Quectel L96 GPS module
 *
 * @version     v1
 * @date        27/05/2019
 * @author      jarribas
 * @warning     author   date    version     .- Edition
 * @bug
 *
 */

/******************************************************************************
 ** Includes
 */

#include "BSP/bsp_gps_quectel-L96.h"
#include "HAL/thread_comm_hal.h"
#include "Comm.h"
#include "hal_data.h"

/******************************************************************************
 ** Macros
 */

/******************************************************************************
 ** Defines
 */
#define MAX_GPS_CMD_LENGTH 100
#define MAX_GPS_DATA_LENGTH 13  ///< 12 bytes payload + NULL char

#define MAX_GPS_NMEA_LENGTH 82  ///< Maximum sentence length, including the $ and <CR><LF> is 82 bytes.
/******************************************************************************
 ** Typedefs
 */

/******************************************************************************
 ** Constants
 */

/******************************************************************************
 ** Externals
 */
extern gps_config_t R100_gps;

/******************************************************************************
 ** Globals
 */
static bool_t gps_new_pos = FALSE;
static bool_t gps_test_pkg_received = FALSE;
static bool_t gps_running = FALSE;

/******************************************************************************
 ** Locals
 */

/******************************************************************************
 ** Prototypes
 */


/******************************************************************************
** Name:    Gps_Get_Lat_Long_Data
*****************************************************************************/
/**
** @brief   Get GPS NMEA Protocol latitude and longitude
**
** @param   none
**
** @return  ERROR_ID_e error code
******************************************************************************/
ERROR_ID_e  Gps_Get_Lat_Long_Data(void)
{
    ERROR_ID_e error = eERR_NONE;

    uint8_t i,j = 0;
    uint8_t comma_pos[15];
    uint8_t gps_nmea_msg[MAX_GPS_NMEA_LENGTH];

    memset(gps_nmea_msg,0,MAX_GPS_NMEA_LENGTH);

    memset(comma_pos,0,15);

    // get GPS NMEA Message
    //Comm_Uart_Get_Msg_Buff_Ptr(gps_nmea_msg, MAX_GPS_NMEA_LENGTH);
    Comm_Uart_Get_Msg_Buff_Ptr((char *)gps_nmea_msg, MAX_GPS_NMEA_LENGTH);


//elena $GNRMC,084543.000,A,4311.1590,N,00230.5851,W,0.00,116.90,010921,,,A,V*10
//memcpy(gps_nmea_msg,"$GNRMC,084543.000,A,4311.1590,N,00230.5851,W,0.00,116.90,010921,,,A,V*10",MAX_GPS_NMEA_LENGTH);

    // Check if it is a GNRMC packet
    if(memcmp(gps_nmea_msg, "$GNRMC", 6) == 0)
    {
        for(i = 0; i< MAX_GPS_NMEA_LENGTH; i++)
        {
            // Detect the all the ',' positions of the NMEA message
            if(gps_nmea_msg[i] == ',')
            {
                comma_pos[j] = i;
                j++;
                if(j == 14) break;
            }
        }

        // check that the sizes are ok depending on the ',' positions for the lat and long values
        if( (comma_pos[3] > (comma_pos[2]+1)) && ( ((comma_pos[3])-(comma_pos[2]+1)) < MAX_GPS_DATA_LENGTH ))
        {
            memcpy(R100_gps.lat_data, &gps_nmea_msg[comma_pos[2]+1], (size_t) ((comma_pos[3])-(comma_pos[2]+1)));
            R100_gps.N_S = (char_t) gps_nmea_msg[comma_pos[3]+1];
        }
        else
        {
            memset(R100_gps.lat_data,0,sizeof(R100_gps.lat_data));
        }

        if( (comma_pos[5] > (comma_pos[4]+1)) && ( ((comma_pos[5])-(comma_pos[4]+1)) < MAX_GPS_DATA_LENGTH ))
        {
            memcpy(R100_gps.long_data,&gps_nmea_msg[comma_pos[4]+1], (size_t) ((comma_pos[5])-(comma_pos[4]+1)));
            R100_gps.E_W = (char_t) gps_nmea_msg[comma_pos[5]+1];
        }
        else
        {
            memset(R100_gps.long_data,0,sizeof(R100_gps.long_data));
        }
    }

    return error;
}

/******************************************************************************
** Name:    Gps_Is_New_Position_Available
*****************************************************************************/
/**
** @brief   Check if a new GPS NMEA Protocol latitude and longitude available
**
** @param   none
**
** @return  bool_t new position available
******************************************************************************/
bool_t  Gps_Is_New_Position_Available(void)
{
    return gps_new_pos;
}

/******************************************************************************
** Name:    Gps_Is_New_Position_Available
*****************************************************************************/
/**
** @brief   Sets TRUE if a new GPS NMEA Protocol latitude and longitude is available
**
** @param   available TRUE if a new GPS NMEA Protocol latitude and longitude is available, FALSE otherwise
**
** @return  none
******************************************************************************/
void  Gps_Set_New_Position_Available(bool_t available)
{
    gps_new_pos = available;
}

/******************************************************************************
** Name:    Gps_Is_New_Position_Available
*****************************************************************************/
/**
** @brief   Check if a GPS Protocol package has arrived
**
** @param   none
**
** @return  bool_t TRUE if package has arrived
******************************************************************************/
bool_t  GPS_Is_Test_Package_Received(void)
{
    return gps_test_pkg_received;
}

/******************************************************************************
** Name:    Gps_Set_Test_Package_Received
*****************************************************************************/
/**
** @brief   Sets TRUE if a new GPS Protocol package has arrived
**
** @param   received if true, a GPS test package has been received
**                   if false, a GPS test package has not been received
**
** @return  none
******************************************************************************/
void  Gps_Set_Test_Package_Received(bool_t received)
{
    gps_test_pkg_received = received;
}

/******************************************************************************
** Name:    GPS_Is_Running
*****************************************************************************/
/**
** @brief   Check if a GPS power on package has arrived
**
** @param   none
**
** @return  bool_t TRUE if package has arrived
******************************************************************************/
bool_t  GPS_Is_Running(void)
{
    return gps_running;
}

/******************************************************************************
** Name:    GPS_Set_Running
*****************************************************************************/
/**
** @brief   Sets TRUE if a new GPS power on package has arrived
**
** @param   received if true, a GPS power on package has been received
**                   if false, a GPS power on package has not been received
**
** @return  none
******************************************************************************/
void  GPS_Set_Running(bool_t received)
{
    gps_running = received;
}
