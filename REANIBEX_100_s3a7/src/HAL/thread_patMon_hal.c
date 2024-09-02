/******************************************************************************
 * Name      : S3A7_REANIBEX_100                                              *
 * Copyright : Osatu S. Coop                                                  *
 * Compiler  : MinGW32                                                        *
 * Target    : Reanibex Series                                                *
 ******************************************************************************/

/*!
 * @file        thread_patMon_entry.c
 * @brief
 *
 * @version     v1
 * @date        16/01/2018
 * @author      ltorres
 * @warning     author   date    version     .- Edition
 * @bug
 *
 */

/******************************************************************************
 ** Includes
 */


#include <device_init.h>
#include "Trace.h"

#include "types_basic.h"
#include "types_app.h"
#include "R100_Errors.h"
#include "R100_Tables.h"
#include "thread_DRD_entry.h"           // to know when is analyzing
#include "thread_core_entry.h"          // to know the device settings

#include "thread_defibrillator_hal.h"   // to know when is charging
#include "thread_patMon_hal.h"
#include "thread_patMon.h"
#include "sysMon_Battery.h"


/******************************************************************************
 ** Macros
 */

/******************************************************************************
 ** Defines
 */

#define NUM_KOEFS_ECG_FILTER        16          ///< Number of steps pre-processing ECG samples

///< Zp value segmentation (in ohms)
#define ZP_OPEN_CIRC_THR            2800        ///< Threshold for open circuit
#define ZP_BAD_CONN_THR             300         ///< Threshold for bad connection
#define ZP_GOOD_CONN_THR            15          ///< Threshold for good connection

//#define APP_ERR_TRAP(a)             if (a) __BKPT(0);

#define PMK_PULSE_LEN               7           ///< Number of samples to define a PMK pulse


#define ZP_ADCS_MINIMUM             (zp_table_adcs[ 0])             ///< Minimum ZP value  (short in electrodes)
#define ZP_ADCS_FOR_VALID_ECG       (zp_table_adcs[15])             ///< Limit to consider (about 300 ohms)
#define ZP_ADCS_MAXIMUM             (zp_table_adcs[33])             ///< Maximum ZP value  (open circuit)


/******************************************************************************
 ** Typedefs
 */

// pacemaker pulse descriptor
typedef struct {
    int32_t     data[PMK_PULSE_LEN];            ///< Array with PMK samples
    uint8_t     data_count;                     ///< Data index used to insert the PMK pulse
    uint8_t     time_delay;                     ///< Delay to insert the PMK samples
    uint16_t    time_refractory;                ///< Refractory time in samples
} PMK_PULSE_DESC_t;

/******************************************************************************
 ** Typedefs
 */

/******************************************************************************
 ** Constants
 */

// LPF for offset Removal with FC = 60Hz
const float_t   IIR_LPF_A [2]  = { (float_t) ( 1.0000), (float_t) (-0.8273) };
const float_t   IIR_LPF_B [2]  = { (float_t) ( 0.0864), (float_t) ( 0.0864) };

// HPF for offset Removal with FC = @see comments
const float_t   IIR_HPF_A [2]  = { (float_t) ( 1.0000), (float_t) (-0.9973) };   // to be used for FC = 0,67 Hz (used in R700)
//const float_t IIR_HPF_A [2]  = { (float_t) ( 1.0000), (float_t) (-0.9960) };   // to be used for FC = 1,00 Hz
const float_t   IIR_HPF_B [2]  = { (float_t) ( 1.0000), (float_t) (-1.0000) };

// Notch 50Hz (2x second order in cascade)
const float_t   IIR_N50_1A [3] = { (float_t) ( 1.0000), (float_t) (-1.899154663085938), (float_t) ( 0.997741699218750) };
const float_t   IIR_N50_1B [3] = { (float_t) ( 1.0000), (float_t) (-1.902114868164063), (float_t) ( 1.000000000000000) };

const float_t   IIR_N50_2A [3] = { (float_t) ( 1.0000), (float_t) (-1.900787353515625), (float_t) ( 0.997756958007813) };
const float_t   IIR_N50_2B [3] = { (float_t) ( 1.0000), (float_t) (-1.902114868164063), (float_t) ( 1.000000000000000) };

// Notch 60Hz (2x second order in cascade)
const float_t   IIR_N60_1A [3] = { (float_t) ( 1.0000), (float_t) (-1.856491088867188), (float_t) ( 0.997741699218750) };
const float_t   IIR_N60_1B [3] = { (float_t) ( 1.0000), (float_t) (-1.859558105468750), (float_t) ( 1.000000000000000) };

const float_t   IIR_N60_2A [3] = { (float_t) ( 1.0000), (float_t) (-1.858428955078125), (float_t) ( 0.997756958007813) };
const float_t   IIR_N60_2B [3] = { (float_t) ( 1.0000), (float_t) (-1.859558105468750), (float_t) ( 1.000000000000000) };

/******************************************************************************
 ** Externals
 */

/******************************************************************************
 ** Globals
 */

/******************************************************************************
 ** Locals
 */

// pointers to Notch filter coefficients
static float_t *pNF1A;
static float_t *pNF1B;
static float_t *pNF2A;
static float_t *pNF2B;

static uint8_t  ads_buffer[ADS_BUFFER_SIZE];            // buffer to communicate with the ADS device

static uint32_t ecg_nSample;                            // sample number in the current ECG acquisition engine
static int16_t  ecg_data [ECG_BUFFER_SIZE];             // data from ECG channel
static uint32_t zp_data  [ZP_BUFFER_SIZE];              // data from ZP  channel
static uint32_t zp_ohms;                                // formal ZP value (in ohms)
static  int32_t zp_adcs;                                // formal ZP value (in ADCs)
static  int32_t zp_adcs_raw;                            // raw ZP value (in ADCs)
static uint32_t zp_cal;                                     // ZP calibration value (in ADCs)
static  int32_t zp_offset_static = 0;                       // static  offset for the ZP value (in ADCs)
static  int32_t zp_offset_dynamic = 0;                      // dynamic offset for the ZP value (in ADCs)

static uint32_t    zp_table_adcs[MAX_ZP_ITEMS];             // compensated ADC table used to convert from ADCs to Ohms
static ZP_MATRIX_t device_zp_matrix[MAX_ZP_MATRIX_ITEMS];   // device ZP data matrix

static  int32_t temperature_adcs;                       // temperature value in ADCs (the PGA gain is set to 4)

static uint32_t idx_ecg;                                // index to write in the ECG data buffer
static uint32_t idx_zp;                                 // index to write in the ZP  data buffer

static float_t      IIR_ecg_dly [NUM_KOEFS_ECG_FILTER]; // array for delayed registers filtering ECGs

static uint32_t     wrong_data_cont = 0;                // counter for wrong frames from the ADS device (debug purposes)
static bool         cal_running = false;                // flag to identify when the calibration process is ON

static ERROR_ID_e   ads_status = eERR_ADS_DEVICE_ID;    // AED operational status (by default, an unknown device)

static PMK_PULSE_DESC_t     pmk_pulse_desc;             // Pacemaker pulse descriptor


/******************************************************************************
 ** Prototypes
 */

static ERROR_ID_e   ADS_Send_Command (uint8_t cmd);
static ERROR_ID_e   ADS_Run(void);
static ERROR_ID_e   ADS_Stop(void);
static ERROR_ID_e   ADS_Init(void);
static ERROR_ID_e   ADS_Read_Samples    (uint32_t *pStatus, uint32_t *pCh1, uint32_t *pCh2);
static ERROR_ID_e   ADS_Read_Single_Reg (uint8_t regId, uint8_t *pData);
static ERROR_ID_e   ADS_Write_Single_Reg(uint8_t regId, uint8_t my_data);

static void         ECG_Filter_Restart  (void);
static int16_t      ECG_Offset_Removal  (int32_t sample);
static void         ZP_Update_Value     (uint32_t zp_sample);


/******************************************************************************
 ** Name:    ADS_Send_Command
 *****************************************************************************/
/**
 ** @brief   send a single command to the ADS device
 **
 ** @param   cmd       command to send
 **
 ** @return  operation result or error code
 ******************************************************************************/
static ERROR_ID_e ADS_Send_Command(uint8_t cmd)
{
    ssp_err_t err;

    ads_buffer[0] = cmd;
    err = spi_patmon.p_api->write (spi_patmon.p_ctrl, ads_buffer, 1, SPI_BIT_WIDTH_8_BITS, TX_WAIT_FOREVER);
    return (err == SSP_SUCCESS) ? eERR_NONE : eERR_ADS_DEVICE_ACCESS;
}

/******************************************************************************
 ** Name:    ADS_Read_Single_Reg
 *****************************************************************************/
/**
 ** @brief   read from a single register
 **
 ** @param   regId     register identifier
 ** @param   pData     pointer to variable to store the read data
 **
 ** @return  operation result or error code
 ******************************************************************************/
static ERROR_ID_e ADS_Read_Single_Reg(uint8_t regId, uint8_t *pData)
{
    ssp_err_t err;

    ads_buffer[0] = (uint8_t) ADS_CMD_RREG | regId;
    ads_buffer[1] = 0x00;
    err = spi_patmon.p_api->writeRead (spi_patmon.p_ctrl, ads_buffer, ads_buffer, 3, SPI_BIT_WIDTH_8_BITS, TX_WAIT_FOREVER);

    // store the received data into the user variable (if proceed)
    if (pData)
    {
        *pData = ads_buffer[2];
    }

    return (err == SSP_SUCCESS) ? eERR_NONE : eERR_ADS_DEVICE_ACCESS;
}

/******************************************************************************
 ** Name:    ADS_Write_Single_Reg
 *****************************************************************************/
/**
 ** @brief   write into a single register
 **
 ** @param   regId     register identifier
 ** @param   my_data   data to write in the ADS register
 **
 ** @return  operation result or error code
 ******************************************************************************/
static ERROR_ID_e ADS_Write_Single_Reg(uint8_t regId, uint8_t my_data)
{
    ssp_err_t err;

    ads_buffer[0] = (uint8_t) ADS_CMD_WREG | regId;
    ads_buffer[1] = 0x00;
    ads_buffer[2] = my_data;
    err = spi_patmon.p_api->write (spi_patmon.p_ctrl, ads_buffer, 3, SPI_BIT_WIDTH_8_BITS, TX_WAIT_FOREVER);
    return (err == SSP_SUCCESS) ? eERR_NONE : eERR_ADS_DEVICE_ACCESS;
}

/******************************************************************************
 ** Name:    ADS_Read_Samples
 *****************************************************************************/
/**
 ** @brief   read converted samples from the ADS device
 **
 ** @param   pStatus   pointer to status word
 ** @param   pCh1      pointer to channel 1 data
 ** @param   pCh2      pointer to channel 2 data
 **
 ** @return  operation result or error code
 ******************************************************************************/
static ERROR_ID_e ADS_Read_Samples(uint32_t *pStatus, uint32_t *pCh1, uint32_t *pCh2)
{
    ssp_err_t err;
    uint8_t *pByte;

    // initialize the samples ...
    if (pStatus) *pStatus = 0;
    if (pCh1) *pCh1 = 0;
    if (pCh2) *pCh2 = 0;

    err = spi_patmon.p_api->read (spi_patmon.p_ctrl, ads_buffer, 9, SPI_BIT_WIDTH_8_BITS, TX_WAIT_FOREVER);

    if (err == SSP_SUCCESS)
    {
        if (pStatus)
        {
            pByte = (uint8_t *) pStatus;
            pByte[2] = ads_buffer[0];
            pByte[1] = ads_buffer[1];
            pByte[0] = ads_buffer[2];
        }
        if (pCh1)               // impedance
        {
            pByte = (uint8_t *) pCh1;
            pByte[2] = ads_buffer[3];
            pByte[1] = ads_buffer[4];
            pByte[0] = ads_buffer[5];
        }
        if (pCh2)               // ECG
        {
            pByte = (uint8_t *) pCh2;
            pByte[2] = ads_buffer[6];
            pByte[1] = ads_buffer[7];
            pByte[0] = ads_buffer[8];
        }
    }

    return (err == SSP_SUCCESS) ? eERR_NONE : eERR_ADS_DEVICE_ACCESS;
}

/******************************************************************************
 ** Name:    ADS_Run
 *****************************************************************************/
/**
 ** @brief   set the ADS running and converting ECG
 **
 ** @param   none
 **
 ** @return  operation result or error code
******************************************************************************/
static ERROR_ID_e ADS_Run(void)
{
//    ADS_Send_Command (ADS_CMD_WAKEUP);        // wake-up from standby mode
    ADS_Send_Command (ADS_CMD_START);           // start conversions
    ADS_Send_Command (ADS_CMD_RDATAC);          // enable read data continuous mode

    // set running by pin ...
//  g_ioport.p_api->pinWrite (PAT_START, IOPORT_LEVEL_HIGH);
    ecg_nSample = 0;                            // restart the ECG sample number

    return eERR_NONE;
}

/******************************************************************************
 ** Name:    ADS_Stop
 *****************************************************************************/
/**
 ** @brief   stop the ADS and set in Standby or low power mode
 **
 ** @param   none
 **
 ** @return  operation result or error code
******************************************************************************/
static ERROR_ID_e ADS_Stop(void)
{
    ADS_Send_Command (ADS_CMD_SDATAC);          // stop RDATAC to allow read from registers ...
    ADS_Send_Command (ADS_CMD_STOP);            // stop conversion

    // set in stop mode by pin ...
//  g_ioport.p_api->pinWrite (PAT_START, IOPORT_LEVEL_LOW);

    return eERR_NONE;
}

/******************************************************************************
 ** Name:    Adjust_Zp_Table
 *****************************************************************************/
/**
 ** @brief   adjust the ZP table to the current device & temperature
 **          assume that the calibration table is ascending and the
 **          max & min values are descending values in the matrix
 **
 ** @param   zp_adcs_cal    read ADCs in the calibration resistor
 ** @param   my_temp        current temperature in ºC
 **
 ** @return  none
******************************************************************************/
static void Adjust_Zp_Table (uint32_t zp_adc_cal, int32_t my_temp)
{
    uint32_t i;                 // global use counter
    float_t  min, max;          // minimum and maximum values compensated in temperature
    float_t  fs_a, fs_b;        // Full scale values
    float_t  dv_a, dv_b;        // differential values
    float_t  adc_cal;           // calibration value in ADCs

    // limit the temperature to the matrix range ....
    if (my_temp <  0) my_temp = 0;
    if (my_temp > 60) my_temp = 60;

    adc_cal = (float_t) zp_adc_cal;
    if (device_zp_matrix[0].adc_cal > device_zp_matrix[1].adc_cal)
    {
        // decreasing table
        if (adc_cal > device_zp_matrix[0].adc_cal) adc_cal = device_zp_matrix[0].adc_cal;
        if (adc_cal < device_zp_matrix[MAX_ZP_MATRIX_ITEMS-1].adc_cal) adc_cal = device_zp_matrix[MAX_ZP_MATRIX_ITEMS-1].adc_cal;
    } else {
        // increasing table
        if (adc_cal < device_zp_matrix[0].adc_cal) adc_cal = device_zp_matrix[0].adc_cal;
        if (adc_cal > device_zp_matrix[MAX_ZP_MATRIX_ITEMS-1].adc_cal) adc_cal = device_zp_matrix[MAX_ZP_MATRIX_ITEMS-1].adc_cal;
    }

    //////////////////////////////////////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////////////////////////////////////
    // to get the minimum & maximum values for the "zp_table" (select one of the next root items ...)
    //  *** based on the "zp_adcs_cal" interpolation in the "device_zp_matrix"
    //  *** based on the "my_temp"     interpolation in the "device_zp_matrix"
#if 1
    if (device_zp_matrix[0].adc_cal > device_zp_matrix[1].adc_cal)
    {
        // decreasing table
        for (i=1; i<MAX_ZP_MATRIX_ITEMS; i++)
        {
            if ((adc_cal <= device_zp_matrix[i-1].adc_cal) &&
                (adc_cal >= device_zp_matrix[i  ].adc_cal)) break;
        }
    }
    else {
        // increasing table
        for (i=1; i<MAX_ZP_MATRIX_ITEMS; i++)
        {
            if ((adc_cal >= device_zp_matrix[i-1].adc_cal) &&
                (adc_cal <= device_zp_matrix[i  ].adc_cal)) break;
        }
    }

    // at this point, the "i" index points to the horizontal range in the matrix
    fs_a = device_zp_matrix[i].adc_cal - device_zp_matrix[i-1].adc_cal;
    dv_a = adc_cal - device_zp_matrix[i-1].adc_cal;
#else
    for (i=1; i<MAX_ZP_MATRIX_ITEMS; i++)
    {
        if ((my_temp >= device_zp_matrix[i-1].temp) &&
            (my_temp <= device_zp_matrix[i  ].temp)) break;
    }

    // at this point, the "i" index points to the horizontal range in the matrix
    fs_a = (float_t) (device_zp_matrix[i].temp - device_zp_matrix[i-1].temp);
    dv_a = (float_t) (my_temp - device_zp_matrix[i-1].temp);
#endif

    fs_b = device_zp_matrix[i].adc_max - device_zp_matrix[i-1].adc_max;
    dv_b = (dv_a * fs_b) / fs_a;
    max = device_zp_matrix[i-1].adc_max + dv_b;

    fs_b = device_zp_matrix[i].adc_min - device_zp_matrix[i-1].adc_min;
    dv_b = (dv_a * fs_b) / fs_a;
    min = device_zp_matrix[i-1].adc_min + dv_b;

    // check-point to use with emulator ...
    fs_b = (float_t) (device_zp_matrix[i].temp - device_zp_matrix[i-1].temp);
    dv_b = (dv_a * fs_b) / fs_a;
    my_temp = device_zp_matrix[i-1].temp + (int32_t) dv_b;

    // get the max & min differences to use during interpolation
    fs_a = max - min;

    // assign the minimum value
    zp_table_adcs[0] = (uint32_t) min;

    // interpolate the entire table
    for (i=1; i<(MAX_ZP_ITEMS-1); i++)
    {
        zp_table_adcs[i] = (uint32_t) (min + (fs_a * golden_zp_table[i].ikte));
    }
    zp_table_adcs[MAX_ZP_ITEMS-1] = MAX_ZP_ADCS;
}

/******************************************************************************
 ** Name:    ADS_Init
 *****************************************************************************/
/**
 ** @brief   initializes the ADS1292R device
 **
 ** @param   none
 **
 ** @return  operation result or error code
******************************************************************************/
static ERROR_ID_e ADS_Init(void)
{
//#define DEMOD_PHASE         0b11000010   //   0,00º
//#define DEMOD_PHASE         0b11000110   //  11,25º
//#define DEMOD_PHASE         0b11001010   //  22,50º
//#define DEMOD_PHASE         0b11001110   //  33,75º

//#define DEMOD_PHASE         0b11010010   //  45,00º
//#define DEMOD_PHASE         0b11010110   //  56,25º *****
//#define DEMOD_PHASE         0b11011010   //  67,50º
//#define DEMOD_PHASE         0b11011110   //  78,75º ***

//#define DEMOD_PHASE         0b11100010   //  90,00º
//#define DEMOD_PHASE         0b11100110   // 101,25º **
//#define DEMOD_PHASE         0b11101010   // 112,50º
//#define DEMOD_PHASE         0b11101110   // 123,75º **

#define DEMOD_PHASE         0b11110010     // 135,00º ***
//#define DEMOD_PHASE         0b11110110   // 146,25º
//#define DEMOD_PHASE         0b11111010   // 157,50º
//#define DEMOD_PHASE         0b11111110   // 168,75º

    uint8_t dev_id;         // device identifier
    uint8_t num_oks;        // number of successful identifiers
    uint32_t i;                 // global use counter
    float_t  ref_a, ref_b;      // reference values to adjust the matrix ranges

    // initialize the SPI comm port
    // WARNING. Assume that the maximum clock speed in the SPI controller
    // can be up to twice the internal oscillator frequency (512KHz).
    // Thus, the maximum SPI clock can be 1024 KHz ...
    spi_patmon.p_api->open (spi_patmon.p_ctrl, spi_patmon.p_cfg);

    // connect the ZP channel to the patient
    g_ioport.p_api->pinWrite (PAT_CAL, IOPORT_LEVEL_LOW);

    // connect the ZP channel to the impedance calibration circuit
    //g_ioport.p_api->pinWrite (PAT_CAL, IOPORT_LEVEL_HIGH);

    // restart the ADS device (the device needs 1" for power-on)
    tx_thread_sleep (OSTIME_10MSEC);
    g_ioport.p_api->pinWrite (PAT_nRST, IOPORT_LEVEL_LOW);
    tx_thread_sleep (OSTIME_10MSEC);
    g_ioport.p_api->pinWrite (PAT_nRST, IOPORT_LEVEL_HIGH);
    tx_thread_sleep (OSTIME_10MSEC);
    g_ioport.p_api->pinWrite (PAT_START, IOPORT_LEVEL_LOW);
    tx_thread_sleep (OSTIME_10MSEC);

    // restart the ECG filter
    ECG_Filter_Restart ();

    // stop RDATAC to allow read from registers ...
    for (num_oks=0; num_oks<3; )
    {
        tx_thread_sleep (OSTIME_10MSEC);
        ADS_Send_Command (ADS_CMD_SDATAC);
        ADS_Read_Single_Reg (ADS_REG_ID, &dev_id);
        if (dev_id == ADS1292R_IDENT)
        {
            num_oks++;
        }
        else {
            num_oks = 0;
        }
    }
    tx_thread_sleep (OSTIME_10MSEC);

    // just in case, stop the ADS to configure the internal registers
    ADS_Stop();

    // conversion data rate = 1KSPS
    ADS_Write_Single_Reg (ADS_REG_CONFIG1, 0x03);

    /////////////////////////////////////////////////////////
    // generate test signals ...
    // ADS_Write_Single_Reg (ADS_REG_CONFIG2, 0xA3);
    // ADS_Write_Single_Reg (ADS_REG_CH1SET,  0x05);
    // ADS_Write_Single_Reg (ADS_REG_CH2SET,  0x05);

    /////////////////////////////////////////////////////////
    // read from electrode inputs
    // full scale differential voltage
    //      +-Vref/gain = +-2,42/1 = +-2420mV
    //      2420mV / 2exp23 = 0,2885 uV/bit
    ADS_Write_Single_Reg (ADS_REG_CONFIG2, 0xA0);       // enable the Reference buffer (Vref = 2,42V)
//  ADS_Write_Single_Reg (ADS_REG_CH1SET,  0x41);       // short Channel-1 to force ZP calibration (Gain = 4)
    ADS_Write_Single_Reg (ADS_REG_CH1SET,  0x40);       // connect Channel-1 to electrodes for ZP  (Gain = 4)
    ADS_Write_Single_Reg (ADS_REG_CH2SET,  0x10);       // connect Channel-2 to electrodes for ECG (Gain = 1)

//  ADS_Write_Single_Reg (ADS_REG_CH1SET,  0x40);       // connect Channel-1 to electrodes for ZP  (Gain = 4)
//  ADS_Write_Single_Reg (ADS_REG_CH2SET,  0x40);       // connect Channel-2 to electrodes for ECG (Gain = 4)
//  ADS_Write_Single_Reg (ADS_REG_CH2SET,  0x00);       // connect Channel-2 to electrodes for ECG (Gain = 6)
//  ADS_Write_Single_Reg (ADS_REG_CH2SET,  0x60);       // connect Channel-2 to electrodes for ECG (Gain = 12)

    ADS_Write_Single_Reg (ADS_REG_RESP1, DEMOD_PHASE);  // RESP1 = enable modulator & demodulator - Respiration Phase

    /////////////////////////////////////////////////////////
    // RESP2 = 32KHz, internal feed (default values)
    // ADS_Write_Single_Reg (ADS_REG_RESP2, 0x02);      // default values ...
    ADS_Write_Single_Reg (ADS_REG_RESP2, 0b10000011);   // enable calibration (32 KHz) !!!
    ADS_Send_Command (ADS_CMD_OFFSETCAL);               // calibrate respiration circuits
    ADS_Run();
    tx_thread_sleep  (OSTIME_500MSEC);                  // needs 4xTdrx16 (Tdr at 125 sps) = 512 msecs.
    tx_thread_sleep  (OSTIME_100MSEC);
    ADS_Send_Command (ADS_CMD_SDATAC);                  // stop RDATAC to allow read from registers ...
    ADS_Send_Command (ADS_CMD_STOP);                    // stop conversion
    ADS_Write_Single_Reg (ADS_REG_RESP2, 0b00000011);   // disable calibration (32 KHz) !!!
    tx_thread_sleep  (OSTIME_20MSEC);

    /////////////////////////////////////////////////////////////////////////////////////
    // initialize the device zp-matrix (used to compensate the golden ZP_table at @25ºC)
    device_zp_matrix[1].adc_min = (float_t) Get_Device_Info()->zp_adcs_short;
    device_zp_matrix[1].adc_cal = (float_t) Get_Device_Info()->zp_adcs_calibration;

    ref_a = golden_zp_matrix[1].adc_min - device_zp_matrix[1].adc_min;
    ref_b = golden_zp_matrix[1].adc_cal - device_zp_matrix[1].adc_cal;

    // calculate the projected values to the device matrix (applying ONLY offset)
    for (i=0;i<MAX_ZP_MATRIX_ITEMS; i++)
    {
        device_zp_matrix[i].temp    = golden_zp_matrix[i].temp;
        device_zp_matrix[i].adc_min = golden_zp_matrix[i].adc_min - ref_a;
        device_zp_matrix[i].adc_max = golden_zp_matrix[i].adc_max - ref_a;
        device_zp_matrix[i].adc_cal = golden_zp_matrix[i].adc_cal - ref_b;
    }

    Adjust_Zp_Table (Get_Device_Info()->zp_adcs_calibration, 25);

    ADS_Read_Samples (NULL, NULL, NULL);                // execute a fake reading !!!
    ADS_Run ();                                         // set the ADS running

    return eERR_NONE;
}

/******************************************************************************
 ** Name:    ECG_Filter_Restart
 *****************************************************************************/
/**
 ** @brief   restart sample delays used filtering ECG
 **
 ** @param   none
 **
 ** @return  none
******************************************************************************/
static void ECG_Filter_Restart (void)
{
    uint16_t    i;

    // restart all items
    for (i=0; i<NUM_KOEFS_ECG_FILTER; i++)
    {
        IIR_ecg_dly [i] = (float_t) 0.0;
    }
}

/******************************************************************
###################################################################
###  template to use for IIR filter implementation
###################################################################
static double iirfilter(float_t x1)
{
    static float_t b[] = {1,  -1.4, 1};
    static float_t a[] = {1, -1.3, 0.5};
    static float_t v1m1 = 0, v2m1 = 0;

    float_t y1, v1m, v2m;

    y1  = (b[0] * x1 + v1m1);
    v1m = (b[1] * x1 + v2m1) - (a[1] * y1);
    v2m = (b[2] * x1)        - (a[2] * y1);
    v1m1 = v1m;
    v2m1 = v2m;
    return y1;
}
*/

/******************************************************************************
 ** Name:    ECG_Offset_Removal
 *****************************************************************************/
/**
 ** @brief   remove the DC offset from the ECG samples
 **
 ** @param   sample        ECG sample
 **
 ** @return  modified sample
******************************************************************************/
static int16_t ECG_Offset_Removal(int32_t sample)
{
    float_t ecg_cs;                                 // ECG current sample
    float_t lpf_cs;                                 // LPF current sample
    float_t hpf_cs;                                 // HPF current sample
    float_t nf1_cs;                                 // NF1 current sample
    float_t nf2_cs;                                 // NF2 current sample
    float_t out_cs;                                 // OUT current sample
    float_t v1m, v2m;

    // extend the sign (from 24-bit unsigned to 32-bit signed)
    if (sample & 0x800000)
    {
        sample |= (int32_t) 0xFF000000;
    }

    // convert to float ...
    ecg_cs = (float_t) sample;
    lpf_cs = ecg_cs;
    hpf_cs = ecg_cs;
    out_cs = ecg_cs;

    // execute the IIR for the LPF
    lpf_cs = (IIR_LPF_B[0] * ecg_cs) + (IIR_LPF_B[1] * IIR_ecg_dly[0]) - (IIR_LPF_A[1] * IIR_ecg_dly[1]);
    IIR_ecg_dly[0] = ecg_cs;
    IIR_ecg_dly[1] = lpf_cs;

    // execute the IIR for the HPF
    hpf_cs = (IIR_HPF_B[0] * lpf_cs) + (IIR_HPF_B[1] * IIR_ecg_dly[2]) - (IIR_HPF_A[1] * IIR_ecg_dly[3]);
    IIR_ecg_dly[2] = lpf_cs;
    IIR_ecg_dly[3] = hpf_cs;

    // execute the IIR for the Notch
    if (pNF1A)
    {
        // STEP 1
        nf1_cs = (hpf_cs             + IIR_ecg_dly[4]);
        v1m    = (hpf_cs * pNF1B[1]) + IIR_ecg_dly[5] - (nf1_cs * pNF1A[1]);
        v2m    = (hpf_cs           )                  - (nf1_cs * pNF1A[2]);
        IIR_ecg_dly[4] = v1m;
        IIR_ecg_dly[5] = v2m;

        // STEP 2
        nf2_cs = (nf1_cs             + IIR_ecg_dly[6]);
        v1m    = (nf1_cs * pNF2B[1]) + IIR_ecg_dly[7] - (nf2_cs * pNF2A[1]);
        v2m    = (nf1_cs           )                  - (nf2_cs * pNF2A[2]);
        IIR_ecg_dly[6] = v1m;
        IIR_ecg_dly[7] = v2m;
        out_cs = nf2_cs;
    }
    else {
        out_cs = hpf_cs;
    }

    // scale the samples to get +-20mV of full scale in 16 bits
    //      20mV / 2exp15 = 0,610 uV/bit
    //      0,2885 uV/bit / 0,610 uV/bit = 0,4727
    // getting 1mV = 1639 ADCs
//    return ((int16_t) (out_cs * (float_t) 0.4727f));

    // scale the samples to get +-20mV of full scale in 13 bits
    //      20mV / 2exp12 = 4,883 uV/bit
    //      0,2885 uV/bit / 4,883 uV/bit = 0,0591
    // getting 1mV = 204 ADCs
    return ((int16_t) (out_cs * (float_t) 0.0591f));
}

extern int16_t CU_ECG_Offset_Removal(int32_t sample){
    return ECG_Offset_Removal(sample);
}

/******************************************************************************
 ** Name:    ZP_Update_Value
 *****************************************************************************/
/**
 ** @brief   Update the ZP value. This function is called in each sample
 ** capture at 1 Ksps. The function filters and decimates the received samples
 ** to obtain a FS_ZP impedance measurements.
 **
 ** @param   zp_sample     sample to calculate the ZP value (24-bit sample)
 **
 ** @return  none
******************************************************************************/
static void ZP_Update_Value (uint32_t zp_sample)
{
#define ZP_MEAS             200             // number of measurements to compute the "official" ZP value
#define IGNORE_OFFSET       0               // activate (set to '1') to generate the ZP table
#define REPORT_OHMS         1               // report Ohms vs. ADCs when request for the patient impedance

    static uint32_t zp_raw [ZP_MEAS];       // buffer to store the ZP raw data (assume that is initialized at 0x00)
    static uint32_t zp_raw_acc = 0;         // accumulated value to compute the "official" ZP value
           uint32_t zp_raw_idx;             // index to write in the ZP raw buffer
           int32_t  my_adcs;                // local impedance in ADCs
           int32_t  my_temp;                // local temperature in degrees

    static uint32_t aux_idx = 0;            // index to write in the auxiliary ZP buffer
    static uint32_t aux_acc = 0;            // accumulated value for sample decimation
    static uint32_t cal_acc = 0;            // accumulated value for offset calculation

    static uint32_t my_free_counter = 0;    // free counter
    static uint32_t my_cali_counter = 0;    // calibration counter
    static uint32_t my_zp_sample;

    // implement a calibration procedure when DRD is processing the ECG
    if (cal_running)
    {
        cal_acc += zp_sample;
    }
    else {
        my_zp_sample = zp_sample;
    }

#if (IGNORE_OFFSET == 0)

// report OHMS vs. ADCs
#if (REPORT_OHMS == 0)
    // calibrate periodically when the ZP is out of the analysis range
    if (my_cali_counter >= 20000) my_cali_counter = 0;
#else
    if (patMon_Get_Zp_Segment () == eZP_SEGMENT_GOOD_CONN)
    {
        // calibrate when start the DRD analysis because the ZP is in the analysis range
        if (!cal_running && !DRD_Is_Running ()) my_cali_counter = 0;
    }
    else {
        // calibrate periodically when the ZP is out of the analysis range
        if (my_cali_counter >= 20000) my_cali_counter = 0;
    }

    /////////////////////////////////////////////////////////////////////////
    // if capacitor is in charge process, restart the calibrator
    if (!cal_running && (Defib_Get_State () == eDEFIB_STATE_CHARGING)) my_cali_counter = 0;

#endif /* REPORT_OHMS */

    my_cali_counter++;
    if (my_cali_counter == 100)
    {
        // set as calibrating (use the same time slot to measure the on-board temperature)
        cal_running = true;
        cal_acc = 0;
        g_ioport.p_api->pinWrite (PAT_CAL, IOPORT_LEVEL_HIGH);

        // connect Channel-1 to temperature sensor (Gain = 4)
        ADS_Send_Command (ADS_CMD_STOP);                            // stop conversion
        ADS_Write_Single_Reg (ADS_REG_CH1SET, 0x44);                // select temperature channel (gain = 4)
        ADS_Write_Single_Reg (ADS_REG_RESP1, DEMOD_PHASE & 0x7F);   // disable demodulator
        ADS_Send_Command (ADS_CMD_START);                           // start conversions
    }
    if (my_cali_counter == 150)
    {
        // the temperature value has been read in the previous 50 msec (from 100 to 150 in the calibration counter)
        temperature_adcs = (int32_t) cal_acc / 50;

        // connect Channel-1 to electrodes for ZP  (Gain = 4)
        ADS_Send_Command (ADS_CMD_STOP);                    // stop conversion
        ADS_Write_Single_Reg (ADS_REG_CH1SET, 0x40);        // select input channel (gain = 4)
        ADS_Write_Single_Reg (ADS_REG_RESP1, DEMOD_PHASE);  // enable demodulator with the selected phase
        ADS_Send_Command (ADS_CMD_START);                   // start conversions
    }
    if (my_cali_counter == 200)
    {
        cal_acc = 0;
    }
    if (my_cali_counter == (200 + ZP_MEAS))
    {
        g_ioport.p_api->pinWrite (PAT_CAL, IOPORT_LEVEL_LOW);

        //////////////////////////////////////////////////////////////////////////////
        // the capacitor charge is an asynchronous process ...
        // validate the calibration ONLY if the defibrillator is not charging !!!
        if (Defib_Get_State () != eDEFIB_STATE_CHARGING)
        {
            // adjust the ZP table in function of the read calibration impedance & temperature
            zp_cal = (cal_acc / ZP_MEAS);
            my_temp = patMon_Get_Temperature();
            Adjust_Zp_Table (zp_cal, my_temp);
        }
    }
    if (my_cali_counter == (200 + ZP_MEAS + 100))
    {
        cal_running = false;
    }
#else
    zp_cal = zp_offset_static = zp_offset_dynamic = 0;
#endif  /* IGNORE_OFFSET */

    // ZP filter to update the official ZP value in ADCs
    zp_raw_idx = my_free_counter % ZP_MEAS;
    my_free_counter++;
    zp_raw_acc -= zp_raw[zp_raw_idx];
    zp_raw_acc += my_zp_sample;
    zp_raw[zp_raw_idx] = my_zp_sample;
    zp_adcs_raw = (int32_t) (zp_raw_acc / ZP_MEAS);
    my_adcs = zp_adcs_raw + zp_offset_static + zp_offset_dynamic;
    if (my_adcs > MAX_ZP_ADCS) my_adcs = MAX_ZP_ADCS;
    zp_adcs = my_adcs;

    // ZP samples decimation and filter before register in the pre-processed ZP buffer ...
    aux_acc += my_zp_sample;
    aux_idx ++;
    if (aux_idx >= RESAMPLE_ZP)
    {
        // register the new data in the ZP buffer
        zp_data [idx_zp++] = (aux_acc / RESAMPLE_ZP);
        if (idx_zp >= ZP_BUFFER_SIZE) idx_zp = 0;
        aux_acc = 0;
        aux_idx = 0;
    }
}

/******************************************************************************
 ** Name:    PMK_Pulse_Detect_and_Remove
 *****************************************************************************/
/**
 ** @brief   Detect a single PMK pulse and remove it from the ECG signal
 **          to be recovered after all ECG filters have been applied
 **
 ** @param   pSample    reference to the current sample
 **
 ** @return  none
 ******************************************************************************/
static void PMK_Pulse_Detect_and_Remove (int32_t *pSample)
{
#define PMK_PULSE_AMPLITUDE   6932              // minimum PMK Pulse amplitude in ADCs
                                                // 0,2885 * 6932 = 2mV

    static  int32_t ecg_fifo[PMK_PULSE_LEN];    // FIFO for PMK pulse detector
    int32_t add_L, add_C, add_R;                // intermediate adds to evaluate left and right ramps
    uint8_t ramp_desc = 0;                      // ramp descriptor

    // update the ECG FIFO with the new sample and report the older one
    // this operation insert a 'N' samples delay in the ECG signal
    ecg_fifo[0] = ecg_fifo[1];
    ecg_fifo[1] = ecg_fifo[2];
    ecg_fifo[2] = ecg_fifo[3];
    ecg_fifo[3] = ecg_fifo[4];
    ecg_fifo[4] = ecg_fifo[5];
    ecg_fifo[5] = ecg_fifo[6];
    ecg_fifo[6] = *pSample;
    *pSample    = ecg_fifo[0];

    // if refractory time is enabled, ignore the pulse
    if (pmk_pulse_desc.time_refractory)
    {
        pmk_pulse_desc.time_refractory--;
        return;
    }


    // try to identify a positive/negative PMK pulse in a 'N' samples window
    add_C = 3 * ecg_fifo[3];
    add_L = add_C - (ecg_fifo[0] + ecg_fifo[1] + ecg_fifo[2]);
    add_R = add_C - (ecg_fifo[4] + ecg_fifo[5] + ecg_fifo[6]);
    add_C = (abs (add_L - add_R)) / 2;

    // check for ramp amplitude
    if (add_C < PMK_PULSE_AMPLITUDE) return;

    // register the ramp trends
    if (ecg_fifo[6] >= ecg_fifo[5]) ramp_desc |= 0b01000000;
    if (ecg_fifo[5] >= ecg_fifo[4]) ramp_desc |= 0b00100000;
    if (ecg_fifo[4] >= ecg_fifo[3]) ramp_desc |= 0b00010000;
    if (ecg_fifo[3] >= ecg_fifo[2]) ramp_desc |= 0b00001000;
    if (ecg_fifo[2] >= ecg_fifo[1]) ramp_desc |= 0b00000100;
    if (ecg_fifo[1] >= ecg_fifo[0]) ramp_desc |= 0b00000010;

    // check for positive or negative pulse (3 samples or 2 samples wide)
    if ((ramp_desc == 0b01110000) || (ramp_desc == 0b00001110) ||
        (ramp_desc == 0b00110000) || (ramp_desc == 0b00001100))
    {
        // characterize the pulse as variations respect previous samples
        pmk_pulse_desc.data [6] = ecg_fifo[6] - ecg_fifo[5];
        pmk_pulse_desc.data [5] = ecg_fifo[5] - ecg_fifo[4];
        pmk_pulse_desc.data [4] = ecg_fifo[4] - ecg_fifo[3];
        pmk_pulse_desc.data [3] = ecg_fifo[3] - ecg_fifo[2];
        pmk_pulse_desc.data [2] = ecg_fifo[2] - ecg_fifo[1];
        pmk_pulse_desc.data [1] = ecg_fifo[1] - ecg_fifo[0];
        pmk_pulse_desc.data [0] = 0;

        // fill the intermediate buffer with virtual samples
        add_C = (ecg_fifo[6] - ecg_fifo[0]) / 6;
        ecg_fifo[1] = ecg_fifo[0] + add_C;
        ecg_fifo[2] = ecg_fifo[1] + add_C;
        ecg_fifo[3] = ecg_fifo[2] + add_C;
        ecg_fifo[4] = ecg_fifo[3] + add_C;
        ecg_fifo[5] = ecg_fifo[4] + add_C;

        // apply a refractory time before start the search again
        // the maximum frequency for PMK pulses is 180 ppm's (300 msecs period)
        pmk_pulse_desc.time_refractory = 280;
        pmk_pulse_desc.time_delay = 5; //6
        pmk_pulse_desc.data_count = PMK_PULSE_LEN;
    }
}

/******************************************************************************
 ** Name:    PMK_Pulse_Restore
 *****************************************************************************/
/**
 ** @brief   Restore the PMK pulse to the filtered signal
 **
 ** @param   pSample    reference to the current sample
 **
 ** @return  none
 ******************************************************************************/
static void PMK_Pulse_Restore (int16_t *pSample)
{
    if (pmk_pulse_desc.time_delay) pmk_pulse_desc.time_delay--;

    if ((pmk_pulse_desc.time_delay == 0) && pmk_pulse_desc.data_count)
    {
        if (pmk_pulse_desc.data_count == 7)
        {
            // scale the original samples to the filtered full scale
            //      20mV / 2exp12 = 4,883 uV/bit
            //      0,2885 uV/bit / 4,883 uV/bit = 0,0591
            //      1 / 0,0591 = 17 (rounded value)
            pmk_pulse_desc.data[0] /= 17;
            pmk_pulse_desc.data[1] /= 17;
            pmk_pulse_desc.data[2] /= 17;
            pmk_pulse_desc.data[3] /= 17;
            pmk_pulse_desc.data[4] /= 17;
            pmk_pulse_desc.data[5] /= 17;
            pmk_pulse_desc.data[6] /= 17;

            // update the PMK samples values
            pmk_pulse_desc.data[0] += (int32_t) (*pSample);
            pmk_pulse_desc.data[1] += pmk_pulse_desc.data[0];
            pmk_pulse_desc.data[2] += pmk_pulse_desc.data[1];
            pmk_pulse_desc.data[3] += pmk_pulse_desc.data[2];
            pmk_pulse_desc.data[4] += pmk_pulse_desc.data[3];
            pmk_pulse_desc.data[5] += pmk_pulse_desc.data[4];
            pmk_pulse_desc.data[6] += pmk_pulse_desc.data[5];
        }
             if (pmk_pulse_desc.data_count == 6) *pSample = (int16_t) pmk_pulse_desc.data[1];
        else if (pmk_pulse_desc.data_count == 5) *pSample = (int16_t) pmk_pulse_desc.data[2];
        else if (pmk_pulse_desc.data_count == 4) *pSample = (int16_t) pmk_pulse_desc.data[3];
        else if (pmk_pulse_desc.data_count == 3) *pSample = (int16_t) pmk_pulse_desc.data[4];
        else if (pmk_pulse_desc.data_count == 2) *pSample = (int16_t) pmk_pulse_desc.data[5];
        else if (pmk_pulse_desc.data_count == 1) *pSample = (int16_t) pmk_pulse_desc.data[6];

        // update the data counter
        pmk_pulse_desc.data_count--;
    }
}


/******************************************************************************
 ** Name:    ADS_Read_Frame
 *****************************************************************************/
/**
 ** @brief   Read a single frame from the ADS (analog front end)
 **
 ** @param   none
 **
 ** @return  none
 ******************************************************************************/
void ADS_Read_Frame(void)
{
#define NLOOPS_FOR_INVALID_ECG  10          // each unit is 1 msec (at 1 Ksps)

#define MAX_ECG_GRADIENT        1739        // maximum ECG gradient in ADCs
                                            // 0,2885 * 1739 = 500 uV/msec ramp
//#define MAX_ECG_GRADIENT        868         // maximum ECG gradient in ADCs
                                            // 0,2885 * 868 = 250 uV/msec ramp
//#define MAX_ECG_GRADIENT        346         // maximum ECG gradient in ADCs
                                            // 0,2885 * 346 = 100 uV/msec ramp

           uint32_t sample[3];              // samples read from ADS
            int32_t curr_ecg;               // current  ECG (type casting done)
           uint32_t curr_zp;                // current  ZP

    static  int32_t last_ecg;               // storage for the last ECG sample
    static uint32_t last_zp;                // storage for the last ZP  sample
    static uint32_t zp_too_high_cont = 0;   // counter for ZP too high condition ...

g_ioport.p_api->pinWrite (IOPORT_PORT_06_PIN_01, IOPORT_LEVEL_HIGH);

    if (idx_ecg >= ECG_BUFFER_SIZE)
    {
        idx_ecg = 0;
    }

    ADS_Read_Samples (&sample[0], &sample[1], &sample[2]);

    // ECG type cast with sign expansion
    if (sample[2] & 0x00800000) sample[2] |= 0xFF000000;
    curr_ecg = ( int32_t) sample[2];
    curr_zp  = (uint32_t) sample[1];

    // check the status identifier bits (0b1100)
    if (STATUS_IS_OK(sample[0]))
    {
        //calibrating the ZP, recover the last samples ...
        if (cal_running) curr_ecg = last_ecg;

        // try to detect and remove a PMK pulse from the original ECG signal
        PMK_Pulse_Detect_and_Remove (&curr_ecg);

        // saturate the ECG signal when electric noise appears (accelerates Notch response)
        if ((last_ecg > curr_ecg) && ((last_ecg - curr_ecg) > MAX_ECG_GRADIENT))
        {
            curr_ecg = last_ecg - MAX_ECG_GRADIENT;
        }
        else if ((last_ecg < curr_ecg) && ((curr_ecg - last_ecg) > MAX_ECG_GRADIENT))
        {
            curr_ecg = last_ecg + MAX_ECG_GRADIENT;
        }

        // store the read values (just in case) to use in case of wrong read
        last_ecg = curr_ecg;
        last_zp  = curr_zp;
    }
    else
    {
        // read wrong data, recover the last samples ...
        curr_ecg = last_ecg;
        curr_zp  = last_zp;

        wrong_data_cont++;
    }

    // update the ZP value (zp_adcs)
    ZP_Update_Value (curr_zp);

    /////////////////////////////////////////////////////////////////////////////////////
    // check if ZP value is too high (ignore the count when the ZP is calibrating)
//  if ((sample[1] > ZP_ADCS_FOR_VALID_ECG) && !cal_running)
    if (zp_adcs > (int32_t) ZP_ADCS_FOR_VALID_ECG)
    {
        zp_too_high_cont++;
    }
    else {
        zp_too_high_cont = 0;
    }

    /////////////////////////////////////////////////////////////////////////////////////
    // if the ZP value is too high for a minimum period of time, ignore the ECG samples
    // the counter is add at a 1 KHz rate (1 msec per loop)
    if (zp_too_high_cont < NLOOPS_FOR_INVALID_ECG)
    {
        // Valid ECG --> must remove dc-offset from ECG samples ...
        ecg_data[idx_ecg] = ECG_Offset_Removal (curr_ecg);

        // restore the PMK pulse to the filtered ECG signal
        PMK_Pulse_Restore (&ecg_data[idx_ecg]);
    }
    else {
        // ignore the ECG value
        ecg_data[idx_ecg] = 0;

        // restart the ECG-ZP filters
        ECG_Filter_Restart ();
    }

    if (Is_Test_Mode_Montador())
    {
        // Valid ECG --> must remove dc-offset from ECG samples ...
        ecg_data[idx_ecg] = ECG_Offset_Removal (curr_ecg);
    }

#if (AUDIO_OUT_SHOW_BIOMETRICS == 1)
    {
        uint16_t sample_to_dac;                 // sample to monitor in the audio output ...
        // send the signal to the DAC converter ...
        sample_to_dac = ecg_data[idx_ecg] + 0x8000;  // ECG
        //sample_to_dac = sample[1];                 // ZP
        g_dac0.p_api->write (g_dac0.p_ctrl, sample_to_dac);
    }
#endif

#if 0   // INAXIO
if (rt_ready < 5)
{
//    if ((ecg_nSample & 0x05) == 0x07)
        rt_idx++;
    if (rt_idx >= 600) { rt_idx = 0; rt_ready++; }

    rt_ecg[rt_idx] = ecg_data[idx_ecg];
    rt_ezp[rt_idx] = (uint16_t) zp_adcs_raw;
}
#endif


    // increment the ECG sample counter and the buffer index
    ecg_nSample++;
    idx_ecg++;

g_ioport.p_api->pinWrite (IOPORT_PORT_06_PIN_01, IOPORT_LEVEL_LOW);
}

/******************************************************************************
 ** Name:    g_irq8_callback
 *****************************************************************************/
/**
 ** @brief   IRQ8 interrupt callback process
 **
 ** @param   IRQ identifier
 **
 ** @return  none
 ******************************************************************************/
void g_irq8_callback(external_irq_callback_args_t *p_args)
{
    // if ADS data ready is detected, proceed to read the frame
    if (p_args->channel & IRQ_ADS_DRDY)
    {
        tx_event_flags_set (&g_events_PatMon, IRQ_ADS_DRDY, TX_OR);
    }
}

/******************************************************************************
 ** Name:    patMon_HAL_Init
 *****************************************************************************/
/**
 ** @brief   Initialize the HAL for the patient monitor
 **
 ** @param   myNotch         Notch filter identifier
 **
 ** @return  none
 ******************************************************************************/
void patMon_HAL_Init (NOTCH_t myNotch)
{

#if (AUDIO_OUT_SHOW_BIOMETRICS == 1)
    g_dac0.p_api->open  (g_dac0.p_ctrl, g_dac0.p_cfg);
    g_dac0.p_api->start (g_dac0.p_ctrl);
#endif

    // open external interrupt driver
    g_external_irq.p_api->open (g_external_irq.p_ctrl, g_external_irq.p_cfg);

    // enable external interrupt
    g_external_irq.p_api->enable (g_external_irq.p_ctrl);

    // initialize the pointers for coefficients in the Notch filter
    if (myNotch == NOTCH_50)
    {
        pNF1A = (float_t *) IIR_N50_1A;
        pNF1B = (float_t *) IIR_N50_1B;
        pNF2A = (float_t *) IIR_N50_2A;
        pNF2B = (float_t *) IIR_N50_2B;
    }
    else if (myNotch == NOTCH_60)
    {
        pNF1A = (float_t *) IIR_N60_1A;
        pNF1B = (float_t *) IIR_N60_1B;
        pNF2A = (float_t *) IIR_N60_2A;
        pNF2B = (float_t *) IIR_N60_2B;
    }
    else
    {
        pNF1A = (float_t *) NULL;
        pNF1B = (float_t *) NULL;
        pNF2A = (float_t *) NULL;
        pNF2B = (float_t *) NULL;
    }

    // initialize the analog front end
    ads_status = ADS_Init ();
}

/******************************************************************************
 ** Name:    patMon_Get_ECG_Window
 *****************************************************************************/
/**
 ** @brief   Read the last ECG samples from the ECG buffer
 **
 ** @param   n_ecg_first     first ecg sample to retrieve
 ** @param   nSamples        number of samples to read
 ** @param   pSamples        Pointer to store buffer
 **
 ** @return  window is correct or not
 ******************************************************************************/
bool_t patMon_Get_ECG_Window(uint32_t nEcg_first, uint32_t nSamples, int16_t *pSamples)
{
    uint32_t    first;      // first item in the ECG buffer
    uint32_t    nItems;     // number of items to transfer
    uint32_t    idx, nEcg;  // local copies from global variables ...

    // make local copies
    idx  = idx_ecg;
    nEcg = ecg_nSample;

    // check if the requested ECG window is stored in the ECG frame buffer
    if ((nEcg_first + nSamples) > nEcg)
    {
        // -> The number of requested ECG samples is bigger than the ADQUIRED samples
        return false;
    }
    else if ((nEcg - nEcg_first) >= ECG_BUFFER_SIZE)
    {
        // -> The number of requested ECG samples is bigger than the ECG window size
        return false;
    }

    first  = (idx >= (nEcg - nEcg_first)) ? 0 : ECG_BUFFER_SIZE;
    first += idx;
    first -= (nEcg - nEcg_first);

    ////////////////////////////////////////////////////////////////
    // fill the destination buffer with the registered samples
    if ((first + nSamples) >= ECG_BUFFER_SIZE)
    {
        nItems = ECG_BUFFER_SIZE - first;
        memcpy (&pSamples[0],      &ecg_data[first], sizeof (int16_t) * nItems);
        memcpy (&pSamples[nItems], &ecg_data[0],     sizeof (int16_t) * (nSamples - nItems));
    }
    else {
        memcpy (pSamples, &ecg_data[first], sizeof (int16_t) * nSamples);
    }

    return true;
}

/******************************************************************************
 ** Name:    patMon_Get_ZP_Window
 *****************************************************************************/
/**
 ** @brief   Read the last ZP samples from the ZP buffer
 **
 ** @param   nZp_first       first zp sample to retrieve
 ** @param   nSamples        number of samples to read
 ** @param   pSamples        Pointer to store buffer
 **
 ** @return  window is correct or not
 ******************************************************************************/
bool_t patMon_Get_ZP_Window(uint32_t nZp_first, uint32_t nSamples, uint32_t *pSamples)
{
    uint32_t    first;      // first item in the ECG buffer
    uint32_t    nItems;     // number of items to transfer
    uint32_t    idx, nZp;   // local copies from global variables ...

    // make local copies
    idx = idx_zp;
    nZp = ecg_nSample / RESAMPLE_ZP;
    nZp_first /= RESAMPLE_ZP;

    // check if the requested zp samples are stored in the zp frame buffer
    if ((nZp_first + nSamples) > nZp)
    {
        // -> The number of requested Zp samples is bigger than the ADQUIRED samples
        return false;
    }
    else if ((nZp - nZp_first) >= ZP_BUFFER_SIZE)
    {
        // -> The number of requested Zp samples is bigger than the Zp buffer size
        return false;
    }

    first  = (idx >= (nZp - nZp_first)) ? 0 : ZP_BUFFER_SIZE;
    first += idx;
    first -= (nZp - nZp_first);

    ////////////////////////////////////////////////////////////////
    // fill the destination buffer with the registered samples
    if ((first + nSamples) >= ZP_BUFFER_SIZE)
    {
        nItems = ZP_BUFFER_SIZE - first;
        memcpy (&pSamples[0],      &zp_data[first], sizeof (uint32_t) * nItems);
        memcpy (&pSamples[nItems], &zp_data[0],     sizeof (uint32_t) * (nSamples - nItems));
    }
    else {
        memcpy (pSamples, &zp_data[first], sizeof (uint32_t) * nSamples);
    }

    return true;
}


/******************************************************************************
 ** Name:    patMon_Get_ECG_nSample
 *****************************************************************************/
/**
 ** @brief   Report the current ECG identifier
 **
 ** @param   void
 **
 ** @return  ECG sample identifier
 ******************************************************************************/
uint32_t patMon_Get_ECG_nSample(void)
{
    // report the current ECG sample identifier
    return ecg_nSample;
}

/******************************************************************************
 ** Name:    patMon_Get_Connected_El
 *****************************************************************************/
/**
 ** @brief   Check if electrodes are connected
 **
 ** @param   void
 **
 ** @return  The connected electrodes type
 ******************************************************************************/
EL_TYPE_ID_e patMon_Get_Connected_El(void)
{
   if (pMisc_Settings->glo_patient_adult == false) // adult patient
    {
        return eEL_DISPOSABLE_ADULT;
    }
    else
    {
        return eEL_DISPOSABLE_PEDIATRIC;
    }
}

/******************************************************************************
 ** Name:    patMon_Get_zp_CAL_ADCs
 *****************************************************************************/
/**
 ** @brief   report the calibration impedance value in ADCs
 **
 ** @param   void
 **
 ** @return  The measured impedance value
 ******************************************************************************/
uint32_t patMon_Get_zp_CAL_ADCs(void)
{
    return (zp_cal);
}

/******************************************************************************
 ** Name:    patMon_Get_Zp_ADCs
 *****************************************************************************/
/**
 ** @brief   report the impedance value in ADCs (this value is not compensated !!!)
 **
 ** @param   void
 **
 ** @return  The measured impedance value
 ******************************************************************************/
uint32_t patMon_Get_Zp_ADCs(void)
{
    return ((uint32_t) zp_adcs_raw);        // report the raw value
//  return ((uint32_t) zp_adcs);            // report the compensated value
}

/******************************************************************************
 ** Name:    patMon_Get_Zp_Ohms
 *****************************************************************************/
/**
 ** @brief   report the impedance value (in Ohms, ADCs or virtual)
 **
 ** @param   void
 **
 ** @return  The measured impedance value
 ******************************************************************************/
uint32_t patMon_Get_Zp_Ohms(void)
{
    uint16_t i;                     // global use counter
    uint32_t diff_adcs, diff_ohms;  // increment values used in interpolation
    uint32_t my_adcs;               // value to convert

    // get the current impedance in ADCs
    my_adcs = (uint32_t) zp_adcs;

/*
    // return the impedance value (in ADCs)
    if (pMisc_Settings->glo_zp_virtual == REPORT_ZP_ACDS)
    {
        return (my_adcs);
    }

    // return the simulated impedance (in Ohms)
    if (pMisc_Settings->glo_zp_virtual >= REPORT_ZP_VIRTUAL)
    {
        return (pMisc_Settings->glo_zp_virtual);
    }
*/
    // assume an incremental table
    // be sure that the table finishes with zero
    if (zp_table_adcs[0] >= my_adcs) { zp_ohms = golden_zp_table[0].ohms; return zp_ohms; }
    for (i=1; (1); i++)
    {
        if (i >= MAX_ZP_ITEMS) { zp_ohms = MAX_ZP_OHMS; return zp_ohms; }
        if ((my_adcs >= zp_table_adcs[i-1]) && (my_adcs <= zp_table_adcs[i])) break;
    }

    // interpolate the read value ...
    diff_adcs = (uint32_t) (zp_table_adcs[i] - zp_table_adcs[i-1]);
    diff_ohms = (uint32_t) (golden_zp_table[i].ohms - golden_zp_table[i-1].ohms);
    if (diff_adcs == 0) { zp_ohms = MAX_ZP_OHMS; return zp_ohms; }

    zp_ohms = (((my_adcs - zp_table_adcs[i-1]) * diff_ohms) / diff_adcs) + golden_zp_table[i-1].ohms;
    return (zp_ohms);
}

/******************************************************************************
** Name:    patMon_Get_Zp_Segment
*****************************************************************************/
/**
** @brief   Ask for the patient impedance segment
**
** @param   none
**
** @return  Patient impedance segment
**
******************************************************************************/
ZP_SEGMENT_e patMon_Get_Zp_Segment (void)
{
    uint32_t     my_zp;                             // local & formal ZP value (in ohms)
    ZP_SEGMENT_e segment = eZP_SEGMENT_UNKNOWN;

    // read from the formal Zp value
    my_zp = zp_ohms;

    if (Electrodes_Get_Signature() == eEL_SIGN_MUST_DEMO)
    {
        segment = eZP_SEGMENT_OPEN_CIRC;
        return segment;
    }

    if (my_zp == (uint32_t) DATA_NOT_AVAILABLE)
    {
        return eZP_SEGMENT_UNKNOWN;
    }


    if (my_zp <= ZP_GOOD_CONN_THR)
    {
        segment = eZP_SEGMENT_SHORT_CIRC;
    }
    else if (my_zp <= ZP_BAD_CONN_THR)
    {
        segment = eZP_SEGMENT_GOOD_CONN;
    }
    else if (my_zp <= ZP_OPEN_CIRC_THR)
    {
        segment = eZP_SEGMENT_BAD_CONN;
    }
    else
    {
        segment = eZP_SEGMENT_OPEN_CIRC;
    }

    return segment;
}

/******************************************************************************
 ** Name:    patMon_Get_Temperature
 *****************************************************************************/
/**
 ** @brief   report the temperature value in degrees
 **          be careful, the PGA gain is 4
 **
 **          T (ºC)   = [ ADC (uV) - 145300 uV / 490 uV/ºC ] + 25ºC
 **
 **          ADC (uV) = [ (ADC * 2420 mV * 10exp3) / (2exp23 * 4) ]
 **          ADC (uV) = [ (ADC * 72) / 1000 ]
 **
 ** @param   void
 **
 ** @return  The measured temperature value
 ******************************************************************************/
int32_t patMon_Get_Temperature(void)
{
    int32_t my_degrees;

    // translate from ADCs to degrees
    my_degrees  = (temperature_adcs * 72) / 1000;
    my_degrees -= 145300;
    my_degrees /= 490;
    my_degrees += 25;

    // saturate the temperature value ...
    if (my_degrees < -10) my_degrees = -10;
    if (my_degrees >  70) my_degrees = 70;

    return (my_degrees);
}

/******************************************************************************
 ** Name:    patMon_Get_Status
 *****************************************************************************/
/**
 ** @brief   report the ADS operating status
 **
 ** @param   none
 **
 ** @return  ADS status (error code)
******************************************************************************/
ERROR_ID_e patMon_Get_Status (void)
{
    return ads_status;
}
