/******************************************************************************
 * Name      : R100_S3A7                                                      *
 * Copyright : Osatu S. Coop                                                  *
 * Compiler  : GNU ARM Embedded                                               *
 * Target    : Reanibex Serie 100                                             *
 ******************************************************************************/

/*!
 * @file        Accelerometer.h
 * @brief       LIS3DH accelerometer header file
 *
 * @version     v1
 * @date        19/11/2020
 * @author      lsanz
 * @warning     author   date    version     .- Edition
 * @bug
 *
 */

#ifndef BSP_ACC_LIS3DH_H_
#define BSP_ACC_LIS3DH_H_

/******************************************************************************
 **Includes
 */
#include "R100_Errors.h"
#include "types_basic.h"

/******************************************************************************
 ** Defines
 */

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// ST LIS3DH Register Address Map
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#define LIS3DH_STATUS_REG_AUX         0x07
#define LIS3DH_WHO_AM_I               0x0F
#define LIS3DH_WHO_AM_I_VALUE         0x33

#define LIS3DH_CTRL_REG1              0x20
#define LIS3DH_CTRL_REG2              0x21
#define LIS3DH_CTRL_REG3              0x22
#define LIS3DH_CTRL_REG4              0x23
#define LIS3DH_CTRL_REG5              0x24
#define LIS3DH_CTRL_REG6              0x25
#define LIS3DH_REFERENCE              0x26
#define LIS3DH_STATUS_REG2            0x27
#define LIS3DH_OUT_X_L                0x28
#define LIS3DH_OUT_X_H                0x29
#define LIS3DH_OUT_Y_L                0x2A
#define LIS3DH_OUT_Y_H                0x2B
#define LIS3DH_OUT_Z_L                0x2C
#define LIS3DH_OUT_Z_H                0x2D
#define LIS3DH_FIFO_CTRL_REG          0x2E
#define LIS3DH_FIFO_SRC_REG           0x2F
#define LIS3DH_INT1_CFG               0x30
#define LIS3DH_INT1_SRC               0x31
#define LIS3DH_INT1_THS               0x32
#define LIS3DH_INT1_DURATION          0x33
#define LIS3DH_INT2_CFG               0x34
#define LIS3DH_INT2_SRC               0x35
#define LIS3DH_INT2_THS               0x36
#define LIS3DH_INT2_DURATION          0x37

#define LIS3DH_CLICK_CFG              0x38
#define LIS3DH_CLICK_SRC              0x39
#define LIS3DH_CLICK_THS              0x3A
#define LIS3DH_TIME_LIMIT             0x3B
#define LIS3DH_TIME_LATENCY           0x3C
#define LIS3DH_TIME_WINDOW            0x3D

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// ST LIS3DH Configuration Parameters
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#define ACCEL_XYZ_DATA_CFG            0x01
#define HP_FILTER_CUTOFF              0x23
#define CTRL_REG3_CFG                 0x00
#define CTRL_REG4_CFG                 0x01
#define CTRL_REG1_CFG1                0x18
#define CTRL_REG1_CFG2                0x19

#define ACC_MEASUREMENT_RANGE         2                 ///< +/-2g -> thresold 40-80, 4g -> thresold 20-40, 8g -> thresold 10-20 or 16g -> thresold 5-10
#define ACC_THRESOLD                  25                ///< Thresold value FF (umbral)
#define ACC_THRESOLD_Z                17                ///< Thresold value FF (umbral)
#define ACC_DURATION                  0x0A //100        ///< Duration (t = N/ODR) (e.g: ODR = 25 Hz -> t = 1/25=0,04sec=40msec)
#define ACC_POLARITY_HIGH             0x00              ///< Configure polarity active high
#define ACC_POLARITY_LOW              0x02              ///< Configure polarity active low

#define ACC_ENABLE_XYZ_HIGH_AXIS      0x2A              ///< Enable XYZ axis high event
#define ACC_ENABLE_XYZ_LOW_AXIS       0x15              ///< Enable XYZ axis low event
#define ACC_ENABLE_XY_HIGH_AXIS       0x0A              ///< Enable XY axis high event
#define ACC_ENABLE_Z_HIGH_AXIS        0x20              ///< Enable Z axis high event
#define ACC_ENABLE_Z_LOW_AXIS         0x10              ///< Enable Z axis low event
#define ACC_DISABLE_INT               0x00              ///< Enable Z axis low event

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// ST LIS3DH Device Address
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#define ACCEL_DEVICE_ADD              0x19              ///< If SA0 pad is connected to voltage supply, LSb is ‘1’ (address 0011001b) = 0x19

/******************************************************************************
 ** Typedefs
 */

/******************************************************************************
 ** Globals
 */

/******************************************************************************
 ** Prototypes
 */

ERROR_ID_e          Accelerometer_Presence          (void);
ERROR_ID_e          Accelerometer_Setup             (uint8_t enable_xyz, uint8_t sampleRate, uint8_t accelRange);
ERROR_ID_e          Accel_Interrupt_Init            (uint8_t enable_axis, uint8_t threshold, uint8_t duration, uint8_t polarity);
ERROR_ID_e          ACC_Get_Axis_Data               (uint8_t reg_address, uint8_t *read_reg_data);

#endif /* BSP_ACC_LIS3DH_H_ */
