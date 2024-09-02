/******************************************************************************
* Name      : Defibrillable Rhythm Detector                                      *
* Copyright : Osatu S.Coop                                                    *
* Compiler  : Renesas C/C++ compiler for SH2                                  *
* Target    : Reanibex Series                                                 *
******************************************************************************/

/*!
* @file        functions.h
* @brief       Basic Types Definition
* @version     0.0
* @date        2011-09-21
* @author      EG (egarcia@osatu.com)
* @warning     EG        2011-09-21        V0.0    .- First edition
* @bug
*
*/

#ifndef FUNCTIONS_H
#define FUNCTIONS_H
/******************************************************************************
** Includes
*/
/******************************************************************************
** Defines
*/

#define limf1_FREC        0.75
#define limf2_FREC        40


/******************************************************************************
** Typedefs
*/

/******************************************************************************
** Prototypes
*/
void    function_ordenar_min (int32_t *pSig);
int32_t function_calcular_Pot (int32_t *pSig, int32_t L);
int32_t function_calcular_suma (int32_t *pSig, int32_t L);
int32_t function_calcular_max (int32_t *pSig);
int32_t function_position_max (int32_t *pSig);
float_t function_calcular_freccorte (int32_t *pSign, float_t val);
int32_t function_find_pos (float_t limite);
float_t function_calcular_max1 (float_t *pSig);
void    function_calcular_FFT (int32_t *sfil, int32_t *Amp);

void    AutoCorrelation (int32_t *pcorr, int32_t *x);

/******************************************************************************
*/
#endif  /*FUNCTIONS_H*/
