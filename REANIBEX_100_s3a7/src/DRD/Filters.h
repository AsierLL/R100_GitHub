/******************************************************************************
* Name      : Defibrillable Rhythm Detector                                      *
* Copyright : Osatu S.Coop                                                    *
* Compiler  : Renesas C/C++ compiler for SH2                                  *
* Target    : Reanibex Series                                                 *
******************************************************************************/

/*!
* @file        filters.h
* @brief       Basic Types Definition
* @version     0.0
* @date        2011-09-21
* @author      EG (egarcia@osatu.com)
* @warning     EG        2011-09-21        V0.0    .- First edition
* @bug
*
*/

#ifndef FILTERS_H
#define FILTERS_H

/******************************************************************************
** Includes
*/

/******************************************************************************
** Defines
*/

/******************************************************************************
** Typedefs
*/

/******************************************************************************
** Prototypes
*/
void Ini_Filters (int32_t firstSample);
void IIR_1 (const int32_t *padc, int32_t *sfiltrada);
void IIR_2 (const int32_t *padc, int32_t *sfiltrada/*, float_t *zi0,float_t *prevSam*/);
void IIR_3 (const int32_t *padc, int32_t *sfiltrada/*, float_t *zi0,float_t *prevSam*/);

/******************************************************************************
*/
#endif  /*DRD_H*/
