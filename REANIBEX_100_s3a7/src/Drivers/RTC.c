/******************************************************************************
 * Name      : R100_S3A7                                                      *
 * Copyright : Osatu S. Coop                                                  *
 * Compiler  : GNU ARM Embedded                                               *
 * Target    : Reanibex Serie 100                                             *
 ******************************************************************************/

/*!
 * @file        RTC.c
 * @brief       All functions related to the RTC
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
#include <stdio.h>

#include "hal_data.h"
#include "types_basic.h"
#include "RTC.h"
#include "Keypad.h"

/******************************************************************************
 ** Macros
 */

/******************************************************************************
 ** Defines
 */

// define the VBATT controller address and structure
typedef struct
{
    union
    {
        __IO uint8_t BKRACR; /*!< Backup Register Access Control Register (BKRACR)      */
        struct
        {
            __IO uint8_t BKRACS :3; /*!< Backup Register Access cycle Select            */
        } BKRACR_b; /*!< BitSize                                                        */
    };
    __I uint8_t RESERVED1[856];
    union
    {
        __IO uint8_t VBTCR1; /*!< VBATT Control Register 1 (VBTCR1)                     */
        struct
        {
            __IO uint8_t BPWSWSTP :1; /*!< Battery Power Supply Switch Stop             */
        } VBTCR1_b; /*!< BitSize                                                        */
    };
    __I uint8_t RESERVED2[144];
    union
    {
        __IO uint8_t VBTCR2; /*!< VBATT Control Register 2 (VBTCR2)                     */
        struct
        {
            __IO uint8_t :4;
            __IO uint8_t VBTLVDEN  :1; /*!< VBATT Pin Low Voltage Detect Enable         */
            __IO uint8_t :1;
            __IO uint8_t VBTLVDLVL :2; /*!< VBATT Pin Low Voltage Detect Level Select   */
        } VBTCR2_b; /*!< BitSize                                                        */
    };
    union
    {
        __IO uint8_t VBTSR; /*!< VBATT Status Register (VBTSR)                          */
        struct
        {
            __IO uint8_t VBTRDF  :1; /*!< VBATT_R Reset Detect Flag                     */
            __IO uint8_t VBTBLDF :1; /*!< VBATT Battery Low Detect Flag                 */
            __IO uint8_t :2;
            __IO uint8_t VBTRVLD :1; /*!< VBATT_R Valid                                 */
        } VBTSR_b; /*!< BitSize                                                         */
    };
    union
    {
        __IO uint8_t VBTCMPCR; /*!< VBATT Comparator Control register (VBTCMPCR)        */
        struct
        {
            __IO uint8_t VBTCMPE :1; /*!< Battery Power Supply Switch Stop              */
        } VBTCMPCR_b; /*!< BitSize                                                      */
    };

    __I uint8_t RESERVED3;
    union
    {
        __IO uint8_t VBTLVDICR; /*!< VBATT Pin Low Voltage Detect Interrupt Control Register (VBTLVDICR) */
        struct
        {
            __IO uint8_t VBTLVDIE   :1; /*!< VBATT Pin Low Voltage Detect Interrupt Enable               */
            __IO uint8_t VBTLVDISEL :1; /*!< Pin Low Voltage Detect Interrupt Select                     */
        } VBTLVDICR_b; /*!< BitSize                                                                      */
    };
    __I uint8_t RESERVED4;
    union
    {
        __IO uint8_t VBTWCTLR; /*!< VBATT Wakeup Control Register (VBTWCTLR)                             */
        struct
        {
            __IO uint8_t VWEN :1; /*!< VBATT Wakeup Enable                                               */
        } VBTWCTLR_b; /*!< BitSize                                                                       */
    };
    __I uint8_t RESERVED5;
    union
    {
        __IO uint8_t VBTWCH0OTSR; /*!< VBATT Wakeup I/O 0 Output Trigger Select Register (VBTWCH0OTSR)   */
        struct
        {
            __IO uint8_t :1;
            __IO uint8_t CH0VCH1TE  :1; /*!< VBATWIO0 Output VBATWIO1 Trigger Enable                     */
            __IO uint8_t CH0VCH2TE  :1; /*!< VBATWIO0 Output VBATWIO2 Trigger Enable                     */
            __IO uint8_t CH0VRTCTE  :1; /*!< VBATWIO0 Output RTC Periodic Signal Enable                  */
            __IO uint8_t CH0VRTCATE :1; /*!< VBATWIO0 Output RTC Alarm Signal Enable                     */
            __IO uint8_t CH0VAGTUTE :1; /*!< VBATWIO0 Output AGT Underflow Signal Enable                 */
        } VBTWCH0OTSR_b; /*!< BitSize                                                                    */
    };
    union
    {
        __IO uint8_t VBTWCH1OTSR; /*!< VBATT Wakeup I/O 1 Output Trigger Select Register (VBTWCH1OTSR)   */
        struct
        {
            __IO uint8_t CH1VCH0TE  :1; /*!< VBATWIO1 Output VBATWIO0 Trigger Enable                     */
            __IO uint8_t :1;
            __IO uint8_t CH1VCH2TE  :1; /*!< VBATWIO1 Output VBATWIO2 Trigger Enable                     */
            __IO uint8_t CH1VRTCTE  :1; /*!< VBATWIO1 Output RTC Periodic Signal Enable                  */
            __IO uint8_t CH1VRTCATE :1; /*!< VBATWIO1 Output RTC Alarm Signal Enable                     */
            __IO uint8_t CH1VAGTUTE :1; /*!< VBATWIO1 Output AGT underflow Signal Enable                 */
        } VBTWCH1OTSR_b; /*!< BitSize                                                                    */
    };
    union
    {
        __IO uint8_t VBTWCH2OTSR; /*!< VBATT Wakeup I/O 2 Output Trigger Select Register (VBTWCH2OTSR)   */
        struct
        {
            __IO uint8_t CH2VCH0TE  :1; /*!< VBATWIO2 Output VBATWIO0 Trigger Enable                     */
            __IO uint8_t CH2VCH1TE  :1; /*!< VBATWIO2 Output VBATWIO1 Trigger Enable                     */
            __IO uint8_t :1;
            __IO uint8_t CH2VRTCTE  :1; /*!< VBATWIO2 Output RTC Periodic Signal Enable                  */
            __IO uint8_t CH2VRTCATE :1; /*!< VBATWIO2 Output RTC Alarm Signal Enable                     */
            __IO uint8_t CH2VAGTUTE :1; /*!< VBATWIO2 Output AGT underflow Signal Enable                 */
        } VBTWCH2OTSR_b; /*!< BitSize                                                                    */
    };
    union
    {
        __IO uint8_t VBTICTLR; /*!< VBATT Input Control Register (VBTICTLR)                 */
        struct
        {
            __IO uint8_t VCH0INEN :1; /*!< VBATT Wakeup I/O 0 Input Enable                  */
            __IO uint8_t VCH1INEN :1; /*!< VBATT Wakeup I/O 1 Input Enable                  */
            __IO uint8_t VCH2INEN :1; /*!< VBATT Wakeup I/O 2 Input Enable                  */
        } VBTICTLR_b; /*!< BitSize                                                          */
    };
    union
    {
        __IO uint8_t VBTOCTLR; /*!< VBATT Output Control Register (VBTOCTLR)                */
        struct
        {
            __IO uint8_t VCH0OEN   :1; /*!< VBATT Wakeup I/O 0 Output Enable                */
            __IO uint8_t VCH1OEN   :1; /*!< VBATT Wakeup I/O 1 Output Enable                */
            __IO uint8_t VCH2OEN   :1; /*!< VBATT Wakeup I/O 2 Output Enable                */
            __IO uint8_t VOUT0LSEL :1; /*!< VBATT Wakeup I/O 0 Output Level Selection       */
            __IO uint8_t VOUT1LSEL :1; /*!< VBATT Wakeup I/O 1 Output Level Selection       */
            __IO uint8_t VOUT2LSEL :1; /*!< VBATT Wakeup I/O 2 Output Level Selection       */
        } VBTOCTLR_b; /*!< BitSize                                                          */
    };
    union
    {
        __IO uint8_t VBTWTER; /*!< VBATT Wakeup Trigger Source Enable Register (VBTWTER)    */
        struct
        {
            __IO uint8_t VCH0E  :1; /*!< VBATWIO0 Pin Enable                                */
            __IO uint8_t VCH1E  :1; /*!< VBATWIO1 Pin Enable                                */
            __IO uint8_t VCH2E  :1; /*!< VBATWIO2 Pin Enable                                */
            __IO uint8_t VRTCIE :1; /*!< RTC Periodic Signal Enable                         */
            __IO uint8_t VRTCAE :1; /*!< RTC Alarm Signal Enable                            */
            __IO uint8_t VAGTUE :1; /*!< AGT1 underflow Signal Enable                       */
        } VBTWTER_b; /*!< BitSize                                                           */
    };
    union
    {
        __IO uint8_t VBTWEGR; /*!< VBATT Wakeup Trigger Source Edge Register (VBTWEGR)      */
        struct
        {
            __IO uint8_t VCH0EG :1; /*!< VBATWIO0 Wakeup Trigger Source Edge Select         */
            __IO uint8_t VCH1EG :1; /*!< VBATWIO1 Wakeup Trigger Source Edge Select         */
            __IO uint8_t VCH2EG :1; /*!< VBATWIO2 Wakeup Trigger Source Edge Select         */
        } VBTWEGR_b; /*!< BitSize                                                           */
    };
    union
    {
        __IO uint8_t VBTWFR; /*!< VBATT Wakeup Trigger Source Flag Register (VBTWFR)        */
        struct
        {
            __IO uint8_t VCH0F  :1; /*!< VBATWIO0 Wakeup Trigger Flag                       */
            __IO uint8_t VCH1F  :1; /*!< VBATWIO1 Wakeup Trigger Flag                       */
            __IO uint8_t VCH2F  :1; /*!< VBATWIO2 Wakeup Trigger Flag                       */
            __IO uint8_t VRTCIF :1; /*!< RTC Periodic Wakeup Trigger Flag                   */
            __IO uint8_t VRTCAF :1; /*!< RTC Alarm Wakeup Trigger Flag                      */
            __IO uint8_t VAGTUF :1; /*!< AGT1 underflow Wakeup Trigger Flag                 */
        } VBTWFR_b; /*!< BitSize                                                            */
    };
    __I uint8_t RESERVED6[64];
    union
    {
        __IO uint8_t VBTBKRn[512]; /*!< VBATT Backup Register %s                            */
        struct
        {
            __IO uint8_t VBTBKR :8; /*!< VBTBKR is a 512-byte readable/writeable register
             to store data powered by VBATT.The value of this register is retained even
             when VCC is not powered but VBATT is powered.VBTBKR is initialized
             by VBATT selected voltage power-on-reset.                                      */
        } VBTBKRn_b[512]; /*!< BitSize                                                      */
    };

} R_VBATT_Type;

#define R_VBATT_BASE                        0x4001E0C6UL
#define R_VBATT                             ((R_VBATT_Type *) R_VBATT_BASE)

//#define ACCELERATOR_TEST    (START_TIME_YEAR +  0)            // do not force a test
#define ACCELERATOR_TEST    (START_TIME_YEAR +  5)              // force a test every 5 startups !!!
//#define ACCELERATOR_TEST    (START_TIME_YEAR + 10)              // force a test every 10 startups !!!
//#define ACCELERATOR_TEST    (START_TIME_YEAR + 20)            // force a test every 20 startups !!!
//#define ACCELERATOR_TEST    (START_TIME_YEAR + 40)              // force a test every 40 startups !!!


/******************************************************************************
 ** Typedefs
 */

/******************************************************************************
 ** Constants
 */

/******************************************************************************
 ** Externals
 */

/******************************************************************************
 ** Globals
 */

/******************************************************************************
 ** Locals
 */

static uint32_t     rtc_test_counter;       // test counter
static rtc_time_t   rtc_time;               // RTC start time



/******************************************************************************
 ** Prototypes
 */

/******************************************************************************
 ** Name:    rtc_init
 *****************************************************************************/
/**
 ** @brief   Entry point to use RTC timer.
 ** @param   void
 **
 ** @return  void
 ** @todo    pending things to be done
 ******************************************************************************/
void Rtc_Init(void)
{
    // maintain the power-on active (just in case)
    // g_ioport.p_api->pinWrite (RTCIO_PIN, IOPORT_LEVEL_HIGH);
    // g_ioport.p_api->pinWrite (LED_SHOCK, IOPORT_LEVEL_LOW);

    // Initialize RTC
    iRTC.p_api->open (iRTC.p_ctrl, iRTC.p_cfg);

    iRTC.p_api->calendarCounterStop (iRTC.p_ctrl);

    // get the programmed time
    iRTC.p_api->calendarTimeGet (iRTC.p_ctrl, &rtc_time);

    // Set Calendar Time (can be used as a virtual RTC)
    rtc_time.tm_hour  = START_TIME_HR;
    rtc_time.tm_min   = START_TIME_MIN;
    rtc_time.tm_sec   = START_TIME_SEC;
    rtc_time.tm_isdst = START_TIME_ISDST;
    rtc_time.tm_mday  = START_TIME_MDAY;
    rtc_time.tm_mon   = START_TIME_MON;
    rtc_time.tm_wday  = START_TIME_WDAY;
    rtc_time.tm_yday  = START_TIME_YDAY;
//  rtc_time.tm_year  = ACCELERATOR_TEST;           // DO NOT assign the year. Use as test counter

    // get the test counter from the year register
    rtc_test_counter = (uint32_t) rtc_time.tm_year;

    // check if the test acceleration is enabled or not
    if ((rtc_test_counter > START_TIME_YEAR) && (rtc_test_counter <= ACCELERATOR_TEST))
    {
        // update the test counter and check if the test trigger is detected
        rtc_test_counter--;
        rtc_time.tm_year = (int32_t) rtc_test_counter;
        if (rtc_test_counter == START_TIME_YEAR)
        {
            rtc_time.tm_year = ACCELERATOR_TEST;
            rtc_test_counter = ACCELERATOR_TEST;
        }
    }
    else {
        rtc_time.tm_year = ACCELERATOR_TEST;
        rtc_test_counter = ACCELERATOR_TEST;
    }
    iRTC.p_api->calendarTimeSet (iRTC.p_ctrl, &rtc_time, true);

    // Start calendar counter
    iRTC.p_api->calendarCounterStart (iRTC.p_ctrl);
}

/******************************************************************************
 ** Name:    Rtc_Test_Trigger
 *****************************************************************************/
/**
 ** @brief   Trigger or validate the automatic test
 **          The "TestCounter" is used to accelerate the automatic test
 **          in order to check the test functionality
 **
 ** @param   none
 **
 ** @return  true if must proceed to test (TestCounter = 1) condition
 ******************************************************************************/
bool_t Rtc_Test_Trigger (void)
{
    return false;
    return (rtc_test_counter == ACCELERATOR_TEST);
}

/******************************************************************************
 ** Name:    Rtc_Program_Wakeup
 *****************************************************************************/
/**
 ** @brief   program the wakeup time in the internal RTC (use the alarm to
 **          activate the RTCIC0)
 **
 ** @param   msec --- wakeup time in milliseconds
 **
 ** @return  none
 ******************************************************************************/
void Rtc_Program_Wakeup(uint32_t msec)
{
    rtc_alarm_time_t    alarm_time;

    // Set Calendar Time and alarm
    alarm_time.time.tm_hour  = rtc_time.tm_hour  = START_TIME_HR;
    alarm_time.time.tm_min   = rtc_time.tm_min   = START_TIME_MIN;
    alarm_time.time.tm_sec   = rtc_time.tm_sec   = START_TIME_SEC;
    alarm_time.time.tm_isdst = rtc_time.tm_isdst = START_TIME_ISDST;
    alarm_time.time.tm_mday  = rtc_time.tm_mday  = START_TIME_MDAY;
    alarm_time.time.tm_mon   = rtc_time.tm_mon   = START_TIME_MON;
    alarm_time.time.tm_wday  = rtc_time.tm_wday  = START_TIME_WDAY;
    alarm_time.time.tm_yday  = rtc_time.tm_yday  = START_TIME_YDAY;
    alarm_time.time.tm_year  = rtc_time.tm_year  = (int32_t) rtc_test_counter;

    // Set Calendar Alarm to wake up after the requested period (in seconds)
    alarm_time.time.tm_sec += (int32_t) msec / 1000;
    alarm_time.sec_match = 1;          // alarm when seconds match !!!
    alarm_time.dayofweek_match = 0;
    alarm_time.hour_match = 0;
    alarm_time.mday_match = 0;
    alarm_time.min_match  = 0;
    alarm_time.mon_match  = 0;
    alarm_time.year_match = 0;

    iRTC.p_api->calendarTimeSet  (iRTC.p_ctrl, &rtc_time, true);
    iRTC.p_api->calendarAlarmSet (iRTC.p_ctrl, &alarm_time, true);

    // Disable Register Protection to access the VBATT registers
    // Refer to section 12 of the S3A7 Hardware User's Manual to more details
    R_BSP_RegisterProtectDisable (BSP_REG_PROTECT_OM_LPC_BATT);

    // bit to 1 if the access to this bit is for the first access after a power-on reset
    // Battery power supply switch stop and wait to make the change ...
    R_VBATT->VBTCR1_b.BPWSWSTP = 1;
    while (R_VBATT->VBTSR_b.VBTRVLD != 1) ;

    // Wakeup enable and reset detect flag set to zero
    R_VBATT->VBTWCTLR_b.VWEN = 0;
    R_VBATT->VBTSR_b.VBTRDF = 0;

    // Vbatt wakeup pin is set from 0 to 1 when trigger alarm
    R_VBATT->VBTOCTLR_b.VOUT0LSEL = 0;

    // enable the Vbatt wakeup pin in the channel 0
    R_VBATT->VBTOCTLR_b.VCH0OEN = 1;

    // program the wake-up pin to be active on RTC alarm !!!
    // select the wakeup trigger source (RTC alarm signal)
    R_VBATT->VBTWTER_b.VRTCAE = 1;

    // VBATWIO0 Output RTC Alarm Signal Enable
    R_VBATT->VBTWCH0OTSR_b.CH0VRTCATE = 1;

    // Set the VBTWCTLR.VWEN bit to 1 to activate the VBATT wakeup control function
    R_VBATT->VBTWCTLR_b.VWEN = 1;

    // Clear the VBTCR1.BPWSWSTP bit to 0 to enable the battery power supply switch
    R_VBATT->VBTCR1_b.BPWSWSTP = 0;

    // The RTC alarm signal trigger is asserted, the VBATT wakeup trigger source
    // flag of each event (VBTWFR.VRTCAF) is set to 1
    R_VBATT->VBTWFR_b.VRTCAF = 1;

    // Once configured, turn on protection.
    R_BSP_RegisterProtectEnable (BSP_REG_PROTECT_OM_LPC_BATT);
}

/******************************************************************************
 ** Name:    Rtc_Program_Kill
 *****************************************************************************/
/**
 ** @brief   program the wakeup time in the internal RTC and kill the processor
 **
 ** @param   msec --- wakeup time in milliseconds
 **
 ** @return  none
 ******************************************************************************/
void Rtc_Program_Kill (uint32_t msec)
{
    uint8_t             pin_state;      // RTCIO pin state

    Rtc_Program_Wakeup (msec);

    // wait to switch off ALL power supplies
    /*
    while (1)
    {
        g_ioport.p_api->pinRead (RTCIO_PIN, &pin_state);
        if (pin_state)
        {
            NVIC_SystemReset();
        }
    }
    */
}
