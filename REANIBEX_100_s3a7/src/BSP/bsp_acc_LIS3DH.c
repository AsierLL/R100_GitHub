/******************************************************************************
 * Name      : R100_S3A7                                                      *
 * Copyright : Osatu S. Coop                                                  *
 * Compiler  : GNU ARM Embedded                                               *
 * Target    : Reanibex Serie 100                                             *
 ******************************************************************************/

/*!
 * @file        Accelerometer.c
 * @brief       All functions related to the accelerometer
 *
 * @version     v1
 * @date        10/10/2020
 * @author      lsanz
 * @warning     author   date    version     .- Edition
 * @bug
 *
 */

/******************************************************************************
 ** Includes
 */

#include "I2C_1.h"
#include "bsp_acc_LIS3DH.h"
#include "thread_sysMon.h"
#include "common_data.h"
#include "Trace.h"

/******************************************************************************
 ** Macros
 */

/******************************************************************************
 ** Defines
 */

#define LOW_POWER

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
** Name:    Accelerometer_Presence
*****************************************************************************/
/**
** @brief   Checks for presence of accelerometer by checking who_am_i register
**
** @param   none
**
** @return  bool_t  return presence boolean
******************************************************************************/
ERROR_ID_e Accelerometer_Presence (void)
{
    uint8_t     read_reg = 0;

    I2C1_ReadRegister_ACC (LIS3DH_WHO_AM_I, &read_reg);

    // if identity register OK return affirmative presence
    if (read_reg == LIS3DH_WHO_AM_I_VALUE)
    {
        return eERR_NONE;
    }
    return eERR_COMM_ACC_PRESENCE;
}

/******************************************************************************
** Name:    Accelerometer_Setup
*****************************************************************************/
/**
** @brief   Setup basic configuration of accelerometer
**
** @param   enable_xyz  Variable to enable/disable acc axis
** @param   sampleRate  HZ - Samples per second - 1, 10, 25, 50, 100, 200, 400, 1600, 5000 (values 0x01 to 0x09)
** @param   accelRange  Accelerometer range = 2, 4, 8, 16g (values 0x00 to 0x03)
**
** @return  acc control register errors
******************************************************************************/
ERROR_ID_e Accelerometer_Setup (uint8_t enable_xyz, uint8_t sampleRate, uint8_t accelRange)
{
    uint8_t write_reg_data = 0;     // Register value to write
    uint8_t read_reg_data = 0;        // To read register value

    // low power mode enabled or not
    #ifdef LOW_POWER
    write_reg_data |= 0x08;
    #endif

    // accelerometer sample rate
    write_reg_data |= (uint8_t) (sampleRate << 4);

    // enable all axis of accelerometer
    write_reg_data |= enable_xyz;

    // write in register accel sample rate and axis enable
    I2C1_WriteRegister_ACC (LIS3DH_CTRL_REG1, write_reg_data);

    I2C1_ReadRegister_ACC (LIS3DH_CTRL_REG1, &read_reg_data);
    // Check Register Configured Value
    if(read_reg_data != write_reg_data)
    {
        return eERR_COMM_ACC_CONFIGURATION;
    }

    // accelerometer scale configuration
    write_reg_data = (uint8_t) accelRange;
    // Self test enable && SPI serial interface mode selection
    write_reg_data |= 0x07;

    // write in register scale config
    I2C1_WriteRegister_ACC (LIS3DH_CTRL_REG4, write_reg_data);

    I2C1_ReadRegister_ACC (LIS3DH_CTRL_REG4, &read_reg_data);
    // Check Register Configured Value
    if(read_reg_data != write_reg_data)
    {
        return eERR_COMM_ACC_CONFIGURATION;
    }

    return eERR_NONE;
}

/******************************************************************************
** Name:    Accel_Interrupt_Init
*****************************************************************************/
/**
** @brief   Initialize the interruption reg and configure interruption threshold of accelerometer
**
** @param   enable_axis     Variable to enable/disable acc axis interruption
** @param   threshold       7bit, trigger threshold for interruption. Value changes depending on selected scale
**                          1 LSb = 16 mg @ FS = ±2 g
**                          1 LSb = 32 mg @ FS = ±4 g
**                          1 LSb = 62 mg @ FS = ±8 g
**                          1 LSb = 186 mg @ FS = ±16 g
**
** @param   duration        7bit, duration of the interruption after triggered, depends of ODR configuration. (t = N/ODR) (e.g: ODR = 25 Hz -> t = 1/25=0,04sec=40msec)
** @param   polarity        Configure polarity of interrupt
**
** @return  acc interrupt errors
******************************************************************************/
ERROR_ID_e Accel_Interrupt_Init (uint8_t enable_axis, uint8_t threshold, uint8_t duration, uint8_t polarity)
{
    uint8_t write_reg_data = 0;     // Register value to write
    uint8_t read_reg_data = 0;        // To read register value

    // configure interrupt 2 to detect movement in XYZ axis
    //write_reg_data = 0x2A;
    //write_reg_data = 0x0A; // XY axis only
    if(polarity == 0x00)
    {
        I2C1_WriteRegister_ACC (LIS3DH_INT2_CFG, enable_axis);

        I2C1_ReadRegister_ACC (LIS3DH_INT2_CFG, &read_reg_data);
        // Check Register Configured Value
        if(read_reg_data != enable_axis)
        {
            return eERR_COMM_ACC_INTERRUPT;
        }

        // configure interrupt 2 threshold
        I2C1_WriteRegister_ACC (LIS3DH_INT2_THS, threshold);

        I2C1_ReadRegister_ACC (LIS3DH_INT2_THS, &read_reg_data);
        // Check Register Configured Value
        if(read_reg_data != threshold)
        {
            return eERR_COMM_ACC_INTERRUPT;
        }

        // configure interrupt duration
        I2C1_WriteRegister_ACC (LIS3DH_INT2_DURATION, duration);

        I2C1_ReadRegister_ACC (LIS3DH_INT2_DURATION, &read_reg_data);
        // Check Register Configured Value
        if(read_reg_data != duration)
        {
            return eERR_COMM_ACC_INTERRUPT;
        }

        // enable interruption 2 function
        write_reg_data = 0x20 | polarity;
        I2C1_WriteRegister_ACC (LIS3DH_CTRL_REG6, write_reg_data);

        I2C1_ReadRegister_ACC (LIS3DH_CTRL_REG6, &read_reg_data);
        // Check Register Configured Value
        if(read_reg_data != write_reg_data)
        {
            return eERR_COMM_ACC_INTERRUPT;
        }
    }
    // If polarity is active low, enable INT1 configuration
    if(polarity == 0x02)
    {
        // configure interrupt 1 to detect movement in XYZ axis
        I2C1_WriteRegister_ACC (LIS3DH_INT1_CFG, enable_axis);

        I2C1_ReadRegister_ACC (LIS3DH_INT1_CFG, &read_reg_data);
        // Check Register Configured Value
        if(read_reg_data != enable_axis)
        {
            return eERR_COMM_ACC_INTERRUPT;
        }

        // configure interrupt 1 threshold
        I2C1_WriteRegister_ACC (LIS3DH_INT1_THS, threshold);

        I2C1_ReadRegister_ACC (LIS3DH_INT1_THS, &read_reg_data);
        // Check Register Configured Value
        if(read_reg_data != threshold)
        {
            return eERR_COMM_ACC_INTERRUPT;
        }

        // configure interrupt duration
        I2C1_WriteRegister_ACC (LIS3DH_INT1_DURATION, duration);

        I2C1_ReadRegister_ACC (LIS3DH_INT1_DURATION, &read_reg_data);
        // Check Register Configured Value
        if(read_reg_data != duration)
        {
            return eERR_COMM_ACC_INTERRUPT;
        }

        // enable interruption 1 & 2 function on INT2
        write_reg_data = 0x60 | polarity;
        I2C1_WriteRegister_ACC (LIS3DH_CTRL_REG6, write_reg_data);

        I2C1_ReadRegister_ACC (LIS3DH_CTRL_REG6, &read_reg_data);
        // Check Register Configured Value
        if(read_reg_data != write_reg_data)
        {
            return eERR_COMM_ACC_INTERRUPT;
        }
    }

    return eERR_NONE;
}

/******************************************************************************
** Name:    ACC_Get_Axis_Data
*****************************************************************************/
/**
** @brief   Function to read register of accelerometer
**
** @param   reg_address     register to be read
** @param   read_reg_data   read data
**
** @return  none
******************************************************************************/
ERROR_ID_e ACC_Get_Axis_Data(uint8_t reg_address, uint8_t *read_reg_data)
{

    I2C1_ReadRegister_ACC (reg_address, read_reg_data);

    return eERR_NONE;
}


