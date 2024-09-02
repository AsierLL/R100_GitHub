/******************************************************************************
* Name      : Defibrillable Rhythm Detector                                      *
* Copyright : Osatu S.Coop                                                    *
* Compiler  : Renesas C/C++ compiler for SH2                                  *
* Target    : Reanibex Series                                                 *
******************************************************************************/

/*!
* @file        drd.h
* @brief       Basic Types Definition
* @version     0.0
* @date        2011-09-21
* @author      EG (egarcia@osatu.com)
* @warning     EG        2011-09-21        V0.0    .- First edition
* @bug
*
*/

#ifndef DRD_H
#define DRD_H

/******************************************************************************
** Includes
*/
#include "types_basic.h"

/******************************************************************************
** Defines
*/

#define  ECG_WIN_SIZE_DRD       480     // 3,2 seconds at 150 Hz

/******************************************************************************
** Typedefs
*/
typedef enum
{
    eTYPO_RHYTM_ASISTOLIA_1     = 0,   // Asistolia, No-Shock
    eTYPO_RHYTM_NSR             = 1,   // Ritmo con QRS, No-Shock
    eTYPO_RHYTM_ASISTOLIA_2     = 2,   // Ritmo Lento, No-Shock
    eTYPO_RHYTM_TV              = 3,   // TV, Shock
    eTYPO_RHYTM_TSV             = 4,   // TSV, No-Shock
    eTYPO_RHYTM_TV_LENTA        = 5,   // TV lenta, No-Shock
    eTYPO_RHYTM_FV_1            = 6,   // FV, Shock
    eTYPO_RHYTM_FV_2            = 7,   // FV, Shock
    eTYPO_RHYTM_FV_3            = 8,   // FV, Shock
    eTYPO_RHYTM_LENTO_1         = 9,   // Ritmo lento, No-Shock
    eTYPO_RHYTM_LENTO_2         = 10,  // Ritmo lento, No-Shock
    eTYPO_RHYTM_LENTO_3         = 11,  // Ritmo lento, No-Shock
    eTYPO_RHYTM_LENTO_4         = 12   // Ritmo lento, No-Shock
} TYPO_RHYTM_e;

typedef struct                      ///< Window Single Parameters
{
    float_t         bCP;            ///< Dominio de la pendiente
    float_t         bWT;            ///< Dominio del tiempo
    float_t         bW;             ///< Dominio de la frecuencia
    float_t         CVT;            ///< Coeficiente de variación de la separación entre picos
    float_t         RO;             ///< Valor que cuantifica la ordenación por amplitud de los picos de la autocorrelación
    TYPO_RHYTM_e    wind_rhytm;     ///< Tipo de ritmo de la ventana analizada
} WS_PAR_t;

typedef struct                      ///< Window Double Parameters
{
    WS_PAR_t        w1;             ///< Window Single 1
    WS_PAR_t        w2;             ///< Window Single 2
} DRD_DATA_t;                       // size of this structure must be less or equal to 48 bytes

typedef enum
{
    eRHYTHM_UNKNOWN      = 0x00,  ///< not clasified rythm
    eRHYTHM_ASYSTOLE     = 0x01,  ///< asystole
    eRHYTHM_NORMAL       = 0x02,  ///< normal sinus rhythm
    eRHYTHM_VT_NON_SHOCK = 0x04,  ///< ventricular taquicardia (no shockable)
    eRHYTHM_VT_SHOCK     = 0x08,  ///< ventricular taquicardia
    eRHYTHM_VF_SHOCK     = 0x10   ///< ventricular fibrilation
} RHYTHM_e;

typedef struct
{
    RHYTHM_e            rhythm;    ///< type of rhythm
    DRD_DATA_t          drd_data;  ///< rhythm parameters: bc,bw,fc,ps,vp,rp,ro
} DRD_RESULT_t;

/******************************************************************************
** Prototypes
*/

extern void         DRD_Get_Result   (DRD_RESULT_t *drd_param);
extern char_t*      DRD_Get_Rev_Code (void);
extern bool_t       DRD_Process_Ecg  (const int16_t *pEcg, bool_t flag, RHYTHM_e *p_rhythm, bool_t flag_pediatric);

/******************************************************************************
*/


#endif  /*DRD_H*/
