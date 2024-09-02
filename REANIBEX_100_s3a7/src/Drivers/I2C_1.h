/******************************************************************************
 * Name      : R100_S3A7                                                      *
 * Copyright : Osatu S. Coop                                                  *
 * Compiler  : GNU ARM Embedded                                               *
 * Target    : Reanibex Serie 100                                             *
 ******************************************************************************/

/*!
 * @file        I2C_1.h
 * @brief       Header with functions related to the I2C-1
 *
 * @version     v1
 * @date        16/01/2018
 * @author      ltorres
 * @warning     author   date    version     .- Edition
 * @bug
 *
 */

#ifndef I2C_1_H_
#define I2C_1_H_

/******************************************************************************
 **Includes
 */
#include "types_basic.h"

/******************************************************************************
 ** Defines
 */

// The I2C addresses includes the R/W bit
#define i2C_ADD_RTC             0xD0        ///< I2C address for DS1342 RTC (Write)
#define i2C_ADD_EEPROM          0xA0        ///< I2C address for M24C08 EEPROM memory (8 Kbit) (Write)
#define i2C_ADD_TEMPERATURE     0x9E        ///< I2C address for TMP1075 Temperature Sensor (Write)
#define i2C_ADD_ACCEL           0x32        ///< I2C address for LIS3DH accelerometer (Write)


/******************************************************************************
 ** Typedefs
 */

/******************************************************************************
 ** Globals
 */

/******************************************************************************
 ** Prototypes
 */

void    I2C1_ReadByte          (uint8_t addr, uint8_t *data);
void    I2C1_WriteByte         (uint8_t addr, uint8_t  data);
void    I2C1_Read              (uint8_t addr, uint8_t *data, uint8_t nBytes);
void    I2C1_Write             (uint8_t addr, uint8_t *data, uint8_t nBytes);

void    I2C1_Read_RTC          (uint8_t *data, uint8_t nBytes);
uint8_t I2C1_Read_Temperature  ( int8_t *pTemp);

uint8_t I2C1_Read_Eeprom       (uint16_t addr, uint8_t *data, uint8_t nBytes);
void    I2C1_Write_Eeprom      (uint16_t addr, uint8_t *data, uint8_t nBytes);

void    I2C1_WriteRegister_ACC (uint8_t regAddress, uint8_t data);
void    I2C1_ReadRegister_ACC  (uint8_t regAddress, uint8_t *pData);

#endif /* I2C_1_H_ */

/*
 ** $Log$
 **
 ** end of I2C_1.h
 ******************************************************************************/

