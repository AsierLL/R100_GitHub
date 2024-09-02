/******************************************************************************
* Name      : Defibrillable Rhythm Detector                                      *
* Copyright : Osatu S.Coop                                                    *
* Compiler  : Renesas C/C++ compiler for SH2                                  *
* Target    : Reanibex Series                                                 *
******************************************************************************/

/*!
* @file        param.h
* @brief       Basic Types Definition
* @version     0.0
* @date        2011-09-21
* @author      EG (egarcia@osatu.com)
* @warning     EG        2011-09-21        V0.0    .- First edition
* @bug
*
*/


#ifndef PARAM_H
#define PARAM_H

/******************************************************************************
** Includes
*/

/******************************************************************************
** Defines
*/

// LIMIT OF PS
#define limAsi_PS			6037L

// LIMITS AND DEFINITONS OF bWT
#define valbWT				47
#define posHigh_BWT			175				// posHigh=int32_t((100+valbWT)/2*SNMW/100). L�mite superior 
#define posLow_BWT			63				// posLow=int32_t((100-valbWT)/2*SNMW/100). L�mite inferior

// LIMIT OF bCP
#define valbCP				0.0055

// LIMIT OF BW
#define limInf_bW			0.2
#define limSup_bW			0.8

// LIMITS AND DEFINITIONS OF ASISPEND
#define nW_ASISPEND			6L
#define limPP_ASISPEND		136L

// LIMITS AND DEFINITIOS OF ALGORITMO_TSVTV
#define	limF_TSVTV			10
#define	pbW_TSVTV			3
#define	pPF_REG				-0.2211
#define pPotHigh_REG		0.0503
#define c_REG				13.4292
#define limF_POTHIGH		30L

// LIMITS AND DEFINITIONS OF FIND_PICOS AT REGULARIDAD
#define	NmPEND				100
#define maxp_PICOS			100
#define	limVP_REGPIC		0.25
#define	SepMin_REGPIC		30L
#define limCVT_REGPIC		0.2
#define	limbCP_REGPIC		0.220
#define limfcH_REGPIC		2.5

// LIMITS AND DEFINITIONS OF ALGORITMO_CORR
#define	nMinP_ACF			4L
#define	limCVT1_ACF			0.05
#define	limRO_ACF			0.98
#define	limfC_ACF			2.5
#define limfcaHz_ACF		2.5
#define	limCVT2_ACF			0.1

/******************************************************************************
** Typedefs
*/

/******************************************************************************
** Prototypes
*/

bool_t  calcular_PS (const int32_t *pSig);
float_t calcular_bWT (const int32_t *pSig);
float_t calcular_bCP (const int32_t *pSig);
float_t calcular_bW (int32_t *pSig);
float_t calcular_FD (int32_t *pSig);
bool_t  calcular_AsisPend (int32_t *pSig);

int32_t Find_Picos (int32_t *pCorr, int32_t *pPico, float_t lim, int32_t SepMin);
float_t calcular_CVT (int32_t *pPico, int32_t nMinP, int32_t nP);
float_t calcular_RO (int32_t *pPico, int32_t *pCorr, int32_t nP);
void    algoritmo_Corr (int32_t *pPico, int32_t *pFFT, int32_t numP, float_t pCVT, float_t pRO, int32_t *pCaso, float_t *pfcaHz);

float_t calcular_PF (int32_t *signal, int32_t pos0, float_t bW);
int32_t calcularTSVTV (int32_t *pSig, float_t pfcaHz);
float_t calcular_potHigh (int32_t *signal, int32_t pos0);
bool_t  regularidadPicos (int32_t *sw2, float_t bCP);


/******************************************************************************
*/
#endif  /*PARAM_H*/
