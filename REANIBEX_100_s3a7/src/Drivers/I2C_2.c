/******************************************************************************
 * Name      : R100_S3A7                                                      *
 * Copyright : Osatu S. Coop                                                  *
 * Compiler  : GNU ARM Embedded                                               *
 * Target    : Reanibex Serie 100                                             *
 ******************************************************************************/

/*!
 * @file        I2C_2.c
 * @brief       All functions related to the I2C-2
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

// include the i2C semaphore to access to different devices
#include <thread_sysMon.h>

#include "hal_data.h"
#include "types_basic.h"
#include "I2C_2.h"

/******************************************************************************
 ** Macros
 */

/******************************************************************************
 ** Defines
 */

// I/O ports
#define     I2C2_SCL            IOPORT_PORT_02_PIN_02
#define     I2C2_SDA            IOPORT_PORT_02_PIN_03

#define     I2C2_CLK_PULSE      { g_ioport.p_api->pinWrite (I2C2_SCL, IOPORT_LEVEL_HIGH); \\
		                          g_ioport.p_api->pinWrite (I2C2_SCL, IOPORT_LEVEL_LOW); }



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
 ** Name:    I2C2_StartCondition
 *****************************************************************************/
/**
 ** @brief   Generate the START condition in the i2C line
 **
 ******************************************************************************/
static void I2C2_StartCondition (void)
{
    g_ioport.p_api->pinWrite (I2C2_SDA, IOPORT_LEVEL_HIGH);
    g_ioport.p_api->pinWrite (I2C2_SCL, IOPORT_LEVEL_HIGH);
    g_ioport.p_api->pinWrite (I2C2_SDA, IOPORT_LEVEL_LOW);
    g_ioport.p_api->pinWrite (I2C2_SCL, IOPORT_LEVEL_LOW);
}

/******************************************************************************
 ** Name:    I2C2_StopCondition
 *****************************************************************************/
/**
 ** @brief   Generate the STOP condition in the i2C line
 **
 ******************************************************************************/
static void I2C2_StopCondition (void)
{
    g_ioport.p_api->pinWrite (I2C2_SDA, IOPORT_LEVEL_LOW);
    g_ioport.p_api->pinWrite (I2C2_SCL, IOPORT_LEVEL_HIGH);
    g_ioport.p_api->pinWrite (I2C2_SDA, IOPORT_LEVEL_HIGH);
}

/******************************************************************************
 ** Name:    I2C2_TX_Byte
 *****************************************************************************/
/**
 ** @brief   Function to send a data byte to the i2C device
 **
 ** @param   data --- byte to send
 ** @return  operation error code (0 if no error)
 **
 ******************************************************************************/
static uint8_t I2C2_TX_Byte(uint8_t data)
{
    uint8_t i, mask;

    // Send the data
    mask = 0x80;
    for (i=0; i<8; i++)
    {
        g_ioport.p_api->pinWrite (I2C2_SDA, (mask & data) ? IOPORT_LEVEL_HIGH : IOPORT_LEVEL_LOW);
        g_ioport.p_api->pinWrite (I2C2_SCL, IOPORT_LEVEL_HIGH);
        g_ioport.p_api->pinWrite (I2C2_SCL, IOPORT_LEVEL_LOW);
        mask >>= 0x01;
    }

    // Receive ACK from slave
    g_ioport.p_api->pinWrite (I2C2_SDA, IOPORT_LEVEL_HIGH);
    g_ioport.p_api->pinWrite (I2C2_SCL, IOPORT_LEVEL_HIGH);
    g_ioport.p_api->pinRead  (I2C2_SDA, &mask);
    g_ioport.p_api->pinWrite (I2C2_SCL, IOPORT_LEVEL_LOW);

    // Return the error code in function of the slave acknowledge
    return mask;
}

/******************************************************************************
 ** Name:    I2C2_RX_Byte
 *****************************************************************************/
/**
 ** @brief   Function to get a data byte from the i2C device
 **
 ** @param   ack_flag   if the operation must ACK the slave or not
 ** @return  read data
 **
 ******************************************************************************/
static uint8_t I2C2_RX_Byte (bool_t ack_flag)
{
    uint32_t i, my_data;
    uint8_t  mask;

    // Read the data
    my_data = 0x00;
    g_ioport.p_api->pinWrite (I2C2_SDA, IOPORT_LEVEL_HIGH);
    for (i=0; i<8; i++)
    {
        my_data <<= 0x01;
        g_ioport.p_api->pinWrite (I2C2_SCL, IOPORT_LEVEL_HIGH);
        g_ioport.p_api->pinRead  (I2C2_SDA, &mask);
        g_ioport.p_api->pinWrite (I2C2_SCL, IOPORT_LEVEL_LOW);
        my_data |= mask;
    }

    // Send ACK or NACK condition to the slave
    g_ioport.p_api->pinWrite (I2C2_SDA, ack_flag ? IOPORT_LEVEL_LOW : IOPORT_LEVEL_HIGH);
    g_ioport.p_api->pinWrite (I2C2_SCL, IOPORT_LEVEL_HIGH);
    g_ioport.p_api->pinWrite (I2C2_SCL, IOPORT_LEVEL_LOW);
    g_ioport.p_api->pinWrite (I2C2_SDA, IOPORT_LEVEL_HIGH);

    // Return the read data
    return ((uint8_t) my_data);
}

/******************************************************************************
 ** Name:    I2C2_ReadByte
 *****************************************************************************/
/**
 ** @brief   Function to read a byte data from a specific address
 **
 ** @param   addr   --- slave device address
 ** @param   pData  --- Pointer to memory to store the read data
 **
 ******************************************************************************/
void I2C2_ReadByte(uint8_t addr, uint8_t *pData)
{
    uint8_t error_code;     // Error code in the i2C communication

    if (tx_semaphore_get (&i2c_2_semaphore, TX_WAIT_FOREVER) == TX_SUCCESS)
    {
        // Generates the start condition
        I2C2_StartCondition ();

        // Write the address with READ condition
        error_code = I2C2_TX_Byte (addr | 0x01);
        if (!error_code)
        {
            *pData = I2C2_RX_Byte (FALSE);
        }

        // Generates the stop condition
        I2C2_StopCondition ();

        tx_semaphore_put (&i2c_2_semaphore);
    }
}

/******************************************************************************
 ** Name:    I2C2_Read
 *****************************************************************************/
/**
 ** @brief   Function to read data from a specific address
 **
 ** @param   addr   --- slave device address
 ** @param   data   --- Pointer to memory to store the read data
 ** @param   nBytes --- data size in bytes
 **
 ******************************************************************************/
void I2C2_Read(uint8_t addr, uint8_t *data, uint8_t nBytes)
{
    uint8_t error_code;     // Error code in the i2C communication
    uint8_t i;              // Global use counter

    //Check input parameters ...
    if (!data || !nBytes) return;

    if (tx_semaphore_get (&i2c_2_semaphore, TX_WAIT_FOREVER) == TX_SUCCESS)
    {
        // Generates the start condition
        I2C2_StartCondition ();

        // Write the address with READ condition
        error_code = I2C2_TX_Byte (addr | 0x01);
        if (!error_code)
        {
            // Read data bytes ...
            for (i=0; i<(nBytes-1); i++)
            {
                data[i] = I2C2_RX_Byte (TRUE);
            }

            data[i] = I2C2_RX_Byte (FALSE);
        }

        // Generates the stop condition
        I2C2_StopCondition ();

        tx_semaphore_put (&i2c_2_semaphore);
    }
}

/******************************************************************************
 ** Name:    I2C2_WriteByte
 *****************************************************************************/
/**
 ** @brief   Function to write a data byte at specific address
 **
 ** @param   addr --- slave device address
 ** @param   data --- a byte data to write
 **
 ******************************************************************************/
void I2C2_WriteByte(uint8_t addr, uint8_t data)
{
    uint8_t error_code;     // Error code in the i2C communication

    if (tx_semaphore_get (&i2c_2_semaphore, TX_WAIT_FOREVER) == TX_SUCCESS)
    {
        // Generates the start condition
        I2C2_StartCondition ();

        // Write the address
        error_code = I2C2_TX_Byte (addr);
        if (!error_code)
        {
            error_code = I2C2_TX_Byte (data);
        }

        // Generates the stop condition
        I2C2_StopCondition ();

        tx_semaphore_put (&i2c_2_semaphore);
    }
}

/******************************************************************************
 ** Name:    I2C2_Write
 *****************************************************************************/
/**
 ** @brief   Function to write data at specific address
 **
 ** @param   addr   --- slave device address
 ** @param   data   --- data buffer to write
 ** @param   nBytes --- number of data in bytes
 **
 ******************************************************************************/
void I2C2_Write(uint8_t addr, uint8_t *data, uint8_t nBytes)
{
    uint8_t error_code;     // Error code in the i2C communication
    uint8_t i;              // Global use counter

    if (tx_semaphore_get (&i2c_2_semaphore, TX_WAIT_FOREVER) == TX_SUCCESS)
    {
        // Generates the start condition
        I2C2_StartCondition ();

        // Write the address and continue with data
        error_code = I2C2_TX_Byte (addr);
        for (i=0; !error_code && (i<nBytes); i++)
        {
            error_code = I2C2_TX_Byte (data[i]);
        }

        // Generates the stop condition
        I2C2_StopCondition ();

        tx_semaphore_put (&i2c_2_semaphore);
    }
}


/******************************************************************************
 ** Name:    I2C2_Write_NFC
 *****************************************************************************/
/**
 ** @brief   Function to write data at specific address in the NTag memory
 **
 ** @param   block_idx   --- block idx
 ** @param   data        --- data buffer to write
 ** @param   nBytes      --- number of data in bytes
 **
 ******************************************************************************/
void I2C2_Write_NFC (uint8_t block_idx, uint8_t *data, uint8_t nBytes)
{
    uint8_t error_code;     // Error code in the i2C communication
    uint8_t i;              // Global use counter

    if (tx_semaphore_get (&i2c_2_semaphore, TX_WAIT_FOREVER) == TX_SUCCESS)
    {
        // Generates the start condition
        I2C2_StartCondition ();

        // Write the address and continue with data
        error_code = I2C2_TX_Byte (i2C_ADD_NTAG & 0xFE);
        error_code = I2C2_TX_Byte (block_idx);
        for (i=0; !error_code && (i<nBytes); i++)
        {
            error_code = I2C2_TX_Byte (data[i]);
        }

        // Generates the stop condition
        I2C2_StopCondition ();

        tx_semaphore_put (&i2c_2_semaphore);
    }
}

/******************************************************************************
 ** Name:    I2C2_Write_OW
 *****************************************************************************/
/**
 ** @brief   Function to write a data byte at specific address
 **
 ** @param   command --- command sent to DS2482
 ** @param   data    --- a byte data to write
 **
 ******************************************************************************/
void I2C2_Write_OW(uint8_t command, uint8_t data)
{
    uint8_t error_code;     // Error code in the i2C communication

    if (tx_semaphore_get (&i2c_2_semaphore, TX_WAIT_FOREVER) == TX_SUCCESS)
    {
        // Generates the start condition
        I2C2_StartCondition ();

        // Write the address
        error_code = I2C2_TX_Byte (i2C_ADD_OW_MASTER);
        if (!error_code)
        {
            error_code = I2C2_TX_Byte (command);
            if ((command == OWC_WRITE_BYTE) ||
                (command == OWC_SET_REG_POINTER))
            {
                error_code = I2C2_TX_Byte (data);
            }
        }

        // Generates the stop condition
        I2C2_StopCondition ();

        tx_semaphore_put (&i2c_2_semaphore);
    }
}

/******************************************************************************
 ** Name:    I2C2_ReadByte_OW
 *****************************************************************************/
/**
 ** @brief   Function to read data byte at specific address
 **
 ** @param   pData        --- pointer to data store
 ** @param   nBytes       --- number of byte data to read
 **
 ******************************************************************************/
void I2C2_ReadByte_OW(uint8_t *pData, uint8_t nBytes)
{
    uint8_t  my_buffer[16];
    uint8_t  status, i;

    UNUSED(pData);
    UNUSED(nBytes);

    my_buffer[0] = OWC_WRITE_BYTE;
    my_buffer[1] = 0xCC;
    I2C2_Write (i2C_ADD_OW_MASTER, my_buffer, 2);
    I2C2_ReadByte (i2C_ADD_OW_MASTER, &status);

    //Busy polling until 1-wire line is freed
    while ((status & 0x01))
    {
        I2C2_ReadByte (i2C_ADD_OW_MASTER, &status);
    }

    my_buffer[1] = 0xF0;
    I2C2_Write (i2C_ADD_OW_MASTER, my_buffer, 2);
    tx_thread_sleep (OSTIME_10MSEC);

    my_buffer[1] = 0x00;
    I2C2_Write (i2C_ADD_OW_MASTER, my_buffer, 2);
    tx_thread_sleep (OSTIME_10MSEC);
    I2C2_Write (i2C_ADD_OW_MASTER, my_buffer, 2);
    tx_thread_sleep (OSTIME_10MSEC);

    my_buffer[0] = OWC_SET_REG_POINTER;
    my_buffer[1] = OWC_REG_READ_DATA;

    for (i=0; i<8; i++)
    {
        I2C2_WriteByte (i2C_ADD_OW_MASTER, OWC_READ_BYTE);
        tx_thread_sleep (OSTIME_10MSEC);
        I2C2_Write (i2C_ADD_OW_MASTER, my_buffer, 2);
        tx_thread_sleep (OSTIME_10MSEC);
        I2C2_ReadByte (i2C_ADD_OW_MASTER, &status);
    }
}
