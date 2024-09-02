/******************************************************************************
* Name      : Defibrillable Rhythm Detector                                      *
* Copyright : Osatu S.Coop                                                    *
* Compiler  : Renesas C/C++ compiler for SH2                                  *
* Target    : Reanibex Series                                                 *
******************************************************************************/

/*!
* @file        general.h
* @brief       Basic Types Definition
* @version     0.0
* @date        2011-09-21
* @author      EG (egarcia@osatu.com)
* @warning     EG        2011-09-21        V0.0    .- First edition
* @bug
*
*/

#ifndef GENERAL_h
#define GENERAL_h

/******************************************************************************
** Includes
*/

/******************************************************************************
** Defines
*/
// GENERAL PARAMETER DEFINITION
#define FS			    150L      ///< Sampling frecuency 

#define PI				3.14159265358979
#define NMW				ECG_WIN_SIZE_DRD
#define NMF				2


#define NW				2L

#define SNMW			240L
#define NFFT			1024L

#define TRUE			1
#define FALSE			0

#define ESCALA		    4095L
#define RANGEmV			20

// TYPE AND SHOCK DEFINITION
#define SHOCK			1
#define NOSHOCK			0

// LIMITS OF MAIN DEFINITION
#define bWT_REG			-11.2572
#define bCP_REG			14.9565
#define bW_REG			0.5831
#define d_REG			-4.0186
#define limREG			0

#define limbW_FD		0.65
#define limFD_FD		2.5

#define limbWT_QRS		0.235
#define limbCP_QRS		0.48

#define	maxP_PIC		100L
#define limVal_PIC		0.14
#define	SepMin_PIC		18
#define limMin_PIC		2

#define limfcaHz_CAS1	2.5
#define limfcaHz_CAS2	1.3333
#define limbCP_CAS		0.145
#define limfC_CAS		2.65
#define limbWT_CAS		0.400
#define limCVT_CAS		0.15
#define limnP_CAS		3

/******************************************************************************
** Typedefs
*/

/******************************************************************************
** Prototypes
*/

/******************************************************************************
*/
#endif  /*GENERAL_H*/
