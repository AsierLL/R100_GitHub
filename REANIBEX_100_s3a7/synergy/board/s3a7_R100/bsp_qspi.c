/***********************************************************************************************************************
 * Copyright [2015] Renesas Electronics Corporation and/or its licensors. All Rights Reserved.
 *
 * This file is part of Renesas SynergyTM Software Package (SSP)
 *
 * The contents of this file (the "contents") are proprietary and confidential to Renesas Electronics Corporation
 * and/or its licensors ("Renesas") and subject to statutory and contractual protections.
 *
 * This file is subject to a Renesas SSP license agreement. Unless otherwise agreed in an SSP license agreement with
 * Renesas: 1) you may not use, copy, modify, distribute, display, or perform the contents; 2) you may not use any name
 * or mark of Renesas for advertising or publicity purposes or in connection with your use of the contents; 3) RENESAS
 * MAKES NO WARRANTY OR REPRESENTATIONS ABOUT THE SUITABILITY OF THE CONTENTS FOR ANY PURPOSE; THE CONTENTS ARE PROVIDED
 * "AS IS" WITHOUT ANY EXPRESS OR IMPLIED WARRANTY, INCLUDING THE IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS FOR A
 * PARTICULAR PURPOSE, AND NON-INFRINGEMENT; AND 4) RENESAS SHALL NOT BE LIABLE FOR ANY DIRECT, INDIRECT, SPECIAL, OR
 * CONSEQUENTIAL DAMAGES, INCLUDING DAMAGES RESULTING FROM LOSS OF USE, DATA, OR PROJECTS, WHETHER IN AN ACTION OF
 * CONTRACT OR TORT, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THE CONTENTS. Third-party contents
 * included in this file may be subject to different terms.
 **********************************************************************************************************************/

/**********************************************************************************************************************
 * File Name    : bsp_qspi.c
 * Description  : QSPI initialization.
 **********************************************************************************************************************/

/*******************************************************************************************************************//**
 * @addtogroup BSP_DK2M_QSPI
 * @brief QSPI initialization
 *
 * This file contains code that initializes the QSPI flash controller connected to a N25Q256A Micron Serial NOR Flash
 * Memory mounted on a DK2 development board.
 * @{
 **********************************************************************************************************************/

/***********************************************************************************************************************
 * Includes
 **********************************************************************************************************************/
#include "bsp_api.h"

#if defined(BSP_BOARD_S3A7_DK)

/***********************************************************************************************************************
 * Configuration parameters
 **********************************************************************************************************************/

/* Number of bytes to be used for the address (3 or 4) */
#define BSP_PRV_QSPI_NUM_ADDRESS_BYTES 3U
#if BSP_PRV_QSPI_NUM_ADDRESS_BYTES != 3U && BSP_PRV_QSPI_NUM_ADDRESS_BYTES != 4U
#error BSP_PRV_QSPI_NUM_ADDRESS_BYTES must be equal to 3 or 4
#endif

/* XIP entry and exit comfirmation codes for the flash device */
#define BSP_PRV_QSPI_N25Q256A_XIP_ENTRY_CODE 0U
#define BSP_PRV_QSPI_N25Q256A_XIP_EXIT_CODE  0xffU

/* Number of dummy clocks to set on the flash device for FAST READ operations */
#define BSP_PRV_QSPI_N25Q256A_NUM_DUMMY_CLOCKS 5U

/* QSSL high between cycles */
#define BSP_PRV_QSPI_N25Q256A_DE_SELECT_DELAY 4U

/* Read mode to operate the device in */
#define BSP_PRV_QSPI_READ_MODE QSPI_READMODE_FAST_QUAD_IO

/* Enter XIP mode after bsp_qspi_init is called. */
#define BSP_PRV_QSPI_XIP_MODE_AFTER_INIT 1U

/* Is prefetch used for ROM access mode */
#define BSP_PRV_QSPI_ROM_PREFTECH_MODE 1U

/* QSPI Clock rate */
#define BSP_PRV_QSPI_CLOCK_RATE QSPI_CLK_DIV5  ///< QSPI CLK runs at 24.00 MHz if PCLKA is set to 120MHz

/* Flash device page size */
#define BSP_PRV_QSPI_N25Q256A_PAGE_SIZE (256U)

/***********************************************************************************************************************
 * Private global variables
 **********************************************************************************************************************/

/* Non-volatile configuration register */
static n25q256a_nv_cfg          nv_cfg = {{(uint8_t) 0U}};

/* Volatile configuration register */
static n25q256a_volatile_cfg    volatile_cfg = {{(uint8_t) 0U}};

/* Device ID and characteristics */
static n25q256a_characteristics device_characteristics =
{
    0, 0, 0
};

/* Device flag status */
static n25q256a_flag_status flag_status = {{(uint8_t) 0U}};

/* Flash Erase sizes */
static uint32_t n25q256a_flash_erase_sizes[] =
{
    4096U,          ///< 4KB
    65536U,         ///< 64KB
    33554432U       ///< 32MB
};

/* Flash erase commands for 4byte address */
static uint8_t n25q256a_4byte_erase_commands[]=
{
    0x21U,      ///< QSPI COMMAND 4BYTE_SUBSECTOR_ERASE
    0xDCU,      ///< QSPI COMMAND 4BYTE_SECTOR_ERASE
    0xC7U       ///< QSPI_COMMAND_CHIP_ERASE
};

/* Flash erase commands */
static uint8_t n25q256a_erase_commands[]=
{
    0x20U,     ///< QSPI COMMAND SUBSECTOR_ERASE
    0xD8U,     ///< QSPI COMMAND SECTOR_ERASE
    0xC7U      ///< QSPI_COMMAND_CHIP_ERASE
};
/*******************************************************************************************************************//**
 * @brief   Reset the flash device
 **********************************************************************************************************************/
static void bsp_qspi_device_reset ()
{
    /* Reset the flash device. */
    R_QSPI->SFMCOM        = QSPI_COMMAND_RESET_ENABLE; /* Write the command */
    R_QSPI->SFMCMD_b.DCOM = 1U;                         /* Close the SPI bus cycle */
    R_QSPI->SFMCOM        = QSPI_COMMAND_RESET_MEMORY; /* Write the command */
    R_QSPI->SFMCMD_b.DCOM = 1U;                         /* Close the SPI bus cycle */
}

/*******************************************************************************************************************//**
 * @brief   Enable or disable XIP mode on the flash device
 *
 * @param[in]  enable_mode  0 = disable XIP mode, 1 = enable XIP mode
 *
 **********************************************************************************************************************/
static void bsp_qspi_device_xip_mode (bool enable_mode)
{
    bool              write_in_progress;
    volatile uint32_t timeout;

    R_QSPI->SFMCMD_b.DCOM = 1U;                                      /* Enter Direct Communication mode */

    /* Enable or disable XIP mode in the flash device */
    volatile_cfg.xip = enable_mode ? 0U : 1U;

    /* Program the volatile configuration register in the device */
    R_QSPI->SFMCOM        = QSPI_COMMAND_WRITE_ENABLE;              /* Enable writing */
    R_QSPI->SFMCMD_b.DCOM = 1U;                                     /* Close the SPI bus cycle */
    R_QSPI->SFMCOM        = QSPI_COMMAND_WRITE_VOLATILE_CFGREG;     /* Write the command */
    R_QSPI->SFMCOM        = volatile_cfg.entire_cfg;                /* Write the volatile configuration register */
    R_QSPI->SFMCMD_b.DCOM = 1U;                                     /* Close the SPI bus cycle */

    /* Wait for the write to complete */
    write_in_progress = 1;
    timeout           = 0xfffU;
    while (write_in_progress)
    {
        bsp_qspi_status_get(&write_in_progress);
        timeout--;
        if (0 == timeout)
        {
            return;
        }
    }

    R_QSPI->SFMCOM        = QSPI_COMMAND_WRITE_DISABLE; /* disable writing */
    R_QSPI->SFMCMD_b.DCOM = (uint32_t)1;                /* close the SPI bus cycle */

    R_QSPI->SFMCMD_b.DCOM = (uint32_t)0;                /* enter ROM access mode */
}

/*******************************************************************************************************************//**
 * This function reads the flag status register from the device which indicates errors that occurred during various
 * operations like erasing and programming.
 **********************************************************************************************************************/
static void bsp_qspi_read_status_flag_register (n25q256a_flag_status * p_flag_status)
{
    uint8_t regval;

    /* Send command to read status */
    R_QSPI->SFMCOM = QSPI_COMMAND_READ_FLAG_STATUS_REGISTER;

    /* Read the device status register */
    regval = R_QSPI->SFMCOM_b.SFMD;

    /* Close the SPI bus cycle */
    R_QSPI->SFMCMD_b.DCOM = 1U;

    p_flag_status->entire_cfg = regval;

}

/*******************************************************************************************************************//**
 * @brief   Enter or exit XIP mode
 *
 * @param[in]  enter_mode  0 = exit XIP mode, 1 = enter XIP mode
 *
 **********************************************************************************************************************/
static void bsp_qspi_xip_mode (bool enter_mode)
{
    volatile uint32_t i = 0;
    volatile uint32_t timeout;

    SSP_PARAMETER_NOT_USED(i);

    R_QSPI->SFMCMD_b.DCOM = (uint32_t)0;

    if (enter_mode)
    {
        R_QSPI->SFMSDC_b.SFMXD  = (uint32_t)BSP_PRV_QSPI_N25Q256A_XIP_ENTRY_CODE;              /* Set the XIP entry
                                                                                                * confirmation
                                                                                                * code */
        R_QSPI->SFMSDC_b.SFMXEN = true;                                                        /* Enter XIP mode in QSPI
                                                                                                * controller */
        i                       = *(volatile uint32_t *) BSP_PRV_QSPI_DEVICE_PHYSICAL_ADDRESS; /* Read from the device
                                                                                                * set the
                                                                                                * code */
        /* Wait for the controller to enter XIP mode */
        timeout                 = 0xfffU;
        while (0 == R_QSPI->SFMSDC_b.SFMXST)
        {
            timeout--;
            if (0 == timeout)
            {
                return;
            }
        }
    }
    else
    {
        R_QSPI->SFMSDC_b.SFMXD  = BSP_PRV_QSPI_N25Q256A_XIP_EXIT_CODE;                         /* Set the XIP exit
                                                                                                * confirmation
                                                                                                * code */
        i                       = *(volatile uint32_t *) BSP_PRV_QSPI_DEVICE_PHYSICAL_ADDRESS; /* Read from the device
                                                                                                * to
                                                                                                * set the
                                                                                                * code */
        R_QSPI->SFMSDC_b.SFMXEN = false;                                                       /* Exit XIP mode in the
                                                                                                * QSPI
                                                                                                * controller
                                                                                                * block */
        i                       = *(volatile uint32_t *) BSP_PRV_QSPI_DEVICE_PHYSICAL_ADDRESS; /* Read from the device
                                                                                                * to
                                                                                                * set the
                                                                                                * code */

        /* Wait for the controller to exit XIP mode */
        timeout = 0xfffU;
        while (R_QSPI->SFMSDC_b.SFMXST)
        {
            timeout--;
            if (0 == timeout)
            {
                return;
            }
        }
    }
}

/***********************************************************************************************************************
 * Functions
 **********************************************************************************************************************/

/*******************************************************************************************************************//**
 * @brief   Initializes QSPI.
 *
 * This function initializes QSPI and Micron Serial Flash Memory device on the board.
 *
 * @note This function is executed once after reset.
 **********************************************************************************************************************/
void bsp_qspi_init (void)
{
    uint32_t regval;
    bool     write_in_progress;

    /* enable clocks to the QSPI block */
    R_MSTP->MSTPCRB_b.MSTPB6 = 0U;

    /* Initialized unused bits */
    R_QSPI->SFMSPC = 0x10U;
    R_QSPI->SFMCST = 0U;
    R_QSPI->SFMSIC = 0U;
    R_QSPI->SFMPMD = 0U;
    R_QSPI->SFMCNT1 = 0U;

    /* Set the SPI clock rate */
    R_QSPI->SFMSKC_b.SFMDV = BSP_PRV_QSPI_CLOCK_RATE;

    /* enter direct communication mode */
    R_QSPI->SFMCMD_b.DCOM = 1U;

    /* Reset the flash device */
    bsp_qspi_device_reset();

    /* Read the ID of the device. Confirm it is the correct device. */
    R_QSPI->SFMCOM                         = QSPI_COMMAND_READ_ID;  /* Write the command */
    device_characteristics.manufacturer_id = R_QSPI->SFMCOM_b.SFMD; /* Read the manufacturer ID */
    device_characteristics.memory_type     = R_QSPI->SFMCOM_b.SFMD; /* Read the memory type */
    device_characteristics.memory_capacity = R_QSPI->SFMCOM_b.SFMD; /* Read the memory capacity */
    R_QSPI->SFMCMD_b.DCOM                  = 1U;                    /* Close the SPI bus cycle */

    if ((BSP_PRV_QSPI_MANUFACTURER_ID != device_characteristics.manufacturer_id) ||
        (BSP_PRV_QSPI_MEMORY_TYPE != device_characteristics.memory_type) ||
        (BSP_PRV_QSPI_MEMORY_CAPACITY != device_characteristics.memory_capacity))
    {
        device_characteristics.manufacturer_id = 0U;
        device_characteristics.memory_type     = 0U;
        device_characteristics.memory_capacity = 0U;
        return;
    }

    /* Read the non-volatile configuration of the device */
    R_QSPI->SFMCOM        = QSPI_COMMAND_READ_NONVOLATILE_CFGREG;  /* Write the command */
    regval                = (uint32_t)(R_QSPI->SFMCOM_b.SFMD << 8);/* Read the nv configuration register */
    regval               |= R_QSPI->SFMCOM_b.SFMD;                 /* Read the nv configuration register */
    R_QSPI->SFMCMD_b.DCOM = 1U;                                    /* Close the SPI bus cycle */
    nv_cfg.entire_cfg          =  regval;

    /* Change the configuration of the device if it differs from the configuration specified above */
    if (((4U - BSP_PRV_QSPI_NUM_ADDRESS_BYTES) != nv_cfg.address_bytes) ||
        (BSP_PRV_QSPI_N25Q256A_NUM_DUMMY_CLOCKS != nv_cfg.number_dummy_clock_cycles))
    {
        nv_cfg.address_bytes             = (4U - BSP_PRV_QSPI_NUM_ADDRESS_BYTES);
        nv_cfg.number_dummy_clock_cycles = BSP_PRV_QSPI_N25Q256A_NUM_DUMMY_CLOCKS;

        regval = nv_cfg.entire_cfg;

        /* Program the non-volatile configuration register in the device */
        R_QSPI->SFMCOM        = QSPI_COMMAND_WRITE_ENABLE;             /* Enable writing */
        R_QSPI->SFMCMD_b.DCOM = 1U;                                    /* Close the SPI bus cycle */
        R_QSPI->SFMCOM        = QSPI_COMMAND_WRITE_NONVOLATILE_CFGREG; /* Write the command */
        R_QSPI->SFMCOM        = (regval >> 8);                         /* Write the nv configuration register */
        R_QSPI->SFMCOM        = (uint32_t)regval;                      /* Write the nv configuration register */
        R_QSPI->SFMCMD_b.DCOM = 1U;                                    /* Close the SPI bus cycle */

        /* Wait for the write to complete */
        do
        {
            bsp_qspi_status_get(&write_in_progress);
        } while (write_in_progress);

        /* close the SPI bus cycle */
        R_QSPI->SFMCMD_b.DCOM = 1U;
    }

    /* Read the volatile configuration of the device */
    R_QSPI->SFMCOM        = QSPI_COMMAND_READ_VOLATILE_CFGREG;     /* Write the command */
    regval                = R_QSPI->SFMCOM_b.SFMD;                 /* Read the volatile configuration register */
    R_QSPI->SFMCMD_b.DCOM = 1U;                                    /* Close the SPI bus cycle */
    volatile_cfg.entire_cfg          =  regval;

    /* Set the number of dummy cycles in the flash device */
    volatile_cfg.num_dummy_clock_cycles = BSP_PRV_QSPI_N25Q256A_NUM_DUMMY_CLOCKS;
    regval = volatile_cfg.entire_cfg;

    /* Program the volatile configuration register in the device */
    R_QSPI->SFMCOM        = QSPI_COMMAND_WRITE_ENABLE;              /* Enable writing */
    R_QSPI->SFMCMD_b.DCOM = 1U;                                     /* Close the SPI bus cycle */
    R_QSPI->SFMCOM        = QSPI_COMMAND_WRITE_VOLATILE_CFGREG;     /* Write the command */
    R_QSPI->SFMCOM        = (uint32_t)regval;                       /* Write the volatile configuration register */
    R_QSPI->SFMCMD_b.DCOM = 1U;                                     /* Close the SPI bus cycle */

    /* Wait for the write to complete */
    do
    {
        bsp_qspi_status_get(&write_in_progress);
    } while (write_in_progress);

    /* disable writing */
    R_QSPI->SFMCOM = QSPI_COMMAND_WRITE_DISABLE;

    /* close the SPI bus cycle */
    R_QSPI->SFMCMD_b.DCOM = 1U;

    /* Read the flag status of the device to determine the addressing mode of the device. */
    bsp_qspi_read_status_flag_register(&flag_status);

    /* If the device is in 4-byte addressing mode then configure the QSPI block that way as well */
    if (flag_status.addressing_4_byte_mode)
    {
        R_QSPI->SFMSAC_b.SFMAS  = 3U; /* 4 byte address */
        R_QSPI->SFMSAC_b.SFM4BC = 1U; /* Select default instruction code */
    }

    /* Set the number of dummy cycles in QSPI peripheral */
    R_QSPI->SFMSDC_b.SFMDN = (uint32_t)(BSP_PRV_QSPI_N25Q256A_NUM_DUMMY_CLOCKS - 2U);

    /* Micron QSPI needs 50ns de-select  (QSSL high between cycles) for nonREAD commands */
    /* (20ns for a read command) Need 3 and a bit clock (i.e. 4) cycles at 60MHz */
    R_QSPI->SFMSSC_b.SFMSW = BSP_PRV_QSPI_N25Q256A_DE_SELECT_DELAY;

    /* Set the read mode */
    R_QSPI->SFMSMD_b.SFMRM = BSP_PRV_QSPI_READ_MODE;

#if   BSP_PRV_QSPI_ROM_PREFTECH_MODE
    R_QSPI->SFMSMD_b.SFMPFE = 1U;
#endif

#if BSP_PRV_QSPI_XIP_MODE_AFTER_INIT
    /* Enable XIP mode on the flash device */
    bsp_qspi_xip_enter();
#endif

    /* Enter ROM access mode */
    R_QSPI->SFMCMD_b.DCOM = (uint32_t)0;
}

/*******************************************************************************************************************//**
 * @brief   Enter XIP mode
 *
 * This function enters XIP mode in both the QSPI controller block and on the flash device.
 *
 **********************************************************************************************************************/
void bsp_qspi_xip_enter (void)
{
    /* Check there is no serial transfer in progress */
    while (R_QSPI->SFMCST_b.COMBSY == (uint32_t)1)
    {
    }

    /* Since the device on the S3 DK board is 25Q256A 13E40, we need first need to set the XIP bit (bit 3) */
    /* In the volatile config register to enter XIP mode */
    bsp_qspi_device_xip_mode(true);

    /* then drive the XIP confirmation bit to 0  during */
    /* the next FAST-READ cycle */
    bsp_qspi_xip_mode(true);
}

/*******************************************************************************************************************//**
 * @brief   Exit XIP mode
 *
 * This function exits XIP mode in both the QSPI controller block and on the flash device.
 *
 **********************************************************************************************************************/
void bsp_qspi_xip_exit (void)
{
    /* Check there is no serial transfer in progress */
    while (R_QSPI->SFMCST_b.COMBSY == (uint32_t)1)
    {
    }

    bsp_qspi_xip_mode(false);
}

/*******************************************************************************************************************//**
 * @brief   Get the status from the QSPI device.
 *
 * This function reads the status byte from the device and returns the write status. Used by write and erase operations
 * in the QSPI module driver.
 *
 * @param[in]  p_write_in_progress  Pointer to a boolean that indicates if a write or erase is in progress.
 *
 **********************************************************************************************************************/
void bsp_qspi_status_get (bool * p_write_in_progress)
{
    uint32_t        regval;
    n25q256a_status status;

    /* Send command to read status */
    R_QSPI->SFMCOM = QSPI_COMMAND_READ_STATUS_REGISTER;

    /* Read the device status register */
    regval = R_QSPI->SFMCOM_b.SFMD;

    /* Close the SPI bus cycle */
    R_QSPI->SFMCMD_b.DCOM = 1U;

    status.entire_cfg = regval;

    *p_write_in_progress  = (status.write_in_progress > 0U);
}


/*******************************************************************************************************************//**
 * @brief   Get the flags from the QSPI device.
 *
 * This function reads the flag status register from the device which indicates errors that occurred during various
 * operations like erasing and programming.
 *
 **********************************************************************************************************************/
void bsp_qspi_flags_get (bool * p_addressing_4_byte_mode,
                         bool * p_program_suspended,
                         bool * p_erase_failure,
                         bool * p_program_failure,
                         bool * p_protection_failure)
{
    n25q256a_flag_status flag_stat;

    bsp_qspi_read_status_flag_register(&flag_stat);

    *p_addressing_4_byte_mode = (flag_stat.addressing_4_byte_mode > 0U);
    *p_erase_failure          = (flag_stat.erase > 0U);
    *p_program_failure        = (flag_stat.program > 0U);
    *p_protection_failure     = (flag_stat.protection > 0U);
    *p_program_suspended      = (flag_stat.program_suspend > 0U);
}

/*******************************************************************************************************************//**
 * @brief   Get the current configuration of the QSPI device.
 *
 * This function reads the volatile and non-volatile registers of the device and returns portions of these to the QSPI
 * module driver for use in direct communication mode.
 *
 * @param[out] num_address_bytes      Number of bytes used for the address - 3 bytes or 4 bytes
 * @param[out] spi_mode               SPI mode used - 0 = Extended, 1 = Dual, 2 = Quad
 *
 **********************************************************************************************************************/
void bsp_qspi_config_get (uint8_t  * p_manufacturer_id,
                          uint8_t  * p_memory_type,
                          uint8_t  * p_memory_capacity,
                          uint32_t * p_max_eraseable_size,
                          uint32_t * p_num_address_bytes,
                          uint32_t * p_spi_mode,
                          uint32_t * p_page_size,
                          bool     * p_xip_mode)
{
    *p_manufacturer_id    = device_characteristics.manufacturer_id;
    *p_memory_type        = device_characteristics.memory_type;
    *p_memory_capacity    = device_characteristics.memory_capacity;
    *p_max_eraseable_size = 4U; /* 4k bytes */
    *p_num_address_bytes  = (uint32_t)(flag_status.addressing_4_byte_mode + 3U);
    *p_spi_mode           = nv_cfg.quad_io_protocol ? 2U : nv_cfg.dual_io_protocol ? 1U : 0U;
    *p_page_size          = BSP_PRV_QSPI_N25Q256A_PAGE_SIZE;
#if BSP_PRV_QSPI_XIP_MODE_AFTER_INIT
    *p_xip_mode           = true;
#else
    *p_xip_mode           = false;
#endif
}
/*******************************************************************************************************************//**
 * @brief   Get the supported erase sizes of Flash.
 *
 * QSPI Flash erase sizes are arranged in acceding order.Function returns the address of the ordered array of
 * erase size to upper layer.
 *
 **********************************************************************************************************************/
void bsp_qspi_erase_sizes_get(uint32_t ** pp_sizes, uint8_t *p_len)
{
    *pp_sizes = &n25q256a_flash_erase_sizes[0];

    *p_len = (uint8_t)(sizeof(n25q256a_flash_erase_sizes)/sizeof(n25q256a_flash_erase_sizes[0]));
}
/*******************************************************************************************************************//**
 * @brief   Get erase command based on index value.
 *
 * QSPI Flash erase size commands are arranged in a acceding order.
 *
 **********************************************************************************************************************/
void bsp_qspi_erase_command_get(uint8_t *p_erase_command, uint8_t index)
{
    if(4U == (flag_status.addressing_4_byte_mode + 3U))
    {
        *p_erase_command = n25q256a_4byte_erase_commands[index];
    }
    else
    {
        *p_erase_command = n25q256a_erase_commands[index];
    }
}
#endif /* if defined(BSP_BOARD_S3A7_DK) */

/** @} (end addtogroup BSP_DK2M_QSPI) */
