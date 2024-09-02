/******************************************************************************
 * Name      : R100_S3A7                                                      *
 * Copyright : Osatu S. Coop                                                  *
 * Compiler  : GNU ARM Embedded                                               *
 * Target    : Reanibex Serie 100                                             *
 ******************************************************************************/

/*!
 * @file        thread_defibrillator_hal.h
 * @brief       Header with functions related to the defibrillator BSP service
 *
 * @version     v1
 * @date        06/06/2018
 * @author      ltorres
 * @warning     author   date    version     .- Edition
 * @bug
 *
 */

#ifndef THREAD_DEFIBRILLATOR_HAL_H_
#define THREAD_DEFIBRILLATOR_HAL_H_

/******************************************************************************
 ** Includes
 */

#include "types_basic.h"
#include "R100_Errors.h"
#include "CU_header.h"

/******************************************************************************
 ** Defines
 */

#define     DC_LVTTL                3300            ///< Voltage at LVTTL levels (in millivolts)
#define     PRE_CHARGE_VOLTAGE       700            ///< Charge voltage (in volts) for precharge
                                                    ///< NOTE**: this value must be smaller than Zp = 15Ohms & 50J capacitor voltage
#define     MAX_CHARGE_VOLTAGE      1800            ///< Maximum charge voltage in volts
#define     MAX_CHARGE_CURRENT      5000            ///< Maximum charge current in mA

// I/O ports
#define     CHRG_ONOFF              IOPORT_PORT_05_PIN_03

#define     CHRG_PSU_EN             IOPORT_PORT_08_PIN_09
#define     DEF_PSU_EN              IOPORT_PORT_08_PIN_08
#define     IO_PSU_EN               IOPORT_PORT_06_PIN_10

#define     RELAY_CAP_EN            IOPORT_PORT_06_PIN_02
#define     RELAY_HV_SHOCK          IOPORT_PORT_01_PIN_15

#define     HV_OVC_REARM            IOPORT_PORT_06_PIN_03
#define     HV_OVC_FLAG_N           IOPORT_PORT_01_PIN_14
#define     HV_OVCH_FLAG_N          IOPORT_PORT_01_PIN_13

#define     IGBT_1                  IOPORT_PORT_04_PIN_06
#define     IGBT_2                  IOPORT_PORT_04_PIN_05
#define     IGBT_3                  IOPORT_PORT_01_PIN_07
#define     IGBT_4                  IOPORT_PORT_01_PIN_06



/******************************************************************************
 ** Typedefs
 */

///< Defibrillator states
typedef enum {
    eDEFIB_STATE_STANDBY = 0,       ///< Defibrillator in standby (capacitor discharged)
    eDEFIB_STATE_CHARGING,          ///< Defibrillator charging the main capacitor
    eDEFIB_STATE_DISCHARGING,       ///< Defibrillator discharging the main capacitor
    eDEFIB_STATE_CHARGED,           ///< Defibrillator charged to the required voltage
    eDEFIB_STATE_SHOCKING,          ///< Defibrillator shocking the patient
    eDEFIB_STATE_AFTER_SHOCK,       ///< Defibrillator in after shock
    eDEFIB_STATE_IN_ERROR,          ///< Defibrillator with an error
    eDEFIB_STATE_OUT_OF_SERVICE     ///< Defibrillator is out of service
} DEFIB_STATE_e;


typedef struct {
    uint16_t    zp;                 // Patient impedance in ohms
    uint16_t    shock_uTime_1;      // Phase 1 time (in usecs)
    uint16_t    pulse_vc_start;     // Capacitor voltage at pulse start
    uint16_t    pulse_vc_middle;    // Capacitor voltage at pulse middle
    uint16_t    pulse_vc_end;       // Capacitor voltage at pulse end
} DD_SHOCK_t;


/******************************************************************************
 ** Globals
 */

/******************************************************************************
 ** Prototypes
 */

extern  DD_SHOCK_t     *Defib_Get_Shock_DD          (void);
extern  ERROR_ID_e      Defib_Get_ErrorId           (void);
extern  DEFIB_STATE_e   Defib_Get_State             (void);
extern  uint16_t        Defib_Get_Vc                (void);
extern  uint32_t        Defib_Get_IH                (void);
extern  uint16_t        Defib_Get_Vbatt             (void);
extern  uint16_t        Defib_Get_Vdefib            (void);
extern  void            Defib_Set_Out_of_Service    (void);

extern  void            Defib_Disarm                (void);
extern  int            Defib_Charge                (uint16_t voltage, uint16_t current);
extern  int             Defib_Shock                 (uint16_t zp);
extern  void            Defib_Shock_Sync            (uint16_t zp);
extern  void            Defib_Monitor               (void);
extern  ERROR_ID_e      Defib_Initialize            (void);

extern  uint32_t        Get_Audio_Energy            (void);


#endif /* THREAD_DEFIBRILLATOR_HAL_H_ */

/*
 ** $Log$
 **
 ** end of thread_defibrillator_hal.h
 ******************************************************************************/
