/******************************************************************************
 * Name      : R100_S3A7                                                      *
 * Copyright : Osatu S. Coop                                                  *
 * Compiler  : GNU ARM Embedded                                               *
 * Target    : Reanibex Serie 100                                             *
 ******************************************************************************/

/*!
 * @file        sysMon_RF_Comms.h
 * @brief       Header with functions related to the RF comms services
 *
 * @version     v1
 * @date        24/11/2021
 * @author      ilazkanoiturburu
 * @warning     author   date    version     .- Edition
 * @bug
 *
 */

#ifndef SYSMON_RF_COMMS_H_
#define SYSMON_RF_COMMS_H_

/******************************************************************************
 **Includes
 */
#include "R100_Errors.h"
#include "HAL/thread_patMon_hal.h"
#include "HAL/thread_comm_hal.h"
#include "DB_Test.h"
#include "secure_crypto.h"
#include "BSP/bsp_wifi_silabs-WGM110.h"
#include "BSP/bsp_gps_quectel-L96.h"

/******************************************************************************
 ** Defines
 */

#define SIGFOX_ONLY     1
#define SIGFOX_PRIOR    3

#define WIFI_ONLY       2
#define WIFI_PRIOR      4

#define WIFI_1_DAY      1
#define WIFI_7_DAY      7
#define WIFI_30_DAY     29

#define ABORT_USER          0
#define OK_CONNECT          1
#define ROUTER_CONNECT      10
#define ROUTER_NO_CONECT    11
#define SERCER_CONNECT      12
#define SERVER_NO_CONNECT   13

/******************************************************************************
 ** Typedefs
 */

/******************************************************************************
 ** Globals
 */


/******************************************************************************
 ** Prototypes
 */

extern uint32_t     Execute_Get_Gps_Position_Test       (DB_TEST_RESULT_t *pResult, uint32_t tout);
extern void         Execute_Periodic_Gps_Init           (void);
extern void         Execute_Periodic_Gps_Position       (NV_DATA_t *pNV_data, NV_DATA_BLOCK_t *pNV_data_block);
extern void         Execute_Save_Gps_Position           (NV_DATA_t *pNV_data, NV_DATA_BLOCK_t *pNV_data_block);

extern bool_t       Execute_Send_Sigfox_Test            (DB_TEST_RESULT_t *pResult, bool_t auto_test);
extern bool_t       Execute_Send_Sigfox_Exp             (DB_TEST_RESULT_t *pResult, bool_t auto_test);
extern bool_t       Execute_Send_Sigfox_GPS             (DB_TEST_RESULT_t *pResult, bool_t auto_test, SIGFOX_MSG_ID_e gps_id);

extern bool_t       Execute_Wifi_Open                   (DB_TEST_RESULT_t *pResult, bool_t auto_test);
extern bool_t       Execute_Wifi_Init_Host_Alive        (DB_TEST_RESULT_t *pResult, bool_t auto_test);
extern bool_t       Execute_Send_Wifi_Test_Frame        (DB_TEST_RESULT_t *pResult, bool_t auto_test);
extern bool_t       Execute_Send_Wifi_Alert_Frame       (DB_TEST_RESULT_t *pResult, WIFI_MSG_ID_e wifi_msg_id);
extern bool_t       Execute_Send_Wifi_Position_Frame    (DB_TEST_RESULT_t *pResult, SIGFOX_MSG_ID_e gps_id, bool_t auto_test);
extern bool_t       Execute_Send_Wifi_Test_File         (DB_TEST_RESULT_t *pResult, bool_t auto_test);
extern bool_t       Execute_Send_Wifi_Episode           (DB_TEST_RESULT_t *pResult, bool_t auto_test);
extern bool_t       Execute_Receive_Wifi_Upgrade        (DB_TEST_RESULT_t *pResult, bool_t auto_test);
extern bool_t       Execute_Receive_Wifi_Configuration  (DB_TEST_RESULT_t *pResult, bool_t auto_test);

void                Send_Sigfox_Alert                   (SIGFOX_MSG_ID_e alert_id);
void                Send_Sigfox_Test                    (DB_TEST_RESULT_t *pResult);
void                Send_Sigfox_Position                (SIGFOX_MSG_ID_e gps_id);
void                Send_Wifi_Alert                     (WIFI_MSG_ID_e alert_id);
void                Send_Wifi_Test_Frame                (DB_TEST_RESULT_t *pResult);
void                Send_Wifi_Position                  (WIFI_MSG_ID_e alert_id);

extern uint8_t      Is_Sigfox_TX_Enabled                (void);
extern uint8_t      Is_Wifi_TX_Enabled                  (void);
extern bool_t       Is_GPS_TX_Enabled                   (void);
extern uint8_t      Wifi_Trans_Days                     (void);

void                Send_Test_By_Comms                  (DB_TEST_RESULT_t *pResult, bool_t auto_test);

uint32_t            Lis3dh_ACC_Init                     (NV_DATA_BLOCK_t *pNV_data);


#endif /* SYSMON_RF_COMMS_H_ */

/*
 ** $Log$
 **
 ** end of sysMon_RF_Comms.h
 ******************************************************************************/
