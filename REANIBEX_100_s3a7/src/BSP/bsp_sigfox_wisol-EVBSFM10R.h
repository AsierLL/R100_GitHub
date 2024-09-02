/******************************************************************************
 * Name      : R100_S3A7                                                      *
 * Copyright : Osatu S. Coop                                                  *
 * Compiler  : GNU ARM Embedded                                               *
 * Target    : Reanibex Serie 100                                             *
 ******************************************************************************/

/*!
 * @file        bsp_sigfox_wisol-EVBSFM10R.h
 * @brief       BSP Wisol EVBSFM10R header file
 *
 * @version     v1
 * @date        27/05/2019
 * @author      jarribas
 * @warning     author   date    version     .- Edition
 * @bug
 *
 */

#ifndef BSP_BSP_SIGFOX_WISOL_EVBSFM10R_H_
#define BSP_BSP_SIGFOX_WISOL_EVBSFM10R_H_

/******************************************************************************
 **Includes
 */
#include "R100_Errors.h"
#include "types_basic.h"

/******************************************************************************
 ** Defines
 */
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Wisol Sigfox AT Commands for EVBSFM10R
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#define SIGFOX_CMD_AT                 "AT"                     ///< Dummy command. Just returns OK. Used to check communication
#define SIGFOX_CMD_DEVID              "AT$I=10"                ///< Information. Device ID
#define SIGFOX_CMD_PAC                "AT$I=11"                ///< Information. PAC
#define SIGFOX_CMD_TX_FREQ            "AT$IF?"                 ///< Information. Get the currently chosen TX frequency
#define SIGFOX_CMD_RX_FREQ            "AT$DR?"                 ///< Information. Get the currently chosen RX frequency
#define SIGFOX_CMD_TEMP               "AT$T?"                  ///< Return internal temperature in 1/10 of a degree Celsius
#define SIGFOX_CMD_VOLTAGES           "AT$V?"                  ///< Return current voltage and voltage measured during last transmission in mV
#define SIGFOX_CMD_SW_RESET           "AT$P=0"                 ///< Set power mode 0: software reset
#define SIGFOX_CMD_SW_SLEEP           "AT$P=1"                 ///< Set power mode 1: sleep
#define SIGFOX_CMD_SW_DEEP_SLEEP      "AT$P=2"                 ///< Set power mode 2: deep sleep
#define SIGFOX_CMD_SEND_BYTE          "AT$SB="                 ///< Send a bit status (0 or 1)
#define SIGFOX_CMD_SEND_FRAME         "AT$SF="                 ///< Send payload data. 1 to 12 bytes
#define SIGFOX_CMD_MAX_POWER          "ATS302=15"              ///< Set output power to the maximun power level (register 302)
#define SIGFOX_CMD_CW_TEST_ON         "AT$CW=868130000,1,15"   ///< Start Sending a Continuous Wave for emission test for Sigfox certification. param = Frequency, Mode, power
#define SIGFOX_CMD_CW_TEST_OFF        "AT$CW=868130000,0,15"   ///< Stop Sending a Continuous Wave for emission test for Sigfox certification. param = Frequency, Mode, power
#define SIGFOX_CMD_CB_TEST_RANDOM_ON  "AT$CB=-1,1"             ///< Enable a specific Constant Byte pattern for emission testing. param = Pattern (-1 for random or 0-255), Mode
#define SIGFOX_CMD_CB_TEST_RANDOM_OFF "AT$CB=-1,0"             ///< Disable a specific Constant Byte pattern for emission testing. param = Pattern (-1 for random or 0-255), Mode
#define SIGFOX_CMD_SE_TEST_ON         "AT$SE"                  ///< Sensitivity Test. Starts AT$TM-3,255 indefinitely, which corresponds to RX GFSK Testmode
#define SIGFOX_CMD_TM_TX_BPSK_TEST    "AT$TM=0,"               ///< Activates the Sigfox Testmode 0: TX BPSK. param = Config define number of repetitions
#define SIGFOX_CMD_TM_TX_PROT_TEST    "AT$TM=1,"               ///< Activates the Sigfox Testmode 1: TX Protocol. param = Config defines number of repetitions
#define SIGFOX_CMD_TM_RX_PROT_TEST    "AT$TM=2,"               ///< Activates the Sigfox Testmode 2: RX Protocol. param = Config defines number of repetitions
#define SIGFOX_CMD_TM_RX_GFSK_TEST    "AT$TM=3,"               ///< Activates the Sigfox Testmode 3: RX GFSK. param = Config defines number of repetitions
#define SIGFOX_CMD_TM_RX_SENS_TEST    "AT$TM=4,"               ///< Activates the Sigfox Testmode 4: RX Sensitivity. param= ?
#define SIGFOX_CMD_TM_TX_SYNTH_TEST   "AT$TM=5,"               ///< Activates the Sigfox Testmode 5: TX Synthesis. param= ?
#define SIGFOX_CMD_SL_TEST_DEFAULT    "AT$SL"                  ///< Sends a local loop frame. default payload
#define SIGFOX_CMD_SL_TEST_FRAME      "AT$SL="                 ///< Sends a local loop frame. param = Frame
#define SIGFOX_CMD_RL_TEST            "AT$RL"                  ///< Receive local loop

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Wisol Sigfox AT Command Responses from EVBSFM10R
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#define SIGFOX_RESP_OK                "OK\r\n"                          ///< Response to SIGFOX_CMD
#define SIGFOX_RESP_SE_TEST_ON        "ONSEMI Sensitivitiy Test"        ///< Response to SIGFOX_CMD_SE_TEST_ON

/******************************************************************************
 ** Typedefs
 */


/******************************************************************************
 ** Globals
 */


/******************************************************************************
 ** Prototypes
 */
ERROR_ID_e    Sigfox_Check_Module_Status            (void);
ERROR_ID_e    Sigfox_Get_Module_DeviceID            (void);
ERROR_ID_e    Sigfox_Get_Module_PAC                 (void);
ERROR_ID_e    Sigfox_Get_TX_Frequency               (void);
ERROR_ID_e    Sigfox_Get_RX_Frequency               (void);
ERROR_ID_e    Sigfox_Get_Module_Temp                (void);
ERROR_ID_e    Sigfox_Get_Module_Voltages            (void);
ERROR_ID_e    Sigfox_Compare_Test_Frequencies       (void);
ERROR_ID_e    Sigfox_Start_CW_Test                  (void);
ERROR_ID_e    Sigfox_Stop_CW_Test                   (void);
ERROR_ID_e    Sigfox_Start_RX_GFSK_Test             (void);
ERROR_ID_e    Sigfox_Send_Data_Frame                (uint8_t* data);

#endif /* BSP_BSP_SIGFOX_WISOL_EVBSFM10R_H_ */
