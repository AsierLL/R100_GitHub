/******************************************************************************
 * Name      : R100_S3A7                                                      *
 * Copyright : Osatu S. Coop                                                  *
 * Compiler  : GNU ARM Embedded                                               *
 * Target    : Reanibex Serie 100                                             *
 ******************************************************************************/

/*!
 * @file        I2C_2.h
 * @brief       Header with functions related to the I2C-2
 *
 * @version     v1
 * @date        16/01/2018
 * @author      ltorres
 * @warning     author   date    version     .- Edition
 * @bug
 *
 */

#ifndef I2C_2_H_
#define I2C_2_H_

/******************************************************************************
 **Includes
 */
#include "types_basic.h"

/******************************************************************************
 ** Defines
 */

// the i2C addresses includes the R/W bit
#define i2C_ADD_GPIO            0x70    ///< Device address for the PCF8574 I/O Expander (Write)
#define i2C_ADD_AUDIO           0x92    ///< Device address for the Audio amplifier (Write)
#define i2C_ADD_NTAG            0xAA    ///< Device address for the nTAG (Write)
#define i2C_ADD_OW_MASTER       0x30    ///< Device address for the DS2482 single-channel 1-Wire master (Write)

// I2C to 1-Wire interface commands (send to i2C-1Wire transceiver)

#define OWC_DEVICE_RESET        0xF0
#define OWC_SET_REG_POINTER     0xE1
#define OWC_WRITE_BYTE          0xA5
#define OWC_READ_BYTE           0x96
#define OWC_1WIRE_RESET         0xB4

// 1-Wire commands
// ROM commands

#define OWC_READ_ROM            0x33
#define OWC_SKIP_ROM            0xCC

// memory commands

#define OW_WRITE_SCRATCHPAD     0x0F
#define OW_READ_SCRATCHPAD      0xAA
#define OW_COPY_SCRATCHPAD      0x55
#define OW_READ_MEMORY          0xF0


// i2C-1Wire transceiver registers

#define OWC_REG_STATUS          0xF0
#define OWC_REG_READ_DATA       0xE1
#define OWC_REG_CONFIGURATION   0xC3


/******************************************************************************
 ** Typedefs
 */

/******************************************************************************
 ** Globals
 */

/******************************************************************************
 ** Prototypes
 */

void    I2C2_ReadByte          (uint8_t addr, uint8_t *data);
void    I2C2_WriteByte         (uint8_t addr, uint8_t  data);
void    I2C2_Read              (uint8_t addr, uint8_t *data, uint8_t nBytes);
void    I2C2_Write             (uint8_t addr, uint8_t *data, uint8_t nBytes);
void    I2C2_Write_NFC         (uint8_t block_idx, uint8_t *data, uint8_t nBytes);
void    I2C2_Write_OW          (uint8_t command, uint8_t data);
void    I2C2_ReadByte_OW       (uint8_t *pData, uint8_t nBytes);

#endif /* I2C_2_H_ */

/*
 ** $Log$
 **
 ** end of I2C_2.h
 ******************************************************************************/

