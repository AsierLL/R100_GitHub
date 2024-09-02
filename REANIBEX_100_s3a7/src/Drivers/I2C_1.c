/******************************************************************************
 * Name      : R100_S3A7                                                      *
 * Copyright : Osatu S. Coop                                                  *
 * Compiler  : GNU ARM Embedded                                               *
 * Target    : Reanibex Serie 100                                             *
 ******************************************************************************/

/*!
 * @file        I2C_1.c
 * @brief       All functions related to the I2C-1
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
#include "I2C_1.h"

/******************************************************************************
 ** Macros
 */

/******************************************************************************
 ** Defines
 */

// I/O ports
#define     I2C1_SCL            IOPORT_PORT_04_PIN_00
#define     I2C1_SDA            IOPORT_PORT_04_PIN_01

#define     I2C1_CLK_PULSE      { g_ioport.p_api->pinWrite (I2C1_SCL, IOPORT_LEVEL_HIGH); \\
		                          g_ioport.p_api->pinWrite (I2C1_SCL, IOPORT_LEVEL_LOW); }



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
 ** Name:    I2C1_StartCondition
 *****************************************************************************/
/**
 ** @brief   Generate the START condition in the i2C line
 **
 ******************************************************************************/
static void I2C1_StartCondition (void)
{
    g_ioport.p_api->pinWrite (I2C1_SDA, IOPORT_LEVEL_HIGH);
    g_ioport.p_api->pinWrite (I2C1_SCL, IOPORT_LEVEL_HIGH);
    g_ioport.p_api->pinWrite (I2C1_SDA, IOPORT_LEVEL_LOW);
    g_ioport.p_api->pinWrite (I2C1_SCL, IOPORT_LEVEL_LOW);
}

/******************************************************************************
 ** Name:    I2C1_StopCondition
 *****************************************************************************/
/**
 ** @brief   Generate the STOP condition in the i2C line
 **
 ******************************************************************************/
static void I2C1_StopCondition (void)
{
    g_ioport.p_api->pinWrite (I2C1_SDA, IOPORT_LEVEL_LOW);
    g_ioport.p_api->pinWrite (I2C1_SCL, IOPORT_LEVEL_HIGH);
    g_ioport.p_api->pinWrite (I2C1_SDA, IOPORT_LEVEL_HIGH);
}

/******************************************************************************
 ** Name:    I2C1_TX_Byte
 *****************************************************************************/
/**
 ** @brief   Function to send a data byte to the i2C device
 **
 ** @param   data --- byte to send
 **
 ** @return  operation error code (0 if no error)
 ******************************************************************************/
static uint8_t I2C1_TX_Byte(uint8_t data)
{
    uint8_t i, mask;

    // Send the data
    mask = 0x80;
    for (i=0; i<8; i++)
    {
        g_ioport.p_api->pinWrite (I2C1_SDA, (mask & data) ? IOPORT_LEVEL_HIGH : IOPORT_LEVEL_LOW);
        g_ioport.p_api->pinWrite (I2C1_SCL, IOPORT_LEVEL_HIGH);
        g_ioport.p_api->pinWrite (I2C1_SCL, IOPORT_LEVEL_LOW);
        mask >>= 0x01;
    }

    // Receive ACK from slave
    g_ioport.p_api->pinWrite (I2C1_SDA, IOPORT_LEVEL_HIGH);
    g_ioport.p_api->pinWrite (I2C1_SCL, IOPORT_LEVEL_HIGH);
    g_ioport.p_api->pinRead  (I2C1_SDA, &mask);
    g_ioport.p_api->pinWrite (I2C1_SCL, IOPORT_LEVEL_LOW);

    // Return the error code in function of the slave acknowledge
    return mask;
}

/******************************************************************************
 ** Name:    I2C1_RX_Byte
 *****************************************************************************/
/**
 ** @brief   Function to get a data byte from the i2C device
 **
 ** @param   ack_flag   if the operation must ACK the slave or not
 **
 ** @return  read data
 ******************************************************************************/
static uint8_t I2C1_RX_Byte (bool_t ack_flag)
{
    uint32_t i, my_data;
    uint8_t  mask;

    // Read the data
    my_data = 0x00;
    g_ioport.p_api->pinWrite (I2C1_SDA, IOPORT_LEVEL_HIGH);
    for (i=0; i<8; i++)
    {
        my_data <<= 0x01;
        g_ioport.p_api->pinWrite (I2C1_SCL, IOPORT_LEVEL_HIGH);
        g_ioport.p_api->pinRead  (I2C1_SDA, &mask);
        g_ioport.p_api->pinWrite (I2C1_SCL, IOPORT_LEVEL_LOW);
        my_data |= mask;
    }

    // Send ACK or NACK condition to the slave
    g_ioport.p_api->pinWrite (I2C1_SDA, ack_flag ? IOPORT_LEVEL_LOW : IOPORT_LEVEL_HIGH);
    g_ioport.p_api->pinWrite (I2C1_SCL, IOPORT_LEVEL_HIGH);
    g_ioport.p_api->pinWrite (I2C1_SCL, IOPORT_LEVEL_LOW);
    g_ioport.p_api->pinWrite (I2C1_SDA, IOPORT_LEVEL_HIGH);

    // Return the read data
    return ((uint8_t) my_data);
}

/******************************************************************************
 ** Name:    I2C1_ReadByte
 *****************************************************************************/
/**
 ** @brief   Function to read a byte data from a specific address
 **
 ** @param   addr   --- slave device address
 ** @param   pData  --- Pointer to memory to store the read data
 **
 ******************************************************************************/
void I2C1_ReadByte(uint8_t addr, uint8_t *pData)
{
    uint8_t error_code;     // error code in the i2C communication

    if (tx_semaphore_get (&i2c_1_semaphore, TX_WAIT_FOREVER) == TX_SUCCESS)
    {
        // Generates the start condition
        I2C1_StartCondition ();

        // Write the address with READ condition
        error_code = I2C1_TX_Byte (addr | 0x01);
        if (!error_code)
        {
            *pData = I2C1_RX_Byte (FALSE);
        }

        // Generates the stop condition
        I2C1_StopCondition ();

        tx_semaphore_put (&i2c_1_semaphore);
    }
}

/******************************************************************************
 ** Name:    I2C1_Read
 *****************************************************************************/
/**
 ** @brief   Function to read data from a specific address
 **
 ** @param   addr   --- slave device address
 ** @param   data   --- Pointer to memory to store the read data
 ** @param   nBytes --- data size in bytes
 **
 ******************************************************************************/
void I2C1_Read(uint8_t addr, uint8_t *data, uint8_t nBytes)
{
    uint8_t error_code;     // Error code in the i2C communication
    uint8_t i;              // Global use counter

    //Check input parameters ...
    if (!data || !nBytes) return;

    // Generates the start condition
    I2C1_StartCondition ();

    // Write the address with READ condition
    error_code = I2C1_TX_Byte (addr | 0x01);
    if (!error_code)
    {
        // Read data bytes ...
        for (i=0; i<(nBytes-1); i++)
        {
            data[i] = I2C1_RX_Byte (TRUE);
        }

        data[i] = I2C1_RX_Byte (FALSE);
    }
    else {
        memset (data, 0, nBytes);
    }

    // Generates the stop condition
    I2C1_StopCondition ();
}

/******************************************************************************
 ** Name:    I2C1_WriteByte
 *****************************************************************************/
/**
 ** @brief   Function to write a data byte at specific address
 **
 ** @param   addr --- slave device address
 ** @param   data --- a byte data to write
 **
 ******************************************************************************/
void I2C1_WriteByte(uint8_t addr, uint8_t data)
{
    uint8_t error_code;     // Error code in the i2C communication

    if (tx_semaphore_get (&i2c_1_semaphore, TX_WAIT_FOREVER) == TX_SUCCESS)
    {
        // Generates the start condition
        I2C1_StartCondition ();

        // Write the address
        error_code = I2C1_TX_Byte (addr);
        if (!error_code)
        {
            error_code = I2C1_TX_Byte (data);
        }

        // Generates the stop condition
        I2C1_StopCondition ();

        tx_semaphore_put (&i2c_1_semaphore);
    }
}

/******************************************************************************
 ** Name:    I2C1_Write
 *****************************************************************************/
/**
 ** @brief   Function to write data at specific address
 **
 ** @param   addr   --- slave device address
 ** @param   data   --- data buffer to write
 ** @param   nBytes --- number of data in bytes
 **
 ******************************************************************************/
void I2C1_Write(uint8_t addr, uint8_t *data, uint8_t nBytes)
{
    uint8_t error_code;     // Error code in the i2C communication
    uint8_t i;              // Global use counter

    // Generates the start condition
    I2C1_StartCondition ();

    // Write the address and continue with data
    error_code = I2C1_TX_Byte (addr);
    for (i=0; !error_code && (i<nBytes); i++)
    {
        error_code = I2C1_TX_Byte (data[i]);
    }

    // Generates the stop condition
    I2C1_StopCondition ();
}

/******************************************************************************
 ** Name:    I2C1_Read_RTC
 *****************************************************************************/
/**
 ** @brief   Function to read the RTC date and time data
 **
 ** @param   data   --- Pointer to memory to store the read data
 ** @param   nBytes --- data size in bytes
 **
 ******************************************************************************/
void    I2C1_Read_RTC (uint8_t *data, uint8_t nBytes)
{
    uint8_t     error_code;     // Error code in the i2C communication

    if (tx_semaphore_get (&i2c_1_semaphore, TX_WAIT_FOREVER) == TX_SUCCESS)
    {
        // Generates the start condition
        I2C1_StartCondition ();

        // Write the I2C address
        error_code = I2C1_TX_Byte (i2C_ADD_RTC);
        if (!error_code)
        {
            error_code = I2C1_TX_Byte (0x00);   // read from address 0x00
            if(!error_code)
            {
                I2C1_Read (i2C_ADD_RTC, data, nBytes);
                tx_semaphore_put (&i2c_1_semaphore);
                // The read function includes the Stop condition ...
                return;
            }
        }

        // Generates the stop condition
        I2C1_StopCondition ();

        tx_semaphore_put (&i2c_1_semaphore);
    }
}

/******************************************************************************
 ** Name:    I2C1_Read_Temperature
 *****************************************************************************/
/**
 ** @brief   Function to read the temperature from the temperature sensor
 **
 ** @param   pTemp     --- a temperature data pointer
 **
 ** @return  error code
 ******************************************************************************/
uint8_t    I2C1_Read_Temperature (int8_t *pTemp)
{
    uint8_t     error_code = 0;     // Error code in the i2C communication
    uint8_t     my_buffer[4];       // Buffer to write a single register

    if (!pTemp) return 0;

    //////////////////////////////
    // Command a new conversion
    my_buffer[0] = 0x01;        // Select the configuration register
    my_buffer[1] = 0x81;        // One shot mode and maintains shutdown mode

    if (tx_semaphore_get (&i2c_1_semaphore, TX_WAIT_FOREVER) == TX_SUCCESS)
    {
        I2C1_Write (i2C_ADD_TEMPERATURE, my_buffer, 2);

        // Generates the start condition
        I2C1_StartCondition ();

        // Write the I2C address
        error_code = I2C1_TX_Byte (i2C_ADD_TEMPERATURE);
        if (!error_code)
        {
            error_code = I2C1_TX_Byte (0x00);   // Read from address 0x00
            if(!error_code)
            {
                I2C1_StartCondition ();
                // Restart condition to read from read data register
                error_code = I2C1_TX_Byte (i2C_ADD_TEMPERATURE | 0x01);
                if(!error_code)
                {
                    *pTemp = (int8_t) I2C1_RX_Byte (FALSE);
                }
            }
        }

        // In case of error consider the worst temperature for charging the capacitor
        if (error_code)
        {
            *pTemp = 2;
        }

        // Generates the stop condition
        I2C1_StopCondition ();

        tx_semaphore_put (&i2c_1_semaphore);
    }

    return error_code;
}

/******************************************************************************
 ** Name:    I2C1_Read_Eeprom
 *****************************************************************************/
/**
 ** @brief   Function to read EEPROM data from a specific address
 **
 ** @param   addr   --- EEPROM memory address to read from
 ** @param   data   --- Pointer to memory to store the read data
 ** @param   nBytes --- data size in bytes
 **
 ** @return  error code
 ******************************************************************************/
uint8_t    I2C1_Read_Eeprom (uint16_t addr, uint8_t *data, uint8_t nBytes)
{
    uint8_t     error_code;     // Error code in the i2C communication
    uint8_t     dev_add;        // Device address

    if (tx_semaphore_get (&i2c_1_semaphore, TX_WAIT_FOREVER) == TX_SUCCESS)
    {
        // Generates the start condition
        I2C1_StartCondition ();

        // Assign the device address with the memory address extension bits
        dev_add = (uint8_t) (i2C_ADD_EEPROM + ((addr >> 7) & 0x06));

        // Write the I2C address
        error_code = I2C1_TX_Byte (dev_add);
        if (!error_code)
        {
            error_code = I2C1_TX_Byte ((uint8_t) (addr & 0xFF));   // Read from selected address
            if(!error_code)
            {
                I2C1_Read (dev_add, data, nBytes);
                tx_semaphore_put (&i2c_1_semaphore);
                // The read function includes the Stop condition ...
                return error_code;
            }
        }

        // Generates the stop condition
        I2C1_StopCondition ();

        tx_semaphore_put (&i2c_1_semaphore);
    }
    return error_code;
}

/******************************************************************************
 ** Name:    I2C1_Write_Eeprom
 *****************************************************************************/
/**
 ** @brief   Function to write EEPROM data at specific address
 **
 ** @param   addr   --- EEPROM memory address to write in
 ** @param   data   --- data buffer to write
 ** @param   nBytes --- number of data in bytes
 **
 ******************************************************************************/
void    I2C1_Write_Eeprom (uint16_t addr, uint8_t *data, uint8_t nBytes)
{
    uint8_t     error_code;     // Error code in the i2C communication
    uint8_t     dev_add;        // Device address

    if (tx_semaphore_get (&i2c_1_semaphore, TX_WAIT_FOREVER) == TX_SUCCESS)
    {
        // Generates the start condition
        I2C1_StartCondition ();

        // Assign the device address with the memory address extension bits
        dev_add = (uint8_t) (i2C_ADD_EEPROM + ((addr >> 7) & 0x06));

        // Write the I2C address
        error_code = I2C1_TX_Byte (dev_add);
        if (!error_code)
        {
            error_code = I2C1_TX_Byte ((uint8_t) (addr & 0xFF));   // Read from selected address
            if(!error_code)
            {
                I2C1_Write (dev_add, data, nBytes);
                tx_semaphore_put (&i2c_1_semaphore);
                // The write function includes the Stop condition ...
                return;
            }
        }

        // Generates the stop condition
        I2C1_StopCondition ();

        tx_semaphore_put (&i2c_1_semaphore);
    }
}

/******************************************************************************
 ** Name:    I2C1_WriteRegister_ACC
 *****************************************************************************/
/**
 ** @brief   Function to write data byte at specific register address
 **
 ** @param   regAddr   --- address of the device register
 ** @param   pData     --- a byte data to write
 **
 ******************************************************************************/
void I2C1_WriteRegister_ACC(uint8_t regAddress, uint8_t data)
{
    uint8_t error_code;     // Error code in the i2C communication

    if (tx_semaphore_get (&i2c_1_semaphore, TX_WAIT_FOREVER) == TX_SUCCESS)
    {
        // Generates the start condition
        I2C1_StartCondition ();

        // Write the address
        error_code = I2C1_TX_Byte (i2C_ADD_ACCEL);
        if (!error_code)
        {
            error_code = I2C1_TX_Byte (regAddress);
            if (!error_code)
            {
                error_code = I2C1_TX_Byte (data);
            }
        }

        // Generates the stop condition
        I2C1_StopCondition ();

        tx_semaphore_put (&i2c_1_semaphore);
    }
}

/******************************************************************************
 ** Name:    I2C1_ReadRegister_ACC
 *****************************************************************************/
/**
 ** @brief   Function to read data byte at specific register address
 **
 ** @param   regAddr   --- address of the device register
 ** @param   pData     --- a byte data to read
 **
 ******************************************************************************/
void I2C1_ReadRegister_ACC(uint8_t regAddress, uint8_t *pData)
{
    uint8_t error_code;     // Error code in the i2C communication

    if (tx_semaphore_get (&i2c_1_semaphore, TX_WAIT_FOREVER) == TX_SUCCESS)
    {
        // Generates the start condition
        I2C1_StartCondition ();

        // Write the I2C address
        error_code = I2C1_TX_Byte (i2C_ADD_ACCEL);
        if (!error_code)
        {
            error_code = I2C1_TX_Byte (regAddress);       // Command to set pointer of DS2482 read registers
            if(!error_code)
            {
                I2C1_StartCondition ();
                // Restart condition to read from read data register
                error_code = I2C1_TX_Byte (i2C_ADD_ACCEL | 0x01);
                if(!error_code)
                {
                    *pData = I2C1_RX_Byte (FALSE);
                }
            }
        }

        // Generates the stop condition
        I2C1_StopCondition ();

        tx_semaphore_put (&i2c_1_semaphore);
    }
}
