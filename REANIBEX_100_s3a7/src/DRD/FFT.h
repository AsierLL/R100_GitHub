/******************************************************************************
* Name      : Defibrillable Rhythm Detector                                      *
* Copyright : Osatu S.Coop                                                    *
* Compiler  : Renesas C/C++ compiler for SH2                                  *
* Target    : Reanibex Series                                                 *
******************************************************************************/

/*!
* @file        FFT.h
* @brief
* @version     0.0
* @date        2011-09-21
* @author      EG (egarcia@osatu.com)
* @warning     EG        2011-09-21        V0.0    .- First edition
* @bug
*
*/

#ifndef FFT_H
#define FFT_H

/******************************************************************************
** Includes
*/

/******************************************************************************
** Defines
*/

// Scale definitions
#define EFFTALLSCALE    (-1L)
#define EFFTMIDSCALE    0x55555555L
#define EFFTNOSCALE     0x00000000L

/******************************************************************************
** Typedefs
*/

/******************************************************************************
** Prototypes
*/
void FftReal    (int32_t output[], int32_t size, int32_t scale);
void GenBitRev  (int32_t size);
void GenTwiddle (int32_t sz);

#endif /*FFT_H*/

/*
** $Log$
**
** end of Fft.h
******************************************************************************/

