/******************************************************************************
 * Name      : R100_S3A7                                                      *
 * Copyright : Osatu S. Coop                                                  *
 * Compiler  : GNU ARM Embedded                                               *
 * Target    : Reanibex Serie 100                                             *
 ******************************************************************************/

/*!
 * @file        frcp.c
 * @brief
 *
 * @version     v1
 * @date        17/02/2021
 * @author      Ilazkanoiturburu
 * @warning     author   date    version     .- Edition
 * @bug
 *
 */

/******************************************************************************
 ** Includes
 */

#include <stdio.h>
#include <string.h>

#include "types_basic.h"
#include "frcp.h"


 /******************************************************************************
 * * Defines
 */
#define GAIN            (0.0007633691581868662f)       ///< Gain for IIR filter


#define FS              100             ///< Sampling frequency
#define NMW             (2*FS)          ///< Samples per window (assume that a single window correspond to 2 seconds)
#define NMF             2               ///< Samples pending filter

#define NMAX            ((NMW+NMF)/2)   ///< Number of max or min in a window (used for max and min arrays)

#define CC_IN_BLOCK     16              ///< Number of compressions in a single block (used to store the last compressions info)
#define CC_NCP          10              ///< Number of cc to update adaptive threshold (must be lower than stored CC_IN_BLOCK)

#define PD_WINSIZE      9               ///< Window size to use in the peak detector (PD) -> MUST use an odd number

#define N_PEAKS         8               ///< Number of max or min in a window (used for max and min arrays)

#define N_FREQUENCIES   10              ///< Number of partial frequency values used to calculate the compression frequency

// Parameters cc_detection_algorithm
#define DMIN            19              ///< Minimum distance to detect a cc (0.19*FS)
#define DMAX            95              ///< Maximum distance to detect a cc (0.95*FS)
#define INTERV_CC_MIN   22              ///< (0.22*FS)

//Parameter cc_update_block
#define TIME_BREATHE    (1*FS)          ///< Number of samples without compressions to detect a Breath condition

// Parameter update_threshold
// Use the next conversion rule:
//      412 ADCs / ohm
//      0,25 ohms = 103 ADCs
//      0,14 ohms =  58 ADCs
#define FACTOR          4               ///< Threshold update factor = (1/0.25) -- 25%
#define AMP_TH_MAX      103             ///< Max amplitude threshold for cc detection
#define AMP_TH_MIN      58              ///< Min amplitude threshold for cc detection
#define AMP_1_OHM       412             ///< Amplitude for 1 ohm


/// Defines the minimum and maximum position and amplitudes
typedef struct {
    uint32_t        pos;                ///< Position  of the interes point
    uint32_t        amp;                ///< Amplitude of the interes point
} FRCP_POINT_t;

/******************************************************************************
 * * Constants
 */

// IIR High Band Filter (IIR) - Low Pass Filter
// Chebychev Orden 3 (2 en cascada de orden 2,1)
//      Fs = 100 Hz (sampling rate)
//      Fc = 2,6 Hz
// Coeficientes y ganancias calculados mediante la utilidad tf2sos

const float_t b1[3] = {(float_t) 1, (float_t) 1,                  (float_t) 0};
const float_t a1[3] = {(float_t) 1, (float_t) -0.852951624532311, (float_t) 0};
const float_t b2[3] = {(float_t) 1, (float_t) 2,                  (float_t) 1};
const float_t a2[3] = {(float_t) 1, (float_t) -1.812948130052366, (float_t) 0.854478362848698};

/******************************************************************************
 * * Locals
 */

static float_t sf1 [NMF + NMW];         // Auxiliar arrays for filter implementation
static float_t sf2 [NMF + NMW];         // Auxiliar arrays for filter implementation

static float_t zi0[4];                  // IIR filter initial conditions
static float_t sf_prevSam[2];           // Store the 2 last samples from the previous window

static uint32_t impe_filt[NMW];         // filtered impedance array

static uint32_t prevSam[PD_WINSIZE];    // window used in the peak detector


static uint16_t old_good_value;                     // Variable to detect if the frequency detected was between 90 and 120 cc/per minute
static uint16_t buff_freq_value[N_FREQUENCIES];     // Buffer to store frequency values got from FRCP_Process_Zpwin (in bpms)

static uint16_t flag_ult_min;           // Flag to account if the last sample of the last window was a relative minimum
static uint16_t flag_first_window;      // Flag that indicates if we are in the first window of analyses

static FRCP_POINT_t  gbl_min;           // keeps the minimum from the previous window
static FRCP_POINT_t  gbl_max;           // keeps the maximum from the previous window

static uint32_t gbl_win_idx;            // Keep account of the number of window we are processing (used for conversion from relative extremes in global samples)

static uint32_t gbl_ult_cc;             // Location of the last cc detected
static uint32_t gbl_amp_min;            // Minimum amplitude of the adjacent samples to detect a cc

static uint16_t frcp_frequency;         // Frequency of the cc value in cc/minute

static FRCP_POINT_t block[CC_IN_BLOCK];         // Positions/Amplitudes of the cc in a single block

/******************************************************************************
** Name:    FRCP_IIR_0_2
*****************************************************************************/
/**
 ** @brief   IIR second order filtering function.
 **
 ** @param   firstSample: first sample value used to initialize the filter
 **          pFilter    : pointer to output signal
 **
 ** @return  None.
 **
 ******************************************************************************/
 static void  FRCP_IIR_0_2 (const float_t *pSignal, float_t *pFilter)
{
    uint32_t i;
    float_t k1 = b1[1];
    float_t k2 = b1[2];
    float_t k3 = a1[1];         // Store filtering coefficients in local variables
    float_t k4 = a1[2];

    pFilter[0] = zi0[0];
    pFilter[1] = zi0[1];

    // Filtering section
    for (i=NMF; i<NMF+NMW; i++)
    {
        pFilter[i] = pSignal[i]         +
                     pSignal[i-1]  * k1 +
                     pSignal[i-2]  * k2 -
                     pFilter[i-1]  * k3 -
                     pFilter[i-2]  * k4;
    }

    zi0[0] = pFilter[NMF+NMW-2];
}

/******************************************************************************
** Name:    FRCP_IIR_1_2
*****************************************************************************/
/**
 ** @brief   IIR second order filtering function.
 **
 ** @param   pSignal: pointer to input signal (NMW buffersize).
 **          pFilter: pointer to output signal
 **
 ** @return  None.
 **
 ******************************************************************************/
 static void  FRCP_IIR_1_2 (const uint32_t *pSignal, float_t *pFilter)
{
    uint32_t i;
    float_t k1 = b1[1];
    float_t k2 = b1[2];         // Store filtering coefficients in local variables
    float_t k3 = a1[1];
    float_t k4 = a1[2];

    // Filtering section
    pFilter[0] = zi0[0];
    pFilter[1] = zi0[1];
    pFilter[2] = (float_t) pSignal[0] +  sf_prevSam[1] * k1 +  sf_prevSam[0]*k2 - pFilter[1] * k3 - pFilter[0] * k4;
    pFilter[3] = (float_t) pSignal[1] + (float_t) pSignal[0] * k1 + sf_prevSam[1]*k2 - pFilter[2] * k3 - pFilter[1] * k4;
    for (i=4; i<NMF+NMW; i++)
    {
        pFilter[i] = (float_t) pSignal[i-2]      +
                     (float_t) pSignal[i-3] * k1 +
                     (float_t) pSignal[i-4] * k2 -
                     pFilter[i-1] * k3 -
                     pFilter[i-2] * k4;
    }

    zi0[0] = pFilter[NMF+NMW-2];
    zi0[1] = pFilter[NMF+NMW-1];
    sf_prevSam[0] = (float_t) pSignal[NMW-2];
    sf_prevSam[1] = (float_t) pSignal[NMW-1];
}

/******************************************************************************
 * * Name:    FRCP_IIR_2_2
 *****************************************************************************/
/**
 ** @brief   Second order IIR filtering function.
 **
 ** @param   pSignal: pointer to input signal.
 **          pFilter: pointer to output signal.
 **          k0: filter constant.
 ** @return  None.
 **
 ******************************************************************************/
 static void  FRCP_IIR_2_2 (const float_t *pSignal, float_t *pFilter)
{
    uint32_t i;
    float_t k1 = b2[1];
    float_t k2 = b2[2];     // Store filtering coefficients in local variables
    float_t k3 = a2[1];
    float_t k4 = a2[2];

    pFilter[0] = zi0[2];
    pFilter[1] = zi0[3];

    // Filtering section
    for (i=NMF; i<NMF+NMW; i++)
    {
        pFilter[i] = pSignal[i]        +
                     pSignal[i-1] * k1 +
                     pSignal[i-2] * k2 -
                     pFilter[i-1] * k3 -
                     pFilter[i-2] * k4;
    }

    zi0[2] = pFilter[NMF+NMW-2];
    zi0[3] = pFilter[NMF+NMW-1];
}

/******************************************************************************
 * * Name:   FRCP_IIR
 *****************************************************************************/
/**
 * * @brief  IIR passband filtering function (2.5 - 30 Hz). Implementation based
 **          on 10th order butterworth filter composed of a 5 filters cascade (2nd
 **          order each one)
 **
 ** @param   pSignal: pointer to input signal(ADC units).
 **          pFilter: pointer to output signal.
 **
 ******************************************************************************/
 static void  FRCP_IIR (const uint32_t *pSignal, uint32_t *pFilter)
{
    uint32_t i;                     // Global use index
    float_t  sample;

    FRCP_IIR_1_2 (pSignal, sf1);    // Filtering
    FRCP_IIR_2_2 (sf1, sf2);

    for (i=0; i<NMW; i++)
    {
        sample = (GAIN * sf2[NMF+i]);    // Apply Filter Gain to Normalize Result
        pFilter[i] = (uint32_t) sample;
    }
}

/******************************************************************************
 * * Name:    FRCP_Ini_Filters
 *****************************************************************************/
/**
 ** @brief   Filter initialization function.
 **
 ** @param   firstSample: first sample value.
 **
 ** @return  None.
 **
 ******************************************************************************/
 static void  FRCP_Ini_Filters (uint32_t firstSample)
{
    int32_t i;                                      // Global use index

    // Restart Z0 array
    zi0[0] = zi0[1] = zi0[2] = zi0[3] = (float_t) 0.0; // Restart filter coefficients

    //////////////////////////////////////
    // Calculate Zo for BPF 2.5-30;
    //////////////////////////////////////////
    for (i=0; i<NMF+NMW; i++)                    // Fill array with 1st sample
    {
        sf2[i] = (float_t)firstSample;
    }

    FRCP_IIR_0_2 (sf2, sf1);            // Filtering
    FRCP_IIR_2_2 (sf1, sf2);

    // initialize previous samples buffer
    sf_prevSam[0] = sf_prevSam[1] = (float_t) firstSample;
}

/**
 * @brief Function that calculates the median of an array of integers
 *
 * @param x array of values
 * @param n size of the array
 *
 * @return median value of the array
 **/
static uint16_t FRCP_median(uint16_t *x, uint32_t n)
{
    uint16_t  temp;
    uint32_t  i, j;

    // the following two loops sort the array x in ascending order
    for (i=0; i<(n-1); i++)
    {
        for (j=(i+1); j<n; j++)
        {
            if (x[j] < x[i])
            {
                // swap elements
                temp = x[i];
                x[i] = x[j];
                x[j] = temp;
            }
        }
    }

    i = n/2;
    if (n%2 == 0)
    {
        // if there is an even number of elements, return mean of the two elements in the middle
        return (uint16_t) ((x[i] + x[i - 1]) / 2);
    } else {
        // else return the element in the middle
        return x[i];
    }

}

/**
 * @brief Funcion que identifica un posible maximo en un tramo de impedancia
 *
 * @param impe_filt tramo de impedancia a analizar
 * @return if there is a valid maximum
 **/
static bool_t  find_peak_max (const uint32_t *pBuffer)
{
    uint32_t i;
    uint32_t my_peak;
    bool_t   ll = FALSE;        // lower values in the left
    bool_t   lr = FALSE;        // lower values in the right

    my_peak = pBuffer[PD_WINSIZE/2];

    // the candidate MUST be higher or equal to the rest of neighbors
    for (i=0; i<PD_WINSIZE; i++)
    {
        if (my_peak < pBuffer[i]) return FALSE;
    }

    // the candidate MUST find lower values in both sides
    for (i=0; i<(PD_WINSIZE/2); i++)
    {
        if (my_peak > pBuffer[i]) { ll = TRUE; };
    }
    for ( ; i<PD_WINSIZE; i++)
    {
        if (my_peak > pBuffer[i]) { lr = TRUE; };
    }

    // return the operation result
    return ((ll == TRUE) && (lr == TRUE)) ? TRUE : FALSE;
}

/**
 * @brief Funcion que identifica un posible minimo en un tramo de impedancia
 *
 * @param impe_filt tramo de impedancia a analizar
 * @return if there is a valid minimum
 **/
static bool_t  find_peak_min (const uint32_t *pBuffer)
{
    uint32_t i;
    uint32_t my_peak;
    bool_t   hl = FALSE;        // higher values in the left
    bool_t   hr = FALSE;        // higher values in the right

    my_peak = pBuffer[PD_WINSIZE/2];

    // the candidate MUST be lower or equal to the rest of neighbors
    for (i=0; i<PD_WINSIZE; i++)
    {
        if (my_peak > pBuffer[i]) return FALSE;
    }

    // the candidate MUST find higher values in both sides
    for (i=0; i<(PD_WINSIZE/2); i++)
    {
        if (my_peak < pBuffer[i]) { hl = TRUE; };
    }
    for ( ; i<PD_WINSIZE; i++)
    {
        if (my_peak < pBuffer[i]) { hr = TRUE; };
    }

    // return the operation result
    return ((hl == TRUE) && (hr == TRUE)) ? TRUE : FALSE;
}

/**
 * @brief Funcion que recibe un tramo de impedancia y devuelve sus maximos y minimos relativos
 *
 * @param impe_filt tramo de impedancia a analizar
 * @param pMin      posiciones/amplitudes de los minimos localizados (en muestras relativas al tramo)
 * @param pMax      posiciones/amplitudes de los maximos localizados (en muestras relativas al tramo)
 * @param cont_min  numero de minimos localizados
 * @param cont_max  numero de maximos localizados
 * @return void
 **/
static void  find_peaks (const uint32_t* my_impe_filt, FRCP_POINT_t *pMin, FRCP_POINT_t *pMax, uint32_t* cont_min, uint32_t* cont_max )
{
    uint32_t i;                 // global use counter
    uint32_t jMin = 0;          // initial index for minimum
    uint32_t jMax = 0;          // initial index for maximum
    uint32_t my_min = 0;
    uint32_t my_max = 0;
    FRCP_POINT_t checker;       // used to filter the peaks

    // recover the last MAX-MIN from the last window
    if (gbl_min.amp != 0)
    {
        pMin[0] = gbl_min;
        my_min = jMin = 1;
    }
    if (gbl_max.amp != 0)
    {
        pMax[0] = gbl_max;
        my_max = jMax = 1;
    }

    for (i=0; i<NMW; i++)
    {
        memcpy (&prevSam[0], &prevSam[1], sizeof (uint32_t) * (PD_WINSIZE-1));
        prevSam[PD_WINSIZE-1] = my_impe_filt[i];

        if ((0 == flag_ult_min) && (find_peak_min(prevSam) == TRUE))        // try to find a MIN
        {
            // Last window contains a MIN
            pMin[my_min].pos = i;
            pMin[my_min].amp = prevSam[PD_WINSIZE/2];
            my_min++;
            flag_ult_min = 1;
        }
        else if ((1 == flag_ult_min) && (find_peak_max(prevSam) == TRUE))   // try to find a MAX
        {
            // Last window contains a MAX
            pMax[my_max].pos = i;
            pMax[my_max].amp = prevSam[PD_WINSIZE/2];
            my_max++;
            flag_ult_min = 0;
        }

        // Validate the peak
        if ((my_min >= 2) && (my_max >= 2))
        {
            checker.amp = (pMax[my_max-2].amp - pMin[my_min-2].amp + pMax[my_max-2].amp - pMin[my_min-1].amp)/2;
            checker.pos =  pMin[my_min-1].pos - pMin[my_min-2].pos;
            if ((checker.pos <= DMIN) || (checker.amp <= gbl_amp_min))
            {
                my_min--;
                my_max--;
            }
        }

        // Check if the maximum number of peaks have been reach
        if ((my_min >= N_PEAKS) || (my_max >= N_PEAKS))
        {
            break;
        }
    }

    // Conversion from relative samples to global samples
    for (i=jMin; i < my_min; i++) { pMin[i].pos += gbl_win_idx; }
    for (i=jMax; i < my_max; i++) { pMax[i].pos += gbl_win_idx; }
    gbl_win_idx += NMW;

    // store the MIN-MAX to be processed in the next window
    gbl_min = pMin[my_min-1];
    gbl_max = pMax[my_max-1];

    for (i=my_min; i<N_PEAKS; i++) { pMin[i].pos = 0; pMin[i].amp = 0; }
    for (i=my_max; i<N_PEAKS; i++) { pMax[i].pos = 0; pMax[i].amp = 0; }

    // set the counter values
    *cont_min = my_min;
    *cont_max = my_max;
}

/**
 * @brief Funcion que calcula para cada fluctuacion  dos parametros:
 *           amplitud (media de las diferencias de amplitudes entre un maximo y el minimo anterior y entre un maximo y el minimo siguiente)
 *           duracion (distancia entre los dos minimos)
 *
 * @param pMin      posiciones/amplitudes de los minimos localizados (en muestras relativas al tramo)
 * @param pMax      posiciones/amplitudes de los maximos localizados (en muestras relativas al tramo)
 * @param nPoints   numero de puntos a calcular
 * @param pCC_raw   media de las diferencias de amplitudes entre un maximo y el minimo anterior y entre el maximo y el minimo siguiente
 *                  distancia entre los minimos adyacentes al maximo (en muestras)
 * @return void
 **/
static void  cc_calculate_features (FRCP_POINT_t *pMin, FRCP_POINT_t *pMax, uint32_t nPoints, FRCP_POINT_t *pCC_raw )
{
    uint32_t i;

    for (i = 0; i < nPoints; i++)
    {
        // Medium value of the amplitudes detected
        pCC_raw[i].amp = (pMax[i].amp - pMin[i].amp + pMax[i].amp - pMin[i + 1].amp)/2;

        // Medium value of the positions (durations) detected
        pCC_raw[i].pos = pMin[i + 1].pos - pMin[i].pos;
    }

    for (; i < N_PEAKS; i++)
    {
        pCC_raw[i].pos = 0;
        pCC_raw[i].amp = 0;
    }
}

/**
 * @brief Funcion que a partir de los arrays de features (par�metros) de cada fluctuaci�n decide �sta ha sido provocada por una fluctuaci�n o no.
 *
 * @param pCC_raw   distancia entre los minimos adyacentes al maximo (en muestras)
 *                  media de las diferencias de amplitudes entre un maximo y el minimo anterior y entre el maximo y el minimo siguiente
 * @param pMax      posiciones/amplitudes de los maximos localizados (en muestras relativas al tramo)
 * @param cont_max  n�mero de maximos
 * @param pCC       posici�n de las compresiones detectadas en muestras absolutas
 *                  array de amplitudes de las compresiones detectadas
 * @param cont_cc   n�mero de compresiones detectadas
 *
 * @return void
 **/
static void cc_detection_algorithm ( FRCP_POINT_t *pMax, FRCP_POINT_t *pCC_raw, uint32_t nPoints, FRCP_POINT_t *pCC, uint32_t *cont_cc )
{
    static uint32_t ult_maximo;         // Location of the last max of the previous window
    uint32_t        i;                  // global use counter

    *cont_cc = 0;                       // Number of cc detected

    if (flag_first_window == 0)
    {
        //If not first window, store the last cc form the previous window (ult_maximo) in gbl_ult_cc
        gbl_ult_cc = ult_maximo;
    }

    for (i=0; i<nPoints; i++)
    {
        if ((pCC_raw[i].pos > DMIN) &&
            (pCC_raw[i].pos < DMAX) &&
            (pCC_raw[i].amp > gbl_amp_min) &&
             ((pMax[i].pos - gbl_ult_cc) > INTERV_CC_MIN))
        {   // cc detected
            gbl_ult_cc = ult_maximo = pMax[i].pos;          //update for the last cc detected
            pCC[*cont_cc].pos = ult_maximo;
            pCC[*cont_cc].amp = pCC_raw[i].amp;             //store the amplitudes of the cc, later used in check block
            (*cont_cc)++;
        }
    }

    for (i=*cont_cc; i < N_PEAKS; i++)
    {
        pCC[i].pos = 0;
        pCC[i].amp = 0;
    }
}

/**
 * @brief Function to initialize the block buffer
 *
 * @param pBlock    pointer to CC's array
 *
 * @return void
 **/
 static void  reset_block (FRCP_POINT_t *pBlock)
{
    int32_t      i;

    for (i=0; i<CC_IN_BLOCK; i++)
    {
        pBlock[i].pos = 0;
        pBlock[i].amp = 0;
    }
}

/**
 * @brief Funcion que comprueba si las ultimas compresiones detectadas pertenecen a un bloque de compresiones.
 * Devuelve un array de compresiones de bloque y otro array que indica el indice la ultima compresion del bloque
 * que se habia usado para calcular la frecuencia y actualizar el umbral
 *
 * @param pCC       pointer to array containing CC's position and amplitude of the last window
 * @param cont_cc   number of detected compressions in the window
 * @param pBlock    pointer to array containing CC's position and amplitude of the last block
 *
 * @return void
 **/
 static void  cc_update_block (FRCP_POINT_t *pCC, uint32_t cont_cc, FRCP_POINT_t *pBlock)
{
    uint32_t      i, j;

    if (cont_cc < 1)
    {
        // If no cc detected in the window, array reset and bye..bye
        reset_block (pBlock);
    }
    else {
        // check if there is a new block in the current window
        for (i=0; i<(cont_cc-1); i++)
        {
            // check if a new block is detected !!!
            if ((pCC[i+1].pos - pCC[i].pos) > TIME_BREATHE) { break; }
        }

        if (i >= (cont_cc-1))
        {
            // shift the block buffer to free positions to fill with the new window data
            for (i=cont_cc, j=0; i<CC_IN_BLOCK; i++, j++)
            {
                pBlock[j] = pBlock[i];
            }
            i = 0;
            j = CC_IN_BLOCK - cont_cc;
        }
        else {
            // new block is detected !!!
            reset_block (pBlock);
            i++;
            j = CC_IN_BLOCK - cont_cc + i;
        }
        // insert the new window data into the block ...
        for (; i<cont_cc; i++, j++)
        {
            pBlock[j] = pCC[i];
        }
    }
}

/**
 * @brief Funcion que a partir de un array de posiciones de las compresiones del �ltimo bloque calcula la frecuencia.
 *
 * @param pBlock        pointer to array containing position and amplitude of the last block
 *
 * @return compression frequency in bpm (beats per minute)
 **/
static uint16_t  cc_calculate_frequency(const FRCP_POINT_t *pBlock)
{
    uint32_t i, j;
    uint16_t my_freq;
    uint16_t interv[CC_IN_BLOCK];           // Intervals are the distance between adjacent cc

    // loop to get the last "CC_IN_BLOCK-8" intervals (distance between adjacent cc's)
    for (j=0,i=(CC_IN_BLOCK-1); i>8; i--)
    {
        if (pBlock[i].pos && pBlock[i-1].pos)
        {
            interv[j++] = (uint16_t) (pBlock[i].pos - pBlock[i-1].pos);
        }
    }
    if (j>2)
    {
        if (j >= 8) j= 8;
        my_freq = (uint16_t) ((60*FS) / FRCP_median(interv, j));
    }
    else {
        my_freq = 0;
    }

    return(my_freq);
}

/**
 * @brief Funcion que actualiza el umbral de amplitud en Funcion de las amplitudes de las ultimas
 * compresiones que se han insertado en el bloque
 *
 * @param pBlock        pointer to array containing position and amplitude of the last block
 * @param var_umb       array de ultimos valores a tener en cuenta para el umbral
 *
 * @return updated threshold
 **/
static uint32_t  cc_update_threshold (const FRCP_POINT_t *pBlock)
{
    int32_t   i, j;
    uint32_t  my_amp;            // intermediate amplitude
    uint32_t  suma = 0;

    j = CC_IN_BLOCK - CC_NCP - 1;

    for (i=0; i<CC_NCP; i++, j++)
    {
        if (pBlock[j].amp == 0)
        {
            suma += AMP_TH_MIN;
        }
        else {
            my_amp = (pBlock[j].amp < AMP_1_OHM) ? pBlock[j].amp : AMP_1_OHM;
            suma += my_amp; //add a new element
        }
    }
    // update the minimum value of the threshold
    my_amp = suma / (CC_NCP*FACTOR);
    if (my_amp > AMP_TH_MAX) return AMP_TH_MAX;
    if (my_amp < AMP_TH_MIN) return AMP_TH_MIN;
    return my_amp;
}

/**
 * @brief Funcion que actualiza el valor de frecuencia de las compresiones
 *
 * @param new_frequency     ultimo valor de FRCP
 *
 * @return filtered FRCP frequency
 **/
static uint16_t cc_update_frequency (uint16_t new_frequency)
{
    uint32_t i;
    uint32_t nFreq;                     // number of non-zero frequencies
    uint32_t freq_filter_value = 0;     //Medium value of the buff_freq_value buffer.

    if (new_frequency != 0)
    {
        // Store the value frequency value from FRCP_Process_Zpwin in buff_freq_value.
        memcpy ((uint8_t *) &buff_freq_value[0], (uint8_t *) &buff_freq_value[1], sizeof (uint16_t) * (N_FREQUENCIES-1));
        buff_freq_value[N_FREQUENCIES-1] = new_frequency;

        // calculate the frequency average value
        for (i=0, nFreq=0; i < N_FREQUENCIES; i++)
        {
            if (buff_freq_value[i])
            {
                freq_filter_value += buff_freq_value[i];
                nFreq++;
            }
        }
        freq_filter_value /= nFreq;     //Medium value of the buffer calculated

        if (old_good_value == 1)        //If the last frequency value was in range...
        {
            if ((freq_filter_value < 90) || (freq_filter_value > 120))
            {
                old_good_value = 0;
            }
            return (uint16_t) freq_filter_value;
        }
        else        // If last frequency value was NOT in range...
        {
            if ((new_frequency >= 90) && (new_frequency <= 120))
            {
                old_good_value = 1;
                buff_freq_value[0] = 100;
                buff_freq_value[1] = 100;
                buff_freq_value[2] = 100;       // If the frequency value was in range, the first 5 samples of the buffer are filled, so if the next value
                buff_freq_value[3] = 100;       // is way out of range, the deviation would be smaller
                buff_freq_value[4] = 100;
                return new_frequency;
            }
            else
            {
                return (uint16_t) freq_filter_value;
            }
        }
    }

    // when frequency is zero, return zero
    return 0;
}


//////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////

/******************************************************************************
** Name:    FRCP_Init
*****************************************************************************/
/**
** @brief   Initialize the FRCP controller
**
** @param   first sample of the window
**
** @return  none
**
******************************************************************************/
void  FRCP_Init (uint32_t first_sample)
{
    uint32_t    i;

    FRCP_Ini_Filters(first_sample);      // Just the first sample of the first window is needed as input
    gbl_amp_min = AMP_TH_MIN;

    old_good_value = 0;
    memset ((uint8_t *)buff_freq_value, 0, sizeof(buff_freq_value));

    flag_ult_min      = 0;     // Last sample was not a min
    flag_first_window = 1;     // First window of analysis
    gbl_win_idx       = 1;     // GLobal samples index set to 1.

    // initialize previous samples buffer used to calculate the minimum & maximum
    for (i=0; i<PD_WINSIZE; i++)
    {
        prevSam[i] = first_sample;
    }

    // initialize the minimum and maximum descriptors
    gbl_min.pos = gbl_max.pos = 0;
    gbl_min.amp = gbl_max.amp = 0;

    gbl_ult_cc = (uint32_t)-2.0*INTERV_CC_MIN; //If we are in the first window analysis, a value is set to always complies
    //gbl_ult_cc = 0; //If we are in the first window analysis, a value is set to always complies
}

/**
 * @brief Funcion que retorna el valor de frecuencia de las compresiones
 *
 * @return frecuencia de las compresiones
 **/
uint16_t FRCP_Get_Frequency(void)
{
    return frcp_frequency;
}

/******************************************************************************
** Name:    freq_feedback
*****************************************************************************/
/**
** @brief   Calcule the FRCP of the current window
**
** @param   signal se�al de impedancia (Ohm)
**
******************************************************************************/
void FRCP_Process_Zpwin(uint32_t *signal)
{
    uint32_t      cont_min, cont_max;           // Counter of the max and min

    FRCP_POINT_t  min[N_PEAKS];                 // Array to store the minimum position and amplitudes in window
    FRCP_POINT_t  max[N_PEAKS];                 // Array to store the maximum position and amplitudes in window

    FRCP_POINT_t  cc_win_raw [N_PEAKS];         // Array to store the CC candidates in window (position and amplitudes)
    FRCP_POINT_t  cc_win     [N_PEAKS];         // Array to store the CC valids     in window (position and amplitudes)

    uint32_t      cc_win_counter_raw;           // Number of cc's in the raw  window
    uint32_t      cc_win_counter;               // Number of cc's in the fine window
    uint16_t      win_frequency;                // Frequency of the current window

    // window Filtering
    FRCP_IIR (signal, impe_filt);

    // find absolute minimum and maximum peaks in a single window
    find_peaks(impe_filt, min, max, &cont_min, &cont_max);

    // process the window to reach CC points
    if (cont_max > 0)
    {
        // WARNING --> process the maximum-minimum points in function of the number of minimums
        cc_win_counter_raw = (cont_min < cont_max) ? cont_max - 1 : cont_min - 1;

        // calculate the raw CC points
        cc_calculate_features(min, max, cc_win_counter_raw, cc_win_raw);

        // calculate the pure CC points
        cc_detection_algorithm(max, cc_win_raw, cc_win_counter_raw, cc_win, &cc_win_counter);

        // check if detected cc's belong to the same block
        cc_update_block(cc_win, cc_win_counter, block);

        // calculate the RCP frequency of the current window
        win_frequency  = cc_calculate_frequency (block);
        frcp_frequency = cc_update_frequency (win_frequency);

        // update the "gbl_amp_min" threshold
        gbl_amp_min = cc_update_threshold (block);

        // clear the first window flag
        flag_first_window = 0;
    }
    else {
        // no maximun points are detected in the window. Thus, set the first window flag
        flag_first_window = 1;
    }
}
