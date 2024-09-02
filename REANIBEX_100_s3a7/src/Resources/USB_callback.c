/******************************************************************************
 * Name      : R100_S3A7                                                      *
 * Copyright : Osatu S. Coop                                                  *
 * Compiler  : GNU ARM Embedded                                               *
 * Target    : Reanibex Serie 100                                             *
 ******************************************************************************/

/*!
 * @file        USB_callback.c
 * @brief       All functions related to manage USB and SD storage
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

#include "bsp_api.h"
#include "ux_api.h"

#include "types_basic.h"
#include "common_data.h"

/******************************************************************************
 ** Macros
 */

#define USB_ACCESS_KEY      "ESTEFANYDAT"     ///< Key to unlock the access to uSD through USB port

#define uSD_LOCKED          1
#define uSD_UNLOCKED        0
#define KEY_SIZE            12

/******************************************************************************
 ** Defines
 */

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

// uint8_t access_key [12] = USB_ACCESS_KEY;           // access key
extern uint8_t      access_key[KEY_SIZE];

/******************************************************************************
 ** Locals
 */

static  FX_MEDIA   *sd_media = (FX_MEDIA *)NULL;    ///< handler of the SD media card
static  uint8_t     uSD_state = 3;                  ///< enable a number of writing operations before lock the device
                                                    ///< 1 --> locked device
                                                    ///< 0 --> unlocked device

/******************************************************************************
 ** Prototypes
 */

/******************************************************************************
 ** Name:    ux_device_msc_media_status
 *****************************************************************************/
/**
 ** @brief   USBX media status callback function
 **
 ** @param   storage        instance of the storage class
 ** @param   lun            LUN (logical unit number)
 ** @param   media_id       is not currently used (always zero)
 ** @param   media_status   must be filled out exactly like the media status callback return value
 **
 ** @return  uint           media status error code (see USBX Device Stack User Guide)
 **
 ******************************************************************************/
UINT ux_device_msc_media_status(VOID*  storage,
                                ULONG  lun,
                                ULONG  media_id,
                                ULONG* media_status)
{
    /* Suppress warnings. */
    SSP_PARAMETER_NOT_USED(storage);
    SSP_PARAMETER_NOT_USED(lun);
    SSP_PARAMETER_NOT_USED(media_id);

    /* The ATA drive never fails. This is just for demo only !!!! */
    *media_status = UX_SUCCESS;
    return (UX_SUCCESS);
}

/******************************************************************************
 ** Name:    ux_device_msc_media_read
 *****************************************************************************/
/**
 ** @brief   USBX read media callback function
 **
 ** @param   storage        instance of the storage class
 ** @param   lun            LUN (logical unit number)
 ** @param   data_pointer   address of the buffer to be used for reading
 ** @param   number_blocks  the number of sectors to read/write
 ** @param   lba            the sector address to read
 ** @param   media_status   must be filled out exactly like the media status callback return value
 **
 ** @return  uint           UX_SUCCESS or UX_ERROR indicating the operation result
 **
 ******************************************************************************/
UINT ux_device_msc_media_read(VOID* storage,
                                ULONG  lun,
                                UCHAR* data_pointer,
                                ULONG  number_blocks,
                                ULONG  lba,
                                ULONG* media_status)
{
    UINT    fx_res;
    ULONG   i;

    UNUSED(storage);
    UNUSED(lun);

    sd_media = &sd_fx_media;

    /* return the sectors requested by the host */
    for (i=0; i<number_blocks; i++, lba++)
    {
        fx_res = fx_media_read (sd_media, lba, &data_pointer[i*512]);
        if (fx_res != FX_SUCCESS)
        {
            fx_media_flush (sd_media);
            *media_status = UX_ERROR;
            return UX_ERROR;
        }
    }

    // successful operation
    *media_status = UX_SUCCESS;
    return (UX_SUCCESS);
}

/******************************************************************************
 ** Name:    ux_device_msc_media_write
 *****************************************************************************/
/**
 ** @brief   USBX write media callback function
 **
 ** @param   storage        instance of the storage class
 ** @param   lun            LUN (logical unit number)
 ** @param   data_pointer   address of the buffer to be used for writing
 ** @param   number_blocks  the number of sectors to read/write
 ** @param   lba            the sector address to read
 ** @param   media_status   must be filled out exactly like the media status callback return value
 **
 ** @return  uint           UX_SUCCESS or UX_ERROR indicating the operation result
 **
 ******************************************************************************/
UINT ux_device_msc_media_write(VOID* storage,
                                ULONG  lun,
                                UCHAR* data_pointer,
                                ULONG  number_blocks,
                                ULONG  lba,
                                ULONG* media_status)
{
    UINT    fx_res;
    ULONG   i;

    UNUSED(storage);
    UNUSED(lun);

    // 512 bytes per sector.
    // FAT32 maximum : 8 sectors per cluster × 268,435,444 clusters = 1,099,511,578,624 bytes (≈1,024 GB)
    // FAT32 = 2^28=268435456 --> 268435456/512=524288
    // Total clusters: 242304, Total sector:11515648, Sector per cluster 64
    // manage the unlock controller for the uSD device
    if (uSD_state > uSD_LOCKED)
    {
        uSD_state--;
        for (i=0; i<(number_blocks * 512); i++)
        {
            if ((memcmp (&data_pointer[i], access_key, sizeof(USB_ACCESS_KEY)-1) == 0) || (memcmp (&data_pointer[i], access_key, 8) == 0))
            {
                uSD_state = uSD_UNLOCKED;
                break;
            }
        }
    }
    if (uSD_state == uSD_LOCKED) return UX_ERROR;

    sd_media = &sd_fx_media;

    if (lba < sd_media->fx_media_reserved_sectors)
    {
        // successful operation
        *media_status = UX_SUCCESS;
        return (UX_SUCCESS);
    }

    /* write the sectors requested by the host */
    for (i=0; i<number_blocks; i++, lba++)
    {
        fx_res = fx_media_write (sd_media, lba, &data_pointer[i*512]);
        if (fx_res != FX_SUCCESS)
        {
            fx_media_flush (sd_media);
            *media_status = UX_ERROR;
            return UX_ERROR;
        }
    }

    // ... flush data to physical media
    fx_res = fx_media_flush (sd_media);
    *media_status = (fx_res == FX_SUCCESS) ? UX_SUCCESS : UX_ERROR;

    // return the operation result
    return (*media_status);
}
