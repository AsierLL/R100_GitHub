/******************************************************************************
 * Name      : R100_S3A7                                                      *
 * Copyright : Osatu S. Coop                                                  *
 * Compiler  : GNU ARM Embedded                                               *
 * Target    : Reanibex Serie 100                                             *
 ******************************************************************************/

/*!
 * @file        bsp_gps_quectel-L96.h
 * @brief       Quectel L96 GPS module header file
 *
 * @version     v1
 * @date        27/05/2019
 * @author      jarribas
 * @warning     author   date    version     .- Edition
 * @bug
 *
 */

#ifndef BSP_BSP_GPS_QUECTEL_L96_H_
#define BSP_BSP_GPS_QUECTEL_L96_H_

/******************************************************************************
 **Includes
 */
/*lint -save -e537 Spurious warning ignored due to include guards*/
#include "R100_Errors.h"
#include "types_basic.h"
/*lint -restore*/

/******************************************************************************
 ** Defines
 */

/******************************************************************************
 ** Typedefs
 */

/******************************************************************************
 ** Globals
 */

/******************************************************************************
 ** Prototypes
 */

ERROR_ID_e    Gps_Get_Lat_Long_Data           (void);
bool_t        Gps_Is_New_Position_Available   (void);
void          Gps_Set_New_Position_Available  (bool_t available);
bool_t        GPS_Is_Test_Package_Received    (void);
void          Gps_Set_Test_Package_Received   (bool_t received);
bool_t        GPS_Is_Running                  (void);
void          GPS_Set_Running                 (bool_t received);

#endif /* BSP_BSP_GPS_QUECTEL_L96_H_ */
