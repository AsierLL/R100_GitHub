/******************************************************************************
 * Copyright : Osatu S. Coop                                                  *
 * Compiler  : GNU ARM Embedded                                               *
 * Target    : Reanibex Serie 100                                             *
 ******************************************************************************/

/*!
 * @file        thread_sysMon_entry.c
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

#include <stdio.h>
#include <ctype.h>
#include <time.h>

#include <device_init.h>

#include "R100_Errors.h"
#include "types_basic.h"
#include "types_app.h"
#include "Trace.h"
#include "Comm.h"
#include "I2C_1.h"
#include "I2C_2.h"
#include "RTC.h"
#include "Keypad.h"
#include "DB_Test.h"
#include "DB_Episode.h"
#include "bsp_acc_LIS3DH.h"

#include "HAL/thread_defibrillator_hal.h"
#include "HAL/thread_patMon_hal.h"
#include "HAL/thread_audio_hal.h"
#include "thread_comm_entry.h"
#include "thread_patMon_entry.h"
#include "thread_sysMon_entry.h"
#include "thread_sysMon.h"
#include "thread_audio_entry.h"
#include "thread_audio.h"
#include "thread_core_entry.h"
#include "thread_core.h"
#include "thread_acc.h"
#include "thread_acc_entry.h"

#include "thread_comm.h"
#include "thread_api.h"
#include "sysMon_RF_Comms.h"
#include "sysMon_Battery.h"

#ifdef UNIT_TESTS
#include "unit_tests.h"
#endif

/******************************************************************************
 * Macros
 */

#define HEX_TO_ASCII(x)                 (uint8_t) (((x<=9) ? (x+'0') : ((x + 'A') - 10)))       ///< Hex to ascii converter

/******************************************************************************
 * Defines
 */

//#define   DEF_PSU_EN                  IOPORT_PORT_08_PIN_08 ###### defined in "thread_defibrillator_hal.h"
//#define   IO_PSU_EN                   IOPORT_PORT_06_PIN_10 ###### defined in "thread_defibrillator_hal.h"
#define     EXEC_TEST                   IOPORT_PORT_06_PIN_01

#define     LED_BLINK_OFF               0x00            ///< Set the blink led (ON_OFF led) in Off
#define     LED_BLINK_ON                0x01            ///< Set the blink led (ON_OFF led) in On

#define     FLASH_DATA_ADD              0x40100000      ///< Data flash memory address
#define     FLASH_DATA_SZ               0x0004000       ///< 16K flash memory
#define     FLASH_BLOCK_SIZE            1024            ///< 16 blocks divided into 1k each block

#define     TIME_TO_AUTOTEST            3               ///< Time to execute the automatic test in UTC 0

#define     ACCELERATOR_TIME            0               ///< Accelerate the time count for test purposes

#define     WARNING_PERIOD_SAT          1200            ///< Period to play CALL SAT (20min)

/******************************************************************************
 * Typedefs
 */

/******************************************************************************
 * Constants
 */

/******************************************************************************
 * Externals
 */

/******************************************************************************
 * Globals
 */

POWER_ON_SOURCE_e   poweron_event;                  ///< Identify the power-up source
uint32_t     thread_supervisor;                     ///< Thread supervisor
uint8_t      access_key[12];                        ///< USB asscess key
bool_t       first_time_gps = true;                 ///< Flag to transmit gps position only once
bool_t       write_elec_last_event = false;         ///< Write las electrodes registered event in test
bool_t       global_poweron_event = false;          ///< Save Power On event
ERROR_ID_e   comm_error = eERR_NONE;

//uint32_t    zp_table_adcs[35];                    ///< Compensated ADC table used to convert from ADCs to Ohms

/******************************************************************************
 * Locals
 */

static uint32_t             pon_date;                       ///< Power on date (formatted)
static uint32_t             pon_time;                       ///< Power on time (formatted) UTC 0

static ELECTRODES_DATA_t    electrodes_data;                ///< Connected electrodes information
static bool_t               electrodes_presence_flag;       ///< Electrodes presence flag
static bool_t               electrodes_pending_event;       ///< Electrodes pending event to register
static ZP_SEGMENT_e         electrodes_zp_segment;          ///< ZP segment to be processed in the electrodes context

static NV_DATA_t            nv_data;                        ///< Non volatile data first block
static NV_DATA_BLOCK_t      nv_data_block;                  ///< Non volatile data second block

static bool_t               test_mode           = false;    ///< Flag indicating the test mode activation
static bool_t               test_mode_montador  = false;    ///< Flag indicating the test mode activation for PSB test
//static bool_t               mode_demo           = false;  ///< Flag indicating the DEMO mode
static bool_t               flag_sysmon_task    = false;    ///< Flag to indicate if the task Sysmon is initialized or not
static bool_t               flag_power_off      = false;    ///< Flag to power-off device or not
static bool_t               first_time          = true;     ///< First time the electrodes are plugged
static bool_t               pon_acc_active      = false;    ///< Check whether ACC is activated or not

static uint8_t              pic_version_major, pic_version_minor;     // Mayor and minor version

/******************************************************************************
 * Prototypes
 */

// TODO place the siganture in the appropiate place
// const unsigned char __attribute__((section (".Sign_Section"))) my_signature[] = { "Reanibex_100" };
unsigned char __attribute__((section (".Sign_Section"))) my_signature[] = { "Reanibex_100" };
static uint32_t Check_Battery_Voltage (uint32_t my_vb, int8_t my_temperature, AUDIO_ID_e *pAudio_msg);

static void prvGetRegistersFromStack( uint32_t *pulFaultStackAddress )
{
    /* These are volatile to try and prevent the compiler/linker optimising them
    away as the variables never actually get used.  If the debugger won't show the
    values of the variables, make them global my moving their declaration outside
    of this function. */
    volatile uint32_t r0;
    volatile uint32_t r1;
    volatile uint32_t r2;
    volatile uint32_t r3;
    volatile uint32_t r12;
    volatile uint32_t lr; /* Link register. */
    volatile uint32_t pc; /* Program counter. */
    volatile uint32_t psr;/* Program status register. */

    UNUSED(r0);
    UNUSED(r1);
    UNUSED(r2);
    UNUSED(r3);
    UNUSED(r12);
    UNUSED(lr);
    UNUSED(pc);
    UNUSED(psr);

    r0 = pulFaultStackAddress[ 0 ];
    r1 = pulFaultStackAddress[ 1 ];
    r2 = pulFaultStackAddress[ 2 ];
    r3 = pulFaultStackAddress[ 3 ];

    r12 = pulFaultStackAddress[ 4 ];
    lr = pulFaultStackAddress[ 5 ];
    pc = pulFaultStackAddress[ 6 ];
    psr = pulFaultStackAddress[ 7 ];

    /* When the following line is hit, the variables contain the register values. */
    for( ;; );
}

void HardFault_Handler (void);
void HardFault_Handler (void)
{
    /*
    __asm volatile
        (
            " tst lr, #4                                                \n"
            " ite eq                                                    \n"
            " mrseq r0, msp                                             \n"
            " mrsne r0, psp                                             \n"
            " ldr r1, [r0, #24]                                         \n"
            " ldr r2, handler2_address_const                            \n"
            " bx r2                                                     \n"
            " handler2_address_const: .word prvGetRegistersFromStack    \n"
        );*/
    BSP_CFG_HANDLE_UNRECOVERABLE_ERROR(0);
}

/******************************************************************************
 ** Name:    Check_Test_Abort_User
 *****************************************************************************/
/**
 ** @brief   Check if user abort running test
 **
 ** @param   pResult     Pointer to test result
 **
 ** @return  True or False
 *****************************************************************************/
bool_t Check_Test_Abort_User (DB_TEST_RESULT_t *pResult)
{
    if  (Key_Read (KEY_ONOFF) == KEY_STATUS_ON) return TRUE;
    if ((Key_Read (KEY_COVER) == KEY_STATUS_ON) && (pResult->test_id == TEST_MANUAL_ID)) return TRUE;

    return FALSE;
}

/******************************************************************************
 ** Name:    Fill_Update_Reg
 *****************************************************************************/
/**
 ** @brief   Initializes the update register data string
 **
 ** @param   pStr  : pointer to a data string
 ** @param   pDate : pointer to a date struct
 ** @param   pTime : pointer to a time struct
 ** 
 ** @return  none
 *****************************************************************************/
static void Fill_Update_Reg (char_t *pStr, DEVICE_DATE_t *pDate, DEVICE_TIME_t *pTime)
{
    // Fill the string
    strcpy (pStr, "20xx-MM-DD//HH:mm:ss//xxxx");
    pStr[2] = (char_t) ((pDate->year  / 10) + '0');
    pStr[3] = (char_t) ((pDate->year  % 10) + '0');
    pStr[5] = (char_t) ((pDate->month / 10) + '0');
    pStr[6] = (char_t) ((pDate->month % 10) + '0');
    pStr[8] = (char_t) ((pDate->date  / 10) + '0');
    pStr[9] = (char_t) ((pDate->date  % 10) + '0');

    pStr[12] = (char_t) ((pTime->hour / 10) + '0');
    pStr[13] = (char_t) ((pTime->hour % 10) + '0');
    pStr[15] = (char_t) ((pTime->min  / 10) + '0');
    pStr[16] = (char_t) ((pTime->min  % 10) + '0');
    pStr[18] = (char_t) ((pTime->sec  / 10) + '0');
    pStr[19] = (char_t) ((pTime->sec  % 10) + '0');

    pStr[22] = (char_t)(((APP_REV_CODE >>24) & 0x0F) + '0');
    pStr[23] = (char_t)(((APP_REV_CODE >>16) & 0x0F) + '0');
    pStr[24] = (char_t)(((APP_REV_CODE >>8)  & 0x0F) + '0');
    pStr[25] = (char_t)((APP_REV_CODE & 0x0F) + '0');

    pStr[26] = '\r';
    pStr[27] = '\n';
}

/******************************************************************************
 ** Name:    RTC_Test_Time
 *****************************************************************************/
/**
 ** @brief   Compensate UTC time zone with TEST TIME
 **
 ** @param   none
 **
 ** @return  Autotest time (UTC applied)
 *****************************************************************************/
static uint8_t RTC_Test_Time (void)
{
    int32_t     aux;

    // Check the UTC time
    if (((int8_t) Get_Device_Settings()->misc.glo_utc_time < UTC_MIN) ||
        ((int8_t) Get_Device_Settings()->misc.glo_utc_time > UTC_MAX)) return 0;

    // Positive UTC values
    if (TIME_TO_AUTOTEST - (Get_Device_Settings()->misc.glo_utc_time) > 24)
    {
        aux = 24 - (TIME_TO_AUTOTEST - (Get_Device_Settings()->misc.glo_utc_time));
        return ((uint8_t) aux);
    }

    // Negative/positive UTC values
    if (TIME_TO_AUTOTEST - (Get_Device_Settings()->misc.glo_utc_time) >= 0)
    {
        return ((uint8_t) (TIME_TO_AUTOTEST - (Get_Device_Settings()->misc.glo_utc_time)));
    }

    // Negative UTC values
    if (TIME_TO_AUTOTEST - (Get_Device_Settings()->misc.glo_utc_time) < 0)
    {
        aux = abs(TIME_TO_AUTOTEST - (Get_Device_Settings()->misc.glo_utc_time));
        return ((uint8_t) (24 - aux));
    }

    return TIME_TO_AUTOTEST;
}

extern CU_RTC_Test_Time(){
    return RTC_Test_Time();
}

/******************************************************************************
 ** Name:    Is_SAT_Error
 *****************************************************************************/
/**
 ** @brief   Determine if an error requires a SAT service or not
 **
 ** @param   dev_error     Error code to check
 **
 ** @return  If the error is categorized as SAT or not
 *****************************************************************************/
static bool_t Is_SAT_Error (ERROR_ID_e dev_error)
{
    if (R100_Errors_cfg[dev_error].warning_error == 1)  // Critical_error
    {
        nv_data.status_led_blink = (nv_data.status_led_blink == LED_BLINK_ON) ? R100_Errors_cfg[dev_error].led_flashing : LED_BLINK_OFF;
        return true;
    }

    return false;
}
extern bool_t CU_Is_SAT_Error (ERROR_ID_e dev_error){
    return Is_SAT_Error(dev_error);
}


/******************************************************************************
 ** Name:    NV_Data_Read
 *****************************************************************************/
/**
 ** @brief   Function that gets the non volatile data from the internal Data Flash
 **
 ** @param   pNV_data to the structure to fill
 ** @param   pNV_data_block to the structure to fill
 **
 ** @return  none
 *****************************************************************************/
int NV_Data_Read (NV_DATA_t *pNV_data, NV_DATA_BLOCK_t *pNV_data_block)
{
    uint8_t     i, xor, xor_block;
    uint8_t     *pData, *pData_block;

    if(pNV_data == NULL || pNV_data_block == NULL){
        return 1;
    }

    // Assign the buffer pointer
    pData = (uint8_t *) pNV_data;
    pData_block = (uint8_t *) pNV_data_block;

    // Read the non volatile structure
    g_flash0.p_api->open (g_flash0.p_ctrl, g_flash0.p_cfg);
    g_flash0.p_api->read (g_flash0.p_ctrl, pData, FLASH_DATA_ADD, sizeof(NV_DATA_t));
    g_flash0.p_api->read (g_flash0.p_ctrl, pData_block, FLASH_DATA_ADD + FLASH_BLOCK_SIZE, sizeof(NV_DATA_BLOCK_t));
    g_flash0.p_api->close(g_flash0.p_ctrl);

    // Check the data integrity
    xor = 0;
    for (i=0; i<sizeof(NV_DATA_t); i++) xor ^= pData[i];

    xor_block = 0;
    for (i=0; i<sizeof(NV_DATA_BLOCK_t); i++) xor_block ^= pData_block[i];

    // Check some NV data
    if(pNV_data->time_warning >= (24 << 16))
    {
        pNV_data->time_warning = 0;
    }

    if ((pNV_data->must_be_0x55 != 0x55) || xor)
    {
        pNV_data->time_warning = 0;
        pNV_data->time_test    = RTC_Test_Time();
        pNV_data->test_id      = 0;
        pNV_data->test_pending = false;
        pNV_data->update_flag  = 0;
    }

    if ((pNV_data_block->must_be_0x55 != 0x55) || xor_block)
    {
        pNV_data_block->acc_pos_hvi = 4;
    }
    return SSP_SUCCESS;
}

/******************************************************************************
 ** Name:    NV_Data_Write
 *****************************************************************************/
/**
 ** @brief   Function that updates the test id and saves it to data flash memory
 **
 ** @param   pNV_data to the structure to write in the Data Flash
 **
 ** @return  none
 *****************************************************************************/
//void NV_Data_Write (NV_DATA_t *pNV_data, NV_DATA_BLOCK_t *pNV_data_block)
int NV_Data_Write (NV_DATA_t *pNV_data, NV_DATA_BLOCK_t *pNV_data_block)
{
    uint8_t     i, xor;
    uint8_t     *pData, *pData_block;
    uint32_t    nBytes;
    NV_DATA_t   my_NV_data;
    NV_DATA_BLOCK_t my_NV_data_block;

    // Assign the buffer pointer
    pData = (uint8_t *) pNV_data;
    pData_block = (uint8_t *) pNV_data_block;

    // Read the current data and if there are not changes to register, bye-bye
    NV_Data_Read (&my_NV_data, &my_NV_data_block);

    if ((memcmp ((uint8_t *) &my_NV_data, pData, sizeof(NV_DATA_t)) == 0) &&
        (memcmp ((uint8_t *) &my_NV_data_block, pData_block, sizeof(NV_DATA_BLOCK_t)) == 0)) return 4;

    if(memcmp ((uint8_t *) &my_NV_data, pData, sizeof(NV_DATA_t)) != 0)
    {
        // Set the integrity items
        pNV_data->must_be_0x55 = 0x55;
        xor = 0;
        for (i=0; i<sizeof(NV_DATA_t)-1; i++) xor ^= pData[i];
        pData[i] = xor;
    }
    if((memcmp ((uint8_t *) &my_NV_data_block, pData_block, sizeof(NV_DATA_BLOCK_t)) != 0))
    {
        pNV_data_block->must_be_0x55 = 0x55;
        xor = 0;
        for (i=0; i<sizeof(NV_DATA_BLOCK_t)-1; i++) xor ^= pData_block[i];
        pData_block[i] = xor;
    }
    // WARNING --> Be sure that the operation can be executed with the device powered
    // Power on the external circuits, adding an extra time to stabilize the power supplies ...
    g_ioport.p_api->pinWrite (IO_PSU_EN,  IOPORT_LEVEL_HIGH);
    tx_thread_sleep (OSTIME_20MSEC);

    // Write into the non volatile structure
    g_flash0.p_api->open (g_flash0.p_ctrl, g_flash0.p_cfg);
    //if(memcmp ((uint8_t *) &my_NV_data, pData, sizeof(NV_DATA_t)) != 0)
    {
        nBytes = sizeof(NV_DATA_t) + ((sizeof(NV_DATA_t) % 4) ? (4 - (sizeof(NV_DATA_t) % 4)) : 0);
        g_flash0.p_api->erase(g_flash0.p_ctrl, FLASH_DATA_ADD, nBytes/4);
        //g_flash0.p_api->blankCheck(g_flash0.p_ctrl, FLASH_DATA_ADD, nBytes, &blankCheck);
        g_flash0.p_api->write(g_flash0.p_ctrl, (uint32_t) pData, FLASH_DATA_ADD, nBytes);
    }
    //if((memcmp ((uint8_t *) &my_NV_data_block, pData_block, sizeof(NV_DATA_BLOCK_t)) != 0))
    {
        nBytes = sizeof(NV_DATA_BLOCK_t) + ((sizeof(NV_DATA_BLOCK_t) % 4) ? (4 - (sizeof(NV_DATA_BLOCK_t) % 4)) : 0);
        g_flash0.p_api->erase(g_flash0.p_ctrl, FLASH_DATA_ADD + FLASH_BLOCK_SIZE, nBytes/4);
        g_flash0.p_api->write(g_flash0.p_ctrl, (uint32_t) pData_block, FLASH_DATA_ADD + FLASH_BLOCK_SIZE, nBytes);
    }
    g_flash0.p_api->close(g_flash0.p_ctrl);

    return 5;
}

/******************************************************************************
 ** Name:    Is_Sysmon_Task_Initialized
 *****************************************************************************/
/**
 ** @brief   Is task initialized?
 **
 ** @param   none
 **
 ** @return  flag system monitor task
 *****************************************************************************/
bool_t  Is_Sysmon_Task_Initialized (void)
{
    return flag_sysmon_task;
}

/******************************************************************************
 ** Name:    Get_NV_Data
 *****************************************************************************/
/**
 ** @brief   Function that gets the non volatile data
 **
 ** @param   none
 **
 ** @return  pointer to nv_data
 *****************************************************************************/
NV_DATA_t* Get_NV_Data (void)
{
    return &nv_data;
}

/******************************************************************************
 ** Name:    Get_NV_Data_Block
 *****************************************************************************/
/**
 ** @brief   Function that gets the second block of the non volatile data
 **
 ** @param   none
 **
 ** @return  pointer to nv_data
 *****************************************************************************/
NV_DATA_BLOCK_t* Get_NV_Data_Block (void)
{
    return &nv_data_block;
}

/******************************************************************************
 ** Name:    Set_NV_Data_Error_IF_NOT_SAT
 *****************************************************************************/
/**
 ** @brief   Function that sets the non volatile data error
 **
 ** @param   error   error identifier
 **
 ** @return  void
 *****************************************************************************/
void Set_NV_Data_Error_IF_NOT_SAT (ERROR_ID_e error)
{
    if(Is_SAT_Error (nv_data.error_code) == false)
    {
        nv_data.error_code = error;
    }
}

/******************************************************************************
 ** Name:    Set_NV_Data_Error_IF_NOT_SAT_Comms
 *****************************************************************************/
/**
 ** @brief   Function that sets the non volatile data error
 **
 ** @param   error   error identifier
 **
 ** @return  void
 *****************************************************************************/
void Set_NV_Data_Error_IF_NOT_SAT_Comms (ERROR_ID_e error)
{
    if(Is_SAT_Error (nv_data.error_code) == false)
    {
        if(nv_data.error_code == eERR_NONE)
        {
            nv_data.error_code = error;
        }
    }
    comm_error = error;
}

/******************************************************************************
 ** Name:    Save_Comms_Error
 *****************************************************************************/
/**
 ** @brief   Save Communication error
 **
 ** @param   error   error identifier
 **
 ** @return  void
 *****************************************************************************/
ERROR_ID_e Save_Comms_Error(void)
{
    return comm_error;
}

/******************************************************************************
 ** Name:    Is_Test_Mode
 *****************************************************************************/
/**
 ** @brief   Function that report if the devioce is in test mode or not
 **
 ** @param   none
 **
 ** @return  test mode
 *****************************************************************************/
bool_t Is_Test_Mode (void)
{
    return test_mode;
}

/******************************************************************************
 ** Name:    Is_Test_Mode_Montador
 *****************************************************************************/
/**
 ** @brief   Function that report if the devioce is in test mode montador or not
 **
 ** @param   none
 **
 ** @return  montador mode
 *****************************************************************************/
bool_t Is_Test_Mode_Montador (void)
{
    return test_mode_montador;
}

/******************************************************************************
 ** Name:    Is_Mode_Demo
 *****************************************************************************/
/**
 ** @brief   Function that report if the device is in mode DEMO or not
 **
 ** @param   none
 **
 ** @return  DEMO mode
 *****************************************************************************/
/*bool_t Is_Mode_Demo (void)
{
    return mode_demo;
}*/

/******************************************************************************
 ** Name:    Set_Mode_Demo
 *****************************************************************************/
/**
 ** @brief   Function that report if the device is in mode DEMO or not
 **
 ** @param   aux     aux variable to set DEMO mode
 **
 ** @return  none
 *****************************************************************************/
/*void Set_Mode_Demo (bool_t aux)
{
    mode_demo = aux;
}*/

/******************************************************************************
 ** Name:    Device_pw
 *****************************************************************************/
/**
 ** @brief   Generates the device password for USB operation based on
 **          device serial number
 **
 ** @param   pPassword   generated device USB password
 ** @param   pDevice_sn  device serial number
 ** @param   size        number of bytes to process
 **
 ** @return  none
 *****************************************************************************/
void Device_pw (uint8_t *pPassword, uint8_t *pDevice_sn, uint32_t size)
{
    ATCA_STATUS status = ATCA_NOT_INITIALIZED;
    uint8_t public_key_data[96] = {0};
    uint8_t zero_buf[32] = {0};
    int pub_size = 0;
    bool is_locked_config = false;
    bool is_locked_data = false;

    uint32_t my_carry_add;
    uint8_t  my_buff[4];
    uint8_t  i,j;      // Global index

    my_buff[0] = my_buff[1] = my_buff[2] = my_buff[3] = 0;
    my_carry_add = 0;

    for (i=0; i<size; i++)
    {
        my_carry_add += pDevice_sn[i];
        my_buff[1]   ^= pDevice_sn[i];              // Xor all char in sn

        // Add all set bit in byte
        for (j=0; j<8; j++)
        {
            if (pDevice_sn[i] & (0x01<<j)) my_buff[2]++;
        }
    }

    // Add all char in sn
    my_buff[0] = (uint8_t) my_carry_add;

    // Add offset to bit set count
    my_buff[2] = (uint8_t) (my_buff[2] + (my_carry_add >> 8));

    // Calculate nibble combination
    i = (uint8_t) ( (my_buff[0] >> 4) + (my_buff[1] >> 4) + (my_buff[2] >> 4));
    j = (uint8_t) ( (my_buff[0] + my_buff[1] + my_buff[2]) << 4);
    my_buff[3] = (uint8_t) ((i & 0x0F) + (j & 0xF0));

    // Create string
    pPassword[0] = HEX_TO_ASCII ((my_buff[0] >> 4));
    pPassword[1] = HEX_TO_ASCII ((my_buff[0] & 0x0F));

    pPassword[2] = HEX_TO_ASCII ((my_buff[1] >> 4));
    pPassword[3] = HEX_TO_ASCII ((my_buff[1] & 0x0F));

    pPassword[4] = HEX_TO_ASCII ((my_buff[2] >> 4));
    pPassword[5] = HEX_TO_ASCII ((my_buff[2] & 0x0F));

    pPassword[6] = HEX_TO_ASCII ((my_buff[3] >> 4));
    pPassword[7] = HEX_TO_ASCII ((my_buff[3] & 0x0F));

    pPassword[8]  = 'D';
    pPassword[9]  = 'A';
    pPassword[10] = 'T';
    pPassword[11] = 0x00;           // NULL termination string

    status = Crypto_ATECC_Init();
    if (status == ATCA_SUCCESS)
    {
        status = atcab_is_config_locked(&is_locked_config);
        status = atcab_is_data_locked(&is_locked_data);
        if(is_locked_config == true && is_locked_data == true)
        {
            // Read public key stored in slot
            status = read_cert(1, public_key_data, &pub_size);
            if(memcmp(&public_key_data[64], zero_buf, 8) != 0)
            {
                memcpy(pPassword, &public_key_data[64], 8);
            }
        }
    }
}

/******************************************************************************
 ** Name:    Refresh_Wdg
 *****************************************************************************/
/**
 ** @brief   Refresh the watchdog timer
 **
 ** @param   none
 **
 ** @return  none
 *****************************************************************************/
inline void Refresh_Wdg (void)
{
    iWDT.p_api->refresh(iWDT.p_ctrl);
}

/******************************************************************************
 ** Name:    Pasatiempos
 *****************************************************************************/
/**
 ** @brief   Pass time doing nothing, but refreshing the watchdog !!!
 **
 ** @param   nTicks      number of ticks to stay in the hamaca
 **
 ** @return  none
 *****************************************************************************/
void Pasatiempos (uint32_t nTicks)
{
    uint32_t loop_time;

    while (nTicks)
    {
        Refresh_Wdg();
        loop_time = (nTicks > OSTIME_100MSEC) ? OSTIME_100MSEC : nTicks;
        tx_thread_sleep (loop_time);
        nTicks -= loop_time;
    }
}

/******************************************************************************
** Name:    Pasatiempos_Listening
******************************************************************************/
/**
 ** @brief   Pass time doing nothing while is listening an audio message
 **          and refreshing the watchdog !!!
 **
 ** @param   none
 **
 ** @return  none
 *****************************************************************************/
void Pasatiempos_Listening (void)
{
    do {
        Refresh_Wdg ();
        tx_thread_sleep (OSTIME_100MSEC);
    } while (Is_Audio_Playing ());
}

/******************************************************************************
 ** Name:    Save_Data_Before_Off
 *****************************************************************************/
/**
 ** @brief   Save data before power off
 **
 ** @param   ec  error code to save in NFC
 **
 ** @return  none
 ****************************************************************************/
static void Save_Data_Before_Off (ERROR_ID_e ec)
{
    // Set the test time to 3:00 am
    nv_data.time_test = RTC_Test_Time();
    Trace_Arg (TRACE_NEWLINE, "  AUTOTEST_HOUR= %4d", nv_data.time_test);
    nv_data.error_code = ec;

    // Save in NFC error code and battery and electrode updated info
    NFC_Write_Device_Info(true);

    // Wait while playing all pending audio messages ...
    // Pasatiempos_Listening ();
}

/******************************************************************************
 ** Name:    Lock_on_Panic
 *****************************************************************************/
/**
 ** @brief   Lock the device because the program can not be executed normally
 **
 ** @param   none
 **
 ** @return  none
 *****************************************************************************/
void Lock_on_Panic(uint8_t error, uint8_t num)
{
    uint32_t    i;

    for (i=1; i<10; i++)
    {
        Led_On  (LED_ONOFF);
        Led_On  (LED_PATYPE);
        Led_On  (LED_SHOCK);        tx_thread_sleep (OSTIME_100MSEC);
        Led_Off (LED_ONOFF);
        Led_Off (LED_PATYPE);
        Led_Off (LED_SHOCK);        tx_thread_sleep (OSTIME_100MSEC);

        if ((i % 3) == 0) Pasatiempos (OSTIME_500MSEC);
    }

    Trace_Arg (TRACE_NEWLINE, "SD ERROR = %d", (uint32_t)error);
    Trace_Arg (TRACE_NEWLINE, "POS = %d",  (uint32_t)num);

    // Bye-Bye
    nv_data.status_led_blink = LED_BLINK_OFF;
    R100_PowerOff();
}

/******************************************************************************
 ** Name:    Identify_PowerOn_Event
 *****************************************************************************/
/**
 ** @brief   Identifies the agent that roots the power on
 **
 ** @param   none
 **
 ** @return  source of the power on
 ****************************************************************************/
static POWER_ON_SOURCE_e Identify_PowerOn_Event(void)
{
    uint8_t pin_state;      // RTCIO pin state
    int32_t pin_mask;       // Mask of all power on agents (RTC, Cover, On-Off)

    // Check if PCB test must be done
    g_ioport.p_api->pinRead (EXEC_TEST, &pin_state);
    if (pin_state == 0)
    {
        return PON_TEST_MONTADOR;
    }

    // Powered from ON-OFF button ???
    g_ioport.p_api->pinRead (KEY_ONOFF, &pin_state);
    pin_mask  = (uint8_t) (pin_state ? 0x00 : 0x01);

    // Powered when the cover has been removed ...
    g_ioport.p_api->pinRead (KEY_COVER, &pin_state);
    pin_mask |= (uint8_t) (pin_state ? 0x02 : 0x00);

    // Powered automatically from RTC ???
    g_ioport.p_api->pinRead (RTCIO_PIN, &pin_state);
    pin_mask |= (uint8_t) (pin_state ? 0x04 : 0x00);

    // Powered when USB is connected
    g_ioport.p_api->pinRead (VBUS_DET, &pin_state);
    pin_mask |= (uint8_t) (pin_state ? 0x08 : 0x00);

    // SHOCK button pressed ???
    g_ioport.p_api->pinRead (KEY_SHOCK, &pin_state);
    pin_mask |= (uint8_t) (pin_state ? 0x00 : 0x10);

    // PAT_TYPE button pressed ???
    g_ioport.p_api->pinRead (KEY_PATYPE, &pin_state);
    pin_mask |= (uint8_t) (pin_state ? 0x00 : 0x20);

    // If powered from KEY_ONOFF, check the other keys to identify as Test request
    // PAT_TYPE pressed ===> force a manual test
    //      0x28 = mask to check buttons and VBUS_DET
    //      0x21 = when pat type and ONOFF buttons are pressed with no VBUS in the USB port

    if ((pin_mask & 0x21) == 0x21)
    {
        return PON_TEST;
        // TODO apply changes to PCBs so it does not accidentally trigger Manual Test
        return PON_BUTTON;
    }

    // If accelerometer is running but KEY_COVER is opened
    if(pon_acc_active == true && pin_mask == 0x06) return PON_COVER;

    // If the cover is open and it power on by the cover, it means that it has been power on by accelerometer and not by the cover, then ignore the opening by cover
    // DESCOMENTAR ACC if(pin_mask == 0x02 && Comm_ACC_Get_INT2()) return PON_RTC; //if(pin_mask == 0x02 && nv_data.prev_cover_status == true) return PON_RTC;

    // Analyze the mask of all power on agents (RTC, Cover, On-Off)
    switch (pin_mask & 0x0F)
    {
        // DESCOMENTAR ACC case 0b00000000: return PON_ACC;
        case 0b00000001: return PON_BUTTON;     // ONOFF pressed
        case 0b00000010: return PON_COVER;
        case 0b00000011: return PON_BUTTON;
        case 0b00000100: return PON_RTC;
        case 0b00000101: return PON_BUTTON;
        //case 0b00000110: return PON_COVER;
        case 0b00000110: return PON_RTC;
        case 0b00000111: return PON_BUTTON;
        case 0b00001000: return PON_USB;
        case 0b00001001: return PON_USB;
        case 0b00001010: return PON_USB;
        case 0b00001100: return PON_USB;
        case 0b00001011: return PON_USB;
        case 0b00001101: return PON_USB;
        case 0b00001110: return PON_USB;
        case 0b00001111: return PON_USB;
        default        : return PON_COVER;      // By default, just in case
    }
}

/******************************************************************************
 ** Name:    RTC_Normalize_Time
 *****************************************************************************/
/**
 ** @brief   Compensate UTC time zone
 **
 ** @param   pRTC     pointer to the structure to fill
 **
 ** @return  none
 *****************************************************************************/
static void RTC_Normalize_Time (BATTERY_RTC_t *pRTC)
{
    struct tm   my_time;
    struct tm  *pTime;
    time_t      utc_time;

    // Check the UTC time
    if ((pRTC->utc_time < UTC_MIN) || (pRTC->utc_time > UTC_MAX)) return;

    // Fill the "tm" structure
    my_time.tm_sec = pRTC->sec;
    my_time.tm_min = pRTC->min;
    my_time.tm_hour = pRTC->hour;
    my_time.tm_mday = pRTC->date;
    my_time.tm_mon = pRTC->month - 1;
    my_time.tm_year = pRTC->year;
    my_time.tm_wday = 0;            // Can be ignored
    my_time.tm_yday = 0;            // Can be ignored
    my_time.tm_isdst = 0;           // Use standard time

    // Correct the UTC time
    utc_time = mktime (&my_time);
    utc_time += (pRTC->utc_time * 3600);
    pTime = gmtime (&utc_time);

    // Fill the "application time" structure
    pRTC->sec   = (uint8_t) pTime->tm_sec;
    pRTC->min   = (uint8_t) pTime->tm_min;
    pRTC->hour  = (uint8_t) pTime->tm_hour;
    pRTC->date  = (uint8_t) pTime->tm_mday;
    pRTC->month = (uint8_t) (pTime->tm_mon + 1);
    pRTC->year  = (uint8_t) pTime->tm_year;
}

/******************************************************************************
 ** Name:    OneWire_Wait_While_Busy
 *****************************************************************************/
/**
 ** @brief   Wait until OneWire is free
 **
 ** @param   none
 **
 ** @return  none
 *****************************************************************************/
static void OneWire_Wait_While_Busy (void)
{
    uint8_t  status;
    uint8_t  my_buffer[2];

    my_buffer[0] = OWC_SET_REG_POINTER;
    my_buffer[1] = OWC_REG_STATUS;

    I2C2_Write (i2C_ADD_OW_MASTER, my_buffer, 2);

    // Be sure that the OneWire controller is free
    do {
        I2C2_ReadByte (i2C_ADD_OW_MASTER, &status);

        // Check the SD bit (short detected)
        if (status & 0x04)
        {
            I2C2_WriteByte (i2C_ADD_OW_MASTER, OWC_1WIRE_RESET);
            I2C2_ReadByte (i2C_ADD_OW_MASTER, &status);
        }
        Refresh_Wdg ();                             // Refresh the watchdog timer
    } while (status & 0x01);
}

/******************************************************************************
 ** Name:    Electrodes_Presence
 *****************************************************************************/
/**
 ** @brief   Try to detect the presence of electrodes
 **
 ** @param   none
 **
 ** @return  electrodes presence flag (true if are present)
 *****************************************************************************/
static bool_t Electrodes_Presence (void)
{
    uint8_t  status;

    // 1-Wire reset/presence-detect cycle (be careful --- the reset pulse needs at least 70 usecs)
    I2C2_WriteByte (i2C_ADD_OW_MASTER, OWC_1WIRE_RESET);
    I2C2_ReadByte  (i2C_ADD_OW_MASTER, &status);
    I2C2_ReadByte  (i2C_ADD_OW_MASTER, &status);
    I2C2_ReadByte  (i2C_ADD_OW_MASTER, &status);
    I2C2_ReadByte  (i2C_ADD_OW_MASTER, &status);

    // Return the presence flag (ppd-bit)
    return ((status & 0x02) != 0);
}

/******************************************************************************
 ** Name:    OneWire_Write_Byte
 *****************************************************************************/
/**
 ** @brief   Write a single byte in the OneWire device (locking function)
 **
 ** @param   command identifier
 **
 ** @return  none
 *****************************************************************************/
static void OneWire_Write_Byte (uint8_t cmd)
{
    uint8_t  my_buffer[4];

    // Be sure that the OneWire controller is free
    OneWire_Wait_While_Busy();

    // Write the command in 1-wire line
    my_buffer[0] = OWC_WRITE_BYTE;
    my_buffer[1] = cmd;
    I2C2_Write (i2C_ADD_OW_MASTER, my_buffer, 2);
}

/******************************************************************************
 ** Name:    OneWire_Read_Array
 *****************************************************************************/
/**
 ** @brief   Write a single byte in the OneWire device (locking function)
 **
 ** @param   pArray     pointer to destination array
 ** @param   nBytes     number of bytes to read
 **
 ** @return  none
 ******************************************************************************/
static void OneWire_Read_Array (uint8_t *pArray, uint8_t nBytes)
{
    uint8_t  my_buffer[4];
    uint8_t  i;

    // Read data register from DS2482
    my_buffer[0] = OWC_SET_REG_POINTER;
    my_buffer[1] = OWC_REG_READ_DATA;

    // Be sure that the OneWire controller is free
    OneWire_Wait_While_Busy();

    for (i=0; i<nBytes; i++)
    {
        I2C2_WriteByte (i2C_ADD_OW_MASTER, OWC_READ_BYTE);
        OneWire_Wait_While_Busy();
        I2C2_Write (i2C_ADD_OW_MASTER, my_buffer, 2);
        I2C2_ReadByte (i2C_ADD_OW_MASTER, &pArray[i]);
    }
}

/******************************************************************************
 ** Name:    Electrodes_Write_Scratchpad
 *****************************************************************************/
/**
 ** @brief   Writes a single scratchpad into electrodes memory
 **
 ** @param   pData     pointer to data
 ** @param   nBytes    number of bytes to write (must be up to 8)
 ** @param   address   memory address to write in
 **
 ** @return  none
 ******************************************************************************/
static void Electrodes_Write_Scratchpad (uint8_t *pData, uint8_t nBytes, uint8_t address)
{
    uint8_t  auth_pattern[4];
    uint8_t  i;

    tx_thread_sleep (OSTIME_10MSEC);

    // Reset 1-wire device
    OneWire_Wait_While_Busy();
    I2C2_WriteByte (i2C_ADD_OW_MASTER, OWC_1WIRE_RESET);  //OWC_1WIRE_RESET   OWC_DEVICE_RESET

    OneWire_Write_Byte (OWC_SKIP_ROM);          // write the skip ROM command in 1-wire line
    OneWire_Write_Byte (OW_WRITE_SCRATCHPAD);   // write scratchpad command in 1-wire line
    OneWire_Write_Byte (address);               // write TA1 address
    OneWire_Write_Byte (0x00);                  // write TA2 address
    for (i=0; i<nBytes; i++)
    {
        OneWire_Write_Byte (pData[i]);          // Write the data to scratchpad
    }

    // Read CRC
    //OneWire_Read_Array (auth_pattern, 2);

    // Reset 1-wire device
    OneWire_Wait_While_Busy();
    I2C2_WriteByte (i2C_ADD_OW_MASTER, OWC_1WIRE_RESET);

    OneWire_Write_Byte (OWC_SKIP_ROM);          // SKIP ROM command
    OneWire_Write_Byte (OW_READ_SCRATCHPAD);    // Read scratchpad command

    // Read from data register the data written in the scratchpad
    OneWire_Read_Array (auth_pattern, 3);

    // Reset 1-wire device
    OneWire_Wait_While_Busy();
    I2C2_WriteByte (i2C_ADD_OW_MASTER, OWC_1WIRE_RESET);

    OneWire_Write_Byte (OWC_SKIP_ROM);          // SKIP ROM command
    OneWire_Write_Byte (OW_COPY_SCRATCHPAD);    // Copy scratchpad command

    // Authorization code match
    for (i=0; i<3; i++)
    {
        OneWire_Write_Byte (auth_pattern[i]);
    }

    // Wait "Tprog" to complete the write operation
    tx_thread_sleep (OSTIME_20MSEC);
    OneWire_Wait_While_Busy();
    OneWire_Read_Array (auth_pattern, 1);       // Must read 0xAA as a 01 sequence
    OneWire_Wait_While_Busy();
    I2C2_WriteByte (i2C_ADD_OW_MASTER, OWC_1WIRE_RESET);
}

/******************************************************************************
 ** Name:    Electrodes_Write_Event
 *****************************************************************************/
/**
 ** @brief   Writes event into electrodes memory
 **
 ** @param   pEvent    pointer to data
 **
 ** @return  none
 ******************************************************************************/
static void Electrodes_Write_Event (ELECTRODES_EVENT_t *pEvent)
{
    uint8_t *pData;

    if (Is_Battery_Mode_Demo()) return;  // In DEMO mode do not save events

    electrodes_presence_flag = Electrodes_Presence ();
    if (electrodes_presence_flag)
    {
        pData = (uint8_t *) pEvent;
        Electrodes_Write_Scratchpad (&pData[0], 8, 0x20);       // write date & time
        Electrodes_Write_Scratchpad (&pData[8], 8, 0x28);       // write event identifier and checksum
    }
}

/******************************************************************************
 ** Name:    Electrodes_Read_Data
 *****************************************************************************/
/**
 ** @brief   Read the electrodes all data
 **
 ** @param   pEvent    pointer to data
 **
 ** @return  none
 *****************************************************************************/
static void Electrodes_Read_Data (ELECTRODES_DATA_t *pData)
{
    //uint8_t     checksum;       // Data structure checksum
    uint64_t data_error = 0x7f7f7f7f7f7f7f7f; //Electrodes incorrect data

    // Updates the presence flag !!!
    electrodes_presence_flag = Electrodes_Presence();

    Refresh_Wdg ();                             // Refresh the watchdog timer
    // Some premature function return ...
    if (!pData) { return; }
    if (!electrodes_presence_flag) { memset ((uint8_t*) pData, 0, sizeof (ELECTRODES_DATA_t)); return; }
    if (pData->sn) { return; }

    // Read the serial number
    OneWire_Write_Byte (OWC_READ_ROM);                  // Write the read ROM command in 1-wire line
    OneWire_Read_Array ((uint8_t *) &pData->sn, 8);     // Read data register from DS2482
    Refresh_Wdg ();                                     // Refresh the watchdog timer

    // Check if electrodes read data is correct
    if(memcmp((uint64_t*)&pData->sn,&data_error,8) == 0) return;

    // Updates the presence flag !!!
    electrodes_presence_flag = Electrodes_Presence();
    if (electrodes_presence_flag)
    {
        // Read the electrodes info
        OneWire_Write_Byte (OWC_SKIP_ROM);                  // Write the skip ROM command in 1-wire line
        OneWire_Write_Byte (OW_READ_MEMORY);                // Write the read command in 1-wire line
        OneWire_Write_Byte (0x00);                          // Write TA1 address
        OneWire_Write_Byte (0x00);                          // Write TA2 address
        OneWire_Read_Array ((uint8_t *) &pData->info, sizeof (ELECTRODES_INFO_t));
        Refresh_Wdg ();                                     // Refresh the watchdog timer

        // To reset the one wire
        OneWire_Wait_While_Busy();
        I2C2_WriteByte (i2C_ADD_OW_MASTER, OWC_1WIRE_RESET);
        Refresh_Wdg ();                             // Refresh the watchdog timer

        // Read the event register
        OneWire_Write_Byte (OWC_SKIP_ROM);                  // Write the skip ROM command in 1-wire line
        OneWire_Write_Byte (OW_READ_MEMORY);                // Write the read command in 1-wire line
        OneWire_Write_Byte (0x20);                          // Write TA1 address
        OneWire_Write_Byte (0x00);                          // Write TA2 address
        OneWire_Read_Array ((uint8_t *) &pData->event, sizeof (ELECTRODES_EVENT_t));
        Refresh_Wdg ();                             // Refresh the watchdog timer

        // To reset the one wire
        OneWire_Wait_While_Busy();
        I2C2_WriteByte (i2C_ADD_OW_MASTER, OWC_1WIRE_RESET);

        /*// Check the data integrity ...
        checksum = Get_Checksum_Xor ((uint8_t *) &pData->info, sizeof(ELECTRODES_INFO_t)-1);

        if (checksum != pData->info.checksum_add)
        {
            memset ((uint8_t*) pData, 0, sizeof (ELECTRODES_DATA_t));
        }*/

    }
    else {
        memset ((uint8_t*) pData, 0, sizeof (ELECTRODES_DATA_t));
    }
}

/******************************************************************************
 ** Name:    R100_Check_SAT
 *****************************************************************************/
/**
 ** @brief   Power off the device due to critical error
 **
 ** @param   dev_error         current device error
 ** @param   force_warning     force the warning message
 **
 ** @return  none
 ****************************************************************************/
static void R100_Check_SAT (ERROR_ID_e dev_error, bool_t force_warning)
{
    uint32_t        err_open;           // SD media open result ...
    uint8_t         audio_num = 0;

    // Check if the error code requires SAT
    if (Is_SAT_Error (dev_error) == FALSE)
    {
        return;
    }

    Trace_Arg (TRACE_NEWLINE, "  DEVICE ERROR= %4d", dev_error);

    // Check if led status must be blink or not
    nv_data.status_led_blink = (nv_data.status_led_blink == LED_BLINK_ON) ? R100_Errors_cfg[dev_error].led_flashing : LED_BLINK_OFF;

    nv_data.error_code = dev_error;

    // A critical error is detected --> warn the user periodically
    if (force_warning || (diff_time_seconds (pon_time, nv_data.time_warning) >= WARNING_PERIOD_SAT))
    {
        // Startup is nor RTC mode, so reconfigure clocks with PLL configuration
        bsp_clock_init_Reconfigure();

        // Register the time for the next warning
        nv_data.time_warning = pon_time;

        // Maintain the power-on switch active
        Rtc_Program_Wakeup (WAKEUP_POWERON);

        // Power on the external circuits, adding an extra time to stabilize the power supplies ...
        g_ioport.p_api->pinWrite (IO_PSU_EN,  IOPORT_LEVEL_HIGH);
        tx_thread_sleep (OSTIME_20MSEC);

        // Report the startup event
        Trace (TRACE_TIME_STAMP | TRACE_NEWLINE, "Powering Off --> Critical Error");

        // Load Device Settings from NFC (the device info is read to be available in configuration mode)
        Settings_Open_From_NFC ();

        // Load Device Info --> NFC data MUST be read before !!!
        //Device_Init (NULL);
        // Get the device info
        //memcpy(&my_Device_Info, Get_Device_Info(), sizeof(DEVICE_INFO_t));
        
        Battery_I2C_Init(&pon_date, &pon_time);

        Inc_RTCWarning();

        // Resume task used for test ...
        tx_thread_resume (&thread_audio);
        //tx_thread_resume (&thread_comm);

        // Open the uSD
        err_open = fx_media_init0_open ();
        if ((err_open != FX_SUCCESS) && (err_open != FX_PTR_ERROR))
        {
            Lock_on_Panic ((uint8_t)err_open, 10);
        }
        tx_thread_sleep (OSTIME_100MSEC);

        // Load Audio resources from SD-Card and proceed to play the SAT message
        if (err_open = Load_Audio_Resources(&audio_num), err_open != eERR_NONE)
        {
            Lock_on_Panic ((uint8_t)err_open, 11);
        }
        else
        {
            tx_thread_sleep (OSTIME_100MSEC);
            Audio_Message (eAUDIO_CMD_PLAY, eAUDIO_CALL_SAT, TRUE);
            //Pasatiempos_Listening ();
        }
    }

    // Bye-Bye
    R100_PowerOff();
}

/******************************************************************************
 ** Name:    R100_Check_Cover
 *****************************************************************************/
/**
 ** @brief   Check if cover has been opened during an extended period of time
 **
 ** @param   none
 **
 ** @return  none
 *****************************************************************************/
static POWER_ON_SOURCE_e R100_Check_Cover (void)
{
    uint32_t    ref_timeout = 0;
    uint32_t    err_open;           // SD media open result ...
    uint8_t     pin_state;          // Cover pin state
    bool_t      flag_send_sigfox_wifi = true;
    uint8_t     audio_num = 0;

    // check if cover is open
    g_ioport.p_api->pinRead (KEY_COVER, &pin_state);
    if (!pin_state)
    {
        // cover is closed
        nv_data.open_cover_first = true;

        //Check device status
        if (nv_data.prev_cover_status)
        {
            // maintain the power-on switch active
            Rtc_Program_Wakeup (WAKEUP_POWERON);

            // Power on the external circuits, adding an extra time to stabilize the power supplies ...
            g_ioport.p_api->pinWrite (IO_PSU_EN, IOPORT_LEVEL_HIGH);
            tx_thread_sleep (OSTIME_20MSEC);

            // Startup is nor RTC mode, so reconfigure clocks with PLL configuration
            bsp_clock_init_Reconfigure();
            Battery_I2C_Init(&pon_date, &pon_time);

            if (Is_SAT_Error (nv_data.error_code) == false ) Check_Device_Led_Status();

            Refresh_Wdg ();
            // Load Device Settings from NFC (the device info is read to be available in configuration mode)
            Settings_Open_From_NFC ();
            Refresh_Wdg ();

            // Load Device Info --> NFC data MUST be read before !!!
            Device_Init (NULL);

            // resume task used for alerting ...
            tx_thread_resume (&thread_comm);

            // open the uSD
            err_open = fx_media_init0_open ();
            if ((err_open != FX_SUCCESS) && (err_open != FX_PTR_ERROR))
            {
                Lock_on_Panic ((uint8_t)err_open, 12);
            }
            tx_thread_sleep (OSTIME_100MSEC);

            // Check if must send by sigfox
            if ((Is_Sigfox_TX_Enabled() == SIGFOX_ONLY || Is_Sigfox_TX_Enabled() == SIGFOX_PRIOR) && nv_data.open_cover_tx_flag == 1)
            {
                nv_data.open_cover_tx_flag = 0;
                Inc_SigfoxCover();

                Refresh_Wdg ();
                Send_Sigfox_Alert(MSG_ID_COVER_CLOSE_ALERT);    // Generate and Send Test report using Sigfox

                ref_timeout = tx_time_get() + OSTIME_20SEC;
                while((tx_time_get() <= ref_timeout) && Comm_Is_Sigfox_Alert_Sended() == FALSE)
                {
                    poweron_event = Identify_PowerOn_Event();
                    if(poweron_event == PON_BUTTON || poweron_event == PON_COVER || poweron_event == PON_TEST || poweron_event == PON_USB)
                    {
                        nv_data.prev_cover_status = false;          // cover close
                        return poweron_event;
                    }
                    Pasatiempos(OSTIME_100MSEC);
                }
            }

            // Check if must send by wifi
            if((Is_Wifi_TX_Enabled() == WIFI_ONLY || Is_Wifi_TX_Enabled() == WIFI_PRIOR) && nv_data.open_cover_tx_flag == 1) //WIFI_WITH_PATIENT
            {
                nv_data.open_cover_tx_flag = 0;

                Refresh_Wdg ();
                Send_Wifi_Alert(WIFI_MSG_ID_COVER_CLOSE_ALERT);     // Generate and Send Alert report using Wifi

                ref_timeout = tx_time_get() + OSTIME_60SEC*3;
                while((tx_time_get() <= ref_timeout))
                {
                    if((Is_Wifi_TX_Enabled() == WIFI_ONLY && Comm_Is_Wifi_Initialized() == FALSE && Comm_Is_Wifi_Init_Finished() == TRUE) ||
                       (Is_Wifi_TX_Enabled() == WIFI_ONLY && Comm_Is_Wifi_Alert_Frame_Sended()) || 
                       (Is_Wifi_TX_Enabled() == WIFI_PRIOR && Comm_Is_Wifi_Alert_Frame_Sended() && Comm_Is_Sigfox_Alert_Sended()))
                    {
                        break;
                    }
                    poweron_event = Identify_PowerOn_Event();
                    if(poweron_event == PON_BUTTON || poweron_event == PON_COVER || poweron_event == PON_TEST || poweron_event == PON_USB)
                    {
                        nv_data.prev_cover_status = false;          // cover close
                        return poweron_event;
                    }
                    Pasatiempos(OSTIME_100MSEC);
                }
            }

            nv_data.prev_cover_status = false;          // cover close
            R100_PowerOff();
        }
        return PON_RTC;
    }
    else
    {
        nv_data.prev_cover_status = true;          // cover open
    }

    // do not blink the status LED (periodic blink) while the cover is opened
    nv_data.status_led_blink = LED_BLINK_OFF;

    if (nv_data.open_cover_first == true) flag_send_sigfox_wifi = false; // In this case do not send sigfox message

    // open cover detected
    if ((nv_data.open_cover_first == true)        ||
        (diff_time_seconds (pon_time, nv_data.open_cover) >= WARNING_PERIOD_SAT))
    {
        // Startup is nor RTC mode, so reconfigure clocks with PLL configuration
        bsp_clock_init_Reconfigure();

        // register the time for the next warning
        nv_data.open_cover = pon_time;

        nv_data.open_cover_first = false;

        // maintain the power-on switch active
        Rtc_Program_Wakeup (WAKEUP_POWERON);

        // Power on the external circuits, adding an extra time to stabilize the power supplies ...
        g_ioport.p_api->pinWrite (IO_PSU_EN, IOPORT_LEVEL_HIGH);
        tx_thread_sleep (OSTIME_20MSEC);

        Refresh_Wdg ();
        // Load Device Settings from NFC (the device info is read to be available in configuration mode)
        Settings_Open_From_NFC ();
        Refresh_Wdg ();

        // Load Device Info --> NFC data MUST be read before !!!
        Device_Init (NULL);

        Battery_I2C_Init(&pon_date, &pon_time);

        Inc_OpenCover();

        // resume task used for alerting ...
        tx_thread_resume (&thread_audio);
        tx_thread_resume (&thread_comm);

        // open the uSD
        err_open = fx_media_init0_open ();
        if ((err_open != FX_SUCCESS) && (err_open != FX_PTR_ERROR))
        {
            Lock_on_Panic ((uint8_t)err_open, 12);
        }
        tx_thread_sleep (OSTIME_100MSEC);

        // Load Audio resources from SD-Card and proceed to play the SAT message
        if (err_open = Load_Audio_Resources(&audio_num), err_open != eERR_NONE)
        {
            Lock_on_Panic ((uint8_t)err_open, 13);
        }
        else
        {
            tx_thread_sleep (OSTIME_100MSEC);
            Audio_Message (eAUDIO_CMD_PLAY, eAUDIO_OPEN_COVER, TRUE);
            Pasatiempos_Listening ();
        }

        R100_Check_SAT(Get_NFC_Device_Info()->error_code,true);
        R100_Check_SAT(nv_data.error_code,true);

        // Check if must send by sigfox
        if (Is_Sigfox_TX_Enabled() == SIGFOX_ONLY || Is_Sigfox_TX_Enabled() == SIGFOX_PRIOR)
        {
            if((nv_data.open_cover_tx_flag == 0) && (flag_send_sigfox_wifi))
            {
                Refresh_Wdg ();
                Send_Sigfox_Alert(MSG_ID_COVER_OPEN_ALERT);     // Generate and Send Test report using Sigfox
                Refresh_Wdg ();

                nv_data.open_cover_tx_flag = 1;
                Inc_SigfoxCover();

                //wait till sigfox message is sent
                ref_timeout = tx_time_get() + OSTIME_20SEC;
                while((tx_time_get() <= ref_timeout) && Comm_Is_Sigfox_Alert_Sended() == FALSE)
                {
                    poweron_event = Identify_PowerOn_Event();
                    if(poweron_event == PON_BUTTON || poweron_event == PON_COVER || poweron_event == PON_TEST || poweron_event == PON_USB)
                    {
                        return poweron_event;
                    }
                    Pasatiempos(OSTIME_100MSEC);
                }
            }
        }

        // Check if must send by wifi
        if (Is_Wifi_TX_Enabled() == WIFI_ONLY || Is_Wifi_TX_Enabled() == WIFI_PRIOR) //WIFI_WITH_PATIENT
        {
            if((nv_data.open_cover_tx_flag == 0) && (flag_send_sigfox_wifi))
            {
                Refresh_Wdg ();
                Send_Wifi_Alert(WIFI_MSG_ID_COVER_OPEN_ALERT);      // Generate and Send Alert report using Wifi
                Refresh_Wdg ();

                nv_data.open_cover_tx_flag = 1;

                //wait till wifi message is sent
                ref_timeout = tx_time_get() + OSTIME_60SEC*3;
                while((tx_time_get() <= ref_timeout))
                {
                    if((Is_Wifi_TX_Enabled() == WIFI_ONLY && Comm_Is_Wifi_Initialized() == FALSE && Comm_Is_Wifi_Init_Finished() == TRUE) ||
                       (Is_Wifi_TX_Enabled() == WIFI_ONLY && Comm_Is_Wifi_Alert_Frame_Sended()) ||
                       (Is_Wifi_TX_Enabled() == WIFI_PRIOR && Comm_Is_Wifi_Alert_Frame_Sended() && Comm_Is_Sigfox_Alert_Sended()))
                    {
                        break;
                    }
                    poweron_event = Identify_PowerOn_Event();
                    if(poweron_event == PON_BUTTON || poweron_event == PON_COVER || poweron_event == PON_TEST || poweron_event == PON_USB)
                    {
                        return poweron_event;
                    }
                    Pasatiempos(OSTIME_100MSEC);
                }
            }
        }

        // Bye-Bye
        R100_PowerOff();
    }
    return PON_RTC;
}


/******************************************************************************
 ** Name:    R100_Check_Warning
 *****************************************************************************/
/**
 ** @brief   Check if beep audio must be emitted due to warning error
 **
 ** @param   none
 **
 ** @return  none
 ******************************************************************************/
static void R100_Check_Warning (void)
{
    uint32_t        err_open;           // SD media open result ...
    uint8_t         audio_num = 0;

    // Bug4689
    if (nv_data.default_settings.warning_alert == FALSE)
    {
        return;
    }

    // Emit a beep sound if electrodes or battery is wrong
    if (diff_time_seconds (pon_time, nv_data.time_warning) >= WARNING_PERIOD_SAT)
    {
        nv_data.time_warning = pon_time;        // reset counter

        if (R100_Errors_cfg[nv_data.error_code].led_flashing == 0)
        {
            // Power on the external circuits, adding an extra time to stabilize the power supplies ...
            g_ioport.p_api->pinWrite (IO_PSU_EN, IOPORT_LEVEL_HIGH);
            tx_thread_sleep (OSTIME_20MSEC);

            Refresh_Wdg ();
            // Load Device Settings from NFC (the device info is read to be available in configuration mode)
            Settings_Open_From_NFC ();
            Refresh_Wdg ();
            // Load Device Info --> NFC data MUST be read before !!!
            /*Device_Init (NULL);
            Set_Device_Date_time();*/

            Battery_I2C_Init(&pon_date, &pon_time);

            Inc_RTCWarning();

            Battery_Write_Statistics_Comms (&battery_statistics_comms);

            // resume task used for alerting ...
            tx_thread_resume (&thread_audio);

            // open the uSD
            err_open = fx_media_init0_open ();
            if ((err_open != FX_SUCCESS) && (err_open != FX_PTR_ERROR))
            {
                Lock_on_Panic ((uint8_t)err_open, 14);
            }
            tx_thread_sleep (OSTIME_100MSEC);

            // Load Audio resources from SD-Card and proceed to play the SAT message
            if (err_open = Load_Audio_Resources(&audio_num), err_open != eERR_NONE)
            {
                Lock_on_Panic ((uint8_t)err_open, 15);
            }
            else
            {
                tx_thread_sleep (OSTIME_100MSEC);
                Audio_Message (eAUDIO_CMD_PLAY, eAUDIO_TONE, TRUE);
                Pasatiempos_Listening ();
            }
        }
    }
}

/******************************************************************************
 ** Name:    R100_Check_Update
 *****************************************************************************/
/**
 ** @brief   Check if firmware has been updated
 **
 ** @param   paux to power-on type
 **
 ** @return  none
 ******************************************************************************/
static void R100_Check_Update (POWER_ON_SOURCE_e* paux)
{
    uint8_t         my_buf[] = {"L#"};  // Life-beat command for boot processor
    uint32_t        attributes, err_open;   // SD media open result ...
    uint32_t        nBytes;
    uint32_t        my_prev_ver;
    FX_FILE         version_file;
    uint8_t         boot_msg;           // boot processor response char
    DEVICE_DATE_t   upgrade_date;
    DEVICE_TIME_t   upgrade_time;
    char_t          update_buffer[28];
    uint8_t         audio_num = 0;

    // update performed flag
    if (nv_data.update_flag)
    {
        Led_On (LED_ONOFF);
        // reset update flag
        //nv_data.update_flag = 0;

        // maintain the power-on switch active
        Rtc_Program_Wakeup (WAKEUP_POWERON);
        Refresh_Wdg ();

        // Power on the external circuits, adding an extra time to stabilize the power supplies ...
        g_ioport.p_api->pinWrite (IO_PSU_EN, IOPORT_LEVEL_HIGH);
        tx_thread_sleep (OSTIME_20MSEC);

        // send life-beat to boot processor (including NULL character)
        boot_msg = Boot_Send_Message(my_buf, 3);    // dummy transfer
        Refresh_Wdg ();
        boot_msg = Boot_Send_Message(my_buf, 3);    // test with Life-Beat
        Refresh_Wdg ();
        UNUSED(boot_msg);

        // Load Device Settings from NFC (the device info is read to be available in configuration mode)
        Settings_Open_From_NFC ();
        Refresh_Wdg ();

        // Load Device Info --> NFC data MUST be read before !!!
        Device_Init(NULL);

        // open the uSD
        err_open = fx_media_init0_open ();
        if ((err_open != FX_SUCCESS) && (err_open != FX_PTR_ERROR))
        {
            Lock_on_Panic ((uint8_t)err_open, 16);
        }
        tx_thread_sleep (OSTIME_100MSEC);

        // Check if a new configuration file is present in the uSD to update the Device Settings
        Settings_Open_From_uSD ();

        R100_Check_SAT(Get_NFC_Device_Info()->error_code,true);

        // resume task used for alerting ...
        tx_thread_resume (&thread_audio);

        // Load Audio resources from SD-Card and proceed to play audio message
        if (err_open = Load_Audio_Resources(&audio_num), err_open != eERR_NONE)
        {
            Lock_on_Panic ((uint8_t)err_open, 17);
        }

        tx_thread_sleep (OSTIME_100MSEC);
        Refresh_Wdg ();

        //Ask for update process result
        Refresh_Wdg ();
        my_buf[0]='M';
        pic_version_major = Boot_Send_Message(my_buf, 3);    // test with Life-Beat
        Refresh_Wdg ();                             // refresh the watchdog timer
        my_buf[0]='m';
        pic_version_minor = Boot_Send_Message(my_buf, 3);    // test with Life-Beat

        Trace_Arg (TRACE_NEWLINE, "PIC MAYOR VERSION = %3d", pic_version_major);
        Trace_Arg (TRACE_NEWLINE, "PIC MINOR VERSION = %3d", pic_version_minor);

        if ((pic_version_major == 0) ||
            (pic_version_minor == 0))
            {
                // old pic software
                pic_version_major = pic_version_minor= '0';
            }
        Refresh_Wdg ();

        //ONLY for PIC mayor version greater or equal to '1'
        if (pic_version_major >= '1')
        {
            my_buf[0] = 'V';
            boot_msg = Boot_Send_Message(my_buf, 3);
            Trace_Arg (TRACE_NEWLINE, "PIC ack : %1c ", boot_msg);
            Refresh_Wdg ();
        }
        else
        {
            boot_msg = 'T';
        }

        // Load and read version file on SD
        err_open = (uint8_t) fx_file_open(&sd_fx_media, &version_file, FNAME_VERSION, FX_OPEN_FOR_READ);
        if (err_open == FX_SUCCESS)
        {
            err_open = (uint8_t) fx_file_read(&version_file, (uint8_t *) &my_prev_ver, sizeof(uint32_t), &nBytes);
            err_open = (uint8_t) fx_file_close (&version_file);
            err_open = (uint8_t) fx_file_delete (&sd_fx_media, FNAME_VERSION);
            err_open = (uint8_t) fx_media_flush (&sd_fx_media);
        }
        else
        {
            nv_data.error_code = eERR_BOOT_PROCESSOR_UPDATE;
        }

        if ((APP_REV_CODE == my_prev_ver) || (err_open != FX_SUCCESS))
        {
            Trace_Arg (TRACE_NEWLINE, "Old version : %5d ", my_prev_ver);
            Trace_Arg (TRACE_NEWLINE, "New version : %5d ", APP_REV_CODE);
            while (1)
            {
                Audio_Message (eAUDIO_CMD_PLAY, eAUDIO_UPGRADE_ERROR, TRUE);
                Pasatiempos_Listening ();
                if(poweron_event = Identify_PowerOn_Event(), poweron_event == PON_BUTTON || 
                    Get_Device_Info()->develop_mode != DEVELOP_MANUFACTURE_CONTROL) break;
            }
        }

        if (boot_msg != 'T')
        {
            //Audio_Message (eAUDIO_CMD_CONCAT, eAUDIO_CALL_SAT, TRUE);
            //Pasatiempos_Listening ();
            nv_data.error_code = eERR_BOOT_PROCESSOR_UPDATE;
            // Bye-Bye
            R100_PowerOff();
        }

        // Read Time
        battery_RTC.utc_time = (int8_t) Get_Device_Settings()->misc.glo_utc_time;
        RTC_Normalize_Time (&battery_RTC);
        Set_Device_Date_time();
        // Calculate upgrade date
        Get_Date (&upgrade_date.year, &upgrade_date.month, &upgrade_date.date);
        Get_Time (&upgrade_time.hour, &upgrade_time.min,   &upgrade_time.sec);

        // Register upgrade data ONLY ONCE, in order to asjust batery remaining capacity
        err_open = (uint8_t) fx_file_attributes_read (&sd_fx_media, FNAME_DATE, (UINT *)&attributes);
        Refresh_Wdg ();
        if (err_open == FX_NOT_FOUND)
        {
            err_open = (uint8_t) fx_file_create(&sd_fx_media, FNAME_DATE);
            err_open = (uint8_t) fx_file_open(&sd_fx_media, &version_file, FNAME_DATE, FX_OPEN_FOR_WRITE);
            tx_mutex_get(&usd_mutex, TX_WAIT_FOREVER);
            err_open = (uint8_t) fx_file_write (&version_file, &upgrade_date, sizeof (DEVICE_DATE_t));
            err_open = (uint8_t) fx_media_flush (&sd_fx_media);
            err_open = (uint8_t) fx_file_close (&version_file);
            err_open = (uint8_t) fx_file_attributes_set(&sd_fx_media, FNAME_DATE, FX_HIDDEN);
            tx_mutex_put(&usd_mutex);
       }

        // Register the update date, time and app version
        err_open = (uint8_t) fx_file_attributes_read (&sd_fx_media, UPDATE_REG, (UINT *)&attributes);
        Refresh_Wdg ();
        if (err_open == FX_NOT_FOUND)
        {
            err_open = (uint8_t) fx_file_create(&sd_fx_media, UPDATE_REG);
        }

        Fill_Update_Reg(update_buffer, &upgrade_date, &upgrade_time);

        err_open = (uint8_t) fx_file_open(&sd_fx_media, &version_file, UPDATE_REG, FX_OPEN_FOR_WRITE);
        tx_mutex_get(&usd_mutex, TX_WAIT_FOREVER);
        err_open = (uint8_t) fx_file_write (&version_file, update_buffer, sizeof (update_buffer));
        err_open = (uint8_t) fx_media_flush (&sd_fx_media);
        err_open = (uint8_t) fx_file_close (&version_file);
        err_open = (uint8_t) fx_file_attributes_set(&sd_fx_media, UPDATE_REG, FX_HIDDEN);
        tx_mutex_put(&usd_mutex);

        Audio_Message (eAUDIO_CMD_CONCAT, eAUDIO_DEVICE_READY, TRUE);
        Pasatiempos_Listening ();

        *paux = PON_TEST;
    }
}

/******************************************************************************
 ** Name:    Electrodes_Get_Signature
 *****************************************************************************/
/**
 ** @brief   Get electrodes signature
 **
 ** @param   none
 **
 ** @return  electrodes signature
 ******************************************************************************/
EL_SIGN_e Electrodes_Get_Signature (void)
{
    if (electrodes_presence_flag)
    {
        if (Is_Battery_Mode_Demo())
        {
            if ((strcmp((char_t *) electrodes_data.info.name, "TRAINER") == 0) || (strcmp((char_t *) electrodes_data.info.name, "DEMO") == 0))
            {
                return eEL_SIGN_DEMO;
            }
            else return eEL_SIGN_MUST_DEMO;
        }
        else
        {
            if ((strcmp((char_t *) electrodes_data.info.name, "TRAINER") == 0) || (strcmp((char_t *) electrodes_data.info.name, "DEMO") == 0))
            {
                return eEL_SIGN_MUST_DEMO;
            }
        }

        if (electrodes_data.info.expiration_date >= pon_date)
        {
            return eEL_SIGN_BEXEN;
        }
        else if (electrodes_data.info.expiration_date)
        {
            return eEL_SIGN_BEXEN_EXPIRED;
        }
        else
        {
            return eEL_SIGN_UNKNOWN;
        }
    }
    return ((electrodes_zp_segment == eZP_SEGMENT_OPEN_CIRC) ? eEL_SIGN_NONE : eEL_SIGN_UNKNOWN);
}

extern void setElectrodes_presence_flag(bool_t mode){
    electrodes_presence_flag = mode;
}

extern void setElectrodes_zp_segment(bool_t mode){
    electrodes_zp_segment = mode;
}

extern void setElectrodes_data_Name(uint8_t* name){
    memcpy(electrodes_data.info.name, name, strlen(name)+1);
}

extern void setElectrodes_data_ExpDate(uint32_t date){
    electrodes_data.info.expiration_date = date;
}

extern void setMode_Demo(uint8_t* name){
    memcpy(battery_info.name, name, sizeof(name)+1);
}

extern void setPon_Date(uint32_t value){
    pon_date = value;
}


/******************************************************************************
 ** Name:    Electrodes_Get_Data
 *****************************************************************************/
/**
 ** @brief   Get the electrodes data (sn, info and event register)
 **
 ** @param   pData  pointer to electrodes structure
 **
 ** @return  none
 ******************************************************************************/
void Electrodes_Get_Data (ELECTRODES_DATA_t* pData)
{
    if (pData) { memcpy ((uint8_t *) pData, (uint8_t *) &electrodes_data, sizeof(ELECTRODES_DATA_t)); }
}

/******************************************************************************
 ** Name:    Electrodes_Register_Event
 *****************************************************************************/
/**
 ** @brief   Command to register electrodes event
 **
 ** @param   none
 **
 ** @return  none
 ******************************************************************************/
void Electrodes_Register_Event (void)
{
    electrodes_presence_flag = Electrodes_Presence();
    electrodes_pending_event = electrodes_presence_flag;
}

/******************************************************************************
 ** Name:    Electrodes_Register_Shock
 *****************************************************************************/
/**
 ** @brief   Command to register shock sending alert
 **
 ** @param   none
 **
 ** @return  none
 ******************************************************************************/
void Electrodes_Register_Shock (void)
{
    // In DEMO mode do not send anything
    if (Is_Battery_Mode_Demo()) return;

    // Check if must send by sigfox
    if ((!Is_Battery_Mode_Demo()) && (Is_Sigfox_TX_Enabled() == SIGFOX_ONLY || Is_Sigfox_TX_Enabled() == SIGFOX_PRIOR) && (Get_Device_Settings()->misc.glo_patient_alert == TRUE))
    {
        Send_Sigfox_Alert(MSG_ID_SHOCK_DONE_ALERT);     // Send message shock performed
    }

    // Check if must send by wifi
    if((!Is_Battery_Mode_Demo()) && (Is_Wifi_TX_Enabled() == WIFI_ONLY || Is_Wifi_TX_Enabled() == WIFI_PRIOR) && (Get_Device_Settings()->misc.glo_patient_alert == TRUE)) //WIFI_WITH_PATIENT
    {
        Send_Wifi_Alert(WIFI_MSG_ID_SHOCK_DONE_ALERT);      // Generate and Send Alert report using Wifi
    }
}

/******************************************************************************
** Name:    Execute_Test_Leds_Keys
*****************************************************************************/
/**
** @brief   Function that executes the electrode test
**
** @param   pMisc       pointer to result parameters
**
** @return  none
******************************************************************************/
static ERROR_ID_e Execute_Test_Leds_Keys (TEST_MISC_t *pMisc)
{
    char_t      rx_msg[2];
    uint32_t    cnt = 0;
    uint32_t    key = 0;

    // First check leds
    Led_Off (LED_ONOFF);
    Led_Off (LED_SHOCK);
    Led_Off (LED_PATYPE);

    tx_thread_sleep (OSTIME_100MSEC);
    rx_msg[0] = 0;

    Refresh_Wdg ();
    while (cnt < 30)
    {
        cnt++;
        Led_Off (LED_ONOFF);
        Led_Off (LED_SHOCK);
        Led_Off (LED_PATYPE);
        tx_thread_sleep (OSTIME_100MSEC);

        Comm_Uart_Send("TL0");
        Refresh_Wdg ();
        if (Comm_Uart_Wifi_Receive(rx_msg, 0, OSTIME_3SEC, 0) == eERR_NONE)
        {
            if(rx_msg[0] == 'O' && rx_msg[1] == 'A')// all leds OFF
            {
                break;
            }
            Refresh_Wdg ();
        }
    }
    if (cnt == 30)
    {
        pMisc->leds_keys = eERR_LEDS;
        return eERR_LEDS;
    }
    cnt = 0;
    Refresh_Wdg ();
    while (cnt < 30)
    {
        cnt++;
        Led_On (LED_ONOFF);
        Led_Off (LED_SHOCK);
        Led_Off (LED_PATYPE);
        tx_thread_sleep (OSTIME_100MSEC);
        Comm_Uart_Send("TL1");
        Refresh_Wdg ();
        if (Comm_Uart_Wifi_Receive(rx_msg, 0, OSTIME_3SEC, 0) == eERR_NONE)
        {
            if(rx_msg[0] == 'O' && rx_msg[1] == 'B')// led ONOFF ON, all leds OFF
            {
                Trace (TRACE_TIME_STAMP | TRACE_NEWLINE, "LED ONOFF OK");
                break;
            }
            Refresh_Wdg ();
        }
    }

    if (cnt == 30)
    {
        pMisc->leds_keys = eERR_LEDS;
        return eERR_LEDS;
    }
    cnt = 0;
    Refresh_Wdg ();
    while (cnt < 30)
    {
        cnt++;
        Led_Off (LED_ONOFF);
        Led_On (LED_SHOCK);
        Led_Off (LED_PATYPE);
        tx_thread_sleep (OSTIME_100MSEC);
        Comm_Uart_Send("TL2");
        Refresh_Wdg ();
        if (Comm_Uart_Wifi_Receive(rx_msg, 0, OSTIME_3SEC, 0) == eERR_NONE)
        {
            if(rx_msg[0] == 'O' && rx_msg[1] == 'C')// led LED_SHOCK ON, all leds OFF
            {
                Trace (TRACE_TIME_STAMP | TRACE_NEWLINE, "LED_SHOCK OK");
                break;
            }
            Refresh_Wdg ();
        }

    }
    if (cnt == 30)
    {
        pMisc->leds_keys = eERR_LEDS;
        return eERR_LEDS;
    }
    cnt = 0;
    Refresh_Wdg ();
    while (cnt < 30)
    {
        cnt++;
        Led_Off (LED_ONOFF);
        Led_Off (LED_SHOCK);
        Led_On (LED_PATYPE);
        Comm_Uart_Send("TL3");
        Refresh_Wdg ();
        tx_thread_sleep (OSTIME_100MSEC);
        if (Comm_Uart_Wifi_Receive(rx_msg, 0, OSTIME_3SEC, 0) == eERR_NONE)
        {
            if(rx_msg[0] == 'O' && rx_msg[1] == 'D')// led LED_PATYPE ON, all leds OFF
            {
                Trace (TRACE_TIME_STAMP | TRACE_NEWLINE, "LED_PATYPE OK");
                break;
            }
            Refresh_Wdg ();
        }
    }
    Led_Off (LED_ONOFF);
    Led_Off (LED_SHOCK);
    Led_Off (LED_PATYPE);
    if (cnt == 30)
    {
        pMisc->leds_keys = eERR_LEDS;
        return eERR_LEDS;
    }
/*
    // Second check keys
    Trace ((TRACE_NEWLINE),"");
    Trace_Arg (TRACE_NO_FLAGS, "  ** KEY_ONOFF = %7d", Key_Read(KEY_ONOFF));

    Trace ((TRACE_NEWLINE),"");
    Trace_Arg (TRACE_NO_FLAGS, "  ** KEY_SHOCK = %7d", Key_Read(KEY_SHOCK));

    Trace ((TRACE_NEWLINE),"");
    Trace_Arg (TRACE_NO_FLAGS, "  ** KEY_COVER = %7d", Key_Read(KEY_COVER));

    Trace ((TRACE_NEWLINE),"");
    Trace_Arg (TRACE_NO_FLAGS, "  ** KEY_PATYPE = %7d", Key_Read(KEY_PATYPE));
*/
    //return OK; // production fixture is in error, so Keys can not test
    
    key = 1;
    for (cnt = 0; cnt < 50; cnt++)
    {
        Refresh_Wdg ();
        switch (key)
        {
            case 1:
                Comm_Uart_Send("TK1");
                tx_thread_sleep (OSTIME_1SEC);

                if ((Key_Read(KEY_ONOFF) == KEY_STATUS_ON) &&
                    (Key_Read(KEY_SHOCK) == KEY_STATUS_OFF) &&
                    //(Key_Read(KEY_COVER) == KEY_STATUS_OFF) &&
                    (Key_Read(KEY_PATYPE) == KEY_STATUS_OFF))
                {
                    Trace (TRACE_TIME_STAMP | TRACE_NEWLINE, "KEY_ONOFF OK");
                    Refresh_Wdg ();
                    key = 2;
                }
                break;
            case 2:
                Comm_Uart_Send("TK2");
                tx_thread_sleep (OSTIME_1SEC);

                if ((Key_Read(KEY_ONOFF) == KEY_STATUS_OFF) &&
                    (Key_Read(KEY_SHOCK) == KEY_STATUS_ON) &&
                    //(Key_Read(KEY_COVER) == KEY_STATUS_OFF) &&
                    (Key_Read(KEY_PATYPE) == KEY_STATUS_OFF))
                {
                    Trace (TRACE_TIME_STAMP | TRACE_NEWLINE, "KEY_SHOCK OK");
                    Refresh_Wdg ();
                    key = 3;
                }
                break;
            case 3:
                Comm_Uart_Send("TK3");
                tx_thread_sleep (OSTIME_1SEC);

                if ((Key_Read(KEY_ONOFF) == KEY_STATUS_OFF) &&
                    (Key_Read(KEY_SHOCK) == KEY_STATUS_OFF) &&
                    //(Key_Read(KEY_COVER) == KEY_STATUS_OFF) &&
                    (Key_Read(KEY_PATYPE) == KEY_STATUS_ON))
                {
                    Trace (TRACE_TIME_STAMP | TRACE_NEWLINE, "KEY_PATYPE OK");
                    Refresh_Wdg ();
                    key = 4;
                }
                break;
            case 4:
                Comm_Uart_Send("TC0");
                tx_thread_sleep (OSTIME_2SEC);

                if ((Key_Read(KEY_ONOFF) == KEY_STATUS_OFF) &&
                    (Key_Read(KEY_SHOCK) == KEY_STATUS_OFF) &&
                    (Key_Read(KEY_COVER) == KEY_STATUS_ON) &&
                    (Key_Read(KEY_PATYPE) == KEY_STATUS_OFF))
                {
                    Trace (TRACE_TIME_STAMP | TRACE_NEWLINE, "COVER OK");

                    Refresh_Wdg ();
                    return OK;
                }
                break;
            default:
                break;
        }
    }
    pMisc->leds_keys = eERR_KEYS;
    return eERR_KEYS;
}

/******************************************************************************
** Name:    Execute_Test_Electrodes
*****************************************************************************/
/**
** @brief   Function that executes the electrode test
**
** @param   pElectrodes     pointer to result parameters
** @param   auto_test       if the test is automatic or forced by an user
** @param   pat_mode        if the test is done in Patient mode
**
** @return  none
******************************************************************************/
static void Execute_Test_Electrodes (TEST_ELECTRODES_t *pElectrodes, bool_t auto_test, bool_t pat_mode)
{
     uint32_t    my_zp_ohm;                  // patient impedance in electrodes in adcs

    // get data from the 1-Wire bus connected to the electrodes
    Electrodes_Read_Data (&electrodes_data);

    if (pElectrodes)
    {
        // copy the data
        pElectrodes->sn              = electrodes_data.sn;
        pElectrodes->expiration_date = electrodes_data.info.expiration_date;
        pElectrodes->event_date      = electrodes_data.event.date;
        pElectrodes->event_time      = electrodes_data.event.time;
        pElectrodes->event_id        = electrodes_data.event.id;
        pElectrodes->error_code      = 0;
    }

    if (Is_Battery_Mode_Demo()) return;

    // if electrodes are connected ...
    if (electrodes_presence_flag)
    {
        my_zp_ohm = patMon_Get_Zp_Ohms();
        // Check if there is a patient connected
        if ((my_zp_ohm < 300) && (R100_test_result.test_id != 50) && !Is_Test_Mode_Montador())
        {
            //Play a message to inform the user
            Audio_Message (eAUDIO_CMD_CONCAT, eAUDIO_DISCONNECT_PATIENT, TRUE);
        }

        // check expiration date to advice the user (audio message) !!!!
        if ((battery_info.must_be_0xAA == 0xAA) &&
            (battery_statistics.must_be_0x55 == 0x55) &&
            (electrodes_data.info.expiration_date <= pon_date))
        {
            if (pElectrodes) pElectrodes->error_code = eERR_ELECTRODE_EXPIRED;
            if (!auto_test && pat_mode) DB_Episode_Set_Event(eREG_ECG_EL_EXPIRED);
            if (!auto_test)
            {
                Audio_Message (eAUDIO_CMD_CONCAT, eAUDIO_REPLACE_ELECTRODES, TRUE);
                first_time = false;
            }
        }
        else if (electrodes_data.event.id != 0)
        {
            if (pElectrodes) pElectrodes->error_code = eERR_ELECTRODE_USED;
            if (!auto_test)
            {
                DB_Episode_Set_Event(eREG_ECG_EL_USED);
                Audio_Message (eAUDIO_CMD_CONCAT, eAUDIO_REPLACE_ELECTRODES, TRUE);
                first_time = false;
            }
        }

        if ((strcmp((char_t *) electrodes_data.info.name, "TRAINER") == 0) || (strcmp((char_t *) electrodes_data.info.name, "DEMO") == 0))
        {
            pElectrodes->error_code = eERR_ELECTRODE_WRONG;
        }
    }
    else {
        // must connect electrodes ...
        pElectrodes->error_code = eERR_ELECTRODE_NOT_CONNECTED;
        if (!auto_test && !pat_mode) Audio_Message (eAUDIO_CMD_CONCAT, eAUDIO_CONNECT_ELECTRODES_DEVICE, TRUE);
    }
}

/******************************************************************************
** Name:    Execute_Test_Voltage
*****************************************************************************/
/**
** @brief   Function that executes the voltage test
**
** @param   pVoltage     pointer to result parameters
**
** @return  none
******************************************************************************/
static void Execute_Test_Voltage (TEST_VOLTAGE_t *pVoltage)
{
    // Power Supply Enable for the defibrillator functions (charge, discharge & shock)
    g_ioport.p_api->pinWrite (DEF_PSU_EN, IOPORT_LEVEL_HIGH);
    tx_thread_sleep (OSTIME_200MSEC);

    // check voltage values
    pVoltage->dc_main = Defib_Get_Vbatt();
    if (pVoltage->dc_main < SMON_DCMAIN_MIN) { pVoltage->dc_main_error = eERR_SMON_DCMAIN_TOO_LOW;  }
    if (pVoltage->dc_main > SMON_DCMAIN_MAX) { pVoltage->dc_main_error = eERR_SMON_DCMAIN_TOO_HIGH; }

    pVoltage->dc_18v = Defib_Get_Vdefib();
    if (pVoltage->dc_18v < SMON_18V_MIN) { pVoltage->dc_18v_error = eERR_SMON_18V_TOO_LOW;  }
    if (pVoltage->dc_18v > SMON_18V_MAX) { pVoltage->dc_18v_error = eERR_SMON_18V_TOO_HIGH; }
}

/******************************************************************************
** Name:    Execute_Test_Misc
*****************************************************************************/
/**
** @brief   Function that executes the miscellaneous test
**
** @param   pMisc       pointer to result parameters
** @param   pDevice_sn  device serial number
** @param   test_id     test number
**
** @return  none
******************************************************************************/
static void Execute_Test_Misc (TEST_MISC_t *pMisc, char_t *pDevice_sn, uint8_t test_id)
{
    uint8_t         my_buf[] = {"L#"};  // Life-beat command for boot processor
    uint8_t         boot_msg;           // boot processor response char
    uint8_t         tx_data[256];

    pMisc->nfc = eERR_NONE;
    // verify that the read info from NFC is OK
    if (!isalpha((uint8_t) Get_Device_Info()->sn[0]) &&
        !isdigit((uint8_t) Get_Device_Info()->sn[0]) &&
        !Is_Test_Mode_Montador()                     &&
        (Get_Device_Info()->develop_mode != DEVELOP_MANUFACTURE_CONTROL))
    {
        pMisc->nfc = eERR_NFC_INFO;
    }

    memcpy(pDevice_sn, Get_Device_Info()->sn, sizeof(Get_Device_Info()->sn));

    // Check NV_data size
    if (sizeof(NV_DATA_t) > FLASH_DATA_SZ) pMisc->nfc = eERR_NV_SZ;

    // send life-beat to boot processor (including NULL character)
    Refresh_Wdg ();                             // refresh the watchdog timer
    boot_msg = Boot_Send_Message(my_buf, 3);    // dummy transfer
    Refresh_Wdg ();                             // refresh the watchdog timer
    boot_msg = Boot_Send_Message(my_buf, 3);    // test with Life-Beat

    // report bootloader error
    pMisc->boot = (boot_msg == 'Y' ) ? eERR_NONE : eERR_BOOT_PROCESSOR;
    boot_msg = 0;

    // execute some tests
    if ((pMisc->boot == eERR_NONE) &&
       ((test_id == TEST_MANUAL_ID) ||
        (test_id == 30)             ||
        (Is_Test_Mode_Montador())))
    {
        Refresh_Wdg ();                             // refresh the watchdog timer

        // execute SPI memory test
        memset (tx_data, 0, sizeof(tx_data));
        tx_data[0] = 'X';
        tx_data[1] = '#';
        tx_data[2] = (uint8_t) (1 >> 8);
        tx_data[3] = (uint8_t) (1);
        tx_data[4] = (uint8_t) (1 >> 8);
        tx_data[5] = (uint8_t) (1);
        memset(&tx_data[6], 0xAA, 128);     //Read 128 bytes

        boot_msg = Boot_Send_Message(tx_data, (6 + 128 + 1));

        Pasatiempos (OSTIME_2SEC);                  // Wait

        pMisc->boot = (boot_msg == 'N' ) ? eERR_NONE : eERR_BOOT_SPI_MEMORY;

        Refresh_Wdg ();                             // refresh the watchdog timer

        // check if updates erros are saved
        tx_data[0] = 'V';
        tx_data[1] = '#';
        tx_data[2] = 0;
        //boot_msg = '0';

        boot_msg = Boot_Send_Message(tx_data, 3);
        Trace_Arg (TRACE_NEWLINE, "PIC ack : %1c ", boot_msg);
        Refresh_Wdg ();

        if ((boot_msg != 'T') && (boot_msg != 0))
        {
            pMisc->boot = eERR_BOOT_PROCESSOR_UPDATE;
        }
    }

    // execute and register the audio test
    pMisc->audio = Audio_Test (eAUDIO_TONE, ((test_id == TEST_MANUAL_ID) || (test_id == 30) || Is_Test_Mode_Montador()));
    // Check if TEST MODE is enabled
    if ((test_id == TEST_INITIAL_ID) && (Is_Battery_Mode_Demo())) Audio_Message (eAUDIO_CMD_CONCAT, eAUDIO_DEMO_MODE, TRUE);
    if((test_id == TEST_INITIAL_ID)) Audio_Message(eAUDIO_CMD_CONCAT, eAUDIO_CALL_EMERGENCIES, TRUE);
    Refresh_Wdg ();                             // refresh the watchdog timer
}

/******************************************************************************
** Name:    Execute_ECG_Signal
*****************************************************************************/
/**
** @brief   Function that executes the ECG test
**
** @param   none
**
** @return  true or false if code execute correctly or not
******************************************************************************/
/*static bool Execute_ECG_Signal (void)
{
    #define N_SAMPLES   500                // 480*1000/150 = ECG_WIN_SIZE_DRD*100/150
    //#define N_SAMPLES   1000                // 480*1000/150 = ECG_WIN_SIZE_DRD*100/150
    //#define N_SAMPLES   3200                // 480*1000/150 = ECG_WIN_SIZE_DRD*100/150
    uint32_t firstSample;               // first sample in the ECG series
    uint32_t i;                         // global counter
    int16_t  ecg_raw[N_SAMPLES];             // Raw ecg, as captured
    // verify the ECG from RCAL (must be an asistole)
    // wait to stabilize the signal ...

    // Resume threads needed for calibration
    tx_thread_resume (&thread_audio);
    tx_thread_resume (&thread_patMon);

    // Connect ECG sense to ZP_CAL
    g_ioport.p_api->pinWrite (PAT_CAL, IOPORT_LEVEL_HIGH);

    // wait for patMon to stabilize zp values
    Pasatiempos (OSTIME_2SEC);

    // refresh the internal watchdog timer
    firstSample = patMon_Get_ECG_nSample();     // initialize the first sample identifier

    // wait for patMon to read samples
    Pasatiempos (OSTIME_3SEC);

    patMon_Get_ECG_Window (firstSample,N_SAMPLES, ecg_raw);

    // verify that all ECG samples (last 100 msec.) are in a window near the baseline
    for (i=0; i<(N_SAMPLES-1); i++)
    {
        Refresh_Wdg ();
        // allow a xxxuV dispersion
        if (abs (ecg_raw[i] - ecg_raw[i+1]) > (15))
        {
            Trace_Arg (TRACE_NO_FLAGS, "  ** ecg_raw = %7d", (uint32_t) abs (ecg_raw[i] - ecg_raw[i+1]));
            return false;
        }
    }

    //Check external simulated signal
    // Connect ECG sense to external patient
    g_ioport.p_api->pinWrite (PAT_CAL, IOPORT_LEVEL_LOW);

    // wait for patMon to stabilize zp values
    //Pasatiempos (OSTIME_2SEC);

    //while (1)
    //{
    //    firstSample = patMon_Get_ECG_nSample();     // initialize the first sample identifier

    //    // wait for patMon to read samples
    //    Pasatiempos (OSTIME_7SEC);

    //    patMon_Get_ECG_Window (firstSample,N_SAMPLES, ecg_raw);
    //}
    //// verify that ECG samples becomes from simulator
    //for (i=0; i<N_SAMPLES; i++)
    //{
    //    // allow a 320uV dispersion
    //    if (abs (ecg_raw[i] - 100) > (60))
    //    {
    //        return false;
    //    }
    //}
    
    return true;
}*/

/******************************************************************************
** Name:    Execute_calibration
*****************************************************************************/
/**
** @brief   Function that executes the impedance calibration of the device
**
** @param   none
**
** @return  none
******************************************************************************/
static void Execute_calibration (void)
{
    uint32_t    my_zp_ohms;                 // patient impedance in ohms
    uint32_t    my_zp_adcs;                 // patient impedance in adcs
    uint32_t    my_zp_cal;                  // patient impedance calibration in adcs
    char_t      my_string[32];              // local string

    // Resume threads needed for calibration
    //tx_thread_resume (&thread_audio);
    tx_thread_resume (&thread_patMon);

    // wait for patMon to stabilize zp values
    Pasatiempos (OSTIME_5SEC);

    if (Electrodes_Presence())
    {
        Pasatiempos (OSTIME_5SEC);
        // read calibration values
        my_zp_ohms = patMon_Get_Zp_Ohms();
        my_zp_adcs = patMon_Get_Zp_ADCs();
        my_zp_cal  = patMon_Get_zp_CAL_ADCs();
        nv_data.error_code = eERR_NONE;

        // trace some variables ...
        Trace_Arg (TRACE_NO_FLAGS, "  ** Zp = %7d", my_zp_adcs);
        Trace_Arg (TRACE_NO_FLAGS, " # %4d", my_zp_ohms);
        Trace_Arg (TRACE_NO_FLAGS, " #  Zp_cal = %7d", my_zp_cal);
        sprintf (my_string, " #  Tpcb = %2d", (int) patMon_Get_Temperature());
        Trace (TRACE_NO_FLAGS, my_string);
        Trace (TRACE_NEWLINE, " ");

        //Check zp values
        if((((my_zp_adcs < ADS_ZP_0_MIN) ||
            (my_zp_adcs > ADS_ZP_0_MAX) ||
            (my_zp_cal < ADS_CAL_MIN) ||
            (my_zp_cal > ADS_CAL_MAX) ||
            (my_zp_ohms > 4)) && Is_Test_Mode_Montador() == FALSE) ||
            (/*(my_zp_ohms > 60 || my_zp_ohms < 42 )*/ (my_zp_adcs > 5000000 || my_zp_adcs < 4000000) && Is_Test_Mode_Montador() == TRUE))
        {
            nv_data.error_code = eERR_ADS_CALIBRATION;
            if (Is_Test_Mode_Montador() == false) Audio_Message (eAUDIO_CMD_PLAY, eAUDIO_CALL_SAT, TRUE);
            return;
        }

        // Save the values into NFC info
        NFC_Write_Device_ID(true,true);

        // Check if the data is written correctly
        Settings_Open_From_NFC();                   // Read again NFC values

        if (memcmp (&read_nfc_device_id, &write_nfc_device_id, sizeof(NFC_DEVICE_ID_t)) != 0)
        {
            nv_data.error_code = eERR_NFC_INFO;
            if (Is_Test_Mode_Montador() == false) Audio_Message (eAUDIO_CMD_PLAY, eAUDIO_CALL_SAT, TRUE);
        }
        else
        {
            nv_data_block.zp_adcs_short = Get_NFC_Device_ID()->zp_adcs_short;
            nv_data_block.zp_adcs_calibration = Get_NFC_Device_ID()->zp_adcs_calibration;
            NV_Data_Write(&nv_data, &nv_data_block);    // Save the values into NV
            // play message
            if (Is_Test_Mode_Montador() == FALSE) Audio_Message (eAUDIO_CMD_PLAY, eAUDIO_DEVICE_READY, TRUE);
        }

        // Save new calibration values to continue with the test_montador
        if (Is_Test_Mode_Montador())Device_Init(NULL);
    }
    else
    {
        /*if (Is_Test_Mode_Montador() == false)*/ Audio_Message (eAUDIO_CMD_PLAY, eAUDIO_CONNECT_ELECTRODES_DEVICE, TRUE);
    }
}

extern void CU_Execute_Calibration(){
    Execute_calibration();
}

/******************************************************************************
** Name:    Execute_Test_Patmon
*****************************************************************************/
/**
** @brief   Function that executes the Patient Monitor test
**
** @param   pPatmon     pointer to result parameters
** @param   test_id     test ientifier
**
** @return  none
******************************************************************************/
static bool_t Execute_Test_Patmon (TEST_PATMON_t *pPatmon, uint8_t test_id)
{
    uint32_t    my_zp_cal;                  // patient impedance calibration in adcs
    uint32_t    my_zp_ohm;                  // patient impedance in electrodes in adcs

    Pasatiempos (OSTIME_1SEC);          // wait to stabilize ZP measurement

    if (Is_Test_Mode_Montador())        // First calibrate the device
    {
        //Comm_Uart_Send("ZP1");
        //Pasatiempos (OSTIME_1SEC);
        Execute_calibration();
        Refresh_Wdg ();
        //NFC_Write_Device_ID(true, true);      // Configure develop_mode to production mode
        //Comm_Uart_Send("ZP3");
        //Pasatiempos (OSTIME_1SEC);
    }

    my_zp_ohm = patMon_Get_Zp_Ohms();
    my_zp_cal = Get_Device_Info()->zp_adcs_calibration;

    // default error
    pPatmon->ADS_pat = eERR_NONE;

    if (nv_data.error_code == eERR_ADS_CALIBRATION)
    {
        pPatmon->ADS_pat = eERR_ADS_PAT_CONNECTED;
    }

    // Check if there is a patient connected
    if ((my_zp_ohm < 300) &&  (test_id != 50) && !Is_Test_Mode_Montador())
    {
        pPatmon->ADS_pat = eERR_ADS_PAT_CONNECTED;
        //Play a message to inform the user
        Audio_Message (eAUDIO_CMD_CONCAT, eAUDIO_DISCONNECT_PATIENT, TRUE);
    }

    if(nv_data.error_code == eERR_NFC_INFO)
    {
        pPatmon->ADS_cal = eERR_NFC_INFO;
    }
    // Check if calibration impedance is in boundary
    if (((my_zp_cal > ADS_CAL_MAX) || (my_zp_cal < ADS_CAL_MIN)) && Is_Test_Mode_Montador() == FALSE)
    {
        pPatmon->ADS_cal = eERR_ADS_CALIBRATION;
    }
    else {
        pPatmon->ADS_cal = eERR_NONE;
    }

    pPatmon->ADS_comms = patMon_Get_Status ();
    pPatmon->ADS_temperature = patMon_Get_Temperature();
/*In Bizintek fixture ECG has a lot of noise
    if (Is_Test_Mode_Montador() )     // Check ECG signal
    {
        return (Execute_ECG_Signal ());
    }
*/
    return true;
}

/******************************************************************************
** Name:    Execute_Test_Comms
*****************************************************************************/
/**
** @brief   Function that executes the Comms test
**
** @param   pComms     pointer to result parameters
** @param   force      force test
**
** @return  none
******************************************************************************/
static void Execute_Test_Comms (TEST_COMMS_t *pComms, uint8_t force)
{
    DEVICE_INFO_t   *my_pDevice_Info;          // pointer to the device info
    uint32_t ref_timeout = 0;
    uint32_t tout = 0;

    Refresh_Wdg ();                         // refresh the watchdog timer

    // get the device info
    my_pDevice_Info = Get_Device_Info();

    // check the communication with the accelerometer (connected in the i2C-2 bus)
    if (my_pDevice_Info->enable_b.accelerometer || force == 1)
    {
        Comm_ACC_Execute_HW_Test(1);

        // 5 seconds timeout for acc test
        tout = OSTIME_5SEC;
        ref_timeout = tx_time_get() + tout;
        Refresh_Wdg ();
        while((tx_time_get() <= ref_timeout) && (Comm_Is_ACC_HW_Test_Finished() == FALSE))
        {
            Refresh_Wdg ();
            tx_thread_sleep (OSTIME_50MSEC);
        }

        if (Comm_Get_ACC_HW_Test_Result() != eERR_NONE) pComms->accelerometer = eERR_COMM_ACC;
    }
    else {
        pComms->accelerometer = eERR_OPTION_DISABLED;
    }

    // check the communication with the Wifi module (connected in the SCI-2 port)
    if (my_pDevice_Info->enable_b.wifi || force == 2)
    {
        Comm_Wifi_Execute_HW_Test();

        // 15 seconds timeout for Wifi test
        tout = OSTIME_15SEC;
        ref_timeout = tx_time_get() + tout;
        Refresh_Wdg ();

        while((tx_time_get() <= ref_timeout) && (Comm_Is_Wifi_HW_Test_Finished() == FALSE))
        {
            Refresh_Wdg ();
            tx_thread_sleep (OSTIME_50MSEC);
        }

        if (Comm_Get_Wifi_HW_Test_Result() != eERR_NONE) pComms->wifi = eERR_COMM_WIFI_INIT;
    }
    else {
        pComms->wifi = eERR_OPTION_DISABLED;
    }

    // check the communication with the Sigfox module (connected in the SCI-2 port)
    if (my_pDevice_Info->enable_b.sigfox || force == 3)
    {
        Comm_Sigfox_Execute_HW_Test();

        // 15 seconds timeout for sigfox test
        tout = OSTIME_15SEC;
        ref_timeout = tx_time_get() + tout;
        Refresh_Wdg ();

        while((tx_time_get() <= ref_timeout) && (Comm_Is_Sigfox_HW_Test_Finished() == FALSE))
        {
            Refresh_Wdg ();
            tx_thread_sleep (OSTIME_50MSEC);
        }

        if (Comm_Get_Sigfox_HW_Test_Result() != eERR_NONE) pComms->sigfox = eERR_COMM_SIGFOX;
    }
    else {
        pComms->sigfox = eERR_OPTION_DISABLED;
    }

    // check the communication with the GPS module (connected in the SCI-2 port)
    if (my_pDevice_Info->enable_b.gps || force == 4)
    {
        Comm_GPS_Execute_HW_Test();

        // 15 seconds timeout for gps test
        tout = OSTIME_15SEC;
        ref_timeout = tx_time_get() + tout;
        Refresh_Wdg ();

        while((tx_time_get() <= ref_timeout) && (Comm_Is_GPS_HW_Test_Finished() == FALSE))
        {
            Refresh_Wdg ();
            tx_thread_sleep (OSTIME_50MSEC);
        }

        if (Comm_Get_GPS_HW_Test_Result() != eERR_NONE) pComms->gps = eERR_COMM_GPS;
    }
    else {
        pComms->gps = eERR_OPTION_DISABLED;
    }

    // Switch off
    Comm_Select_Uart(eMOD_NONE);
}

/******************************************************************************
** Name:    Execute_Test_GPS
*****************************************************************************/
/**
** @brief   Function that executes the GPS test
**
** @param   pGPS_String     pointer to result string
**
** @return  none
******************************************************************************/
/*static void Execute_Test_GPS (char *pGPS_String)
{
    Refresh_Wdg ();                             // refresh the watchdog timer
    strcpy (pGPS_String, "The GPS position must be represented here .......");
}*/

/******************************************************************************
** Name:    Execute_Test_Defib_Montador
*****************************************************************************/
/**
** @brief   Function that executes the Defibrillator test
**
** @param   pDefib     pointer to result parameters
**
** @return  none
**
******************************************************************************/
static void Execute_Test_Defib_Montador (TEST_DEFIB_t *pDefib)
{
#define TEST_CURRENT        4000    // current used to charge the main capacitor during test

    DEFIB_STATE_e   my_state;       // defibrillator state
    uint32_t        my_time;        // time in milliseconds
    uint32_t        time1, time2;   // tick marks
    uint16_t        vBattery;       // battery voltage
    int8_t          my_temp;        // battery temperature

    ////////////////////////////////////////////////////////////////////
    // Step 1 ---> Charge the main capacitor to the precharge value
    Refresh_Wdg ();                                     // refresh the watchdog timer
    time1 = tx_time_get();

    //If the battery voltage is smaller than the limit, do not run the test and assign the replace battery error.
    Battery_Read_Temperature(&my_temp);
    vBattery = Defib_Get_Vbatt();

    if ((vBattery < Check_Battery_Voltage (vBattery, my_temp, NULL)))
    {
        Audio_Message (eAUDIO_CMD_CONCAT, eAUDIO_REPLACE_BATTERY, TRUE);
        pDefib->error_code = eERR_BATTERY_REPLACE;
        return;
    }

    Defib_Charge(MAX_CHARGE_VOLTAGE, TEST_CURRENT);     // soft charge

    while (1)
    {
        Refresh_Wdg ();                                 // refresh the watchdog timer
        my_state = Defib_Get_State ();
        time2 = tx_time_get();
        my_time = (time2-time1) * MSECS_PER_TICK;
        if (my_state == eDEFIB_STATE_CHARGED)
        {
            pDefib->full_charging_voltage = Defib_Get_Vc();     // voltage of main capacitor
            pDefib->full_charging_time = (uint16_t) my_time;    // step time in milliseconds
            if (my_time < 1000)
            {
                Defib_Charge(0, 0);
                pDefib->error_code = eERR_DEF_CHARGING_FAST;
                Refresh_Wdg ();                                     // refresh the watchdog timer
                return;
            }
            Refresh_Wdg ();
            break;

        }
        // check the charging error (slow charge) and the charging time --> must be at least 1 second !!!
        if (my_state == eDEFIB_STATE_IN_ERROR)
        {
            Defib_Charge(0, 0);
            pDefib->error_code = eERR_DEF_CHARGING_SLOW;
            Refresh_Wdg ();                                     // refresh the watchdog timer
            return;
        }
    }

    ////////////////////////////////////////////////////////////////////
    // Step 2 ---> DisCharge through the shock
    Refresh_Wdg ();

    Defib_Shock (50); // 50 ohms

    time1 = tx_time_get() + OSTIME_5SEC;

    while(tx_time_get() <= time1)
    {
        Refresh_Wdg ();                                     // refresh the watchdog timer
        my_state = Defib_Get_State ();
        if (my_state == eDEFIB_STATE_AFTER_SHOCK)
        {
            pDefib->full_discharg_H_voltage = Defib_Get_Vc();   // voltage of main capacitor
            break;
        }

        if (my_state == eDEFIB_STATE_IN_ERROR)
        {
            Defib_Charge(0, 0);
            pDefib->error_code = eERR_DEF_SHOCK;
            Refresh_Wdg ();                                     // refresh the watchdog timer
            return;
        }
    }

    ////////////////////////////////////////////////////////////////////
    // Step 3 ---> DisCharge through the relay
    Refresh_Wdg ();                                     // refresh the watchdog timer
    time1 = tx_time_get();
    Defib_Charge(0, 0);
    Pasatiempos (OSTIME_1SEC);
    time2 = tx_time_get();
    my_time = (time2-time1) * MSECS_PER_TICK;

    pDefib->full_discharg_R_voltage = Defib_Get_Vc();       // voltage of main capacitor
    pDefib->full_discharg_R_time = (uint16_t) my_time;                 // step time in milliseconds

    // verify that at least 50 V are discharged through the relay in the monitored time ...
    if ((pDefib->full_discharg_H_voltage < pDefib->full_discharg_R_voltage) ||
       ((pDefib->full_discharg_H_voltage - pDefib->full_discharg_R_voltage) < 50))
    {
        pDefib->error_code = eERR_DEF_DISCHARGING_R;
    }

    ////////////////////////////////////////////////////////////////////
    // Step 4 ---> continue discharging to zero
    Refresh_Wdg ();                                     // refresh the watchdog timer
}

/******************************************************************************
** Name:    Execute_Test_Defib
*****************************************************************************/
/**
** @brief   Function that executes the Defibrillator test
**
** @param   pDefib     pointer to result parameters
**
** @return  none
******************************************************************************/
static void Execute_Test_Defib (TEST_DEFIB_t *pDefib)
{
    #define TEST_CURRENT        4000    // current used to charge the main capacitor during test

    DEFIB_STATE_e   my_state;       // defibrillator state
    uint32_t        my_time;        // time in milliseconds
    uint32_t        time1, time2;   // tick marks
    uint16_t        vBattery;       // battery voltage
    int8_t          my_temp;        // battery temperature

    ////////////////////////////////////////////////////////////////////
    // Step 1 ---> Charge the main capacitor to the precharge value
    Refresh_Wdg ();                                     // refresh the watchdog timer
    time1 = tx_time_get();

    //If the battery voltage is smaller than the limit, do not run the test and assign the replace battery error.
    Battery_Read_Temperature(&my_temp);
    vBattery = Defib_Get_Vbatt();

    if ((vBattery < Check_Battery_Voltage (vBattery, my_temp, NULL)))
    {
        Audio_Message (eAUDIO_CMD_CONCAT, eAUDIO_REPLACE_BATTERY, TRUE);
        pDefib->error_code = eERR_BATTERY_REPLACE;
        return;
    }

    Defib_Charge(PRE_CHARGE_VOLTAGE, TEST_CURRENT);     // soft charge

    Inc_TestCharges ();    // increment the number of test charges !!!
    while (1)
    {
        Refresh_Wdg ();                                 // refresh the watchdog timer
        my_state = Defib_Get_State ();
        time2 = tx_time_get();
        my_time = (time2-time1) * MSECS_PER_TICK;
        if (my_state == eDEFIB_STATE_CHARGED)
        {
            pDefib->full_charging_voltage = Defib_Get_Vc();     // voltage of main capacitor
            pDefib->full_charging_time = (uint16_t) my_time;    // step time in milliseconds
            if (my_time < 1000)
            {
                Defib_Charge(0, 0);
                pDefib->error_code = eERR_DEF_CHARGING_FAST;
                Refresh_Wdg ();                                     // refresh the watchdog timer
                return;
            }
            Refresh_Wdg ();
            break;

        }
        // check the charging error (slow charge) and the charging time --> must be at least 1 second !!!
        if (my_state == eDEFIB_STATE_IN_ERROR)
        {
            Defib_Charge(0, 0);
            pDefib->error_code = eERR_DEF_CHARGING_SLOW;
            Refresh_Wdg ();                                     // refresh the watchdog timer
            return;
        }
    }

    ////////////////////////////////////////////////////////////////////
    // Step 2 ---> DisCharge through the H-bridge
    Refresh_Wdg ();                                     // refresh the watchdog timer

    time1 = tx_time_get();
    Defib_Charge(380, TEST_CURRENT);

    // check the current sense circuit
    // WARNING !!! ignore this test because the voltage level to measure is too low
    // Rsense  = 40 mohms
    // current = 46 mA
    // voltage in the ADC input = 1,8 mV
/*  tx_thread_sleep (OSTIME_50MSEC);
    pDefib->full_discharg_H_current = Defib_Get_IH();
    if (pDefib->full_discharg_H_current < 10)
    {
        Defib_Charge(0, 0);
        pDefib->error_code = eERR_DEF_CURRENT_SENSE;
        Audio_Message (eAUDIO_CMD_CONCAT, eAUDIO_CALL_SAT, TRUE);
        return;
    } */

    while (1)
    {
        Refresh_Wdg ();                                     // refresh the watchdog timer
        my_state = Defib_Get_State ();
        time2 = tx_time_get();
        my_time = (time2-time1) * MSECS_PER_TICK;
        if (my_state == eDEFIB_STATE_CHARGED)
        {
            pDefib->full_discharg_H_voltage = Defib_Get_Vc();   // voltage of main capacitor
            pDefib->full_discharg_H_time = (uint16_t) my_time;  // step time in milliseconds
            break;
        }

        // check the discharging error (can not discharge)
        if (my_time > 5000)
        {
            Defib_Charge(0, 0);
            pDefib->error_code = eERR_DEF_DISCHARGING_H;
            Refresh_Wdg ();                                     // refresh the watchdog timer
            return;
        }
    }

    ////////////////////////////////////////////////////////////////////
    // Step 3 ---> DisCharge through the relay
    Refresh_Wdg ();                                     // refresh the watchdog timer
    time1 = tx_time_get();
    Defib_Charge(0, 0);
    Pasatiempos (OSTIME_1SEC);
    time2 = tx_time_get();
    my_time = (time2-time1) * MSECS_PER_TICK;

    pDefib->full_discharg_R_voltage = Defib_Get_Vc();       // voltage of main capacitor
    pDefib->full_discharg_R_time = (uint16_t) my_time;                 // step time in milliseconds

    // verify that at least 50 V are discharged through the relay in the monitored time ...
    if ((pDefib->full_discharg_H_voltage < pDefib->full_discharg_R_voltage) ||
       ((pDefib->full_discharg_H_voltage - pDefib->full_discharg_R_voltage) < 50))
    {
        pDefib->error_code = eERR_DEF_DISCHARGING_R;
    }

    ////////////////////////////////////////////////////////////////////
    // Step 4 ---> continue discharging to zero
    Refresh_Wdg ();                                     // refresh the watchdog timer
}


/******************************************************************************
** Name:    Execute_Test_Battery
*****************************************************************************/
/**
** @brief   Function that executes the battery test
**
** @param   pBattery    pointer to result parameters
** @param   auto_test   if the test is automatic or forced by an user
** @param   pat_mode    if the test is done in Patient mode
**
** @return  none
******************************************************************************/
static void Execute_Test_Battery (TEST_BATTERY_t *pBattery, bool_t auto_test, bool_t pat_mode)
{
    int8_t      my_temp;        // battery temperature
    uint16_t    my_charge;      // battery charge percent

    // fill the battery info !!!
    if ((battery_info.must_be_0xAA == 0xAA) &&
        ((memcmp (battery_info.name, "BEXENCARDIO", sizeof("BEXENCARDIO"))==0) ||
         (memcmp (battery_info.name, "BexenCardio", sizeof("BexenCardio"))==0) ||
         (memcmp (battery_info.name, "DEMO", sizeof("DEMO"))==0)) &&
        (battery_info.nominal_capacity == 4200))
    {
        memcpy (pBattery->sn,   battery_info.sn,   sizeof(battery_info.sn  ));  // Serial Number
        memcpy (pBattery->name, battery_info.name, sizeof(battery_info.name));  // company name -- (default: BexenCardio)
        pBattery->manufacture_date = battery_info.manufacture_date;             // Manufacture date (YYYY.MM.DD)
        pBattery->expiration_date  = battery_info.expiration_date;              // Expiration  date (YYYY.MM.DD)
        pBattery->nominal_capacity = battery_info.nominal_capacity;             // Battery nominal capacity (default: 4200 mAh)
        pBattery->version_major    = battery_info.version_major;                // Battery version major
        pBattery->version_minor    = battery_info.version_minor;                // Battery version minor

        /*if (pBattery->expiration_date < pon_date)
        {
            pBattery->error_code = eERR_BATTERY_EXPIRED;
            Audio_Message (eAUDIO_CMD_CONCAT, eAUDIO_REPLACE_BATTERY, TRUE);
            Pasatiempos_Listening ();
        }*/
    }
    else {
        strcpy ((char_t *) pBattery->name, "Unknown");                                     // company name -- (Unknown)
        pBattery->error_code = eERR_BATTERY_UNKNOWN;
    }
    if(Battery_I2C_Init_RTC (&pon_date, &pon_time) != SSP_SUCCESS) pBattery->error_code = eRR_RTC;

    // fill the battery statistics !!!
    if (battery_statistics.must_be_0x55 == 0x55)
    {
        pBattery->runTime_total   = battery_statistics.runTime_total;           // accumulated run time (in minutes)
        pBattery->nFull_charges   = battery_statistics.nFull_charges;           // number of full charges
        pBattery->rem_charge      = Battery_Get_Charge ();                      // compute the battery charge ...

        if (pBattery->rem_charge <= BATTERY_LOW_CHARGE)
        {
            pBattery->error_code = eERR_BATTERY_LOW;
        }
    }

    // update battery charge & temperature values
    my_charge = pBattery->rem_charge;
    Battery_Read_Temperature(&my_temp);
    pBattery->bat_temperature = (int16_t) my_temp;

    if (!auto_test)
    {
        // If the battery is not identified ....
        if ((battery_info.must_be_0xAA != 0xAA) ||
            (battery_statistics.must_be_0x55 != 0x55) ||
            pBattery->error_code == eERR_BATTERY_UNKNOWN)
        {
            if (!pat_mode)
            {
                Audio_Message (eAUDIO_CMD_CONCAT, eAUDIO_REPLACE_BATTERY, TRUE);
                Pasatiempos_Listening ();
            }
            pBattery->error_code = eERR_BATTERY_UNKNOWN;
            DB_Episode_Set_Event(eREG_BAT_UNKNOWN);
        }
        // check battery charge to advice the user (audio message) !!!!
        else
        {
            if (my_charge <= BATTERY_REPLACE_CHARGE || Defib_Get_Vbatt() <= Check_Battery_Voltage (Defib_Get_Vbatt(), my_temp, NULL))
            {
                pBattery->error_code = eERR_BATTERY_REPLACE;
                DB_Episode_Set_Event(eREG_BAT_VERY_LOW_BATTERY);
                Audio_Message (eAUDIO_CMD_CONCAT, eAUDIO_REPLACE_BATTERY, TRUE);
                Pasatiempos_Listening ();
            }
            else if (my_charge <= BATTERY_LOW_CHARGE)
            {
                pBattery->error_code = eERR_BATTERY_LOW;
                DB_Episode_Set_Event(eREG_BAT_LOW_BATTERY);
                Audio_Message (eAUDIO_CMD_CONCAT, eAUDIO_LOW_BATTERY, TRUE);
                Pasatiempos_Listening ();
            }
            if (my_temp < BATTERY_LOW_TEMP)
            {
                pBattery->error_code = eERR_BATTERY_TEMP_OUT_RANGE;
                DB_Episode_Set_Event(eREG_BAT_LOW_TEMPERATURE);
            }
            else if (my_temp > BATTERY_HIGH_TEMP)
            {
                pBattery->error_code = eERR_BATTERY_TEMP_OUT_RANGE;
                DB_Episode_Set_Event(eREG_BAT_HIGH_TEMPERATURE);
            }
        }
    }
}

/******************************************************************************
 ** Name:    Load_GOLD_File
 *****************************************************************************/
/**
 * @brief   This function use to upload Golden file.
 * 
 * @param   void
 * 
 * @return true Golden firmware uploaded
 * @return false No update found in uSD.
 ******************************************************************************/
static bool_t Load_GOLD_File(void)
{
    uint8_t  fx_res;
    uint8_t  host_ack;
    uint32_t nBytes;
    uint32_t frameId, nFrame;
    static FX_FILE  update_file;
    static uint8_t  tx_data[256];
    static uint8_t n_errors;

    memset (tx_data, 0, sizeof(tx_data));
    tx_data[0] = 'G';
    tx_data[1] = '#';

    // Load and read configuration file on SD
    fx_res = (uint8_t) fx_file_open(&sd_fx_media, &update_file, GOLDEN_FILENAME, FX_OPEN_FOR_READ);

    if (fx_res == 0)
    {
        Trace (TRACE_NEWLINE, "");
        Trace (TRACE_TIME_STAMP | TRACE_NEWLINE, "GOLDEN FILE AVAILABLE !!!");
        Trace (                   TRACE_NEWLINE, "=============================");

        // warn user about incoming firmware update
        Audio_Message (eAUDIO_CMD_PLAY, eAUDIO_UPGRADING, TRUE);

        Refresh_Wdg ();                             // refresh the watchdog timer
        Boot_Sync (3);                              // Synchronize the Boot processor
        Refresh_Wdg ();                             // refresh the watchdog timer

        // Check file length
        nFrame = (uint32_t) ((update_file.fx_file_current_file_size) / 128 );
        if ((update_file.fx_file_current_file_size) % 128 )
        {
            nFrame++;
        }
        for (frameId=1; frameId<=nFrame; frameId++)
        {
            n_errors = 0;
            tx_thread_sleep (OSTIME_10MSEC);
            //Trace_Arg (TRACE_NEWLINE, "frameId:%4d ", frameId);
            tx_data[2] = (uint8_t) (frameId >> 8);
            tx_data[3] = (uint8_t) (frameId);
            tx_data[4] = (uint8_t) (nFrame >> 8);
            tx_data[5] = (uint8_t) (nFrame);
            fx_res = (uint8_t) fx_file_read(&update_file, &tx_data[6], 128, &nBytes);     //Read 128 bytes

            Refresh_Wdg ();                             // refresh the watchdog timer

            if ((frameId%1500) == 0)     //Advice to the user periodically
            {
                Audio_Message (eAUDIO_CMD_CONCAT, eAUDIO_UPGRADING, TRUE);  // warn about incoming update
            }

            do {
                tx_thread_sleep ((uint16_t)OSTIME_10MSEC);
                host_ack = Boot_Send_Message(tx_data, (6 + 128 + 1));
                Refresh_Wdg ();
                if(host_ack != 'N')
                {
                    n_errors++;
                    if (n_errors > 5)
                    {
                        R100_test_result.misc.boot = eERR_BOOT_PROCESSOR_UPDATE;
                        return FALSE;
                    }
                }
            } while (host_ack != 'N');
        }

        fx_res = (uint8_t) fx_file_close (&update_file);
        Refresh_Wdg ();
        fx_res = (uint8_t) fx_file_delete (&sd_fx_media, GOLDEN_FILENAME);
        fx_res = (uint8_t) fx_media_flush (&sd_fx_media);
        Refresh_Wdg ();

        // Wait a little time to save gold programm into SPI memory
        Pasatiempos (OSTIME_1SEC);
        Refresh_Wdg ();

        return TRUE; // golden fw uploaded
    }
    return FALSE; // no update found in uSD
}

/******************************************************************************
 ** Name:    Load_FW_File
 *****************************************************************************/
/**
 * @brief   This function use to upload firmware file.
 * 
 * @param   void
 * 
 * @return true Firmware is upload but update is pending
 * @return false No update found in uSD.
 ******************************************************************************/
static bool_t Load_FW_File(void)
{
    uint8_t  fx_res;
    uint8_t  host_ack;
    uint32_t nBytes, attributes;
    uint32_t frameId, nFrame;
    uint32_t my_app_ver = APP_REV_CODE;
    static FX_FILE  update_file, version_file;
    static uint8_t  tx_data[256];
    static uint8_t n_errors;

    Refresh_Wdg ();
    memset (tx_data, 0, sizeof(tx_data));
    tx_data[0] = 'F';
    tx_data[1] = '#';

    // Load and read configuration file on SD
    fx_res = (uint8_t) fx_file_open(&sd_fx_media, &update_file, UPGRADE_FILENAME, FX_OPEN_FOR_READ);

    R100_test_result.misc.boot = 0;     // reset error

    Refresh_Wdg ();
    if (fx_res == 0)
    {
        Trace (TRACE_NEWLINE, "");
        Trace (TRACE_TIME_STAMP | TRACE_NEWLINE, "UPDATE AVAILABLE !!!");
        Trace (                   TRACE_NEWLINE, "=============================");

        fx_res = (uint8_t) fx_file_attributes_read (&sd_fx_media, FNAME_VERSION, (UINT *)&attributes);
        Refresh_Wdg ();
        if ((fx_res == FX_SUCCESS) || (attributes & FX_ARCHIVE))
        {
            fx_res = (uint8_t) fx_file_delete(&sd_fx_media, FNAME_VERSION);
            fx_res = (uint8_t) fx_media_flush (&sd_fx_media);
        }

        // record current firmware version
        fx_res = (uint8_t) fx_file_create (&sd_fx_media, FNAME_VERSION);
        fx_res = (uint8_t) fx_file_open (&sd_fx_media, &version_file, FNAME_VERSION, FX_OPEN_FOR_WRITE);
        fx_res = (uint8_t) fx_file_write (&version_file, (uint8_t *) &my_app_ver, sizeof(uint32_t));
        fx_res = (uint8_t) fx_media_flush (&sd_fx_media);
        fx_res = (uint8_t) fx_file_close (&version_file);

        // Hidden file
        fx_res = (uint8_t) fx_file_attributes_set(&sd_fx_media, FNAME_VERSION, FX_HIDDEN);

        // warn user about incoming firmware update
        Audio_Message (eAUDIO_CMD_PLAY, eAUDIO_UPGRADING, TRUE);

        Refresh_Wdg ();                             // refresh the watchdog timer
        Boot_Sync (3);                              // Synchronize the Boot processor
        Refresh_Wdg ();                             // refresh the watchdog timer

        // Check file length
        nFrame = (uint32_t) ((update_file.fx_file_current_file_size) / 128 );
        if ((update_file.fx_file_current_file_size) % 128 )
        {
            nFrame++;
        }
        for (frameId=1; frameId<=nFrame; frameId++)
        {
            n_errors = 0;                       // reset number of errors
            tx_thread_sleep (OSTIME_20MSEC);
            //Trace_Arg (TRACE_NEWLINE, "frameId:%4d ", frameId);
            tx_data[2] = (uint8_t) (frameId >> 8);
            tx_data[3] = (uint8_t) (frameId);
            tx_data[4] = (uint8_t) (nFrame >> 8);
            tx_data[5] = (uint8_t) (nFrame);
            fx_res = (uint8_t) fx_file_read(&update_file, &tx_data[6], 128, &nBytes);     //Read 128 bytes

            Refresh_Wdg ();                             // refresh the watchdog timer

            if ((frameId%1500) == 0)     //Advice to the user periodically
            {
                Audio_Message (eAUDIO_CMD_CONCAT, eAUDIO_UPGRADING, TRUE);  // warn about incoming update
            }

            do {
                tx_thread_sleep ((uint16_t)OSTIME_10MSEC);
                host_ack = Boot_Send_Message(tx_data, (6 + 128 + 1));
                Refresh_Wdg ();
                if(host_ack != 'N')
                {
                    n_errors++;
                    if (n_errors > 5)
                    {
                        R100_test_result.misc.boot = eERR_BOOT_PROCESSOR_UPDATE;
                        return FALSE;
                    }
                }
            } while (host_ack != 'N');
        }
        fx_res = (uint8_t) fx_file_close (&update_file);
        Refresh_Wdg ();
        Pasatiempos (OSTIME_100MSEC);
        fx_res = (uint8_t) fx_file_delete (&sd_fx_media, UPGRADE_FILENAME);
        fx_res = (uint8_t) fx_media_flush (&sd_fx_media);
        Refresh_Wdg ();

        // Wait a little time to save gold programm into SPI memory
        Pasatiempos (OSTIME_1SEC);
        Refresh_Wdg ();

        // update the non volatile data before update
        nv_data.update_flag = 1;
        NV_Data_Write(&nv_data, &nv_data_block);

        return TRUE; // update pending
    }
    return FALSE; // no update found in uSD
}

extern bool_t CU_Load_FW_File(){
    return Load_FW_File();
}

/******************************************************************************
 ** Name:    R100_Check_SAT_RTC
 *****************************************************************************/
/**
 ** @brief   Power off the device programming an automatic power-on
 **
 ** @param   dev_error         current device error
 ** @param   force_warning     force the warning message
 **
 ** @return  eERR_NONE
 ******************************************************************************/
static ERROR_ID_e R100_Check_SAT_RTC (ERROR_ID_e dev_error, bool_t force_warning)
{
    //uint32_t        err_open;           // SD media open result ...
    DB_TEST_RESULT_t R100_Test_Result;

    UNUSED(force_warning);

    // copy test variable
    memset(&R100_Test_Result,0,sizeof(DB_TEST_RESULT_t));
    nv_data.test_id++;
    if (nv_data.test_id >= NUM_BASIC_TEST) nv_data.test_id = 0;

    if ((R100_Errors_cfg[dev_error].daily_monthly == 1) &&  // Monthly recovery error
        (nv_data.test_id != TEST_FULL_ID))                  // no monthly test
    {
        R100_Test_Result.test_status = TEST_ABORTED;
        R100_Test_Result.error_code = dev_error;
        return dev_error;       // save this error
    }
    else
    {
        return eERR_NONE;       // reset the error
    }
}

/******************************************************************************
 ** Name:    Test_uSD_Resources
 *****************************************************************************/
/**
 ** @brief   Function to check the presence of the audio resources
 **          and firmware updates
 **
 ** @param   none
 **
 ** @return  none
 ******************************************************************************/
static void Test_uSD_Resources (void)
{
    static  uint32_t        sd_opened = 0;
            uint32_t        err_open;           // SD media open result ...
            uint8_t         fx_res1, fx_res2;
    static  FX_FILE         update_file1, update_file2;
            uint8_t         audio_num = 0;

    // open the uSD
    if (!sd_opened)
    {
        // Load Audio resources from SD-Card
        if (err_open = Load_Audio_Resources(&audio_num), err_open == eERR_AUDIO_MAX)
        {
            //Check if a Firmware and golden files are detected in uSD, if not error
            fx_res1 = (uint8_t) fx_file_open(&sd_fx_media, &update_file1, GOLDEN_FILENAME,  FX_OPEN_FOR_READ);
            fx_res2 = (uint8_t) fx_file_open(&sd_fx_media, &update_file2, UPGRADE_FILENAME, FX_OPEN_FOR_READ);

            if (((fx_res1 != 0) || (fx_res2 != 0)) || audio_num < MAX_AUDIO_MSG)
            {
                //error
                nv_data.error_code = eERR_AUDIO_MAX;
                Trace (TRACE_NEWLINE, "NUMBER OF AUDIOS ERROR!!!");
                Audio_Message (eAUDIO_CMD_PLAY, eAUDIO_CALL_SAT, TRUE);
                Pasatiempos_Listening ();

                // close fx_media before power down
                fx_media_close(&sd_fx_media);
                tx_thread_sleep (OSTIME_200MSEC);

                // this function ends the execution !!!
                R100_PowerOff();
            }
            /*else
            {
                // error in audios
                Set_Audio_Player_in_error();
            }*/

            fx_res1 = (uint8_t) fx_file_close (&update_file1);
            fx_res2 = (uint8_t) fx_file_close (&update_file2);
        }

        sd_opened++;
    }
}

/******************************************************************************
 ** Name:    Execute_Test_Cert
 *****************************************************************************/
/**
 ** @brief   Function to load certificates
 **
 ** @param   pCert     pointer to certificate parameters
 **
 ** @return  none
 ******************************************************************************/
static void Execute_Test_Cert(TEST_CERT_t *pCert)
{
    uint16_t ret_tls_cert_exp, ret_wpa_eap_cert_exp;
    bool_t eap = FALSE, tls = FALSE;

    Check_Certs(&eap, &tls);

    if(eap == FALSE && tls == FALSE) return;

    // Initialize the NetX system.
    NX_Init();
    memset ((uint8_t*) pCert, 0, sizeof(TEST_CERT_t));

    // Check TLS CA & WPA-EAP cert expiration
    if(tls == TRUE)
    {
        ret_tls_cert_exp = Check_TLS_Cacert();
        if (ret_tls_cert_exp != 0)
        {
            pCert->tls_cert_exp = (ret_tls_cert_exp == 1) ? eRR_EXPIRED_SOON_CERT_TLS : eRR_EXPIRED_CERT_TLS;
        }
    }
    if(eap == TRUE)
    {
        ret_wpa_eap_cert_exp = Check_WPA_EAP_Cert();
        if (ret_wpa_eap_cert_exp != 0)
        {
            pCert->wpa_eap_cert_exp = (ret_wpa_eap_cert_exp == 1) ? eRR_EXPIRED_SOON_WPA_EAP : eRR_EXPIRED_WPA_EAP;
        }
    }
}

/******************************************************************************
 ** Name:    Execute_Test
 *****************************************************************************/
/**
 ** @brief   Function that executes the device test:
 **              .- verify the resources memory (checksum)
 **              .- power supplies    (DC-MAIN and +12volts)
 **              .- Batteries percent (battery 1 and 2)
 **              .- Temperature
 **
 ** @param   pResult     pointer to result parameters
 ** @param   auto_test   if the test is automatic or forced by an user
 **
 ** @return  none
 ******************************************************************************/
static void Execute_Test (DB_TEST_RESULT_t *pResult, bool_t auto_test)
{
    uint32_t    err_open;       // SD media open result ...
    int8_t      my_temp;        // Battery temperature

    // Set in test mode blinking the ONOFF led ...
    test_mode = true;
    Led_Blink (LED_ONOFF);

    // By default, blink the status LED while device is sleeping (periodic blink)
    nv_data.status_led_blink = LED_BLINK_ON;

    // Maintain the power-on switch active
    Rtc_Program_Wakeup (WAKEUP_POWERON);

    // Power on the external circuits, adding an extra time to stabilize the power supplies ...
    g_ioport.p_api->pinWrite (IO_PSU_EN, IOPORT_LEVEL_HIGH);
    tx_thread_sleep (OSTIME_20MSEC);

    // Use this timeout to re-Synchronize the Boot processor
    Boot_Sync(1);
    Refresh_Wdg ();

    // Report the startup event
    Trace (TRACE_TIME_STAMP | TRACE_NEWLINE, "Startup: TEST");

    // Initialize the file system for SD-Card, if inserted ...
    // fx_media_init0 ();
    // Resume task used for test ...
    tx_thread_resume (&thread_audio);
    err_open = fx_media_init0_open ();
    if ((err_open != FX_SUCCESS) && (err_open != FX_PTR_ERROR))
    {
        Trace (TRACE_TIME_STAMP + TRACE_NEWLINE, " uSD ERROR !!! LOCK DEVICE  !!!");
        Lock_on_Panic ((uint8_t)err_open, 19);
    }

    Battery_I2C_Init(&pon_date, &pon_time);

    // Initialize the whole structure
    memset (pResult, 0, sizeof(DB_TEST_RESULT_t));

    Refresh_Wdg ();
    // Load Device Settings from NFC (the device info is read to be available in configuration mode)
    Settings_Open_From_NFC ();
    Refresh_Wdg ();

    // Load Device Info --> NFC data MUST be read before !!!
    Device_Init (pResult->device_sn);

    // Check if a new configuration file is present in the uSD to update the Device Settings
    Settings_Open_From_uSD ();

    R100_Check_SAT(Get_NFC_Device_Info()->error_code,true);

    // Compensate the UTC time ...
    battery_RTC.utc_time = (int8_t) Get_Device_Settings()->misc.glo_utc_time;
    RTC_Normalize_Time (&battery_RTC);

    Set_Device_Date_time();

    // Resume task used for test and blink the ONOFF led ...
    tx_thread_resume (&thread_core);
    tx_thread_sleep (OSTIME_20MSEC);

    tx_thread_resume (&thread_patMon);
    tx_thread_resume (&thread_defibrillator);
    tx_thread_resume (&thread_hmi);
    tx_thread_resume (&thread_comm);

    // Test uSD resources (audio and firmware updates)
    Test_uSD_Resources();

    // If an automatic test must be executed, update the test identifier ...
    if (auto_test)
    {
        pResult->error_code = R100_Check_SAT_RTC (nv_data.error_code, false);
        pResult->test_id = nv_data.test_id;
        if(pResult->test_id == TEST_FULL_ID) 
        {
            Inc_TestManual();
        }
        else Inc_DailyTest();
    }
    else 
    {
        // When test is executed due to an user request, force a full test
        pResult->test_id = TEST_MANUAL_ID;

        // Blink all leds to test them
        Led_Blink (LED_ONOFF);
        Led_Blink (LED_SHOCK);
        Led_Blink (LED_PATYPE);

        Inc_TestManual();
    }

    Trace (TRACE_NEWLINE, "");
    Trace (TRACE_TIME_STAMP | TRACE_NEWLINE, "Test running !!!");
    Trace (                   TRACE_NEWLINE, "=============================");

    // Sequential test of devices and accessories
    // Be sure that the battery test is executed before electrodes test to
    // Be able to check the expiration date
    // Don't forget to refresh the watchdog

     // Check if battery voltage is in the working temperature range else do not execute the test
    my_temp = Battery_Get_Temperature();
    if ((my_temp > BATTERY_VERY_LOW_TEMP) && (my_temp < BATTERY_VERY_HIGH_TEMP))
    {
        Execute_Test_Patmon     (&pResult->patmon, nv_data.test_id);
        // Patient impedance is detected...check again if patient is connected
        if (pResult->patmon.ADS_pat == eERR_ADS_PAT_CONNECTED)
        {
            Pasatiempos (OSTIME_10SEC);
            Execute_Test_Patmon (&pResult->patmon, nv_data.test_id);
        }

        // Run battery and electrodes test
        Execute_Test_Battery    (&pResult->battery, auto_test, false);
        Execute_Test_Electrodes (&pResult->electrodes, auto_test, false);

        // If patient impedance is detected abort hardware testing
        if (!pResult->patmon.ADS_pat)
        {
            Execute_Test_Misc       (&pResult->misc, pResult->device_sn, pResult->test_id);
            Execute_Test_Voltage    (&pResult->voltage);
            Execute_Test_Comms      (&pResult->comms, FALSE);
            if (((pResult->test_id == TEST_FULL_ID) ||
                 (pResult->test_id == TEST_MANUAL_ID)) &&
                 (pResult->battery.error_code != eERR_BATTERY_REPLACE) && 
                 ((my_temp > BATTERY_LOW_TEMP) && (my_temp < BATTERY_HIGH_TEMP)))
            {
                if(Is_Wifi_TX_Enabled() != FALSE) Execute_Test_Cert  (&pResult->certificate);
                //Execute_Test_GPS   (pResult->gps_position);
                Execute_Test_Defib (&pResult->defib);
            }

            // In case of electrodes not connected...check again if electrodes are inserted
            if ((pResult->electrodes.error_code == eERR_ELECTRODE_NOT_CONNECTED) && !auto_test)
            {
                Pasatiempos (OSTIME_10SEC);
                Execute_Test_Electrodes (&pResult->electrodes, auto_test, false);
            }

            // Re-assign the error code
            if (pResult->error_code == eERR_NONE)   // there is not error before
            {
                nv_data.error_code = eERR_NONE;
                if (pResult->certificate.tls_cert_exp)      nv_data.error_code = pResult->error_code = pResult->certificate.tls_cert_exp;
                if (pResult->certificate.wpa_eap_cert_exp)  nv_data.error_code = pResult->error_code = pResult->certificate.wpa_eap_cert_exp;
                if (pResult->comms.accelerometer > 90)      nv_data.error_code = pResult->error_code = pResult->comms.accelerometer;
                if (pResult->comms.gps > 90)                nv_data.error_code = pResult->error_code = pResult->comms.gps;
                if (pResult->comms.wifi > 90)               nv_data.error_code = pResult->error_code = pResult->comms.wifi;
                if (pResult->comms.sigfox > 90)             nv_data.error_code = pResult->error_code = pResult->comms.sigfox;
                if (pResult->electrodes.error_code)         nv_data.error_code = pResult->error_code = pResult->electrodes.error_code;
                if (pResult->battery.error_code)            nv_data.error_code = pResult->error_code = pResult->battery.error_code;
                if (pResult->misc.audio)                    nv_data.error_code = pResult->error_code = pResult->misc.audio;
                if (pResult->misc.boot)                     nv_data.error_code = pResult->error_code = pResult->misc.boot;
                if (pResult->misc.nfc)                      nv_data.error_code = pResult->error_code = pResult->misc.nfc;
                if (pResult->defib.error_code)              nv_data.error_code = pResult->error_code = pResult->defib.error_code;
                if (pResult->patmon.ADS_cal)                nv_data.error_code = pResult->error_code = pResult->patmon.ADS_cal;
                if (pResult->patmon.ADS_comms)              nv_data.error_code = pResult->error_code = pResult->patmon.ADS_comms;
                if (pResult->voltage.dc_18v_error)          nv_data.error_code = pResult->error_code = pResult->voltage.dc_18v_error;
                if (pResult->voltage.dc_main_error)         nv_data.error_code = pResult->error_code = pResult->voltage.dc_main_error;
            }
            if (pResult->error_code) pResult->test_status = TEST_NOT_OK;
            nv_data.status_led_blink = R100_Errors_cfg[pResult->error_code].led_flashing;
        }
        else
        {
            // Warning --> Patient is connected or a Patmon error is detected
            if (!Is_SAT_Error (nv_data.error_code))
            {
                nv_data.error_code = pResult->patmon.ADS_pat;
            }
            pResult->error_code = nv_data.error_code;
            pResult->test_status = TEST_ABORTED;
        }
    }
    else
    {
        // Warning --> Temperature out of range
        if (!Is_SAT_Error (nv_data.error_code))
        {
            nv_data.error_code = eERR_BATTERY_TEMP_OUT_RANGE_OFF;
        }
        pResult->error_code = (uint32_t) nv_data.error_code;
        pResult->battery.bat_temperature = (int32_t) my_temp;
        pResult->test_status = TEST_ABORTED;

        // the device is switched OFF --> consider to play a message before to power-off
        // TODO
    }

    if (pResult->error_code)
    {
        nv_data.status_led_blink = (nv_data.status_led_blink == LED_BLINK_ON)? R100_Errors_cfg[pResult->error_code].led_flashing: LED_BLINK_OFF;
        // register the time for the next warning
        nv_data.time_warning = pon_time;
        nv_data.open_cover = pon_time;
    }


    // Inform to the user about the test result
    Refresh_Wdg ();     // refresh the internal watchdog timer
    if (pResult->test_id == TEST_MANUAL_ID)
    {
        if(pResult->test_status != TEST_ABORTED)
        {
            // play the audio message advertising Device state
            if(pResult->battery.error_code != eERR_BATTERY_REPLACE || (Is_SAT_Error(pResult->error_code) == true))
            {
                if (Is_SAT_Error(pResult->error_code))
                {
                    Audio_Message (eAUDIO_CMD_CONCAT, eAUDIO_CALL_SAT, TRUE);
                }
                else
                {
                    if ((nv_data.status_led_blink == LED_BLINK_ON) && (Is_Battery_Mode_Demo() == FALSE))
                    {
                        Audio_Message (eAUDIO_CMD_CONCAT, eAUDIO_DEVICE_READY, TRUE);
                    }
                }
            }
        }

        // In DEMO mode remind with an audio message
        if (Is_Battery_Mode_Demo()) Audio_Message (eAUDIO_CMD_CONCAT, eAUDIO_DEMO_MODE, TRUE);
        Pasatiempos_Listening ();
    }

    // register the result ...
    // auto_  test --> Generate and trace test results (serial port and file)
    // manual test --> Generate and trace test results (serial port and audio)
    Refresh_Wdg ();     // refresh the internal watchdog timer
    DB_Test_Generate_Report (pResult, true);
    Refresh_Wdg ();     // refresh the internal watchdog timer

    // Inform to the user about the test result
    if (Check_Test_Abort_User(pResult))
    {
        nv_data.error_code = pResult->error_code;
        return;
    }
    Refresh_Wdg ();     // refresh the internal watchdog timer

    // If Sigfox and Wifi options are not present, skip all this section
    if ((Is_Sigfox_TX_Enabled() != FALSE) || (Is_Wifi_TX_Enabled() != FALSE))
    {
        Send_Test_By_Comms (pResult, auto_test);
    }

    if(pResult->error_code != eERR_NONE)
    {
        nv_data.error_code = pResult->error_code;
    }
}

/******************************************************************************
 ** Name:    Execute_Startup_Test
 *****************************************************************************/
/**
 ** @brief   Function that executes the device startup test:
 **              .- verify the resources memory (checksum)
 **              .- power supplies    (DC-MAIN and +12volts)
 **              .- Battery state of charge
 **              .- Battery temperature
 **              .- PatMon operative
 **
 ** @param   pResult     pointer to result parameters
 ** @param   test_id     test identifier
 **
 ** @return  none
 ******************************************************************************/
static void Execute_Startup_Test (DB_TEST_RESULT_t *pResult)
{
    // by default, blink the status LED while device is sleeping (periodic blink)
    nv_data.status_led_blink = LED_BLINK_ON;

    // initialize the whole structure
    memset (pResult, 0, sizeof(DB_TEST_RESULT_t));

    // an initial test
    pResult->test_id = TEST_INITIAL_ID;

    Trace (TRACE_NEWLINE, "");
    Trace (TRACE_TIME_STAMP | TRACE_NEWLINE, "Device Running Test!!!");
    Trace (                   TRACE_NEWLINE, "=============================");

    // sequential test of devices and accessories
    // be sure that the battery test is executed before electrodes test to
    // be able to check the expiration date
    // Dont forget to refresh the watchdog
    Execute_Test_Misc       (&pResult->misc, pResult->device_sn, pResult->test_id);
    Execute_Test_Electrodes (&pResult->electrodes, false, true);
    Execute_Test_Voltage    (&pResult->voltage);
    Execute_Test_Patmon     (&pResult->patmon, TEST_INITIAL_ID);
    Execute_Test_Battery    (&pResult->battery, false, true);

    // re-assign the error code
    nv_data.error_code = eERR_NONE;
    if (pResult->electrodes.error_code) nv_data.error_code = pResult->error_code = pResult->electrodes.error_code;
    if (pResult->battery.error_code)    nv_data.error_code = pResult->error_code = pResult->battery.error_code;
    if (pResult->misc.audio)            nv_data.error_code = pResult->error_code = pResult->misc.audio;
    if (pResult->misc.boot)             nv_data.error_code = pResult->error_code = pResult->misc.boot;
    if (pResult->misc.nfc)              nv_data.error_code = pResult->error_code = pResult->misc.nfc;
    if (pResult->patmon.ADS_cal)        nv_data.error_code = pResult->error_code = pResult->patmon.ADS_cal;
    if (pResult->patmon.ADS_comms)      nv_data.error_code = pResult->error_code = pResult->patmon.ADS_comms;
    if (pResult->voltage.dc_18v_error)  nv_data.error_code = pResult->error_code = pResult->voltage.dc_18v_error;
    if (pResult->voltage.dc_main_error) nv_data.error_code = pResult->error_code = pResult->voltage.dc_main_error;

    // register the result ...
    // init  test --> Generate test results (file) with test id = 50
    DB_Test_Generate_Report (pResult, true);

    // save in NFC error code and battery and electrode updated info
    NFC_Write_Device_Info(true);

    if(nv_data.error_code != eERR_NONE)
    {
        // Initial test is alwais sent
        if(Is_Sigfox_TX_Enabled() == SIGFOX_ONLY || Is_Sigfox_TX_Enabled() == SIGFOX_PRIOR)
        {
            // Generate and Send Test report using Sigfox
            Send_Sigfox_Test(pResult);
        }

        if(Is_Wifi_TX_Enabled() == WIFI_ONLY || Is_Wifi_TX_Enabled() == WIFI_PRIOR) //WIFI_WITH_PATIENT
        {
            // Generate and Send Test report using Wifi
            Send_Wifi_Test_Frame(pResult);
        }
    }
}

/******************************************************************************
 ** Name:    Execute_Test_Montador
 *****************************************************************************/
/**
 ** @brief   Function that executes the device test:
 **              .- verify the resources memory (checksum)
 **              .- power supplies    (DC-MAIN and +12volts)
 **              .- Batteries percent (battery 1 and 2)
 **              .- Temperature
 **
 ** @param   pResult     pointer to result parameters
 ** @param   auto_test   if the test is automatic or forced by an user
 **
 ** @return  none
 ******************************************************************************/
static void Execute_Test_Montador (DB_TEST_RESULT_t *pResult, bool_t auto_test)
{
            uint32_t    err_open;           // SD media open result ...
    static  uint32_t    sd_opened = 0;
            ERROR_ID_e  retError = eERR_NONE;
            ATCA_STATUS status = ATCA_NOT_INITIALIZED;
            bool_t      abort_flag = false;
            bool_t      test_flag = false;
            char_t      aux[5];
            uint8_t     audio_num = 0;

    // set in test mode
    test_mode_montador = true;

    // by default, blink the status LED
    nv_data.status_led_blink = LED_BLINK_ON;
    Led_On (LED_ONOFF);

    Refresh_Wdg ();     // refresh the internal watchdog timer

    // maintain the power-on switch active
    Rtc_Program_Wakeup (WAKEUP_POWERON);

    // Power on the external circuits, adding an extra time to stabilize the power supplies ...
    g_ioport.p_api->pinWrite (IO_PSU_EN, IOPORT_LEVEL_HIGH);
    tx_thread_sleep (OSTIME_20MSEC);

    Refresh_Wdg ();     // refresh the internal watchdog timer

    // resume task used for test and blink the ONOFF led ...
    tx_thread_resume (&thread_core);
    tx_thread_sleep (OSTIME_20MSEC); // wait pointer initialization just in case

    tx_thread_resume (&thread_audio);
    //tx_thread_resume (&thread_patMon);
    tx_thread_resume (&thread_defibrillator);
    tx_thread_resume (&thread_hmi);
    tx_thread_resume (&thread_comm);

    // initialize the whole structure
    memset (pResult, 0, sizeof(DB_TEST_RESULT_t));

    Refresh_Wdg ();
    // Load Device Settings from NFC (the device info is read to be available in configuration mode)
    NFC_Write_Settings_Montador ();
    Settings_Open_From_NFC ();
    Refresh_Wdg ();
    // Load Device Info --> NFC data MUST be read before !!!
    Device_Init (pResult->device_sn);
    Set_Device_Date_time();

    // open the uSD
    if (!sd_opened)
    {
       // Initialize fx media in uSD
       err_open = fx_media_init0_open ();
       if ((err_open != FX_SUCCESS) && (err_open != FX_PTR_ERROR))
       {
           Lock_on_Panic ((uint8_t)err_open, 20);
           abort_flag = true;
       }
       tx_thread_sleep (OSTIME_100MSEC);

       // Load Audio resources from SD-Card
       if (err_open = Load_Audio_Resources(&audio_num), err_open != eERR_NONE)
       {
           Lock_on_Panic ((uint8_t)err_open, 21);
       }
       sd_opened++;
    }

    Battery_I2C_Init(&pon_date, &pon_time);

    pResult->test_id = TEST_FULL_ID;        // forze a FULL TEST

    Trace (TRACE_NEWLINE, "");
    Trace (TRACE_TIME_STAMP | TRACE_NEWLINE, "Test running !!!");
    Trace (                   TRACE_NEWLINE, "=============================");

    // Open Uart Comms
    Comm_Uart_Init();
    tx_thread_sleep (OSTIME_100MSEC);

    retError = Comm_Uart_Set_Baud_Rate(BR_115200);
    if (retError != OK)
    {
        return;
    }
    tx_thread_sleep (OSTIME_100MSEC);

    // Use this timeout to re-Synchronize the Boot processor
    Boot_Sync(1);
    Refresh_Wdg ();

    // sequential test of devices and accessories
    // be sure that the battery test is executed before electrodes test to
    // be able to check the expiration date
    // Dont forget to refresh the watchdog
    Trace (TRACE_TIME_STAMP | TRACE_NEWLINE, "Test Crypto Authentication: ATECC608B");
    if(status = Crypto_ATECC_Config(), status == ATCA_SUCCESS)
    {
        Trace (TRACE_TIME_STAMP | TRACE_NEWLINE, "OK: ATECC608B has been configure and locked !!!");
    }
    else
    {
        Trace (TRACE_TIME_STAMP | TRACE_NEWLINE, "ERROR: Check if the ATECC608B is mounted !!!");
        if(status != ATCA_WAKE_FAILED)
        {
            pResult->error_code = eRR_CRYPTO_AUTH;
            Trace_Arg (TRACE_NO_FLAGS | TRACE_NEWLINE, "ERROR:%d ", eRR_CRYPTO_AUTH);
            test_flag = true;
        }
    }

    Trace (TRACE_TIME_STAMP | TRACE_NEWLINE, "Test Miscelanious: NFC");
    Execute_Test_Misc       (&pResult->misc, pResult->device_sn, false);
    if (pResult->misc.nfc == OK)
    {
        Trace (TRACE_TIME_STAMP | TRACE_NEWLINE, "TEST 5 OK");
        //Comm_Uart_Send("OK5");
    }
    else
    {
        Trace (TRACE_TIME_STAMP | TRACE_NEWLINE, "TEST 5 ERROR");
        //Comm_Uart_Send("KO5");
        Trace_Arg (TRACE_NO_FLAGS | TRACE_NEWLINE, "ERROR:%d ", pResult->misc.nfc);
        Audio_Message (eAUDIO_CMD_PLAY, eAUDIO_CALL_SAT, TRUE);
        Pasatiempos_Listening ();
        abort_flag = true;
        test_flag = true;
    }

    if (pResult->misc.boot == OK || (pResult->misc.boot == eERR_BOOT_PROCESSOR_UPDATE))
    {
        Trace (TRACE_TIME_STAMP | TRACE_NEWLINE, "TEST 6 OK");
        //Comm_Uart_Send("OK6");
    }
    else
    {
        Trace (TRACE_TIME_STAMP | TRACE_NEWLINE, "TEST 6 ERROR");
        //Comm_Uart_Send("KO6");
        Trace_Arg (TRACE_NO_FLAGS | TRACE_NEWLINE, "ERROR:%d ", pResult->misc.boot);
        Audio_Message (eAUDIO_CMD_PLAY, eAUDIO_CALL_SAT, TRUE);
        Pasatiempos_Listening ();
        abort_flag = true;
        test_flag = true;
    }

    Trace (TRACE_TIME_STAMP | TRACE_NEWLINE, "Test Miscelanious: AUDIO");
    if (pResult->misc.audio == OK)
    {
        Trace (TRACE_TIME_STAMP | TRACE_NEWLINE, "TEST 7 OK");
        //Comm_Uart_Send("OK7");
    }
    else
    {
        Trace (TRACE_TIME_STAMP | TRACE_NEWLINE, "TEST 7 ERROR");
        //Comm_Uart_Send("KO7");
        Trace_Arg (TRACE_NO_FLAGS | TRACE_NEWLINE, "ERROR:%d ", pResult->misc.audio);
        Audio_Message (eAUDIO_CMD_PLAY, eAUDIO_CALL_SAT, TRUE);
        Pasatiempos_Listening ();
        test_flag = true;
    }

    Trace (TRACE_TIME_STAMP | TRACE_NEWLINE, "Test Voltages...");
    Execute_Test_Voltage    (&pResult->voltage);
    if ((pResult->voltage.dc_main_error == OK) &&
        (pResult->voltage.dc_18v_error == OK))
    {
        Trace (TRACE_TIME_STAMP | TRACE_NEWLINE, "TEST 8 OK");
        //Comm_Uart_Send("OK8");
    }
    else
    {
        Trace (TRACE_TIME_STAMP | TRACE_NEWLINE, "TEST 8 ERROR");
        //Comm_Uart_Send("KO8");
        Trace_Arg (TRACE_NO_FLAGS | TRACE_NEWLINE, "ERROR_DC_MAIN:%d ", pResult->voltage.dc_main_error);
        Trace_Arg (TRACE_NO_FLAGS | TRACE_NEWLINE, "ERROR_18V:%d ", pResult->voltage.dc_18v_error);
        Audio_Message (eAUDIO_CMD_PLAY, eAUDIO_CALL_SAT, TRUE);
        Pasatiempos_Listening ();
        abort_flag = true;
        test_flag = true;
    }

    Trace (TRACE_TIME_STAMP | TRACE_NEWLINE, "Test Patmon...");
    if (Execute_Test_Patmon     (&pResult->patmon, 0) == false)
    {
        Trace (TRACE_TIME_STAMP | TRACE_NEWLINE, "TEST 9 ERROR");
        //Comm_Uart_Send("KO9");
        Audio_Message (eAUDIO_CMD_PLAY, eAUDIO_CALL_SAT, TRUE);
        Pasatiempos_Listening ();
        test_flag = true;
    }
    else
    {
        Trace (TRACE_TIME_STAMP | TRACE_NEWLINE, "TEST 9 OK");
        //Comm_Uart_Send("OK9");
    }
    if ((pResult->patmon.ADS_comms == OK) &&
        (pResult->patmon.ADS_pat == OK) &&
        (pResult->patmon.ADS_cal == OK))
    {
        Trace (TRACE_TIME_STAMP | TRACE_NEWLINE, "TEST 10 OK");
        //Comm_Uart_Send("OK10");
    }
    else
    {
        Trace (TRACE_TIME_STAMP | TRACE_NEWLINE, "TEST 10 ERROR");
        //Comm_Uart_Send("K10");
        Trace_Arg (TRACE_NO_FLAGS | TRACE_NEWLINE, "ERROR ADS COMMS:%d ", pResult->patmon.ADS_comms);
        Trace_Arg (TRACE_NO_FLAGS | TRACE_NEWLINE, "ERROR ADS PAT:%d ", pResult->patmon.ADS_pat);
        Trace_Arg (TRACE_NO_FLAGS | TRACE_NEWLINE, "ERROR ADS CAL:%d ", pResult->patmon.ADS_cal);
        Audio_Message (eAUDIO_CMD_PLAY, eAUDIO_CALL_SAT, TRUE);
        Pasatiempos_Listening ();
        abort_flag = true;
        test_flag = true;
    }

    if ((pResult->patmon.ADS_temperature < 40) && (pResult->patmon.ADS_temperature > 5))
    {
        Trace (TRACE_TIME_STAMP | TRACE_NEWLINE, "TEST 11 OK");
        //Comm_Uart_Send("O11");
    }
    else
    {
        Trace (TRACE_TIME_STAMP | TRACE_NEWLINE, "TEST 11 ERROR");
        //Comm_Uart_Send("K11");
        Audio_Message (eAUDIO_CMD_PLAY, eAUDIO_CALL_SAT, TRUE);
        Trace_Arg (TRACE_NO_FLAGS | TRACE_NEWLINE, "ADS_temperature:%d ", (uint32_t) pResult->patmon.ADS_temperature);
        Pasatiempos_Listening ();
        abort_flag = true;
        test_flag = true;
    }

    Trace (TRACE_TIME_STAMP | TRACE_NEWLINE, "Test Battery...");
    Execute_Test_Battery    (&pResult->battery, auto_test, false);
    if (((strcmp((char_t *) pResult->battery.name, "BexenCardio") == 0) ||
         (strcmp((char_t *) pResult->battery.name, "BEXENCARDIO") == 0)) &&
        (pResult->battery.nominal_capacity == 4200))
    {
            Trace (TRACE_TIME_STAMP | TRACE_NEWLINE, "TEST 12 OK");
            //Comm_Uart_Send("O12");
    }
    else
    {
        Trace (TRACE_TIME_STAMP | TRACE_NEWLINE, "TEST 12 ERROR");
        //Comm_Uart_Send("K12");
        Audio_Message (eAUDIO_CMD_PLAY, eAUDIO_CALL_SAT, TRUE);
        Pasatiempos_Listening ();
        test_flag = true;
        pResult->battery.error_code = eERR_BATTERY_UNKNOWN;
    }

    Trace (TRACE_TIME_STAMP | TRACE_NEWLINE, "Test Electrodes...");
    Execute_Test_Electrodes (&pResult->electrodes, true, false);
    // These data is loaded into fixture's memory
    if (((pResult->electrodes.sn == UINT64_C(13402712795874047789)) ||
         (pResult->electrodes.sn == UINT64_C(9007199559559309869))) &&
         (pResult->electrodes.expiration_date == 132712207))
    {
        Trace (TRACE_TIME_STAMP | TRACE_NEWLINE, "TEST 13 OK");
        //Comm_Uart_Send("O13");
    }
    else
    {
        Refresh_Wdg ();
        Trace (TRACE_TIME_STAMP | TRACE_NEWLINE, "TEST 13 ERROR");
        //Comm_Uart_Send("K13");
        Audio_Message (eAUDIO_CMD_PLAY, eAUDIO_CALL_SAT, TRUE);
        Pasatiempos_Listening ();
        test_flag = true;
        pResult->electrodes.error_code = eERR_ELECTRODE_NOT_CONNECTED;
    }

    Trace (TRACE_TIME_STAMP | TRACE_NEWLINE, "Test Leds&Keys...");
    if (Execute_Test_Leds_Keys (&pResult->misc) == OK)
    {
        Trace (TRACE_TIME_STAMP | TRACE_NEWLINE, "TEST 14 OK");
        //Comm_Uart_Send("O14");
    }
    else
    {
        Trace (TRACE_TIME_STAMP | TRACE_NEWLINE, "TEST 14 ERROR");
        //Comm_Uart_Send("K14");
        Audio_Message (eAUDIO_CMD_PLAY, eAUDIO_CALL_SAT, TRUE);
        Pasatiempos_Listening ();
        test_flag = true;
    }

    /*Trace (TRACE_TIME_STAMP | TRACE_NEWLINE, "Test Accelerometer..."); // WIFI DESCOMENTAR
    Execute_Test_Comms      (&pResult->comms, TRUE);
    if(pResult->comms.accelerometer == OK)
    {
        Trace (TRACE_TIME_STAMP | TRACE_NEWLINE, "Test Accelerometer OK");
    }
    else
    {
        Trace (TRACE_TIME_STAMP | TRACE_NEWLINE, "ERROR ACC PRESENCE");
        Trace_Arg (TRACE_NO_FLAGS | TRACE_NEWLINE, "ERROR:%d ", pResult->comms.accelerometer);
        Audio_Message (eAUDIO_CMD_PLAY, eAUDIO_CALL_SAT, TRUE);
        Pasatiempos_Listening ();
        test_flag = true;
    }*/

    Refresh_Wdg ();
    if (abort_flag == false)
    {
        Trace (TRACE_TIME_STAMP | TRACE_NEWLINE, "Test Desfib...");
        Execute_Test_Defib_Montador (&pResult->defib);
        Refresh_Wdg ();
        if (pResult->defib.error_code)
        {
            Trace (TRACE_TIME_STAMP | TRACE_NEWLINE, "TEST 15 ERROR");
            //Comm_Uart_Send("K15");
            Trace_Arg (TRACE_NO_FLAGS | TRACE_NEWLINE, "ERROR:%d", pResult->defib.error_code);
            Audio_Message (eAUDIO_CMD_PLAY, eAUDIO_CALL_SAT, TRUE);
            Pasatiempos_Listening ();
            test_flag = true;
        }
        else
        {
            Trace (TRACE_TIME_STAMP | TRACE_NEWLINE, "TEST 15 OK");
            //Comm_Uart_Send("O15");

        }
    }
    Trace (TRACE_TIME_STAMP | TRACE_NEWLINE, "");

    // register the result ...
    // auto_  test --> Generate and trace test results (serial port and file)
    // manual test --> Generate and trace test results (serial port and audio)
    DB_Test_Generate_Report (pResult, auto_test);
    Refresh_Wdg ();

    Trace (TRACE_TIME_STAMP | TRACE_NEWLINE, " ");

    // re-assign the error code
    nv_data.error_code = eERR_NONE;
    if (pResult->error_code == eRR_CRYPTO_AUTH) nv_data.error_code = eRR_CRYPTO_AUTH;
    if (pResult->electrodes.error_code) nv_data.error_code = pResult->electrodes.error_code;
    if (pResult->battery.error_code)    nv_data.error_code = pResult->battery.error_code;
    if (pResult->misc.leds_keys)        nv_data.error_code = pResult->misc.leds_keys;
    if (pResult->defib.error_code)      nv_data.error_code = pResult->defib.error_code;
    if (pResult->patmon.ADS_cal)        nv_data.error_code = pResult->patmon.ADS_cal;
    if (pResult->patmon.ADS_comms)      nv_data.error_code = pResult->patmon.ADS_comms;
    if (pResult->patmon.ADS_pat)        nv_data.error_code = pResult->patmon.ADS_pat;
    if (pResult->voltage.dc_18v_error)  nv_data.error_code = pResult->voltage.dc_18v_error;
    if (pResult->voltage.dc_main_error) nv_data.error_code = pResult->voltage.dc_main_error;
    if (pResult->misc.audio)            nv_data.error_code = pResult->misc.audio;
    if (pResult->misc.nfc)              nv_data.error_code = pResult->misc.nfc;
    if (pResult->misc.boot)             nv_data.error_code = pResult->misc.boot;

    if (test_flag == true)
    {
        Trace (TRACE_TIME_STAMP | TRACE_NEWLINE, "*************    FAIL   ************");
        Trace_Arg (TRACE_NEWLINE, "************* DEVICE ERROR %4d", nv_data.error_code);
        Trace_Arg (TRACE_NEWLINE, "************* DEVICE ERROR %4d", nv_data.error_code);
        Trace (TRACE_NEWLINE, "************* CLOSE THESE SCREEN");

        Audio_Message (eAUDIO_CMD_CONCAT, eAUDIO_CALL_SAT, TRUE);
        while (1)
        {
            Pasatiempos (OSTIME_100MSEC);

            /*aux[0] = (char_t)  ((nv_data.error_code / 100) + '0');
            aux[1] = (char_t) (((nv_data.error_code % 100)/10) + '0');
            aux[2] = (char_t)  ((nv_data.error_code %  10) + '0');
            aux[3] = 0;*/

            aux[0] = 'F';
            aux[1] = 'A';
            aux[2] = 'I';
            aux[3] = (char_t)(nv_data.error_code);
            aux[4] = 0;

            Comm_Uart_Send (aux);

            Audio_Message (eAUDIO_CMD_CONCAT, eAUDIO_CALL_SAT, TRUE);
            Pasatiempos_Listening ();
            tx_thread_sleep (OSTIME_1SEC);
        }
    }
    else
    {
        Audio_Message (eAUDIO_CMD_CONCAT, eAUDIO_DEVICE_READY, TRUE);

        Trace (TRACE_TIME_STAMP | TRACE_NEWLINE, "*************    PASS   ************");
        Trace (TRACE_NEWLINE, "*************    CLOSE THESE SCREEN");

        while(1)
        {
            Pasatiempos (OSTIME_100MSEC);
            Comm_Uart_Send ("PAS");

            Audio_Message (eAUDIO_CMD_CONCAT, eAUDIO_DEVICE_READY, TRUE);
            Pasatiempos_Listening ();
            tx_thread_sleep (OSTIME_1SEC);
        }
    }

    // set the test time to 3:00 am
    nv_data.time_test = RTC_Test_Time();

    Pasatiempos_Listening ();
}

/***********************************************************************************************************************
 * @brief      Sets up system clocks in no RTC start up
 **********************************************************************************************************************/
int bsp_clock_init_Reconfigure (void)
{
    ssp_err_t err = SSP_SUCCESS;
    cgc_system_clock_cfg_t sys_cfg;
    cgc_clock_t clock;
    int error = 0;

    R_BSP_CacheSet(BSP_CACHE_STATE_OFF);                            // Turn off cache.

    g_cgc_on_cgc.init();

    R_BSP_CacheSet(BSP_CACHE_STATE_ON);                            // Turn on cache.

    /** PLL Source clock is always the main oscillator. */
    clock = CGC_CLOCK_MAIN_OSC;

    /** Need to start PLL source clock and let it stabilize before starting PLL */
    g_cgc_on_cgc.clockStart(clock, NULL);

    cgc_clock_cfg_t pll_cfg;

    /** Set PLL Divider. */
    pll_cfg.divider = BSP_CFG_PLL_DIV;

    /** Set PLL Multiplier. */
    pll_cfg.multiplier = BSP_CFG_PLL_MUL;

    /** PLL Source clock is always the main oscillator. */
    pll_cfg.source_clock = clock;

    while (SSP_ERR_STABILIZED != g_cgc_on_cgc.clockCheck(clock))
    {
        /** Wait for PLL clock source to stabilize */
    }


    // Start the PLL running. clockStart() will check the requested system clock frequency and
    // switch to High Speed mode if the requested frequency > 32 MHz.
    err = g_cgc_on_cgc.clockStart(CGC_CLOCK_PLL, &pll_cfg);
    clock = CGC_CLOCK_PLL;


    /** If the system clock has failed to start call the unrecoverable error handler. */
    if((SSP_SUCCESS != err) && (SSP_ERR_CLOCK_ACTIVE != err))
    {
        return 2;
    }


    /** MOCO, LOCO, and subclock do not have stabilization flags that can be checked. */
    if ((CGC_CLOCK_MOCO != clock) && (CGC_CLOCK_LOCO != clock) && (CGC_CLOCK_SUBCLOCK != clock))
    {
        while (SSP_ERR_STABILIZED != g_cgc_on_cgc.clockCheck(clock))
        {
            /** Wait for clock source to stabilize */
        }
    }

    sys_cfg.iclk_div  = BSP_CFG_ICK_DIV;
    sys_cfg.pclka_div = BSP_CFG_PCKA_DIV;
    sys_cfg.pclkb_div = BSP_CFG_PCKB_DIV;
    sys_cfg.pclkc_div = BSP_CFG_PCKC_DIV;
    sys_cfg.pclkd_div = BSP_CFG_PCKD_DIV;
    sys_cfg.fclk_div  = BSP_CFG_FCK_DIV;
    sys_cfg.bclk_div  = BSP_CFG_BCK_DIV;

    /** Set which clock to use for system clock and divisors for all system clocks. */
    err = g_cgc_on_cgc.systemClockSet(clock, &sys_cfg);

    /** If the system clock has failed to be configured properly call the unrecoverable error handler. */
    if(SSP_SUCCESS != err)
    {
        return 3;
    }

   return error;
}

/******************************************************************************
** Name:    Send_ACC_Queue
*****************************************************************************/
/**
** @brief   Process events related to acelerometer
**
** @param   ev event identifier
**
** @return  if the event has been processed or not
**
******************************************************************************/
/*static void Send_ACC_Queue(EVENT_ID_e ev)
{
//    Trace_Arg (TRACE_NEWLINE, "  *****************DEF QUEUE AVAILABLE = %5d", (uint32_t) (queue_acc.tx_queue_available_storage));
    if (queue_acc.tx_queue_available_storage == 0) return;
    tx_queue_send(&queue_acc, &ev, OSTIME_1SEC);

}*/

/******************************************************************************
 ** Name:    ACC_Function
 *****************************************************************************/
/**
 ** @brief   Execute ACC Function:
 **
 ** @param   none
 **
 ** @return  none
 *****************************************************************************/
/*static void ACC_Function(void) // DESCOMENTAR ACC
{
    uint32_t        err_open;           // SD media open result ...
    uint8_t         audio_num = 0;

    //Trace (TRACE_TIME_STAMP + TRACE_NEWLINE, " An alert and position will be sent");
    // maintain the power-on switch active
    Rtc_Program_Wakeup (WAKEUP_POWERON);

    // Power on the external circuits except defibrillator circuits
    // Add an extra time to stabilize the power supplies ...
    g_ioport.p_api->pinWrite (IO_PSU_EN,  IOPORT_LEVEL_HIGH);
    tx_thread_sleep (OSTIME_20MSEC);

    //tx_thread_resume (&thread_audio);

    // Initialize the file system for SD-Card, if inserted ...    
    err_open = fx_media_init0_open();
    if (err_open != FX_SUCCESS && err_open != FX_PTR_ERROR)
    {
        Trace (TRACE_TIME_STAMP + TRACE_NEWLINE, " uSD ERROR !!! LOCK DEVICE  !!!");
        Lock_on_Panic ((uint8_t)err_open, 22);
    }
    // Add an extra time to conclude SD-Card initialization ...
    tx_thread_sleep (OSTIME_100MSEC);

    Refresh_Wdg ();
    // Load Device Settings from NFC (the device info is read to be available in configuration mode)
    Settings_Open_From_NFC ();
    Refresh_Wdg ();

    // Load Device Info --> NFC data MUST be read before !!!
    Device_Init (NULL);
    Refresh_Wdg ();
    //tx_thread_sleep (OSTIME_20MSEC);

    //if (Load_Audio_Resources(&audio_num) != eERR_NONE)
    //{
    //    Lock_on_Panic ();
    //}
    //tx_thread_sleep (OSTIME_100MSEC);
    //Refresh_Wdg ();

    // Check if a new configuration file is present in the uSD to update the Device Settings
    Settings_Open_From_uSD ();

    // Check if accelerometer is enabled and configure it
    if ((Get_Device_Settings()->misc.glo_movement_alert) && (Get_Device_Info()->enable_b.accelerometer) && (Get_Device_Info()->enable_b.sigfox || Get_Device_Info()->enable_b.wifi))
    {
        // Nothing to do.
    }
    else
    {
        Trace (TRACE_TIME_STAMP + TRACE_NEWLINE, " ACC/Sigfox/Wifi not enable !!!");
        // Disable accelerometer interrupt if alert or comm are disabled and not configure again
        nv_data_block.acc_pos_hvi = 4;
        if(err_open = Accelerometer_Setup(ACC_ENABLE_XYZ, ACC_SAMPLE_RATE_PD, ACC_2G_ACCEL_RANGE), err_open != eERR_NONE)
        {
            Trace_Arg (TRACE_NO_FLAGS | TRACE_NEWLINE , "ACC CONFIGURE POWER DOWN ERR: %d", (uint32_t) err_open);
        }
        if(err_open = Accel_Interrupt_Init (0, 0, 0, 0), err_open != eERR_NONE)
        {
            Trace_Arg (TRACE_NO_FLAGS | TRACE_NEWLINE , "ACC DISABLE INTERRUPT ERR: %d", (uint32_t) err_open);
        }
        R100_PowerOff();
    }

    //Audio_Message (eAUDIO_CMD_CONCAT, eAUDIO_TONE, TRUE);

    // Disable accelerometer interrupt once it has been activated
    if(err_open = Accel_Interrupt_Init (0, 0, 0, 0), err_open != eERR_NONE)
    {
        Trace_Arg (TRACE_NO_FLAGS | TRACE_NEWLINE , "ACC DISABLE INTERRUPT ERR: %d", (uint32_t) err_open);
    }
    //tx_thread_sleep(OSTIME_100MSEC);
    tx_thread_resume(&thread_acc);
    // check if accel is in static
    pon_acc_active = true;
    while(1)
    {
        tx_thread_relinquish();
        tx_thread_sleep(OSTIME_100MSEC);
        poweron_event = Identify_PowerOn_Event();
        if(poweron_event == PON_BUTTON || poweron_event == PON_COVER || poweron_event == PON_TEST || poweron_event == PON_USB)
        {
            // if new power_on event is detected terminate acc thread
            tx_thread_terminate(&thread_acc);
            break;
        }

    }
}*/

/******************************************************************************
 ** Name:    RTC_Function
 *****************************************************************************/
/**
 ** @brief   Execute RTC Function:
 **
 ** @param   none
 **
 ** @return  power-on source identifier
 *****************************************************************************/
static POWER_ON_SOURCE_e RTC_Function(void)
{
    // Check if firmware has been updated
    R100_Check_Update(&poweron_event);

    if (poweron_event != PON_TEST)
    {
        // Proceed to test the device automatically and periodically...
        // If the battery hour is the programmed autotest hour
        if (battery_RTC.hour != nv_data.time_test)
        {
            // No test must be done
            nv_data.test_pending = true;

            // Check if the device has been left with the cover opened
            poweron_event = R100_Check_Cover();
            if(poweron_event == PON_BUTTON || poweron_event == PON_COVER || poweron_event == PON_TEST || poweron_event == PON_USB)
            {
                return poweron_event;
            }
            R100_Check_SAT(nv_data.error_code, false);  // If SAT is needed, the function send an audio advice and power off

            R100_Check_Warning ();
        }

        if ((battery_RTC.hour == nv_data.time_test) && nv_data.test_pending)
        {
            // Startup is nor RTC mode, so reconfigure clocks with PLL configuration
            bsp_clock_init_Reconfigure();

            // Execute some test ..diary or monthly.
            Led_On (LED_ONOFF);
            nv_data.test_pending = false;

            Execute_Test (&R100_test_result, true);

            if ((nv_data.test_id == TEST_FULL_ID)   &&
                ((R100_test_result.misc.boot == OK) || (R100_test_result.misc.boot == eERR_BOOT_PROCESSOR_UPDATE)) &&
                (R100_test_result.battery.rem_charge > BATTERY_LOW_CHARGE))
            {
                Load_GOLD_File();
                if (Load_FW_File())
                {
                    // Reboot bootloader processor and start update
                    Refresh_Wdg ();

                    // Wait till Synergy is reseted by PIC
                    Led_Off (LED_ONOFF);
                    while(1)
                    {
                        Led_Ongoing_Transmission ();
                        Pasatiempos (OSTIME_500MSEC);
                    }
                }

                if (R100_test_result.misc.boot == eERR_BOOT_PROCESSOR_UPDATE)
                {
                    nv_data.error_code = eERR_BOOT_PROCESSOR_UPDATE;
                }
            }
            R100_PowerOff ();
        }

        // Re-schedule next auto-test
        R100_PowerOff_RTC ();
    }
    return poweron_event;
}

/******************************************************************************
 ** Name:    R100_Startup
 *****************************************************************************/
/**
 ** @brief   Execute some startup functions:
 **
 **                >> maintain the power ON
 **                >> identify the power-on source
 **                -- if (automatic startup):
 **                       >> must initialize the RTOS ... ????
 **                       >> execute some tests
 **                       >> wait an spare time
 **                       >> program the next automatic power-on
 **                       >> switch-off the device
 **
 **                -- if (press button or open the cover)
 **                       >> normal startup. must initialize the RTOS ...
 **
 ** @param   none
 **
 ** @return  power-on source identifier
 *****************************************************************************/
static POWER_ON_SOURCE_e R100_Startup(void)
{
    // DESCOMENTAR ACC uint32_t            err_acc = 0;        // acc configure result
    uint32_t            err_open;           // SD media open result ...
    //POWER_ON_SOURCE_e   poweron_event;      // power on agent identifier
    uint8_t             my_str[16];         // generic string
    uint8_t             audio_num = 0;

    // initialize the internal RTC
    Rtc_Init ();

    // read the non volatile data and switch-on the ON-OFF led as soon as possible
    NV_Data_Read (&nv_data, &nv_data_block);
    if (nv_data.status_led_blink == LED_BLINK_ON) Led_On (LED_ONOFF);

    // refresh the watchdog timer
    Refresh_Wdg ();

    // open the communication with battery devices
    // UTC value is 0 in this point
    if(Battery_I2C_Init_RTC (&pon_date, &pon_time) != SSP_SUCCESS && nv_data.status_led_blink == LED_BLINK_ON)
    {
        nv_data.status_led_blink = LED_BLINK_OFF;
        nv_data.error_code = eRR_RTC;
    }
    // initialize the i2C semaphore ...
    tx_semaphore_ceiling_put (&i2c_2_semaphore, 1);

    // 1.- Must identify the power-up source
    // --> Automatic power-on
    // --> User pressed button
    // --> User opens the cover
    poweron_event = Identify_PowerOn_Event();

    // refresh the watchdog timer
    Refresh_Wdg ();

    // initialize traces
    Trace_Init ();

    // Initialize accelerometer and configure it with positiion // DESCOMENTAR ACC
    /*if(nv_data_block.acc_pos_hvi == 0 && nv_data.default_settings.transmis_mode != 0 && nv_data.default_settings.movement_alert == 1)
    {
        if(err_acc = Lis3dh_ACC_Init(&nv_data_block), err_acc != eERR_NONE)
        {
            if(nv_data.error_code == eERR_NONE) nv_data.error_code = err_acc;
        }
    }*/

    // Execute automatic test ...
    if (poweron_event == PON_RTC)
    {
        poweron_event = RTC_Function();
    }

    // Execute accelerometer function // DESCOMENTAR ACC
    /*if(poweron_event == PON_ACC && nv_data.default_settings.transmis_mode != 0 && nv_data.default_settings.movement_alert == 1)
    {
        // Check if acc is present
        // If acc not present not configure again else try again to solved error
        if(err_acc = Accelerometer_Presence(), err_acc != eERR_NONE)
        {
            if(nv_data.error_code == eERR_NONE) nv_data.error_code = err_acc;
            R100_PowerOff_RTC();
        }
        ACC_Function();
    }*/

    // Startup is nor RTC mode, so reconfigure clocks with PLL configuration
    bsp_clock_init_Reconfigure();

    Led_On (LED_ONOFF);
    nv_data.open_cover_first = true;

    Refresh_Wdg ();                             // refresh the watchdog timer
    // force a Test when all keys are pressed
    if (poweron_event == PON_TEST)
    {
        // Execute some test ...
        Execute_Test (&R100_test_result, false);

        // check if a new firmware is present in the uSD and update
        if (((R100_test_result.misc.boot == OK) || (R100_test_result.misc.boot == eERR_BOOT_PROCESSOR_UPDATE)) &&
           ((R100_test_result.battery.rem_charge > BATTERY_LOW_CHARGE) || (R100_test_result.battery.rem_charge == 0)))
        {
            Load_GOLD_File();
            if (Load_FW_File())
            {
                // Reboot bootloader processor and start update
                Refresh_Wdg ();

                // Wait till Synergy is reseted by PIC
                Led_Off (LED_ONOFF);
                while(1)
                {
                    Led_Ongoing_Transmission ();
                    Pasatiempos (OSTIME_500MSEC);
                }
            }

            if (R100_test_result.misc.boot == eERR_BOOT_PROCESSOR_UPDATE)
            {
               nv_data.error_code = eERR_BOOT_PROCESSOR_UPDATE;
               Audio_Message (eAUDIO_CMD_PLAY, eAUDIO_UPGRADE_ERROR, TRUE);
               Pasatiempos_Listening ();
            }
        }

        // Re-schedule next auto-test
        R100_PowerOff ();
    }

    if (poweron_event == PON_TEST_MONTADOR)
    {
        // Execute some test ...
        Execute_Test_Montador (&R100_test_result, false);

        // Re-schedule next auto-test
        R100_PowerOff ();
    }

    // Power-ON by user (opening the cover OR pressing the power on button)
    // maintain the power-on switch active
    Rtc_Program_Wakeup (WAKEUP_POWERON);

    // Power on the external circuits, adding an extra time to stabilize the power supplies ...
    g_ioport.p_api->pinWrite (IO_PSU_EN, IOPORT_LEVEL_HIGH);
    tx_thread_sleep (OSTIME_20MSEC);

    // initialize the trace port and report the startup event
    if  (poweron_event == PON_BUTTON) Trace (TRACE_TIME_STAMP | TRACE_NEWLINE, "Startup: Button");
    if  (poweron_event == PON_COVER)  Trace (TRACE_TIME_STAMP | TRACE_NEWLINE, "Startup: Cover");
    //if  (poweron_event == PON_TEST)   Trace (TRACE_TIME_STAMP | TRACE_NEWLINE, "Startup: TEST");
    if  (poweron_event == PON_USB)    Trace (TRACE_TIME_STAMP | TRACE_NEWLINE, "Startup: USB");
    nv_data.open_cover_tx_flag = 0; // reset the open cover flag (if not a RTC Startup)
    //nv_data.check_sat_tx_flag = 0; // reset the check sat flag (if not a RTC Startup)

    // Initialize the file system for SD-Card, if inserted ...
    // fx_media_init0 ();
    // resume task used for test ...
    tx_thread_resume (&thread_audio);
    err_open = fx_media_init0_open ();
    if (err_open != FX_SUCCESS && err_open != FX_PTR_ERROR)
    {
        Trace (TRACE_TIME_STAMP + TRACE_NEWLINE, " uSD ERROR !!! LOCK DEVICE  !!!");
        Lock_on_Panic ((uint8_t)err_open, 23);
    }

    // Add an extra time to conclude SD-Card initialization ...
    tx_thread_sleep (OSTIME_100MSEC);

    Battery_I2C_Init(&pon_date, &pon_time);     // NOTE: fx_media_init0_open must be done before
    Refresh_Wdg ();
    // Load Device Settings from NFC (the device info is read to be available in configuration mode)
    Settings_Open_From_NFC ();
    Refresh_Wdg ();
    // Load Device Info --> NFC data MUST be read before !!!
    Device_Init (NULL);

    // Check if a new configuration file is present in the uSD to update the Device Settings
    Settings_Open_From_uSD ();

    if(Is_SAT_Error (nv_data.error_code) == false) nv_data.error_code = Get_NFC_Device_Info()->error_code;

    // compensate the UTC time ...
    battery_RTC.utc_time = (int8_t) Get_Device_Settings()->misc.glo_utc_time;
    RTC_Normalize_Time (&battery_RTC);

    Set_Device_Date_time();
    Refresh_Wdg ();                             // refresh the watchdog timer
    // Load Audio resources from SD-Card
    if ((err_open = Load_Audio_Resources(&audio_num), err_open != eERR_NONE) && (poweron_event != PON_USB))
    {
        Lock_on_Panic ((uint8_t)err_open, 24);
    }

    // Check if accelerometer and sigfox will be enabled // DESCOMENTAR ACC
    /*if ((Get_Device_Settings()->misc.glo_movement_alert) && (Get_Device_Info()->enable_b.accelerometer) && (Get_Device_Info()->enable_b.sigfox || Get_Device_Info()->enable_b.wifi))
    {
        // Variable to confgure acc and interrupt
        nv_data_block.acc_pos_hvi = 0;
    }
    else
    {
        // Disable interrupt and not configure again
        nv_data_block.acc_pos_hvi = 4;
        if(err_open = Accelerometer_Presence(), err_open == eERR_NONE)
        {
            if(err_open = Accelerometer_Setup(ACC_ENABLE_XYZ, ACC_SAMPLE_RATE_PD, ACC_2G_ACCEL_RANGE), err_open != eERR_NONE)
            {
                Trace_Arg (TRACE_NO_FLAGS | TRACE_NEWLINE , "ACC CONFIGURE POWER DOWN : %d", (uint32_t) err_open);
            }
            if(err_open = Accel_Interrupt_Init (0, 0, 0, 0), err_open != eERR_NONE)
            {
                Trace_Arg (TRACE_NO_FLAGS | TRACE_NEWLINE , "ACC DISABLE INTERRUPT : %d", (uint32_t) err_open);
            }
            if(nv_data.error_code == eERR_NONE) nv_data.error_code = err_acc;
        }
    }*/

    // trace the battery info !!!
    Trace     (TRACE_TIME_STAMP + TRACE_NEWLINE, " Battery Info !!!");
    Trace     (TRACE_NO_FLAGS, "RTC (UTC 0)--> ");
    Fill_Generic_Date ((char_t *) my_str, pon_date);
    Trace     (TRACE_NO_FLAGS, (char_t *) my_str);
    Trace     (TRACE_NO_FLAGS, "  ");
    Fill_Generic_Time ((char_t *) my_str, pon_time);
    Trace     (TRACE_NO_FLAGS, (char_t *) my_str);
    Trace_Arg (TRACE_NEWLINE, "  SoC = %2d", Battery_Get_Charge());

    // check if the device requires SAT or not
      if (Is_SAT_Error (nv_data.error_code) &&
       (poweron_event != PON_USB)         &&
       (Get_Device_Info()->develop_mode != DEVELOP_ZP_CALIBRATION) &&
       (Get_Device_Info()->develop_mode != DEVELOP_DELETE_ERRORS))  // if SAT is needed, the function send an audio advice and power off
    {
        R100_Check_SAT (nv_data.error_code, true);
    }

    // Check if mode DEMO is enabled
    /*if (Get_Device_Settings()->misc.glo_demo_mode == TRUE)
    {
        Trace (TRACE_TIME_STAMP + TRACE_NEWLINE, "  MODO DEMO !!!");
        mode_demo = true;
    }*/

    // Add an extra time to conclude initializations ...
    tx_thread_sleep (OSTIME_100MSEC);

    // read from the electrodes to identify them (in the same way, update the electrodes presence flag)
    Electrodes_Read_Data (&electrodes_data);
        // initialize as open circuit
    electrodes_zp_segment = eZP_SEGMENT_OPEN_CIRC;

    // save the current settings and device info to the configuration file when power-up with USB connected
    if (poweron_event == PON_USB)
    {
        Settings_Open_From_NFC();
        Settings_Save_To_uSD();
    }

    // Delete errors and lock the system monitor
    if ((Get_Device_Info()->develop_mode == DEVELOP_DELETE_ERRORS) && (poweron_event != PON_USB) )
    {
        // Resume audio thread
        tx_thread_resume (&thread_audio);

        nv_data.error_code = eERR_NONE;

        // play ready message
        Audio_Message (eAUDIO_CMD_PLAY, eAUDIO_DEVICE_READY, TRUE);

        // time to play audio
        Pasatiempos_Listening ();

        // re-schedule next auto-test and POWER OFF!!
        R100_PowerOff ();
    }


    // ZP calibration mode, lock the system monitor
    if ((Get_Device_Info()->develop_mode == DEVELOP_ZP_CALIBRATION) && (poweron_event != PON_USB) )
    {
        // Use this timeout to re-Synchronize the Boot processor
        Boot_Sync(1);

        // Execute calibration process
        Execute_calibration();

        // time to play audio
        Pasatiempos (OSTIME_3SEC);

        // Save calibration values, re-schedule next auto-test and POWER OFF!!
        R100_PowerOff ();
    }

    // Get USB pass
    //Device_pw (access_key, (uint8_t *) Get_Device_Info()->sn, RSC_NAME_SZ);
    //Refresh_Wdg ();                             // refresh the watchdog timer

    if ((Get_Device_Info()->develop_mode == DEVELOP_SAVE_GPS_POSITION) && (poweron_event != PON_USB) )
    {
        // Execute gps_data obtaining process
        Execute_Save_Gps_Position(&nv_data, &nv_data_block);

        // Save device info to NFC
        //NFC_Write_Device_Info();

        // refresh just in case
        Refresh_Wdg ();

        // Save gps position, re-schedule next auto-test and POWER OFF!!
        R100_PowerOff ();
    }

    if ((Get_Device_Info()->develop_mode == DEVELOP_ATECC) && (poweron_event != PON_USB) )
    {
        ATCA_STATUS status = ATCA_NOT_INITIALIZED;

        Trace (TRACE_TIME_STAMP | TRACE_NEWLINE, "Cnfig Crypto Authentication: ATECC608B");
        if(status = Crypto_ATECC_Config(), status == ATCA_SUCCESS)
        {
            Trace (TRACE_TIME_STAMP | TRACE_NEWLINE, "OK: ATECC608B has been configure and locked !!!");
            if(status = Crypto_ATECC_Inject(), status == ATCA_SUCCESS)
            {
                Trace (TRACE_TIME_STAMP | TRACE_NEWLINE, "OK: The key has been injected !!!");
            }
            else
            {
                Trace (TRACE_TIME_STAMP | TRACE_NEWLINE, "ERROR: The key has not been injected !!!");
            }
        }
        else
        {
            Trace (TRACE_TIME_STAMP | TRACE_NEWLINE, "ERROR: Check if the ATECC608B is mounted !!!");
            if(status != ATCA_WAKE_FAILED)
            {
                nv_data.error_code = eRR_CRYPTO_AUTH;
                Trace_Arg (TRACE_NO_FLAGS | TRACE_NEWLINE, "ERROR:%d ", eRR_CRYPTO_AUTH);
            }
        }

        // refresh just in case
        Refresh_Wdg ();

        // Save gps position, re-schedule next auto-test and POWER OFF!!
        R100_PowerOff ();
    }

    // Enable core (main) thread
    tx_thread_resume (&thread_core);

    // identify the event that has power-on the device
    return poweron_event;
}

/******************************************************************************
 ** Name:    Check_Battery_Voltage
 *****************************************************************************/
/**
 ** @brief   Check that the battery voltage is in range
 **
 ** @param   my_vb              current battery voltage
 ** @param   my_temperature     current temperature
 ** @param   pAudio_msg         pointer to the audio message to report
 **
 ** @return  vLimit_safe    replace battery limit only for test
 ******************************************************************************/
static uint32_t Check_Battery_Voltage (uint32_t my_vb, int8_t my_temperature, AUDIO_ID_e *pAudio_msg)
{
    #define COUNT_VOLTAGE_SAFE      5
            uint32_t    vLimit_safe;                    // limit used to compare the battery voltage
    static  uint8_t     nFails = 0;

    // assign voltage limits
    if (my_temperature >= 25)
    {
       vLimit_safe = VB_25C_SAFE_BATTERY;
    }
    else if (my_temperature <= 0)
    {
        vLimit_safe = VB_00C_SAFE_BATTERY;
    }
    else {
       vLimit_safe  = ((VB_25C_SAFE_BATTERY - VB_00C_SAFE_BATTERY) * (uint32_t) my_temperature) / 25;
       vLimit_safe += VB_00C_SAFE_BATTERY;
    }

    // check if the battery voltage value is OK ...
    nFails = (uint8_t) ((my_vb < vLimit_safe) ? (nFails + 1) : 0);
    if (my_vb <= VB_SAFE_BATTERY)
    {
        nFails = (COUNT_VOLTAGE_SAFE*2);
    }

    battery_statistics.battery_state = nFails;

    if (nFails == COUNT_VOLTAGE_SAFE)
    {
       // parrot the audio message demanding to change the battery
       Trace (TRACE_NEWLINE,"SAFE BATTERY VOLTAGE 1 !!!");
       *pAudio_msg = eAUDIO_LOW_BATTERY;
       DB_Episode_Set_Event(eREG_BAT_LOW_BATTERY);
    }

    if (nFails == (COUNT_VOLTAGE_SAFE*2))
    {
       // parrot the audio message demanding to change the battery
       Trace (TRACE_NEWLINE,"SAFE BATTERY VOLTAGE 2!!!");
       *pAudio_msg = eAUDIO_REPLACE_BATTERY;
       DB_Episode_Set_Event(eREG_BAT_VERY_LOW_BATTERY);
       flag_power_off = false;
    }

    return vLimit_safe;
}

extern CU_Check_Battery_Voltage (uint32_t my_vb, int8_t my_temperature, AUDIO_ID_e *pAudio_msg){
    return Check_Battery_Voltage(my_vb, my_temperature, pAudio_msg);
}
/******************************************************************************
 ** Name:    R100_Program_Autotest
 *****************************************************************************/
/**
 ** @brief   Program the auto test to be executed at least 1 hour after the last manual power off
 **
 ** @param   none
 **
 ** @return  none
 ******************************************************************************/
void R100_Program_Autotest(void)
{
    uint8_t         test_hour;      // hour to program the auto test

    // read the current date & time from the RTC
    Battery_Read_RTC (&battery_RTC);

    // reset open cover alarm
    nv_data.open_cover = pon_time;
    nv_data.time_warning = pon_time;

    // if powering off at the test hour, postpone the test 2 hours
    if (battery_RTC.hour == nv_data.time_test)
    {
        nv_data.time_test++;
        if (nv_data.time_test > 23) nv_data.time_test = 0;
        nv_data.time_test++;
        if (nv_data.time_test > 23) nv_data.time_test = 0;
    }
    else {
        // if powering off in the hour before the test hour, postpone the test 1 hour
        test_hour = (uint8_t) (nv_data.time_test ? (nv_data.time_test - 1) : 23);
        if (battery_RTC.hour == test_hour)
        {
            nv_data.time_test++;
            if (nv_data.time_test > 23) nv_data.time_test = 0;
        }
    }
}

/******************************************************************************
 ** Name:    Check_Electrodes_Type
 *****************************************************************************/
/**
 ** @brief   Check electrodes type
 **
 ** @param   force   force writing statistics of electrodes
 **
 ** @return  none
 ******************************************************************************/
static void Check_Electrodes_Type (bool_t force)
{
    // update the electrodes context (presence flag, info, zp_segment, etc...)
    Electrodes_Read_Data (&electrodes_data);

    if ((electrodes_presence_flag == false) && (force == false))
    {
        first_time = true;
        nv_data.status_led_blink = LED_BLINK_OFF;
        return;
    }

    if (((first_time == true) && (Is_Battery_Mode_Demo() == FALSE)) || force)
    {
        // check expiration date to advice the user (audio message) !!!!
        if ((battery_info.must_be_0xAA == 0xAA) &&
            (battery_statistics.must_be_0x55 == 0x55) &&
            (electrodes_data.info.expiration_date <= pon_date))
        {
            DB_Episode_Set_Event(eREG_ECG_EL_EXPIRED);
            Audio_Message (eAUDIO_CMD_CONCAT, eAUDIO_REPLACE_ELECTRODES, TRUE);
            nv_data.status_led_blink = LED_BLINK_OFF;
        }
        else if (electrodes_data.event.id != 0)
        {
           DB_Episode_Set_Event(eREG_ECG_EL_USED);
           Audio_Message (eAUDIO_CMD_CONCAT, eAUDIO_REPLACE_ELECTRODES, TRUE);
           nv_data.status_led_blink = LED_BLINK_OFF;
        }
        first_time = false;
    }
}

/******************************************************************************
 ** Name:    Check_Electrodes_Type_RTC
 *****************************************************************************/
/**
 ** @brief   Check electrodes type
 **
 ** @param   none
 **
 ** @return  none
 ******************************************************************************/
static void Check_Electrodes_Type_RTC (void)
{
    // maintain the power-on switch active
    Rtc_Program_Wakeup (WAKEUP_POWERON);

    // Power on the external circuits, adding an extra time to stabilize the power supplies ...
    g_ioport.p_api->pinWrite (IO_PSU_EN, IOPORT_LEVEL_HIGH);
    tx_thread_sleep (OSTIME_20MSEC);

    // update the electrodes context (presence flag, info, zp_segment, etc...)
    Electrodes_Read_Data (&electrodes_data);

    if (electrodes_presence_flag == false)
    {
        first_time = true;
        nv_data.status_led_blink = LED_BLINK_OFF;
        if(Is_SAT_Error (nv_data.error_code) == false)
        {
            nv_data.error_code = eERR_ELECTRODE_NOT_CONNECTED;
        }
        return;
    }

    if (Is_Battery_Mode_Demo() == FALSE)
    {
        // check expiration date to advice the user (audio message) !!!!
        if ((battery_info.must_be_0xAA == 0xAA) &&
            (battery_statistics.must_be_0x55 == 0x55) &&
            (electrodes_data.info.expiration_date <= pon_date))
        {
            nv_data.status_led_blink = LED_BLINK_OFF;
            if(Is_SAT_Error (nv_data.error_code) == false)
            {
                nv_data.error_code = eERR_ELECTRODE_EXPIRED;
            }
        }
        else if (electrodes_data.event.id != 0)
        {
            nv_data.status_led_blink = LED_BLINK_OFF;
            if(Is_SAT_Error (nv_data.error_code) == false)
            {
                nv_data.error_code = eERR_ELECTRODE_USED;
            }
        }
    }
}
/******************************************************************************
 ** Name:    Check_Device_Led_Status
 *****************************************************************************/
/**
 ** @brief   Determine if status led must be on
 **
 ** @param   none
 **
 ** @return  none
 ******************************************************************************/
int Check_Device_Led_Status (void)
{
    uint8_t     pin_state;          // Cover pin state
    uint32_t    batt_charge;

    // If the led status is On....skip
    // If the led status is Off... check if can be on
    if (Is_SAT_Error (nv_data.error_code) == true) return CU_ERR_IS_SAT_ERROR;

    // Check Battery status
    batt_charge = Battery_Get_Charge();
    if (batt_charge <= BATTERY_REPLACE_CHARGE)
    {
            nv_data.status_led_blink = LED_BLINK_OFF;
            nv_data.error_code = eERR_BATTERY_REPLACE;
            return CU_ERR_BATTERY_REPLACE_CHARGE;
    }

    if (batt_charge <= BATTERY_LOW_CHARGE)
    {
        nv_data.status_led_blink = LED_BLINK_OFF;
        nv_data.error_code = eERR_BATTERY_LOW;
        return CU_ERR_BATTERY_LOW_CHARGE;
    }

    g_ioport.p_api->pinRead (KEY_COVER, &pin_state);
    if (pin_state)
    {
        nv_data.status_led_blink = LED_BLINK_OFF;
        nv_data.prev_cover_status = true;          // cover open
        return CU_ERR_COVER_OPEN;
    }

    // Check Electrodes Type
    nv_data.status_led_blink = LED_BLINK_ON;
    nv_data.error_code = eERR_NONE;
    nv_data.prev_cover_status = false;          // cover close
    if(Battery_I2C_Init_RTC (&pon_date, &pon_time) != SSP_SUCCESS)
    {
        nv_data.status_led_blink = LED_BLINK_OFF;
        nv_data.error_code = eRR_RTC;
    }
    Check_Electrodes_Type_RTC();
    return CU_ERR_COVER_CLOSE;
}

/******************************************************************************
 ** Name:    R100_PowerOff
 *****************************************************************************/
/**
 ** @brief   Power off the device programming an automatic power-on
 **
 ** @param   none
 **
 ** @return  none
 ******************************************************************************/
void R100_PowerOff(void)
{
    char_t aux[4];

    // In DEMO mode do not blink staus led
    if (Is_Battery_Mode_Demo()) nv_data.status_led_blink = LED_BLINK_OFF;

    if (nv_data.update_flag)
    {
        // reset update flag & increment consumption of update
        nv_data.update_flag = 0;
        Inc_RunTime_Update();
        Inc_TestCharges();
    }

    // check if the battery monitor is present
    if (battery_statistics.must_be_0x55 == 0x55 && battery_statistics_comms.must_be_0x55 == 0x55)
    {
        // Write statistic in the battery
        Battery_Write_Statistics (&battery_statistics);
        Battery_Write_Statistics_Comms (&battery_statistics_comms);
    }

//elena
/*nv_data.time_test++;
if (nv_data.time_test > 23) nv_data.time_test = 0;
Trace_Arg (TRACE_NEWLINE, "  AUTOTEST_HOUR= %4d", nv_data.time_test);
*/
    // Only if device_init has done
    if (Get_Device_Info()->zp_adcs_calibration != 0)
    {
        Check_Electrodes_Type_RTC();
        Save_Data_Before_Off(nv_data.error_code);
    }

    // update the non volatile data
    NV_Data_Write(&nv_data, &nv_data_block);

    // Power off the external circuits
    g_ioport.p_api->pinWrite (IO_PSU_EN, IOPORT_LEVEL_LOW);

    // do not switch-off the LED --> wait to natural extinction due to absence of voltage
    // Led_Off (LED_ONOFF);

    if (Is_Test_Mode_Montador())
    {
        while (1)
        {
            Pasatiempos (OSTIME_100MSEC);
            if (nv_data.error_code)
            {
                Comm_Uart_Send ("FAI");
                aux[0] = (char_t)  ((nv_data.error_code / 100) + '0');
                aux[1] = (char_t) (((nv_data.error_code % 100)/10) + '0');
                aux[2] = (char_t)  ((nv_data.error_code %  10) + '0');
                aux[3] = 0;
                Comm_Uart_Send (aux);
            }
            else
            {
                Comm_Uart_Send ("PAS");
            }
        }

        Trace ((TRACE_NEWLINE),"");
        Trace ((TRACE_NEWLINE),"********SWITCH OFF THE FIXTURE************");
        while (1)
        {
            Refresh_Wdg ();
        }
    }

    // Program the next wake up (msecs) and wait to self destruction !!!
    if(Is_Battery_Mode_Demo())
    {
        Rtc_Program_Kill (LIFE_BEAT_PERIOD_DEMO_MODE); // every 20 minutes
    }
    else Rtc_Program_Kill (LIFE_BEAT_PERIOD);

    //while (1);                  // this instruction will not be executed !!!
}

NV_DATA_t CU_getNV_Data(){
    return nv_data;
}
extern void CU_setBatteryRTC_hour(uint8_t value){
    battery_RTC.hour = value;
}
extern void CU_setNV_Data_time_test(uint8_t value){
    nv_data.time_test = value;
}
extern uint8_t CU_getNV_Data_time_test(){
    return nv_data.time_test;
}
uint32_t CU_getPon_date(){
    return pon_date;
}


/******************************************************************************
 ** Name:    R100_PowerOff_RTC
 *****************************************************************************/
/**
 ** @brief   Power off the device programming an automatic power-on
 **
 ** @param   none
 **
 ** @return  none
 ******************************************************************************/
void R100_PowerOff_RTC(void)
{
    // In DEMO mode do not blink staus led
    if (Is_Battery_Mode_Demo()) nv_data.status_led_blink = LED_BLINK_OFF;

    // update the non volatile data
    NV_Data_Write(&nv_data, &nv_data_block);

    // Power off the external circuits
    g_ioport.p_api->pinWrite (IO_PSU_EN, IOPORT_LEVEL_LOW);

    // Program the next wake up (msecs) and wait to self destruction !!!
    if(Is_Battery_Mode_Demo())
    {
        Rtc_Program_Kill (LIFE_BEAT_PERIOD_DEMO_MODE); // every 20 minutes
    }
    else Rtc_Program_Kill (LIFE_BEAT_PERIOD);

    //while (1);                  // this instruction will not be executed !!!
}

/******************************************************************************
 ** Name:    Check_dead_man
 *****************************************************************************/
/**
 ** @brief   Check if the dead-man effect is triggered to automatic power off
 **          the device. Use timeout of 3 minutes
 **          Assume that the function is called every 500 msecs. aprox.
 **
 ** @param   none
 **
 ** @return  none
 ******************************************************************************/
static void Check_dead_man (void)
{
    static  uint32_t        time_without_patient;   // count the time without a patient connected
            ZP_SEGMENT_e    my_segment;             // ZP segment

    // read and process the patient connection
    my_segment = patMon_Get_Zp_Segment();
    if (my_segment == eZP_SEGMENT_GOOD_CONN)    // other combinations can be analyzed:
    {                                           // eZP_SEGMENT_UNKNOWN, eZP_SEGMENT_SHORT_CIRC, eZP_SEGMENT_BAD_CONN, eZP_SEGMENT_OPEN_CIRC
        time_without_patient = 0;
        return;
    }

    time_without_patient++;
    if (time_without_patient >= (10 * 60 * 2))
    {
        // Send event to poweroff and close/delete episode
        Send_Core_Queue (eEV_KEY_ONOFF);

        // reprogram the auto-test depending on the manual power off
        //R100_Program_Autotest();

        //Check device status
        //Check_Device_Led_Status();

        // this function ends the execution !!!
        //R100_PowerOff();
    }
}

/******************************************************************************
 ** Name:    Run_USB_Mode
 *****************************************************************************/
/**
 ** @brief   Run the application in USB mode. This is a non return function
 **
 ** @param   none
 **
 ** @return  none
 ******************************************************************************/
static void Run_USB_Mode (void)
{
    //uint8_t     my_str[16];                     // generic string
    uint8_t     usb_power;                      // usb bus voltage detection

    // check if the error code requires SAT
    if (Is_SAT_Error (nv_data.error_code))
    {
        Audio_Message (eAUDIO_CMD_PLAY, eAUDIO_CALL_SAT, TRUE);
        Pasatiempos_Listening ();
    }

    // do nothing ...
    while (1)
    {
        // wait for a loop time
        tx_thread_sleep (OSTIME_500MSEC);

        // refresh the internal watchdog timer
        if (thread_supervisor != 0)
        {
            Refresh_Wdg ();
            thread_supervisor = 0;
        }

        // power off if USB cable is disconnected
        g_ioport.p_api->pinRead (VBUS_DET, &usb_power);

        if (!usb_power)
        {
            tx_thread_sleep (OSTIME_1SEC);
            /*memset (my_str, 0, sizeof(my_str));
            memcpy (my_str, access_key, 8);
            strcat ((char_t *) my_str, ".dat");

            // remove the access key (just in case)
            fx_file_delete(&sd_fx_media, (char_t *) my_str);*/

            // close fx_media before power down
            fx_media_close(&sd_fx_media);
            tx_thread_sleep (OSTIME_200MSEC);

            // reprogram the auto-test depending on the manual power off
            R100_Program_Autotest();

            // this function ends the execution !!!
            R100_PowerOff();
        }
    }
}

/******************************************************************************
 ** Name:    Check_Battery
 *****************************************************************************/
/**
 ** @brief   Check the battery availability. If the battery is not available,
 **          advise the user and switch off the device
 **
 ** @param   none
 **
 ** @return  none
 ******************************************************************************/
static void Check_Battery(void)
{
    //uint8_t     cnt_replace_batt = 0;           // Counter to play message three times if need to replace the battery when it is running
    uint8_t     cnt_replace_batt_init = 0;      // Counter to play message three times if need to replace the battery when init the R100

    // wait some time until Defib_Get_Vbatt read voltage
    tx_thread_sleep (OSTIME_100MSEC);

    while(cnt_replace_batt_init < 3)
    {
        // Check battery voltage and if is it low, terminate thread desfibrillator
        if(Battery_Get_Charge () <= BATTERY_REPLACE_CHARGE)
        {
            Set_NV_Data_Error_IF_NOT_SAT (eERR_BATTERY_REPLACE);

            // play the audio message
            Audio_Message (eAUDIO_CMD_PLAY, eAUDIO_REPLACE_BATTERY, TRUE);

            // battery is not identified --> allow to run with an unknown battery
            if (Battery_Get_Charge () == 0)
            {
                return;
            }
            Pasatiempos_Listening ();

            cnt_replace_batt_init++;

            tx_thread_sleep (OSTIME_500MSEC);

            if(cnt_replace_batt_init == 1)
            {
                // Do not detect more Cover close event
                tx_thread_terminate (&thread_drd);
                tx_thread_terminate (&thread_hmi);
                tx_thread_terminate (&thread_patMon);
                // proceed to discharge the main capacitor (just in case)
                Defib_Charge(0, 0);
                tx_thread_sleep (OSTIME_100MSEC);
                // suspend the remaining threads
                tx_thread_terminate (&thread_defibrillator);
                tx_thread_terminate (&thread_recorder);
                tx_thread_terminate (&thread_comm);
            }

            if(cnt_replace_batt_init >= 3)
            {
                tx_thread_terminate (&thread_audio);

                // close fx_media before power down
                fx_media_close(&sd_fx_media);
                tx_thread_sleep (OSTIME_200MSEC);
                // reprogram the auto-test depending on the manual power off
                R100_Program_Autotest();

                // this function ends the execution !!!
                R100_PowerOff();
            }
        }
        else
        {
            cnt_replace_batt_init = 3;
        }
    }
}

/******************************************************************************
 ** Name:    Check_Battery_Temperature
 *****************************************************************************/
/**
 ** @brief   Check the battery temperature to register events
 **
 ** @param   my_temperature     battery temperature value
 ** @param   pAudio_msg         pointer to the audio message to report
 **
 ** @return  none
 ******************************************************************************/
static void Check_Battery_Temperature (int8_t my_temperature, AUDIO_ID_e *pAudio_msg)
{
    // check the temperature value to register the event and play an audio message (if proceed)
    if (my_temperature < BATTERY_VERY_LOW_TEMP)
    {
        Set_NV_Data_Error_IF_NOT_SAT (eERR_BATTERY_TEMP_OUT_RANGE_OFF);
        DB_Episode_Set_Event(eREG_BAT_VERY_LOW_TEMPERATURE);
    }
    else if (my_temperature > BATTERY_VERY_HIGH_TEMP)
    {
        Set_NV_Data_Error_IF_NOT_SAT (eERR_BATTERY_TEMP_OUT_RANGE_OFF);
        DB_Episode_Set_Event(eREG_BAT_VERY_HIGH_TEMPERATURE);
    }
    else if (my_temperature < BATTERY_LOW_TEMP)
    {
        *pAudio_msg = eAUDIO_REPLACE_BATTERY;
        Set_NV_Data_Error_IF_NOT_SAT (eERR_BATTERY_TEMP_OUT_RANGE);
        DB_Episode_Set_Event(eREG_BAT_LOW_TEMPERATURE);
    }
    else if (my_temperature > BATTERY_HIGH_TEMP)
    {
        *pAudio_msg = eAUDIO_REPLACE_BATTERY;
        Set_NV_Data_Error_IF_NOT_SAT (eERR_BATTERY_TEMP_OUT_RANGE);
        DB_Episode_Set_Event(eREG_BAT_HIGH_TEMPERATURE);
    }
}

/******************************************************************************
 ** Name:    Call_SAT
 *****************************************************************************/
/**
 ** @brief   Call SAT service because a critical error is detected
 **          register and report the event
 **
 ** @param   none
 **
 ** @return  none
 ******************************************************************************/
static void Call_SAT(void)
{
    static bool_t      register_SAT_condition = true;  // call sat flag

    // If SAT error thread core is suspended and defibrillator disarmed
    thread_supervisor |= THREAD_ID_sMON;

    // just in case, lock the core task and set the defibrillator out of service ...
    Send_Core_Queue (eEV_LOCK_CORE);

    Defib_Set_Out_of_Service();

    // the first entry in SAT-Error MUST execute next operations ....
    //   .- register the event in the episode (this event closes the episode)
    //   .- register in the test result file
    if (register_SAT_condition)
    {
        // set as event registered !!!
        register_SAT_condition = false;

        // register sat error in episodes
        DB_Episode_Set_Event(eREG_SAT_ERROR);

        // register the sat error in the test log file
        R100_test_result.test_id = TEST_FUNC_ID;
        Execute_Test_Electrodes (&R100_test_result.electrodes, true, false);
        Execute_Test_Battery    (&R100_test_result.battery, true, false);
        DB_Test_Generate_Report (&R100_test_result, true);

        if (Is_Battery_Mode_Demo() == false)
        {
            if(Is_Sigfox_TX_Enabled() == SIGFOX_ONLY || Is_Sigfox_TX_Enabled() == SIGFOX_PRIOR)
            {
                // Generate and Send Test report using Sigfox
                Send_Sigfox_Test(&R100_test_result);
            }

            if(Is_Wifi_TX_Enabled() == WIFI_ONLY || Is_Wifi_TX_Enabled() == WIFI_PRIOR) //WIFI_WITH_PATIENT
            {
                // Generate and Send Test report using Wifi
                Send_Wifi_Test_Frame(&R100_test_result);
            }
        }
    }
}

/******************************************************************************
 ** Name:    Parrot_Audio_Message
 *****************************************************************************/
/**
 ** @brief   Parrot or play periodically an audio message
 **          assume that there is a message to play !!!
 **
 ** @param   message_to_play    identifier of the audio mesage to play
 **
 ** @return  none
 ******************************************************************************/
static void Parrot_Audio_Message (AUDIO_ID_e  message_to_play)
{
    static uint8_t     cnt_replace_batt_sat = 0;    // Counter to play message three times if there is a SAT error when it is running

    // if the battery has to be replaced, advice the user 3 times and bye-bye
    if ((message_to_play == eAUDIO_REPLACE_BATTERY) && (flag_power_off == false) && (Battery_Get_Charge () != 0))
    {
        // Do not detect more Cover close event
        tx_thread_terminate (&thread_drd);
        tx_thread_terminate (&thread_hmi);
        tx_thread_terminate (&thread_patMon);
        // proceed to discharge the main capacitor (just in case)
        Defib_Charge(0, 0);
        tx_thread_sleep (OSTIME_100MSEC);
        // suspend the remaining threads
        tx_thread_terminate (&thread_defibrillator);
        tx_thread_terminate (&thread_recorder);
        tx_thread_terminate (&thread_comm);

        Audio_Message (eAUDIO_CMD_PLAY,   eAUDIO_REPLACE_BATTERY, TRUE);
        Audio_Message (eAUDIO_CMD_CONCAT, eAUDIO_REPLACE_BATTERY, FALSE);
        Audio_Message (eAUDIO_CMD_CONCAT, eAUDIO_REPLACE_BATTERY, FALSE);
        Pasatiempos_Listening ();

        tx_thread_terminate (&thread_audio);
        // close fx_media before power down
        fx_media_close(&sd_fx_media);
        tx_thread_sleep (OSTIME_200MSEC);

        // reprogram the auto-test depending on the manual power off
        R100_Program_Autotest();

        // this function ends the execution !!!
        R100_PowerOff();
    }

    // if must call the SAT service, advice the user
    if (message_to_play == eAUDIO_CALL_SAT)
    {
        // play the message
        Audio_Message (eAUDIO_CMD_PLAY, message_to_play, TRUE);

        cnt_replace_batt_sat++;
        if(cnt_replace_batt_sat >=3)
        {
            Pasatiempos_Listening ();

            // close fx_media before power down
            fx_media_close(&sd_fx_media);
            tx_thread_sleep (OSTIME_200MSEC);

            // reprogram the auto-test depending on the manual power off
            R100_Program_Autotest();

            // this function ends the execution !!!
            R100_PowerOff();
        }
    }
    else
    {
        // concat the audio message
        Audio_Message (eAUDIO_CMD_CONCAT, message_to_play, TRUE);

        // restart the consecutive SAT played audio
        cnt_replace_batt_sat = 0;
    }
}

/******************************************************************************
 ** Name:    sysMon_trace
 *****************************************************************************/
/**
 ** @brief   Trace some items from the system monitor
 **
 ** @param   none
 **
 ** @return  none
 ******************************************************************************/
static void sysMon_trace (void)
{
    uint32_t    my_zp_ohms;         // patient impedance in ohms
    uint32_t    my_zp_adcs;         // patient impedance in adcs
    uint32_t    my_zp_cal;          // patient impedance calibration in adcs
    uint32_t    my_vb;              // battery voltage
    //int8_t      my_temp;            // battery temperature

    UNUSED(my_zp_adcs);

    // read some local values
    my_zp_ohms = patMon_Get_Zp_Ohms();
    my_zp_adcs = patMon_Get_Zp_ADCs();
    my_zp_cal  = patMon_Get_zp_CAL_ADCs();
    my_vb      = Defib_Get_Vbatt();
    //my_temp    = Battery_Get_Temperature();

    // trace some variables ...
    if(Get_Device_Info()->develop_mode != DEVELOP_SAVE_GPS_POSITION)
    {
        Trace (TRACE_NEWLINE, "  ");
        Trace (TRACE_TIME_STAMP, "  ");

        Trace_Arg (TRACE_NO_FLAGS, "  ** Vb (mV) = %5d", my_vb);
        Trace_Arg (TRACE_NO_FLAGS, "  SoC = %2d", Battery_Get_Charge());
        Trace_Arg (TRACE_NO_FLAGS, "  ** Zp_adc = %7d", my_zp_adcs);
        Trace_Arg (TRACE_NO_FLAGS, " Zp_ohms = %4d", my_zp_ohms);
        Trace_Arg (TRACE_NO_FLAGS, " # %7d", my_zp_cal);

        //Trace_Arg (TRACE_NO_FLAGS, " [Min, max] = %7d", zp_table_adcs[0]);
        //Trace_Arg (TRACE_NO_FLAGS, " # %7d", zp_table_adcs[35-2]);
        //Trace_Arg (TRACE_NO_FLAGS, " # %7d", zp_table_adcs[MAX_ZP_ITEMS-2]);

        Trace_Arg (TRACE_NO_FLAGS, "  ** Vc (V) = %4d", Defib_Get_Vc());
        //Trace_Arg (TRACE_NO_FLAGS, "  ** FRCP (bpm) = %4d", FRCP_Get_Frequency());
        //Trace_Arg (TRACE_NO_FLAGS, "  ** Tbatt = %2d", (uint32_t) my_temp);
        Trace_Arg (TRACE_NO_FLAGS, "  ** Tpcb = %2d", (uint32_t) patMon_Get_Temperature());
    }
}

/******************************************************************************
 ** Name:    Generate_Battery_Statistics
 *****************************************************************************/
/**
 ** @brief   Generate battery statistics to be considered to compute
 **          the remaining charge
 **
 ** @param   pAudio_msg     pointer to the audio message to report
 **
 ** @return  none
 ******************************************************************************/
static void Generate_Battery_Statistics (AUDIO_ID_e *pAudio_msg)
{
    uint16_t    my_charge;      // battery charge percent

    my_charge = Battery_Get_Charge ();

    if (my_charge && (my_charge <= BATTERY_REPLACE_CHARGE))
    {
        *pAudio_msg = eAUDIO_REPLACE_BATTERY;
        Trace (TRACE_NEWLINE,"REPLACE BATTERY PERCENT!!!");
        flag_power_off = true;
    }

    else if (my_charge && (my_charge <= BATTERY_LOW_CHARGE))
    {
        *pAudio_msg = eAUDIO_LOW_BATTERY;
        Trace (TRACE_NEWLINE,"LOW BATTERY PERCENT!!!");
    }

    if (my_charge == 0)
    {
        *pAudio_msg = eAUDIO_REPLACE_BATTERY;
    }

    // check if the battery monitor is present
    if (battery_statistics.must_be_0x55 == 0x55 && battery_statistics_comms.must_be_0x55 == 0x55)
    {
        // update the statistics in the battery (time count in this task !!!)
        if(Is_Wifi_TX_Enabled() == WIFI_ONLY || Is_Wifi_TX_Enabled() == WIFI_PRIOR)
        {
            Inc_WifiRunTime();
        }
        else Inc_RunTime();

        // register the statistics ...
        if (Battery_Write_Statistics (&battery_statistics) != SSP_SUCCESS)
        {
            tx_thread_sleep (OSTIME_10MSEC);
            // try to write again
            if (Battery_Write_Statistics (&battery_statistics) != SSP_SUCCESS)
            {
                Trace (TRACE_NEWLINE,"ERROR WRITING STATISTIC!!!");
                *pAudio_msg = eAUDIO_REPLACE_BATTERY;
                battery_statistics.must_be_0x55 = 0;
            }
        }

        // register the statistics ...
        if (Battery_Write_Statistics_Comms (&battery_statistics_comms) != SSP_SUCCESS)
        {
            tx_thread_sleep (OSTIME_10MSEC);
            // try to write again
            if (Battery_Write_Statistics_Comms (&battery_statistics_comms) != SSP_SUCCESS)
            {
                Trace (TRACE_NEWLINE,"ERROR WRITING STATISTIC!!!");
                *pAudio_msg = eAUDIO_REPLACE_BATTERY;
                battery_statistics_comms.must_be_0x55 = 0;
            }
        }
    }
}

/******************************************************************************
 ** Name:    Process_Electrodes_Event
 *****************************************************************************/
/**
 ** @brief   Process and register an electrodes event
 **
 ** @param   none
 **
 ** @return  none
 ******************************************************************************/
static void Process_Electrodes_Event (void)
{
    ssp_err_t   ssp_error;                      // ssp error code
    uint8_t     my_str[16];                     // generic string

    UNUSED(ssp_error);

    // increment the registered number of shocks ...
    electrodes_data.event.id ++;

    // read the battery current date & time from the battery pack
    ssp_error = Battery_Read_RTC (&battery_RTC);

    // register the power on date
    electrodes_data.event.date  = (uint32_t) (battery_RTC.year + 2000) << 16;
    electrodes_data.event.date += (uint32_t)  battery_RTC.month <<  8;
    electrodes_data.event.date += (uint32_t)  battery_RTC.date;

    // register the power on time
    electrodes_data.event.time  = (uint32_t) battery_RTC.hour << 16;
    electrodes_data.event.time += (uint32_t) battery_RTC.min  <<  8;
    electrodes_data.event.time += (uint32_t) battery_RTC.sec;

    // set the checksum
    electrodes_data.event.checksum_xor = electrodes_data.event.id ^ electrodes_data.event.date ^ electrodes_data.event.time;

    // register the event in the electrodes
    Electrodes_Write_Event (&electrodes_data.event);

    {
        // trace some variables ...
        Trace     (TRACE_TIME_STAMP + TRACE_NEWLINE, " PATIENT CONNECTED !!!");
        Trace     (TRACE_NO_FLAGS, "Registered Events --> ");
        Fill_Generic_Date ((char_t *) my_str, electrodes_data.event.date);
        Trace     (TRACE_NO_FLAGS, (char_t *) my_str);
        Trace     (TRACE_NO_FLAGS, "  ");
        Fill_Generic_Time ((char_t *) my_str, electrodes_data.event.time);
        Trace     (TRACE_NO_FLAGS, (char_t *) my_str);
        Trace_Arg (TRACE_NEWLINE, "  PatConn = %2d", electrodes_data.event.id);
    }

    // restart the event flag
    electrodes_pending_event = false;
    write_elec_last_event = true;
}

/******************************************************************************
 ** Name:    thread_sysMon_entry
 *****************************************************************************/
/**
 ** @brief   Startup to run the application and system monitoring
 **
 ** @param   none
 **
 ** @return  none
 ******************************************************************************/
void thread_sysMon_entry (void)
{
    uint32_t    time_to_audio;                  // time to parrot some messages ...
    uint32_t    time_to_audio_low_batt;         // time to parrot LOW_BATT messages ...
    uint32_t    time_to_statistic;              // time to count statistics ...
    uint32_t    time_to_trace;                  // time to send traces ...
    uint32_t    time_to_gps;                    // time to update gps ...
    uint32_t    time_to_gps_init;               // time to init gps ...
    uint32_t    time_to_gps_cmd;                // time to send gps commands
    uint32_t    my_vb, my_v18;                  // battery voltage
    int8_t      my_temp;                        // battery temperature
    uint32_t    my_time;                        // local tick counter
    ssp_err_t   ssp_error;                      // ssp error code
    uint8_t     usb_power, pin_state;           // usb bus voltage detection
    AUDIO_ID_e  message_to_play;                // audio message to play
    DEFIB_STATE_e       my_def_state;           // defibrillator state
    //POWER_ON_SOURCE_e   poweron_event;          // power on event identifier
    wdt_cfg_t   my_cfg;                         // used to check the watchdog configuration

    UNUSED(ssp_error);

    // Execute ASAP --> initializes the internal watchdog timer
    // initialized with 4,3 secs. of timeout
    // the WDT is configured as autostart. This function is used
    // with the debugger
    ssp_error = iWDT.p_api->open(iWDT.p_ctrl, iWDT.p_cfg);
    ssp_error = iWDT.p_api->cfgGet(iWDT.p_ctrl, &my_cfg);
    Refresh_Wdg ();

    flag_sysmon_task = false;

    // Initialize the R100 and identify the power-up event
    poweron_event = R100_Startup ();
    g_ioport.p_api->pinRead (KEY_COVER, &pin_state);
    if (!pin_state && poweron_event == PON_BUTTON)
    {
        global_poweron_event = true;
    }

    // restart the audio to play as a parrot message
    message_to_play = (AUDIO_ID_e) NULL;
/*
    // make some global checks ...
    if (poweron_event == PON_USB)
    {
        // from USB, lock the system monitor to allow the communication with the PC
        Run_USB_Mode ();
    }
    else {
        // check the battery availability. This function can switch off the device
        Check_Battery ();
    }

    // synchronize the Boot processor (fine tuning of the serial port)
    Refresh_Wdg ();                             // refresh the watchdog timer
    Boot_Sync (1);                              // Synchronize the Boot processor
    Refresh_Wdg ();                             // refresh the watchdog timer

    // execute the startup test
    Execute_Startup_Test (&R100_test_result);
    Refresh_Wdg ();

    // initialize some timers
    my_time                 = tx_time_get();
    time_to_statistic       = my_time + OSTIME_30SEC;    // time to count statistics ...
    time_to_audio           = my_time;                   // time to parrot some messages ...
    time_to_audio_low_batt  = my_time;
    time_to_trace           = my_time;                   // time to send traces to serial port ...
    time_to_gps_init        = my_time + OSTIME_10SEC;    // time to update gps ...
    time_to_gps             = my_time + OSTIME_30SEC;    // time to update gps ...
    time_to_gps_cmd         = my_time + OSTIME_60SEC;    // time to update gps ...

    flag_sysmon_task = true;    // task initialized

    Trace (TRACE_NEWLINE, "\n");
    Trace_Arg (TRACE_NEWLINE, "  ** ADC zero = %8d", Get_Device_Info()->zp_adcs_short);
    Trace_Arg (TRACE_NEWLINE, "  ** ADC cal  = %8d", Get_Device_Info()->zp_adcs_calibration);

    // system monitor loop
    while (1)
    {
        // wait for a loop time
        tx_thread_sleep (OSTIME_500MSEC);
        my_time = tx_time_get();

        // refresh the internal watchdog timer
        if (thread_supervisor != 0)
        {
            Refresh_Wdg ();
            thread_supervisor = 0;
        }

        if (Get_Device_Info()->develop_mode != DEVELOP_MANUFACTURE_CONTROL)
        {
            // check the Dead-Man condition
            Check_dead_man();
        }

        // monitor the electrodes
        Check_Electrodes_Type (false);
        electrodes_zp_segment = patMon_Get_Zp_Segment ();
        if ((electrodes_pending_event) && (Is_Battery_Mode_Demo() == false))
        {
            Process_Electrodes_Event();
        }

        // read some local values
        //my_temperature = patMon_Get_Temperature();
        Battery_Read_Temperature(&my_temp);
        my_v18 = Defib_Get_Vdefib();
        my_vb  = Defib_Get_Vbatt();
        my_def_state = Defib_Get_State ();

        /////////////////////////////////////////////////////////////////////////////////////
        // check defibrillator and battery voltage values
        if (my_def_state == eDEFIB_STATE_CHARGING)
        {
            if ((my_v18 < SMON_18V_MIN) && ((nv_data.error_code == eERR_NONE) || (Is_SAT_Error (nv_data.error_code) == false))) { R100_test_result.voltage.dc_18v_error = nv_data.error_code = eERR_SMON_18V_TOO_LOW;  }
            if ((my_v18 > SMON_18V_MAX) && ((nv_data.error_code == eERR_NONE) || (Is_SAT_Error (nv_data.error_code) == false))) { R100_test_result.voltage.dc_18v_error = nv_data.error_code = eERR_SMON_18V_TOO_HIGH; }
        }
        else 
        {
            if (my_def_state != eDEFIB_STATE_SHOCKING)
            {
                Check_Battery_Voltage (my_vb, my_temp, &message_to_play);
                if ((my_vb < SMON_DCMAIN_MIN)  && ((nv_data.error_code == eERR_NONE) || (Is_SAT_Error (nv_data.error_code) == false))) { R100_test_result.voltage.dc_main_error = nv_data.error_code = eERR_SMON_DCMAIN_TOO_LOW;  }
                if ((my_vb > SMON_DCMAIN_MAX)  && ((nv_data.error_code == eERR_NONE) || (Is_SAT_Error (nv_data.error_code) == false))) { R100_test_result.voltage.dc_main_error = nv_data.error_code = eERR_SMON_DCMAIN_TOO_HIGH; }
            }
        }

        if (my_def_state == eDEFIB_STATE_IN_ERROR)
        {
            // register the defibrillator error code
            if ((nv_data.error_code == eERR_NONE) || (Is_SAT_Error (nv_data.error_code) == false))
            {
                R100_test_result.error_code = nv_data.error_code = Defib_Get_ErrorId();
            }
        }

        /////////////////////////////////////////////////////////////////////////////////////
        // with some critical errors, must call the SAT service
        if (Is_SAT_Error (nv_data.error_code))
        {
            // parrot the audio message demanding SAT service
            message_to_play = eAUDIO_CALL_SAT;

            Call_SAT ();
        }

        // send some debug traces ...
        if (my_time >= time_to_trace)
        {
            time_to_trace = my_time + OSTIME_1SEC;
            sysMon_trace ();

            // power off if USB cable is connected
            g_ioport.p_api->pinRead (VBUS_DET, &usb_power);
            if (usb_power)
            {
                // lock the core task anf power off
                Send_Core_Queue (eEV_USB);
                while(1);
            }
        }

        // update the battery statistics ...
        if (my_time >= time_to_statistic)
        {
            time_to_statistic = my_time + OSTIME_60SEC;

            // generate some battery statistics ...
            Generate_Battery_Statistics(&message_to_play);

            // check battery temperature
            Check_Battery_Temperature (my_temp, &message_to_play);

            // use this timeout to re-Synchronize the Boot processor
            Boot_Sync (1);
        }

        ////////////////////////////////////////
        // parrot audio messages ...
        if ((my_time >= time_to_audio) &&
            (message_to_play != eAUDIO_LOW_BATTERY) &&
            (message_to_play))
        {
            time_to_audio = my_time + OSTIME_10SEC;
            Parrot_Audio_Message (message_to_play);
            if (message_to_play != eAUDIO_CALL_SAT) { message_to_play = (AUDIO_ID_e) NULL; }
        }
        if ((my_time >= time_to_audio_low_batt) &&
            (message_to_play == eAUDIO_LOW_BATTERY))
        {
            time_to_audio_low_batt = my_time + OSTIME_60SEC;
            Parrot_Audio_Message (message_to_play);
            message_to_play = (AUDIO_ID_e) NULL;
        }

        //Disable alerts in patient mode if variable is configured
        if(Get_Device_Settings()->misc.glo_patient_alert == FALSE ||
           (Is_Sigfox_TX_Enabled() == FALSE && Is_Wifi_TX_Enabled() == FALSE) ||
           Is_GPS_TX_Enabled() == FALSE)
        {
            first_time_gps = FALSE;
        }

        // update the gps position ...
        // Only send once with patient connected
        if ((my_time >= time_to_gps_init)   &&
//bug4688            (electrodes_zp_segment == eZP_SEGMENT_GOOD_CONN) &&
            (first_time_gps)                &&
            (Is_Battery_Mode_Demo() == FALSE))
        {
            time_to_gps_init = my_time + OSTIME_30SEC;

            Execute_Periodic_Gps_Init();
        }

        if ((my_time >= time_to_gps)   &&
            (first_time_gps)   &&
            (Is_Battery_Mode_Demo() == FALSE))
        {
            if (Gps_Is_New_Position_Available() == TRUE)
            {
                Comm_GPS_Get_Position();
            }
            Execute_Periodic_Gps_Position(&nv_data, &nv_data_block);
            time_to_gps = my_time + OSTIME_10SEC;
        }

        // send commands to the GPS to configure it
        if(GPS_Is_Running() == true)
        {
            // Wait a lite time to stabilize GPS before send command
            time_to_gps_cmd = my_time + OSTIME_2SEC;
            GPS_Set_Running(FALSE);
        }

        // send commands to the GPS to configure it
        if((my_time >= time_to_gps_cmd) && (first_time_gps) && (Is_Battery_Mode_Demo() == FALSE) && Comm_Is_GPS_Cmd_Send() != eERR_NONE)
        {
            Comm_GPS_Send_Cmd();

            time_to_gps = my_time + OSTIME_10SEC;
            time_to_gps_cmd = time_to_gps + OSTIME_15SEC;
        }
    }
*/
}

