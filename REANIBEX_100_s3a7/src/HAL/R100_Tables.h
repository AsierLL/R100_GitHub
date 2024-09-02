/******************************************************************************
 * Name      : R100_S3A7                                                      *
 * Copyright : Osatu S. Coop                                                  *
 * Compiler  : GNU ARM Embedded                                               *
 * Target    : Reanibex Serie 100                                             *
 ******************************************************************************/

/*!
 * @file        R100_Tables.h
 * @brief       Header with ZP and Pulse tables definitions
 *
 * @version     v1
 * @date        16/01/2018
 * @author      ltorres
 * @warning     author   date    version     .- Edition
 * @bug
 *
 */

#ifndef R100_TABLES_H_
#define R100_TABLES_H_

/******************************************************************************
 **Includes
 */

#include "types_basic.h"


/******************************************************************************
 ** Defines
 */

#define MAX_ZP_ITEMS            35          ///< Number of entries in the ZP table

#define MAX_ZP_MATRIX_ITEMS     5           ///< Number of entries in the ZP matrix

#define MAX_ZP_ADCS             0x00FFFFFF  ///< Maximum value for ZP in ADCs
#define MAX_ZP_OHMS             9999        ///< Maximum value for ZP in Ohms


/******************************************************************************
 ** Typedefs
 */

///< Define the ZP Table structure
typedef struct ZP_TABLE_STRUCT
{
    uint32_t    ohms;           ///< Impedance value in ohms
    uint32_t    adcs;           ///< Impedance value in ADCs
    float_t     ikte;           ///< Interpolation kte
} ZP_TABLE_t;
 
///< Define the ZP Matrix structure
typedef struct ZP_MATRIX_STRUCT
{
    int32_t     temp;           ///< Temperature
    float_t     adc_min;        ///< Minimum value in table (zero)
    float_t     adc_max;        ///< Maximum value in table (near 6K)
    float_t     adc_cal;        ///< Calibration value in ADCs
} ZP_MATRIX_t;

///< Define the Energy Table structure
typedef struct ENERGY_TABLE_STRUCT
{
    uint16_t    zp_ohms;        ///< Impedance value in ohms
    uint16_t    t1_time;        ///< T1 time value in usecs
    uint16_t    v_e50;          ///< Voltage capacitor for  50 Joules
    uint16_t    v_e65;          ///< Voltage capacitor for  65 Joules
    uint16_t    v_e75;          ///< Voltage capacitor for  75 Joules
    uint16_t    v_e90;          ///< Voltage capacitor for  90 Joules
    uint16_t    v_e100;         ///< Voltage capacitor for 100 Joules
    uint16_t    v_e150;         ///< Voltage capacitor for 150 Joules
    uint16_t    v_e175;         ///< Voltage capacitor for 175 Joules
    uint16_t    v_e200;         ///< Voltage capacitor for 200 Joules
} ENERGY_TABLE_t;


/******************************************************************************
 ** Globals
 */
extern const ZP_TABLE_t     golden_zp_table  [];    ///< Zp Table to convert ADCs to Ohms
extern const ZP_MATRIX_t    golden_zp_matrix [];    ///< Matrix to compensate ADC deviations
extern const ENERGY_TABLE_t energy_table   [];      ///< Energy table to assign times and voltages
 
/******************************************************************************
 ** Prototypes
 */


#endif /* R100_TABLES_H_ */

/*
 ** $Log$
 **
 ** end of R100_Tables.h
 ******************************************************************************/
