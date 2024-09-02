/******************************************************************************
 * Name      : R100_S3A7                                                      *
 * Copyright : Osatu S. Coop                                                  *
 * Compiler  : GNU ARM Embedded                                               *
 * Target    : Reanibex Serie 100                                             *
 ******************************************************************************/

/*!
 * @file        Comm.h
 * @brief       Header with functions related to the comm modules through serial port
 *
 * @version     v1
 * @date        17/04/2019
 * @author      jarribas
 * @warning     author   date    version     .- Edition
 * @bug
 *
 */

#ifndef COMM_H_
#define COMM_H_

/******************************************************************************
 **Includes
 */

#include "types_basic.h"
#include "R100_Errors.h"

/******************************************************************************
 ** Defines
 */

// FLAGS
#define TRACE_NO_FLAGS              0x00
#define TRACE_TIME_STAMP            0x01
#define TRACE_NEWLINE               0x02

#define MAX_WIFI_CMD_RESPONSE_SIZE  220

/******************************************************************************
 ** Typedefs
 */

/// Baud rates for the Comm UART controller

#define   BR_9600      9600    ///< 9600  bauds
#define   BR_115200    115200  ///< 115200 bauds
#define   BR_115384    115384  ///< 115384 bauds

#define   DATA_BITS_8  8  ///< 8 data bits

#define   STOP_BITS_1  1  ///< 1 stop bits

#define   NO_PARITY    0  ///< No Parity

#define   NO_FLOW_CTRL 0  ///< No Flow Control

/******************************************************************************
 ** Globals
 */

/******************************************************************************
 ** Prototypes
 */

void       Comm_Uart_Init             (void);
ERROR_ID_e Comm_Uart_Set_Baud_Rate    (uint32_t baud_rate);
void       Comm_Uart_Send             (const char *pString);
void       Comm_Uart_Send_Arg         (uint8_t flags, const char *pString, const uint32_t aux_code);
void       Comm_Uart_Send_Data        (const char *pString, uint32_t size);
void       Comm_Uart_Get_Msg_Buff_Ptr (char *buf, int count);

void       Comm_Uart_Wifi_Send        (const char *pString, uint32_t length);
ERROR_ID_e Comm_Uart_Wifi_Receive     (char *rx_buf, uint32_t length, uint32_t tout, uint32_t ack);
ERROR_ID_e Comm_Uart_Wifi_Receive_CFG_FW (char *rx_buf, uint32_t length, uint32_t tout, uint32_t ack, uint32_t *received_length, bool_t wait_length);
void       Comm_Uart_Wifi_Enable_Data_Reception(void);
char*      Comm_Uart_Wifi_Get_Data    (void);

#endif /* TRACE_H_ */

/*
 ** $Log$
 **
 ** end of Comm_Trace.h
 ******************************************************************************/

