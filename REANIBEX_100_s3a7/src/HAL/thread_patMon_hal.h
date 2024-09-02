 /******************************************************************************
 * Name      : R100_S3A7                                                      *
 * Copyright : Osatu S. Coop                                                  *
 * Compiler  : GNU ARM Embedded                                               *
 * Target    : Reanibex Serie 100                                             *
 ******************************************************************************/

/*!
 * @file        thread_patMon_hal.h
 * @brief       Header with functions related to the patMon BSP service
 *
 * @version     v1
 * @date        16/01/2018
 * @author      ltorres
 * @warning     author   date    version     .- Edition
 * @bug
 *
 */

#ifndef THREAD_PATMON_HAL_H_
#define THREAD_PATMON_HAL_H_

/******************************************************************************
 **Includes
 */

#include "types_basic.h"
#include "R100_Errors.h"


/******************************************************************************
 ** Defines
 */

#define PAT_nDRDY           IOPORT_PORT_03_PIN_05
#define PAT_nRST            IOPORT_PORT_04_PIN_03
#define PAT_START           IOPORT_PORT_06_PIN_00
#define PAT_CAL             IOPORT_PORT_05_PIN_05


// ADS1292 command definitions
#define ADS_CMD_WAKEUP      0x02
#define ADS_CMD_STANDBY     0x04
#define ADS_CMD_RESET       0x06
#define ADS_CMD_START       0x08
#define ADS_CMD_STOP        0x0A
#define ADS_CMD_OFFSETCAL   0x1A
#define ADS_CMD_RDATAC      0x10
#define ADS_CMD_SDATAC      0x11
#define ADS_CMD_RDATA       0x12
#define ADS_CMD_RREG        0x20
#define ADS_CMD_WREG        0x40

// ADS1292 register address
#define ADS_REG_ID          0x00
#define ADS_REG_CONFIG1     0x01
#define ADS_REG_CONFIG2     0x02
#define ADS_REG_LOFF        0x03
#define ADS_REG_CH1SET      0x04
#define ADS_REG_CH2SET      0x05
#define ADS_REG_RLD_SENS    0x06
#define ADS_REG_LOFF_SENS   0x07
#define ADS_REG_LOFF_STAT   0x08
#define ADS_REG_RESP1       0x09
#define ADS_REG_RESP2       0x0A
#define ADS_REG_GPIO        0x0B

#define ADS_BUFFER_SIZE     32              ///< Buffer size to use in the ADS transactions

#define FS_ECG              1000            ///< Sampling   frequency for ECG
#define FS_ZP               100             ///< Sampling   frequency for ZP
#define RESAMPLE_ZP         (FS_ECG/FS_ZP)  ///< Resampling frequency for ZP


#define ECG_BUFFER_SIZE     (4096*2)        ///< Buffer size to store ECG samples
#define ZP_BUFFER_SIZE      (512)           ///< Buffer size to store ZP  samples


#define ECG_BASELINE        0x00            ///< Baseline value for ECG
#define ZP_BASELINE         0x00            ///< Baseline value for ZP

#define STATUS_IS_OK(x)     ((x & 0xF01FFF) == 0xC00000)    ///< Macro to check the status bits

// Events managed by the IRQ controller ...
#define IRQ_ADS_DRDY        8           ///< Data Ready from the ADS

#define ADS1292R_IDENT      0b01110011  ///< Device identifier for the ADS1292R device

#define AUDIO_OUT_SHOW_BIOMETRICS   0   ///< To use the audio output as an oscilloscope to show biometrics



/******************************************************************************
 ** Typedefs
 */

typedef enum {
    RCP_BREAK_NONE = 0,         // no break has append
    RCP_BREAK_ZP,               // break due to ZP change
    RCP_BREAK_PATIENT_TYPE      // break due to patient type change
} RCP_BREAK_t;

typedef enum {
    NOTCH_NO = 0,               // do not apply any notch filter
    NOTCH_50,                   // Apply a 50 Hz Notch filter
    NOTCH_60                    // Apply a 60 Hz Notch filter
} NOTCH_t;

typedef enum {
    REPORT_ZP_ACDS = 0,         // report the ZP value in ADCs (for development)
    REPORT_ZP_OHMS,             // report the ZP value in ohms
    REPORT_ZP_VIRTUAL           // report the ZP virtual (for development)
} ZP_REPORT_t;

typedef enum
{
    eZP_SEGMENT_UNKNOWN,
    eZP_SEGMENT_SHORT_CIRC,
    eZP_SEGMENT_GOOD_CONN,
    eZP_SEGMENT_BAD_CONN,
    eZP_SEGMENT_OPEN_CIRC,
    eZP_SEGMENT_MAX
}ZP_SEGMENT_e;

typedef enum
{
    eEL_NONE,
    eEL_DISPOSABLE_ADULT,
    eEL_DISPOSABLE_PEDIATRIC
}EL_TYPE_ID_e;

typedef enum
{
    eEL_SIGN_NONE           = 0,  // no electrodes are detected
    eEL_SIGN_BEXEN          = 1,  // BEXEN electrodes
    eEL_SIGN_BEXEN_EXPIRED  = 2,  // expired BEXEN electrodes
    eEL_SIGN_DEMO           = 3,  // DEMO electrodes
    eEL_SIGN_MUST_DEMO      = 4,  // Must be DEMO electrodes
    eEL_SIGN_UNKNOWN        = 5   // unknown electrodes
} EL_SIGN_e;

/******************************************************************************
 ** Globals
 */

/******************************************************************************
 ** Prototypes
 */

extern  void            ADS_Read_Frame   (void);                // called from patient monitor task
extern  void            patMon_HAL_Init  (NOTCH_t myNotch);     // initialize the patient monitor HAL

extern  bool_t          patMon_Get_ECG_Window   (uint32_t nEcg_first, uint32_t nSamples, int16_t  *pSamples);
extern  bool_t          patMon_Get_ZP_Window    (uint32_t nZp_first,  uint32_t nSamples, uint32_t *pSamples);
extern  uint32_t        patMon_Get_ECG_nSample  (void);
extern  uint32_t        patMon_Get_zp_CAL_ADCs  (void);
extern  uint32_t        patMon_Get_Zp_ADCs      (void);
extern  uint32_t        patMon_Get_Zp_Ohms      (void);
extern  ZP_SEGMENT_e    patMon_Get_Zp_Segment   (void);
extern  EL_TYPE_ID_e    patMon_Get_Connected_El (void);
extern  int32_t         patMon_Get_Temperature  (void);
extern  ERROR_ID_e      patMon_Get_Status       (void);

#endif /* THREAD_PATMON_HAL_H_ */

/*
 ** $Log$
 **
 ** end of thread_patMon_hal.h
 ******************************************************************************/
