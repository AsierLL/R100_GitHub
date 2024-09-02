/* generated common header file - do not edit */
#ifndef COMMON_DATA_H_
#define COMMON_DATA_H_
#include <stdint.h>
#include "bsp_api.h"
#include "fx_api.h"
#include "r_dmac.h"
#include "r_transfer_api.h"
#include "r_sdmmc.h"
#include "r_sdmmc_api.h"
#include "sf_block_media_sdmmc.h"
#include "sf_block_media_api.h"
#include "sf_el_fx.h"
#include "fx_api.h"
#include "r_rspi.h"
#include "r_spi_api.h"
#include "r_cgc_api.h"
#include "r_spi_api.h"
#include "sf_spi.h"
#include "sf_spi_api.h"
#include "r_elc.h"
#include "r_elc_api.h"
#include "r_fmi.h"
#include "r_fmi_api.h"
#include "r_ioport.h"
#include "r_ioport_api.h"
#include "r_cgc.h"
#include "r_cgc_api.h"
#ifdef __cplusplus
extern "C"
{
#endif
void fx_common_init0(void);
/* Transfer on DMAC Instance. */
extern const transfer_instance_t g_transfer;
#ifndef NULL
void NULL(transfer_callback_args_t *p_args);
#endif
/** SDMMC on SDMMC Instance. */
extern const sdmmc_instance_t g_sdmmc;
#ifndef NULL
void NULL(sdmmc_callback_args_t *p_args);
#endif
/** Block Media on SDMMC Instance */
extern sf_block_media_instance_t g_sf_block_media_sdmmc0;
extern sf_el_fx_t g_sf_el_fx_cfg;
extern FX_MEDIA sd_fx_media;

void sd_fx_media_err_callback(void *p_instance, void *p_data);
ssp_err_t fx_media_init0_format(void);
uint32_t fx_media_init0_open(void);
void fx_media_init0(void);
/** SPI config */
extern const spi_cfg_t g_spi0_cfg;
/** RSPI extended config */
extern const spi_on_rspi_cfg_t g_spi0_ext_cfg;
/** SPI on RSPI Instance. */
extern const spi_instance_t g_spi0;
/** SPI instance control */
extern rspi_instance_ctrl_t g_spi0_ctrl;
#ifndef NULL
void NULL(spi_callback_args_t *p_args);
#endif

extern const transfer_instance_t g_spi0_transfer_rx;
extern const transfer_instance_t g_spi0_transfer_tx;

#define SYNERGY_NOT_DEFINED (1)
#define RSPI_TRANSFER_SIZE_1_BYTE (0x52535049)
#if (RSPI_TRANSFER_SIZE_1_BYTE == RSPI_SYNERGY_NOT_DEFINED)

#define g_spi0_P_TRANSFER_TX (&g_spi0_transfer_tx)
#define g_spi0_P_TRANSFER_RX (&g_spi0_transfer_rx)

#else
#if (SYNERGY_NOT_DEFINED == SYNERGY_NOT_DEFINED)
#define g_spi0_P_TRANSFER_TX (NULL)
#else
#define g_spi0_P_TRANSFER_TX (&SYNERGY_NOT_DEFINED)
#endif
#if (SYNERGY_NOT_DEFINED == SYNERGY_NOT_DEFINED)
#define g_spi0_P_TRANSFER_RX (NULL)
#else
#define g_spi0_P_TRANSFER_RX (&SYNERGY_NOT_DEFINED)
#endif
#endif
#undef RSPI_TRANSFER_SIZE_1_BYTE
#undef SYNERGY_NOT_DEFINED
#define g_spi0_P_EXTEND (&g_spi0_ext_cfg)
extern sf_spi_bus_t spi_bus0;
extern spi_api_t const g_spi_on_rspi;

#define spi_bus0_CHANNEL        (0)
#define spi_bus0_OPERATING_MODE (SPI_MODE_MASTER)
#define spi_bus0_CLK_PHASE      (SPI_CLK_PHASE_EDGE_ODD)
#define spi_bus0_CLK_POLARITY   (SPI_CLK_POLARITY_LOW)          
#define spi_bus0_MODE_FAULT     (SPI_MODE_FAULT_ERROR_DISABLE)
#define spi_bus0_BIT_ORDER      (SPI_BIT_ORDER_MSB_FIRST)          
#define spi_bus0_BIT_RATE       (800000)  
#define spi_bus0_P_CALLBACK     (NULL)
#define spi_bus0_P_CONTEXT      (&g_spi0)
#define spi_bus0_RXI_IPL        ((4))
#define spi_bus0_TXI_IPL        ((4))
#define spi_bus0_TEI_IPL        ((4))            
#define spi_bus0_ERI_IPL        ((4))

/** These are obtained by macros in the SPI driver XMLs. */
#define spi_bus0_P_TRANSFER_TX  (g_spi0_P_TRANSFER_TX)
#define spi_bus0_P_TRANSFER_RX  (g_spi0_P_TRANSFER_RX)            
#define spi_bus0_P_EXTEND       (g_spi0_P_EXTEND)

#include "ux_api.h"

/* USBX Host Stack initialization error callback function. User can override the function if needed. */
void ux_v2_err_callback(void *p_instance, void *p_data);

#if !defined(NULL)
/* User Callback for Host Event Notification (Only valid for USB Host). */
extern UINT NULL(ULONG event, UX_HOST_CLASS *host_class, VOID *instance);
#endif

#ifdef UX_HOST_CLASS_STORAGE_H
/* Utility function to get the pointer to a FileX Media Control Block for a USB Mass Storage device. */
UINT ux_system_host_storage_fx_media_get(UX_HOST_CLASS_STORAGE * instance, UX_HOST_CLASS_STORAGE_MEDIA ** p_storage_media, FX_MEDIA ** p_fx_media);
#endif
void ux_common_init0(void);
#include "ux_api.h"
#include "ux_dcd_synergy.h"
#include "sf_el_ux_dcd_fs_cfg.h"
void g_sf_el_ux_dcd_fs_0_err_callback(void *p_instance, void *p_data);
#include "ux_api.h"
#include "ux_dcd_synergy.h"

/* USBX Device Stack initialization error callback function. User can override the function if needed. */
void ux_device_err_callback(void *p_instance, void *p_data);
void ux_device_init0(void);
#include "ux_api.h"
#include "ux_device_class_storage.h"
/* USBX Mass Storage Class User Media Setup Callback Function */
extern void ux_device_class_storage_user_setup(UX_SLAVE_CLASS_STORAGE_PARAMETER *param);
/* USBX Mass Storage Class Media Read User Callback Function */
extern UINT ux_device_msc_media_read(VOID *storage, ULONG lun, UCHAR *data_pointer, ULONG number_blocks, ULONG lba,
        ULONG *media_status);
/* USBX Mass Storage Class Media Write User Callback Function */
extern UINT ux_device_msc_media_write(VOID *storage, ULONG lun, UCHAR *data_pointer, ULONG number_blocks, ULONG lba,
        ULONG *media_status);
/* USBX Mass Storage Class Media Status User Callback Function */
extern UINT ux_device_msc_media_status(VOID *storage, ULONG lun, ULONG media_id, ULONG *media_status);
/* USBX Mass Storage Class Media Activate/Deactivate Callback Function */
#ifndef NULL
extern VOID NULL(VOID *storage);
#endif
#ifndef NULL
extern VOID NULL(VOID *storage);
#endif
/* USBX Mass Storage Class Parameter Setup Function */
void ux_device_class_storage_init0(void);
/** ELC Instance */
extern const elc_instance_t g_elc;
/** FMI on FMI Instance. */
extern const fmi_instance_t g_fmi;
/** IOPORT Instance */
extern const ioport_instance_t g_ioport;
/** CGC Instance */
extern const cgc_instance_t g_cgc;
void g_common_init(void);
#ifdef __cplusplus
} /* extern "C" */
#endif
#endif /* COMMON_DATA_H_ */
