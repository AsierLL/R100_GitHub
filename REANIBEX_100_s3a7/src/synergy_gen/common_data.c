/* generated common source file - do not edit */
#include "common_data.h"

#define FX_COMMON_INITIALIZE (1)
/*******************************************************************************************************************//**
 * @brief     Initialization function that the user can choose to have called automatically during thread entry.
 *            The user can call this function at a later time if desired using the prototype below.
 *            - void fx_common_init0(void)
 **********************************************************************************************************************/
void fx_common_init0(void)
{
    /** Initialize the FileX system. */
    fx_system_initialize ();
}
#if (14) != BSP_IRQ_DISABLED
#if !defined(SSP_SUPPRESS_ISR_g_transfer) && !defined(SSP_SUPPRESS_ISR_DMACELC_EVENT_ADC0_COMPARE_MATCH)
SSP_VECTOR_DEFINE_CHAN(dmac_int_isr, DMAC, INT, 2);
#endif
#endif
dmac_instance_ctrl_t g_transfer_ctrl;
transfer_info_t g_transfer_info =
{ .dest_addr_mode = TRANSFER_ADDR_MODE_FIXED, .repeat_area = TRANSFER_REPEAT_AREA_SOURCE, .irq = TRANSFER_IRQ_EACH,
  .chain_mode = TRANSFER_CHAIN_MODE_DISABLED, .src_addr_mode = TRANSFER_ADDR_MODE_INCREMENTED, .size =
          TRANSFER_SIZE_1_BYTE,
  .mode = TRANSFER_MODE_NORMAL, .p_dest = (void *) NULL, .p_src = (void const *) NULL, .num_blocks = 0, .length = 0, };
const transfer_on_dmac_cfg_t g_transfer_extend =
{ .channel = 2, };
const transfer_cfg_t g_transfer_cfg =
{ .p_info = &g_transfer_info, .activation_source = ELC_EVENT_ADC0_COMPARE_MATCH, .auto_enable = false, .p_callback =
          NULL,
  .p_context = &g_transfer, .irq_ipl = (14), .p_extend = &g_transfer_extend, };
/* Instance structure to use this module. */
const transfer_instance_t g_transfer =
{ .p_ctrl = &g_transfer_ctrl, .p_cfg = &g_transfer_cfg, .p_api = &g_transfer_on_dmac };
#if (14) != BSP_IRQ_DISABLED
#if !defined(SSP_SUPPRESS_ISR_g_sdmmc) && !defined(SSP_SUPPRESS_ISR_SDMMC0)
SSP_VECTOR_DEFINE_CHAN(sdhimmc_accs_isr, SDHIMMC, ACCS, 0);
#endif
#endif
#if (14) != BSP_IRQ_DISABLED
#if !defined(SSP_SUPPRESS_ISR_g_sdmmc) && !defined(SSP_SUPPRESS_ISR_SDMMC0)
SSP_VECTOR_DEFINE_CHAN(sdhimmc_card_isr, SDHIMMC, CARD, 0);
#endif
#endif
#if (BSP_IRQ_DISABLED) != BSP_IRQ_DISABLED
#if !defined(SSP_SUPPRESS_ISR_g_sdmmc) && !defined(SSP_SUPPRESS_ISR_SDMMC0)
SSP_VECTOR_DEFINE_CHAN(sdhimmc_dma_req_isr, SDHIMMC, DMA_REQ, 0);
#endif
#endif
sdmmc_instance_ctrl_t g_sdmmc_ctrl;

/** SDMMC extended configuration */
const sdmmc_extended_cfg_t g_sdmmc_ext_cfg_t =
{ .block_size = 512, .card_detect = SDMMC_CARD_DETECT_NONE, .write_protect = SDMMC_WRITE_PROTECT_WP, };

sdmmc_cfg_t g_sdmmc_cfg =
{ .hw =
{ .media_type = SDMMC_MEDIA_TYPE_CARD, .bus_width = SDMMC_BUS_WIDTH_4_BITS, .channel = 0, },
  .p_callback = NULL,

  .p_extend = (void *) &g_sdmmc_ext_cfg_t,
  .p_lower_lvl_transfer = &g_transfer,

  .access_ipl = (14),
  .sdio_ipl = BSP_IRQ_DISABLED, .card_ipl = (14), .dma_req_ipl = (BSP_IRQ_DISABLED), };
/* Instance structure to use this module. */
const sdmmc_instance_t g_sdmmc =
{ .p_ctrl = &g_sdmmc_ctrl, .p_cfg = &g_sdmmc_cfg, .p_api = &g_sdmmc_on_sdmmc };
static const sf_block_media_on_sdmmc_cfg_t g_sf_block_media_sdmmc0_block_media_cfg =
{ .p_lower_lvl_sdmmc = &g_sdmmc, };
static sf_block_media_sdmmc_instance_ctrl_t g_sf_block_media_sdmmc0_ctrl;
static sf_block_media_cfg_t g_sf_block_media_sdmmc0_cfg =
{ .block_size = 512, .p_extend = &g_sf_block_media_sdmmc0_block_media_cfg };

sf_block_media_instance_t g_sf_block_media_sdmmc0 =
{ .p_ctrl = &g_sf_block_media_sdmmc0_ctrl, .p_cfg = &g_sf_block_media_sdmmc0_cfg, .p_api = &g_sf_block_media_on_sdmmc };
sf_el_fx_t g_sf_el_fx_cfg =
{ .p_lower_lvl_block_media = &g_sf_block_media_sdmmc0 };
#if defined(__ICCARM__)
#define sd_fx_media_err_callback_WEAK_ATTRIBUTE
#pragma weak sd_fx_media_err_callback  = sd_fx_media_err_callback_internal
#elif defined(__GNUC__)
#define sd_fx_media_err_callback_WEAK_ATTRIBUTE   __attribute__ ((weak, alias("sd_fx_media_err_callback_internal")))
#endif
void sd_fx_media_err_callback(void *p_instance, void *p_data)
sd_fx_media_err_callback_WEAK_ATTRIBUTE;
#define SF_EL_FX_FORMAT_MEDIA_ENABLE_sd_fx_media (0)
#define SF_EL_FX_FORMAT_FULL_MEDIA_sd_fx_media (1)
#define SF_EL_FX_AUTO_INIT_sd_fx_media (0)
ssp_err_t SF_EL_FX_Get_MEDIA_Info(sf_block_media_instance_t *p_block_media, uint32_t *sector_size,
        uint32_t *sector_count);
FX_MEDIA sd_fx_media;
uint8_t g_media_memory_sd_fx_media[512];
/*******************************************************************************************************************//**
 * @brief      This is a weak example initialization error function.  It should be overridden by defining a user  function
 *             with the prototype below.
 *             - void sd_fx_media_err_callback(void * p_instance, void * p_data)
 *
 * @param[in]  p_instance arguments used to identify which instance caused the error and p_data Callback arguments used to identify what error caused the callback.
 **********************************************************************************************************************/
void sd_fx_media_err_callback_internal(void *p_instance, void *p_data);
void sd_fx_media_err_callback_internal(void *p_instance, void *p_data)
{
    /** Suppress compiler warning for not using parameters. */
    SSP_PARAMETER_NOT_USED (p_instance);
    SSP_PARAMETER_NOT_USED (p_data);

    /** An error has occurred. Please check function arguments for more information. */
    BSP_CFG_HANDLE_UNRECOVERABLE_ERROR (0);
}

ssp_err_t fx_media_init0_format(void)
{
    uint32_t fx_ret_val = FX_SUCCESS;

    uint32_t sector_size = 512;
    uint32_t sector_count = 50000;

#if SF_EL_FX_FORMAT_FULL_MEDIA_sd_fx_media
    ssp_err_t error = SF_EL_FX_Get_MEDIA_Info (g_sf_el_fx_cfg.p_lower_lvl_block_media, &sector_size, &sector_count);

    if ((error != SSP_SUCCESS) || (sector_count <= 0))
    {
        return SSP_ERR_MEDIA_FORMAT_FAILED;
    }

    sector_count -= 0;
#endif

    if (sizeof(g_media_memory_sd_fx_media) < 512)
    {
        return SSP_ERR_MEDIA_FORMAT_FAILED;
    }

    /* Format media.  */
    fx_ret_val = fx_media_format (&sd_fx_media, // Pointer to FileX media control block.
            SF_EL_FX_BlockDriver, // Driver entry
            &g_sf_el_fx_cfg, // Pointer to Block Media Driver
            g_media_memory_sd_fx_media, // Media buffer pointer
            sizeof(g_media_memory_sd_fx_media), // Media buffer size
            (CHAR *) "Volume 1", // Volume Name
            1, // Number of FATs
            256, // Directory Entries
            0, // Hidden sectors
            sector_count, // Total sectors - Hidden Sectors
            sector_size, // Sector size
            1, // Sectors per cluster
            1, // Heads
            1); // Sectors per track

    if (FX_SUCCESS != fx_ret_val)
    {
        return SSP_ERR_MEDIA_FORMAT_FAILED;
    }

    return SSP_SUCCESS;
}

uint32_t fx_media_init0_open(void)
{
    return fx_media_open (&sd_fx_media, (CHAR *) "sd_fx_media", SF_EL_FX_BlockDriver, &g_sf_el_fx_cfg,
                          g_media_memory_sd_fx_media, sizeof(g_media_memory_sd_fx_media));
}

/*******************************************************************************************************************//**
 * @brief     Initialization function that the user can choose to have called automatically during thread entry.
 *            The user can call this function at a later time if desired using the prototype below.
 *            - void fx_media_init0(void)
 **********************************************************************************************************************/
void fx_media_init0(void)
{
#if SF_EL_FX_FORMAT_MEDIA_ENABLE_sd_fx_media

    ssp_err_t err_format = fx_media_init0_format();

    if(err_format != SSP_SUCCESS)
    {
        sd_fx_media_err_callback((void *)&sd_fx_media,&err_format);
    }

#endif

    uint32_t err_open = fx_media_init0_open ();

    if (err_open != FX_SUCCESS)
    {
        sd_fx_media_err_callback ((void *) &sd_fx_media, &err_open);
    }
}
#define RSPI_TRANSFER_SIZE_1_BYTE (0x52535049)
#define RSPI_SYNERGY_NOT_DEFINED 1
#if (RSPI_SYNERGY_NOT_DEFINED != RSPI_SYNERGY_NOT_DEFINED)
dtc_instance_ctrl_t g_spi0_transfer_tx_ctrl;
uint32_t g_spi0_tx_inter = 0;
transfer_info_t g_spi0_transfer_tx_info[2] =
{
#if (RSPI_TRANSFER_SIZE_1_BYTE == RSPI_SYNERGY_NOT_DEFINED)
    {
        .dest_addr_mode = TRANSFER_ADDR_MODE_FIXED,
        .repeat_area = TRANSFER_REPEAT_AREA_SOURCE,
        .irq = TRANSFER_IRQ_END,
        .chain_mode = TRANSFER_CHAIN_MODE_EACH,
        .src_addr_mode = TRANSFER_ADDR_MODE_INCREMENTED,
        .size = TRANSFER_SIZE_1_BYTE,
        .mode = TRANSFER_MODE_NORMAL,
        .p_dest = (void *) &g_spi0_tx_inter,
        .p_src = (void const *) NULL,
        .num_blocks = 0,
        .length = 0,
    },
    {
        .dest_addr_mode = TRANSFER_ADDR_MODE_FIXED,
        .repeat_area = TRANSFER_REPEAT_AREA_SOURCE,
        .irq = TRANSFER_IRQ_END,
        .chain_mode = TRANSFER_CHAIN_MODE_DISABLED,
        .src_addr_mode = TRANSFER_ADDR_MODE_FIXED,
        .size = TRANSFER_SIZE_4_BYTE,
        .mode = TRANSFER_MODE_NORMAL,
        .p_dest = (void *) NULL,
        .p_src = (void const *) &g_spi0_tx_inter,
        .num_blocks = 0,
        .length = 0,
    },
#else
    {
        .dest_addr_mode = TRANSFER_ADDR_MODE_FIXED,
        .repeat_area = TRANSFER_REPEAT_AREA_SOURCE,
        .irq = TRANSFER_IRQ_END,
        .chain_mode = TRANSFER_CHAIN_MODE_DISABLED,
        .src_addr_mode = TRANSFER_ADDR_MODE_INCREMENTED,
        .size =SYNERGY_NOT_DEFINED,
        .mode = TRANSFER_MODE_NORMAL,
        .p_dest = (void *) NULL,
        .p_src = (void const *) NULL,
        .num_blocks = 0,
        .length = 0,
    },
    {
        .dest_addr_mode = TRANSFER_ADDR_MODE_INCREMENTED,
        .repeat_area = TRANSFER_REPEAT_AREA_SOURCE,
        .irq = TRANSFER_IRQ_END,
        .chain_mode = TRANSFER_CHAIN_MODE_DISABLED,
        .src_addr_mode = TRANSFER_ADDR_MODE_FIXED,
        .size = TRANSFER_SIZE_1_BYTE,
        .mode = TRANSFER_MODE_NORMAL,
        .p_dest =(void *) NULL,
        .p_src = (void const *) &g_spi0_tx_inter,
        .num_blocks = 0,
        .length = 0,
    },
#endif
};

const transfer_cfg_t g_spi0_transfer_tx_cfg =
{
    .p_info = g_spi0_transfer_tx_info,
    .activation_source = SYNERGY_NOT_DEFINED,
    .auto_enable = false,
    .p_callback = NULL,
    .p_context = &g_spi0_transfer_tx,
    .irq_ipl = SYNERGY_NOT_DEFINED
};
/* Instance structure to use this module. */
const transfer_instance_t g_spi0_transfer_tx =
{
    .p_ctrl = &g_spi0_transfer_tx_ctrl,
    .p_cfg = &g_spi0_transfer_tx_cfg,
    .p_api = &g_transfer_on_dtc
};

dtc_instance_ctrl_t g_spi0_transfer_rx_ctrl;
uint32_t g_spi0_rx_inter = 0;
transfer_info_t g_spi0_transfer_rx_info[2] =
{
#if (RSPI_TRANSFER_SIZE_1_BYTE == RSPI_SYNERGY_NOT_DEFINED)
    {
        .dest_addr_mode = TRANSFER_ADDR_MODE_FIXED,
        .repeat_area = TRANSFER_REPEAT_AREA_SOURCE,
        .irq = TRANSFER_IRQ_END,
        .chain_mode = TRANSFER_CHAIN_MODE_EACH,
        .src_addr_mode = TRANSFER_ADDR_MODE_FIXED,
        .size = TRANSFER_SIZE_4_BYTE,
        .mode = TRANSFER_MODE_NORMAL,
        .p_dest = (void *) &g_spi0_rx_inter,
        .p_src = (void const *) NULL,
        .num_blocks = 0,
        .length = 0,
    },
    {
        .dest_addr_mode = TRANSFER_ADDR_MODE_INCREMENTED,
        .repeat_area = TRANSFER_REPEAT_AREA_SOURCE,
        .irq = TRANSFER_IRQ_END,
        .chain_mode = TRANSFER_CHAIN_MODE_DISABLED,
        .src_addr_mode = TRANSFER_ADDR_MODE_FIXED,
        .size = TRANSFER_SIZE_1_BYTE,
        .mode = TRANSFER_MODE_NORMAL,
        .p_dest = (void *) NULL,
        .p_src = (void const *) &g_spi0_rx_inter,
        .num_blocks = 0,
        .length = 0,
    },
#else
    {
        .dest_addr_mode = TRANSFER_ADDR_MODE_INCREMENTED,
        .repeat_area = TRANSFER_REPEAT_AREA_DESTINATION,
        .irq = TRANSFER_IRQ_END,
        .chain_mode = TRANSFER_CHAIN_MODE_DISABLED,
        .src_addr_mode = TRANSFER_ADDR_MODE_FIXED,
        .size = SYNERGY_NOT_DEFINED,
        .mode = TRANSFER_MODE_NORMAL,
        .p_dest = (void *) NULL,
        .p_src = (void const *) NULL,
        .num_blocks = 0,
    },
    {
        .dest_addr_mode = TRANSFER_ADDR_MODE_INCREMENTED,
        .repeat_area = TRANSFER_REPEAT_AREA_SOURCE,
        .irq = TRANSFER_IRQ_END,
        .chain_mode = TRANSFER_CHAIN_MODE_DISABLED,
        .src_addr_mode = TRANSFER_ADDR_MODE_FIXED,
        .size = TRANSFER_SIZE_1_BYTE,
        .mode = TRANSFER_MODE_NORMAL,
        .p_dest = (void *) NULL,
        .p_src = (void const *) &g_spi0_rx_inter,
        .num_blocks = 0,
        .length = 0,
    },
#endif
};

const transfer_cfg_t g_spi0_transfer_rx_cfg =
{
    .p_info = g_spi0_transfer_rx_info,
    .activation_source = SYNERGY_NOT_DEFINED,
    .auto_enable = false,
    .p_callback = NULL,
    .p_context = &g_spi0_transfer_rx,
    .irq_ipl = SYNERGY_NOT_DEFINED
};
/* Instance structure to use this module. */
const transfer_instance_t g_spi0_transfer_rx =
{
    .p_ctrl = &g_spi0_transfer_rx_ctrl,
    .p_cfg = &g_spi0_transfer_rx_cfg,
    .p_api = &g_transfer_on_dtc
};
#endif
#undef RSPI_TRANSFER_SIZE_1_BYTE	
#undef RSPI_SYNERGY_NOT_DEFINED

#if !defined(SSP_SUPPRESS_ISR_g_spi0) && !defined(SSP_SUPPRESS_ISR_SPI0)
SSP_VECTOR_DEFINE_CHAN(spi_rxi_isr, SPI, RXI, 0);
#endif
#if !defined(SSP_SUPPRESS_ISR_g_spi0) && !defined(SSP_SUPPRESS_ISR_SPI0)
SSP_VECTOR_DEFINE_CHAN(spi_txi_isr, SPI, TXI, 0);
#endif
#if !defined(SSP_SUPPRESS_ISR_g_spi0) && !defined(SSP_SUPPRESS_ISR_SPI0)
SSP_VECTOR_DEFINE_CHAN(spi_eri_isr, SPI, ERI, 0);
#endif
#if !defined(SSP_SUPPRESS_ISR_g_spi0) && !defined(SSP_SUPPRESS_ISR_SPI0)
SSP_VECTOR_DEFINE_CHAN(spi_tei_isr, SPI, TEI, 0);
#endif
rspi_instance_ctrl_t g_spi0_ctrl;

/** RSPI extended configuration for RSPI HAL driver */
const spi_on_rspi_cfg_t g_spi0_ext_cfg =
        { .rspi_clksyn = RSPI_OPERATION_SPI,
        /* Communication mode is configured by the driver. write calls use TX_ONLY. read and writeRead use FULL_DUPLEX. */
        .rspi_comm = RSPI_COMMUNICATION_FULL_DUPLEX,
          .ssl_polarity.rspi_ssl0 = RSPI_SSLP_LOW, .ssl_polarity.rspi_ssl1 = RSPI_SSLP_LOW, .ssl_polarity.rspi_ssl2 =
                  RSPI_SSLP_LOW,
          .ssl_polarity.rspi_ssl3 = RSPI_SSLP_LOW, .loopback.rspi_loopback1 = RSPI_LOOPBACK1_NORMAL_DATA,
          .loopback.rspi_loopback2 = RSPI_LOOPBACK2_NORMAL_DATA, .mosi_idle.rspi_mosi_idle_fixed_val =
                  RSPI_MOSI_IDLE_FIXED_VAL_LOW,
          .mosi_idle.rspi_mosi_idle_val_fixing = RSPI_MOSI_IDLE_VAL_FIXING_DISABLE, .parity.rspi_parity =
                  RSPI_PARITY_STATE_DISABLE,
          .parity.rspi_parity_mode = RSPI_PARITY_MODE_ODD, .ssl_select = RSPI_SSL_SELECT_SSL0, .ssl_level_keep =
                  RSPI_SSL_LEVEL_KEEP,
          .clock_delay.rspi_clock_delay_count = RSPI_CLOCK_DELAY_COUNT_1, .clock_delay.rspi_clock_delay_state =
                  RSPI_CLOCK_DELAY_STATE_DISABLE,
          .ssl_neg_delay.rspi_ssl_neg_delay_count = RSPI_SSL_NEGATION_DELAY_1, .ssl_neg_delay.rspi_ssl_neg_delay_state =
                  RSPI_SSL_NEGATION_DELAY_DISABLE,
          .access_delay.rspi_next_access_delay_count = RSPI_NEXT_ACCESS_DELAY_COUNT_1,
          .access_delay.rspi_next_access_delay_state = RSPI_NEXT_ACCESS_DELAY_STATE_DISABLE, .byte_swap =
                  RSPI_BYTE_SWAP_DISABLE, };

const spi_cfg_t g_spi0_cfg =
{ .channel = 0, .operating_mode = SPI_MODE_MASTER, .clk_phase = SPI_CLK_PHASE_EDGE_ODD, .clk_polarity =
          SPI_CLK_POLARITY_LOW,
  .mode_fault = SPI_MODE_FAULT_ERROR_DISABLE, .bit_order = SPI_BIT_ORDER_MSB_FIRST, .bitrate = 800000, .p_transfer_tx =
          g_spi0_P_TRANSFER_TX,
  .p_transfer_rx = g_spi0_P_TRANSFER_RX, .p_callback = NULL, .p_context = (void *) &g_spi0, .p_extend =
          (void *) &g_spi0_ext_cfg,
  .rxi_ipl = (4), .txi_ipl = (4), .eri_ipl = (4), .tei_ipl = (4), };
/* Instance structure to use this module. */
const spi_instance_t g_spi0 =
{ .p_ctrl = &g_spi0_ctrl, .p_cfg = &g_spi0_cfg, .p_api = &g_spi_on_rspi };
static TX_MUTEX sf_bus_mutex_spi_bus0;
static TX_EVENT_FLAGS_GROUP sf_bus_eventflag_spi_bus0;
static sf_spi_ctrl_t *p_sf_curr_ctrl_spi_bus0;

sf_spi_bus_t spi_bus0 =
{ .p_bus_name = (uint8_t *) "spi_bus0", .channel = 0, .freq_hz_min = 0, .p_lock_mutex = &sf_bus_mutex_spi_bus0,
  .p_sync_eventflag = &sf_bus_eventflag_spi_bus0, .pp_curr_ctrl = (sf_spi_ctrl_t **) &p_sf_curr_ctrl_spi_bus0,
  .p_lower_lvl_api = (spi_api_t *) &g_spi_on_rspi, .device_count = 0, };
/***********************************************************************************************************************
 * USB Mass Storage Class Interface Descriptor  for FS mode g_usb_interface_descriptor_storage_0
 **********************************************************************************************************************/
#if defined(__GNUC__)
static volatile const unsigned char g_usb_interface_descriptor_storage_0_full_speed[] BSP_PLACE_IN_SECTION_V2(".usb_interface_desc_fs") BSP_ALIGN_VARIABLE_V2(1)
#else /* __ICCARM__ */
#pragma section = ".usb_interface_desc_fs" 1
__root static const unsigned char g_usb_interface_descriptor_storage_0_full_speed[] BSP_PLACE_IN_SECTION_V2(".usb_interface_desc_fs")
#endif
=
{
    /***********************************************************************
     * Mass Storage Class Interface Descriptor                             *
     ***********************************************************************/
    0x09, /* 0 bLength */
    UX_INTERFACE_DESCRIPTOR_ITEM, /* 1 bDescriptorType */
    0x00, /* 2 bInterfaceNumber */
    0x00, /* 3 bAlternateSetting  : Alternate for SetInterface Request */
    0x02, /* 4 bNumEndpoints      : 2 Endpoints for Interface#1 */
    0x08, /* 5 bInterfaceClass    : Mass Storage Class(0x8) */
    0x06, /* 6 bInterfaceSubClass : SCSI transparent command set(0x6) */
    0x50, /* 7 bInterfaceProtocol : BBB(0x50) */
    0x00, /* 8 iInterface Index */
    /***********************************************************************
     * Mass Storage Class Endpoint Descriptor (Bulk-Out)  for Interface#0x00 *
     ***********************************************************************/
    0x07, /* 0 bLength */
    UX_ENDPOINT_DESCRIPTOR_ITEM, /* 1 bDescriptorType */
    (UX_ENDPOINT_OUT | 1), /* 2 bEndpointAddress */
    UX_BULK_ENDPOINT, /* 3 bmAttributes  */
    0x40, /* 4 wMaxPacketSize 64bytes */
    0x00, /* 5 wMaxPacketSize */
    0x00, /* 6 bInterval */
    /***********************************************************************
     * Mass Storage Class Endpoint Descriptor (Bulk-In) for Interface#0x00 *
     ***********************************************************************/
    0x07, /* 0 bLength */
    UX_ENDPOINT_DESCRIPTOR_ITEM, /* 1 bDescriptorType */
    (UX_ENDPOINT_IN | 2), /* 2 bEndpointAddress */
    UX_BULK_ENDPOINT, /* 3 bmAttributes  */
    0x40, /* 4 wMaxPacketSize 64bytes */
    0x00, /* 5 wMaxPacketSize */
    0x00, /* 6 bInterval */
};

#if defined(UX_DCD_SYNERY_USE_USBHS)
/***********************************************************************************************************************
 * USB Mass Storage Class Interface Descriptor for HS mode g_usb_interface_descriptor_storage_0
 **********************************************************************************************************************/
#if defined(__GNUC__)
static volatile const unsigned char g_usb_interface_descriptor_storage_0_high_speed[] BSP_PLACE_IN_SECTION_V2(".usb_interface_desc_hs") BSP_ALIGN_VARIABLE_V2(1)
#else /* __ICCARM__ */
#pragma section = ".usb_interface_desc_hs" 1
__root static const unsigned char g_usb_interface_descriptor_storage_0_high_speed[] BSP_PLACE_IN_SECTION_V2(".usb_interface_desc_hs")
#endif
=
{
    /***********************************************************************
     * Mass Storage Class Interface Descriptor                             *
     ***********************************************************************/
    0x09, /* 0 bLength */
    UX_INTERFACE_DESCRIPTOR_ITEM, /* 1 bDescriptorType */
    0x00, /* 2 bInterfaceNumber */
    0x00, /* 3 bAlternateSetting  : Alternate for SetInterface Request */
    0x02, /* 4 bNumEndpoints      : 2 Endpoints for Interface#1 */
    0x08, /* 5 bInterfaceClass    : Mass Storage Class(0x8) */
    0x06, /* 6 bInterfaceSubClass : SCSI transparent command set(0x6) */
    0x50, /* 7 bInterfaceProtocol : BBB(0x50) */
    0x00, /* 8 iInterface Index */
    /***********************************************************************
     * Mass Storage Class Endpoint Descriptor (Bulk-Out)  for Interface#0x00 *
     ***********************************************************************/
    0x07, /* 0 bLength */
    UX_ENDPOINT_DESCRIPTOR_ITEM, /* 1 bDescriptorType */
    (UX_ENDPOINT_OUT | 1), /* 2 bEndpointAddress */
    UX_BULK_ENDPOINT, /* 3 bmAttributes  */
    0x00, /* 4 wMaxPacketSize : 512bytes */
    0x02, /* 5 wMaxPacketSize */
    0x00, /* 6 bInterval */
    /***********************************************************************
     * Mass Storage Class Endpoint Descriptor (Bulk-In) for Interface#0x00 *
     ***********************************************************************/
    0x07, /* 0 bLength */
    UX_ENDPOINT_DESCRIPTOR_ITEM, /* 1 bDescriptorType */
    (UX_ENDPOINT_IN | 2), /* 2 bEndpointAddress */
    UX_BULK_ENDPOINT, /* 3 bmAttributes  */
    0x00, /* 4 wMaxPacketSize : 512bytes */
    0x02, /* 5 wMaxPacketSize */
    0x00, /* 6 bInterval */
};
#endif

/* Size of this USB Interface Descriptor */
#define USB_IFD_SIZE_0x00       (sizeof(g_usb_interface_descriptor_storage_0_full_speed))
/* Number of Interface this USB Interface Descriptor has */
#define USB_IFD_NUM_0x00        (1)
#ifndef USB_IFD_NUM_0
#define USB_IFD_NUM_0  USB_IFD_NUM_0x00
#define USB_IFD_SIZE_0 USB_IFD_SIZE_0x00
#else
#ifndef USB_IFD_NUM_1
#define USB_IFD_NUM_1  USB_IFD_NUM_0x00
#define USB_IFD_SIZE_1 USB_IFD_SIZE_0x00
#else
#ifndef USB_IFD_NUM_2
#define USB_IFD_NUM_2  USB_IFD_NUM_0x00
#define USB_IFD_SIZE_2 USB_IFD_SIZE_0x00
#else
#ifndef USB_IFD_NUM_3
#define USB_IFD_NUM_3  USB_IFD_NUM_0x00
#define USB_IFD_SIZE_3 USB_IFD_SIZE_0x00
#else
#ifndef USB_IFD_NUM_4
#define USB_IFD_NUM_4  USB_IFD_NUM_0x00
#define USB_IFD_SIZE_4 USB_IFD_SIZE_0x00
#else
#ifndef USB_IFD_NUM_5
#define USB_IFD_NUM_5  USB_IFD_NUM_0x00
#define USB_IFD_SIZE_5 USB_IFD_SIZE_0x00
#else
#ifndef USB_IFD_NUM_6
#define USB_IFD_NUM_6  USB_IFD_NUM_0x00
#define USB_IFD_SIZE_6 USB_IFD_SIZE_0x00
#else
#ifndef USB_IFD_NUM_7
#define USB_IFD_NUM_7  USB_IFD_NUM_0x00
#define USB_IFD_SIZE_7 USB_IFD_SIZE_0x00
#else
#ifndef USB_IFD_NUM_8
#define USB_IFD_NUM_8  USB_IFD_NUM_0x00
#define USB_IFD_SIZE_8 USB_IFD_SIZE_0x00
#else
#ifndef USB_IFD_NUM_9
#define USB_IFD_NUM_9  USB_IFD_NUM_0x00
#define USB_IFD_SIZE_9 USB_IFD_SIZE_0x00
#endif 
#endif
#endif
#endif
#endif
#endif
#endif
#endif
#endif
#endif
#if defined(__ICCARM__)
#define ux_v2_err_callback_WEAK_ATTRIBUTE
#pragma weak ux_v2_err_callback  = ux_v2_err_callback_internal
#elif defined(__GNUC__)
#define ux_v2_err_callback_WEAK_ATTRIBUTE   __attribute__ ((weak, alias("ux_v2_err_callback_internal")))
#endif
void ux_v2_err_callback(void *p_instance, void *p_data)
ux_v2_err_callback_WEAK_ATTRIBUTE;
/* Static memory pool allocation used by USBX system. */
static CHAR g_ux_pool_memory[18432];
/*******************************************************************************************************************//**
 * @brief      This is a weak example initialization error function.  It should be overridden by defining a user  function 
 *             with the prototype below.
 *             - void ux_v2_err_callback(void * p_instance, void * p_data)
 *
 * @param[in]  p_instance arguments used to identify which instance caused the error and p_data Callback arguments used to identify what error caused the callback.
 **********************************************************************************************************************/
void ux_v2_err_callback_internal(void *p_instance, void *p_data);
void ux_v2_err_callback_internal(void *p_instance, void *p_data)
{
    /** Suppress compiler warning for not using parameters. */
    SSP_PARAMETER_NOT_USED (p_instance);
    SSP_PARAMETER_NOT_USED (p_data);

    /** An error has occurred. Please check function arguments for more information. */
    BSP_CFG_HANDLE_UNRECOVERABLE_ERROR (0);
}

#ifdef UX_HOST_INITIALIZE
/* Function prototype for the function to notify a USB event from the USBX Host system. */
static UINT ux_system_host_change_function(ULONG event, UX_HOST_CLASS * host_class, VOID * instance);

#ifdef USB_HOST_STORAGE_CLASS_REGISTER

/* A semaphore for synchronizing to the USB media storage mount timing. */
static TX_SEMAPHORE ux_host_storage_semaphore_insertion;

/* Temporary storage place for the pointer to a USBX Host Mass Storage Class Instance. */
static UX_HOST_CLASS_STORAGE * g_ux_new_host_storage_instance;

/* Function prototype for the function to register the USBX Host Class Mass Storage. */
static void ux_host_stack_class_register_storage(void);

/* Function prototype for the function to notify changes happened to the USBX Host Class Mass Storage. */
static UINT ux_system_host_storage_change_function(ULONG event, VOID * instance);

/*******************************************************************************************************************//**
 * @brief      This is the function to register the USBX Host Class Mass Storage.
 **********************************************************************************************************************/
static void ux_host_stack_class_register_storage(void)
{
    UINT status;
    status = ux_host_stack_class_register(_ux_system_host_class_storage_name, ux_host_class_storage_entry);
    if (UX_SUCCESS != status)
    {
        ux_v2_err_callback(NULL, &status);
    }

    /* Create semaphores used for the USB Mass Storage Media Initialization. */
    status = tx_semaphore_create(&ux_host_storage_semaphore_insertion, "ux_host_storage_semaphore_insertion", 0);
    if (TX_SUCCESS != status)
    {
        ux_v2_err_callback(NULL, &status);
    }
}

/*******************************************************************************************************************//**
 * @brief      This is the function to notify changes happened to the USBX Host Class Mass Storage.
 * @param[in]  event      Event code from happened to the USBX Host system.
 * @param[in]  instance   Pointer to a USBX Host class instance, which occurs a event.
 * @retval     USBX error code
 **********************************************************************************************************************/
static UINT ux_system_host_storage_change_function(ULONG event, VOID * instance)
{
    if (UX_DEVICE_INSERTION == event) /* Check if there is a device insertion. */
    {
        g_ux_new_host_storage_instance = instance;

        /* Put the semaphore for a USBX Mass Storage Media insertion. */
        tx_semaphore_put (&ux_host_storage_semaphore_insertion);
    }
    else if (UX_DEVICE_REMOVAL == event) /* Check if there is a device removal. */
    {
        g_ux_new_host_storage_instance = NULL;
    }
    return UX_SUCCESS;
}

/*******************************************************************************************************************//**
 * @brief      This is the function to get the FileX Media Control Block for a USB Mass Storage device.
 * @param[in]      new_instance     Pointer to a USBX Host Mass Storage Class instance.
 * @param[in/out]  p_storage_media  Pointer of the pointer to save an instance of the USBX Host Mass Storage Media.
 * @param[in/out]  p_fx_media       Pointer of the pointer to save an instance of FileX Media Control Block.
 * @retval     UX_HOST_CLASS_INSTANCE_UNKNOWN  The instance provided was not for a valid Mass Storage device.
 **********************************************************************************************************************/
UINT ux_system_host_storage_fx_media_get(UX_HOST_CLASS_STORAGE * instance, UX_HOST_CLASS_STORAGE_MEDIA ** p_storage_media,
        FX_MEDIA ** p_fx_media)
{
    UINT error = UX_SUCCESS;
    UX_HOST_CLASS * p_host_class;
    UX_HOST_CLASS_STORAGE_MEDIA * p_storage_media_local;
    FX_MEDIA * p_fx_media_local;
    int index;

    /** Get the USBX Host Mass Storage Class. */
    ux_host_stack_class_get(_ux_system_host_class_storage_name, &p_host_class);

    /** Get the pointer to a media attached to the class container for USB Host Mass Storage. */
    p_storage_media_local = (UX_HOST_CLASS_STORAGE_MEDIA *) p_host_class->ux_host_class_media;

    /** Get the pointer to a FileX Media Control Block. */
    for (index = 0; index < UX_HOST_CLASS_STORAGE_MAX_MEDIA; index++)
    {
        p_fx_media_local = &p_storage_media_local->ux_host_class_storage_media;
        if (p_fx_media_local->fx_media_driver_info != instance)
        {
            /* None of FileX Media Control Blocks matched to this USBX Host Mass Storage Instance. */
            p_storage_media_local++;
        }
        else
        {
            /* Found a FileX Media Control Block used for this USBX Host Mass Storage Instance. */
            break;
        }
    }
    if (UX_HOST_CLASS_STORAGE_MAX_MEDIA == index)
    {
        error = UX_HOST_CLASS_INSTANCE_UNKNOWN;
    }
    else
    {
        *p_storage_media = p_storage_media_local;
        *p_fx_media = p_fx_media_local;
    }

    return error;
}
#endif /* USB_HOST_STORAGE_CLASS_REGISTER */

/*******************************************************************************************************************//**
 * @brief      This is the function to notify a USB event from the USBX Host system.
 * @param[in]  event     Event code from happened to the USBX Host system.
 * @param[in]  usb_class Pointer to a USBX Host class, which occurs a event.
 * @param[in]  instance  Pointer to a USBX Host class instance, which occurs a event.
 * @retval     USBX error code
 **********************************************************************************************************************/
static UINT ux_system_host_change_function(ULONG event, UX_HOST_CLASS * host_class, VOID * instance)
{
    UINT status = UX_SUCCESS;
    SSP_PARAMETER_NOT_USED (event);
    SSP_PARAMETER_NOT_USED (host_class);
    SSP_PARAMETER_NOT_USED (instance);

#if !defined(NULL)
    /* Call user function back for USBX Host Class event notification. */
    status = NULL(event, host_class, instance);
    if (UX_SUCCESS != status)
    {
        return status;
    }
#endif

#ifdef USB_HOST_STORAGE_CLASS_REGISTER
    /* Check the class container if it is for a USBX Host Mass Storage class. */
    if (UX_SUCCESS == _ux_utility_memory_compare(_ux_system_host_class_storage_name, host_class, _ux_utility_string_length_get(_ux_system_host_class_storage_name)))
    {
        status = ux_system_host_storage_change_function(event, instance);
    }
#endif
    return status;
}
#endif

#ifdef USB_HOST_HID_CLASS_REGISTER
/* Function prototype to register USBX Host HID Clients to the USBX Host system. */
static void ux_host_class_hid_clients_register(void);
#endif

/*******************************************************************************************************************//**
 * @brief     Initialization function that the user can choose to have called automatically during thread entry.
 *            The user can call this function at a later time if desired using the prototype below.
 *            - void ux_common_init0(void)
 **********************************************************************************************************************/
void ux_common_init0(void)
{
    UINT status_ux_init;

    /** Initialize the USBX system. */
    status_ux_init = ux_system_initialize ((CHAR *) g_ux_pool_memory, 18432, UX_NULL, 0);
    if (UX_SUCCESS != status_ux_init)
    {
        ux_v2_err_callback (NULL, &status_ux_init);
    }

#ifdef UX_HOST_INITIALIZE
    /** Initialize the USBX Host stack. */
    status_ux_init = ux_host_stack_initialize(ux_system_host_change_function);
    if (UX_SUCCESS != status_ux_init)
    {
        ux_v2_err_callback(NULL,&status_ux_init);
    }

    /** Register USB Host classes. */
#ifdef USB_HOST_HUB_CLASS_REGISTER
    status_ux_init = ux_host_stack_class_register(_ux_system_host_class_hub_name, ux_host_class_hub_entry);
    if (UX_SUCCESS != status_ux_init)
    {
        ux_v2_err_callback(NULL,&status_ux_init);
    }
#endif

#ifdef USB_HOST_CDC_ACM_CLASS_REGISTER
    status_ux_init = ux_host_stack_class_register(_ux_system_host_class_cdc_acm_name, ux_host_class_cdc_acm_entry);
    if (UX_SUCCESS != status_ux_init)
    {
        ux_v2_err_callback(NULL,&status_ux_init);
    }
#endif

#ifdef USB_HOST_HID_CLASS_REGISTER
    status_ux_init = ux_host_stack_class_register(_ux_system_host_class_hid_name, ux_host_class_hid_entry);
    if (UX_SUCCESS != status_ux_init)
    {
        ux_v2_err_callback(NULL,&status_ux_init);
    }

    ux_host_class_hid_clients_register ();
#endif

#ifdef USB_HOST_STORAGE_CLASS_REGISTER
    ux_host_stack_class_register_storage();
#endif

#ifdef USB_HOST_VIDEO_CLASS_REGISTER
    status_ux_init = ux_host_stack_class_register(_ux_system_host_class_video_name, ux_host_class_video_entry);
    if (UX_SUCCESS != status_ux_init)
    {
        ux_v2_err_callback(NULL,&status_ux_init);
    }
#endif

#ifdef USB_HOST_AUDIO_CLASS_REGISTER
    status_ux_init = ux_host_stack_class_register(_ux_system_host_class_audio_name, ux_host_class_audio_entry);
    if (UX_SUCCESS != status_ux_init)
    {
        ux_v2_err_callback(NULL,&status_ux_init);
    }
#endif
#endif /* UX_HOST_INITIALIZE */
}
#if defined(__ICCARM__)
#define g_sf_el_ux_dcd_fs_0_err_callback_WEAK_ATTRIBUTE
#pragma weak g_sf_el_ux_dcd_fs_0_err_callback  = g_sf_el_ux_dcd_fs_0_err_callback_internal
#elif defined(__GNUC__)
#define g_sf_el_ux_dcd_fs_0_err_callback_WEAK_ATTRIBUTE   __attribute__ ((weak, alias("g_sf_el_ux_dcd_fs_0_err_callback_internal")))
#endif
void g_sf_el_ux_dcd_fs_0_err_callback(void *p_instance, void *p_data)
g_sf_el_ux_dcd_fs_0_err_callback_WEAK_ATTRIBUTE;
#if (SF_EL_UX_CFG_FS_IRQ_IPL != BSP_IRQ_DISABLED)
/* USBFS ISR vector registering. */
#if !defined(SSP_SUPPRESS_ISR_g_sf_el_ux_dcd_fs_0) && !defined(SSP_SUPPRESS_ISR_USB)
SSP_VECTOR_DEFINE_UNIT(usbfs_int_isr, USB, FS, INT, 0);
#endif
#endif

/* Prototype function for USBX DCD Initializer. */
void ux_dcd_initialize(void);

#undef SYNERGY_NOT_DEFINED
#define SYNERGY_NOT_DEFINED (1)
/*******************************************************************************************************************//**
 * @brief      This is a weak example initialization error function.  It should be overridden by defining a user  function 
 *             with the prototype below.
 *             - void g_sf_el_ux_dcd_fs_0_err_callback(void * p_instance, void * p_data)
 *
 * @param[in]  p_instance arguments used to identify which instance caused the error and p_data Callback arguments used to identify what error caused the callback.
 **********************************************************************************************************************/
void g_sf_el_ux_dcd_fs_0_err_callback_internal(void *p_instance, void *p_data);
void g_sf_el_ux_dcd_fs_0_err_callback_internal(void *p_instance, void *p_data)
{
    /** Suppress compiler warning for not using parameters. */
    SSP_PARAMETER_NOT_USED (p_instance);
    SSP_PARAMETER_NOT_USED (p_data);

    /** An error has occurred. Please check function arguments for more information. */
    BSP_CFG_HANDLE_UNRECOVERABLE_ERROR (0);
}
#if ((SYNERGY_NOT_DEFINED != SYNERGY_NOT_DEFINED) && (SYNERGY_NOT_DEFINED != SYNERGY_NOT_DEFINED))
/***********************************************************************************************************************
 * The definition of wrapper interface for USBX Synergy Port DCD Driver to get a transfer module instance.
 **********************************************************************************************************************/
static UINT g_sf_el_ux_dcd_fs_0_initialize_transfer_support(ULONG dcd_io)
{
    UX_DCD_SYNERGY_TRANSFER dcd_transfer;
    dcd_transfer.ux_synergy_transfer_tx = (transfer_instance_t *)&SYNERGY_NOT_DEFINED;
    dcd_transfer.ux_synergy_transfer_rx = (transfer_instance_t *)&SYNERGY_NOT_DEFINED;
    return (UINT)ux_dcd_synergy_initialize_transfer_support(dcd_io, (UX_DCD_SYNERGY_TRANSFER *)&dcd_transfer);
} /* End of function g_sf_el_ux_dcd_fs_0_initialize_transfer_support() */
#endif

/***********************************************************************************************************************
 * Initializes USBX Device Controller Driver.
 **********************************************************************************************************************/
void ux_dcd_initialize(void)
{
    UINT status;
    /* Initializes the USB device controller, enabling DMA transfer if transfer module instances are given. */
#if ((SYNERGY_NOT_DEFINED == SYNERGY_NOT_DEFINED) || (SYNERGY_NOT_DEFINED == SYNERGY_NOT_DEFINED))
    status = (UINT) ux_dcd_synergy_initialize (R_USBFS_BASE);
#else
    status = g_sf_el_ux_dcd_fs_0_initialize_transfer_support(R_USBFS_BASE);
#endif
#undef SYNERGY_NOT_DEFINED
    if (UX_SUCCESS != status)
    {
        g_sf_el_ux_dcd_fs_0_err_callback (NULL, &status);
    }
} /* End of function ux_dcd_initialize() */
#if defined(__ICCARM__)
#define ux_device_err_callback_WEAK_ATTRIBUTE
#pragma weak ux_device_err_callback  = ux_device_err_callback_internal
#elif defined(__GNUC__)
#define ux_device_err_callback_WEAK_ATTRIBUTE   __attribute__ ((weak, alias("ux_device_err_callback_internal")))
#endif
void ux_device_err_callback(void *p_instance, void *p_data)
ux_device_err_callback_WEAK_ATTRIBUTE;
/* Definition for the special linker section for USB */
#if defined(__GNUC__)
extern uint32_t __usb_dev_descriptor_start_fs;
extern uint32_t __usb_descriptor_end_fs;
#if defined(UX_DCD_SYNERY_USE_USBHS)
extern uint32_t __usb_dev_descriptor_start_hs;
extern uint32_t __usb_descriptor_end_hs;
#endif
#endif

/* Counter to calculate number of Interfaces */
static uint8_t g_usbx_number_of_interface_count = 0;

/***********************************************************************************************************************
 * USB Device Descriptor for FS mode
 **********************************************************************************************************************/
#if defined(__GNUC__)
static volatile const unsigned char device_framework_full_speed[] BSP_PLACE_IN_SECTION_V2(".usb_device_desc_fs") BSP_ALIGN_VARIABLE_V2(1)
#else /* __ICCARM__ */
#pragma section = ".usb_device_desc_fs" 1
__root static const unsigned char device_framework_full_speed[] BSP_PLACE_IN_SECTION_V2(".usb_device_desc_fs")
#endif
=
{
    0x12, /* 0 bLength */
    UX_DEVICE_DESCRIPTOR_ITEM, /* 1 bDescriptorType */
    0x00, /* 2 bcdUSB BCD(2.0) */
    0x02, /* 3 bcdUSB */
    0x08, /* 4 bDeviceClass    : Device Class */
#if (0x08 == 0x00) || (0x08 == 0x02) || (0x08 == 0x03) || (0x08 == 0x08)
    0x00, /* 5 bDeviceSubClass : none */
    0x00, /* 6 bDeviceProtocol : none */
#elif (0x08 == 0xEF)
    0x02, /* 5 bDeviceSubClass : Common Class(0x02) */
    0x01, /* 6 bDeviceProtocol :Interface Association Descriptor(IAD) */
#elif (0x08 == 0xFF)
    0x00, /* 5 bDeviceSubClass : none */
    0x00, /* 6 bDeviceProtocol : none */
#endif
    0x40, /* 7 bMaxPacketSize0 */
    (uint8_t)(0x045B), /* 8 idVendor */
    (uint8_t)(0x045B >> 8), /* 9 idVendor */
    (uint8_t)(0x0000), /* 10 idProduct */
    (uint8_t)(0x0000 >> 8), /* 11 idProduct */
    (uint8_t)(0x0000), /* 12 bcdDevice */
    (uint8_t)(0x0000 >> 8), /* 13 bcdDevice */
    0x00, /* 14 iManufacturer */
    0x00, /* 15 iProduct */
    0x00, /* 16 iSerialNumber */
    0x01, /* 17 bNumConfigurations */
};

#if defined(UX_DCD_SYNERY_USE_USBHS)
/***********************************************************************************************************************
 * USB Device Descriptor for HS Mode
 **********************************************************************************************************************/
#if defined(__GNUC__)
static volatile const unsigned char device_framework_high_speed[] BSP_PLACE_IN_SECTION_V2(".usb_device_desc_hs") BSP_ALIGN_VARIABLE_V2(1)
#else /* __ICCARM__ */
#pragma section = ".usb_device_desc_hs" 1
__root static const unsigned char device_framework_high_speed[] BSP_PLACE_IN_SECTION_V2(".usb_device_desc_hs")
#endif
=
{
    0x12, /* 0 bLength */
    UX_DEVICE_DESCRIPTOR_ITEM, /* 1 bDescriptorType */
    0x00, /* 2 bcdUSB BCD(2.0) */
    0x02, /* 3 bcdUSB */
    0x08, /* 4 bDeviceClass    : Device Class */
#if (0x08 == 0x00) || (0x08 == 0x02) || (0x08 == 0x03) || (0x08 == 0x08)
    0x00, /* 5 bDeviceSubClass : none */
    0x00, /* 6 bDeviceProtocol : none */
#elif (0x08 == 0xEF) /* Composite device */
    0x02, /* 5 bDeviceSubClass : Common Class(0x02) */
    0x01, /* 6 bDeviceProtocol :Interface Association Descriptor(IAD) */
#elif (0x08 == 0xFF) /* Vendor specific device */
    0x00, /* 5 bDeviceSubClass : none */
    0x00, /* 6 bDeviceProtocol : none */
#endif
    0x40, /* 7 bMaxPacketSize0 */
    (uint8_t)(0x045B), /* 8 idVendor */
    (uint8_t)(0x045B >> 8), /* 9 idVendor */
    (uint8_t)(0x0000), /* 10 idProduct */
    (uint8_t)(0x0000 >> 8), /* 11 idProduct */
    (uint8_t)(0x0000), /* 12 bcdDevice */
    (uint8_t)(0x0000 >> 8), /* 13 bcdDevice */
    0x00, /* 14 iManufacturer */
    0x00, /* 15 iProduct */
    0x00, /* 16 iSerialNumber */
    0x01, /* 17 bNumConfigurations */
    /****************************************************************
     * Device qualifier descriptor                                  *
     ****************************************************************/
    0x0a, /* 0 bLength */
    UX_DEVICE_QUALIFIER_DESCRIPTOR_ITEM, /* 1 bDescriptorType */
    0x00, /* 2 bcdUSB BCD(2.0) */
    0x02, /* 3 bcdUSB */
    0x08, /* 4 bDeviceClass    : Device Class */
#if (0x08 == 0x00) || (0x08 == 0x02) || (0x08 == 0x03) || (0x08 == 0x08)
    0x00, /* 5 bDeviceSubClass : none */
    0x00, /* 6 bDeviceProtocol : none */
#elif (0x08 == 0xEF)
    0x02, /* 5 bDeviceSubClass : Common Class(0x02) */
    0x00, /* 6 bDeviceProtocol : none */
#elif (0x08 == 0xFF)
    0x00, /* 5 bDeviceSubClass : none */
    0x00, /* 6 bDeviceProtocol : none */
#endif
    0x40, /* 7 bMaxPacketSize0 */
    0x01, /* 8 bNumConfigs : 1 */
    0x00, /* 9 Reserve, fixed to 0 */
};
#endif

/***********************************************************************************************************************
 * Configuration Value Auto-calculation for USB Configuration Descriptor
 **********************************************************************************************************************/
#ifndef USB_IFD_NUM_0
#define USB_IFD_NUM_0  (0)
#define USB_IFD_SIZE_0 (0)
#endif
#ifndef USB_IFD_NUM_1
#define USB_IFD_NUM_1  (0)
#define USB_IFD_SIZE_1 (0)
#endif
#ifndef USB_IFD_NUM_2
#define USB_IFD_NUM_2  (0)
#define USB_IFD_SIZE_2 (0)
#endif
#ifndef USB_IFD_NUM_3
#define USB_IFD_NUM_3  (0)
#define USB_IFD_SIZE_3 (0)
#endif
#ifndef USB_IFD_NUM_4
#define USB_IFD_NUM_4  (0)
#define USB_IFD_SIZE_4 (0)
#endif
#ifndef USB_IFD_NUM_5
#define USB_IFD_NUM_5  (0)
#define USB_IFD_SIZE_5 (0)
#endif
#ifndef USB_IFD_NUM_6
#define USB_IFD_NUM_6  (0)
#define USB_IFD_SIZE_6 (0)
#endif
#ifndef USB_IFD_NUM_7
#define USB_IFD_NUM_7  (0)
#define USB_IFD_SIZE_7 (0)
#endif
#ifndef USB_IFD_NUM_8
#define USB_IFD_NUM_8  (0)
#define USB_IFD_SIZE_8 (0)
#endif
#ifndef USB_IFD_NUM_9
#define USB_IFD_NUM_9  (0)
#define USB_IFD_SIZE_9 (0)
#endif

#define USB_NUMBER_OF_INTERFACE USB_IFD_NUM_0 + USB_IFD_NUM_1 + USB_IFD_NUM_2 + USB_IFD_NUM_3 + USB_IFD_NUM_4 + USB_IFD_NUM_5 + USB_IFD_NUM_6 + USB_IFD_NUM_7 + USB_IFD_NUM_8 + USB_IFD_NUM_9
#define USB_TOTAL_LENGTH        USB_IFD_SIZE_0 + USB_IFD_SIZE_1 + USB_IFD_SIZE_2 + USB_IFD_SIZE_3 + USB_IFD_SIZE_4 + USB_IFD_SIZE_5 + USB_IFD_SIZE_6 + USB_IFD_SIZE_7 + USB_IFD_SIZE_8 + USB_IFD_SIZE_9

#if (0 != 0x00)
#define USB_CFG_DESCRIPTOR_bNumInterfaces     (0x00)
#else
#define USB_CFG_DESCRIPTOR_bNumInterfaces     (USB_NUMBER_OF_INTERFACE)
#endif
#if (0 != 0x00)
#define USB_CFG_DESCRIPTOR_wTotalLength       (0x00)
#else
#define USB_CFG_DESCRIPTOR_wTotalLength       (0x09 + USB_TOTAL_LENGTH)
#endif

/***********************************************************************************************************************
 * Consolidated USB Device Descriptor Framework on RAM for IAR build
 **********************************************************************************************************************/
#ifdef __ICCARM__
/* Memory area to complete USB device Descriptor Framework. */
static uint8_t device_framework_full_speed_ram[0x12 + USB_CFG_DESCRIPTOR_wTotalLength];
#if defined(UX_DCD_SYNERY_USE_USBHS)
static uint8_t device_framework_high_speed_ram[0x12 + 0x0a + USB_CFG_DESCRIPTOR_wTotalLength];
#endif
#endif

/***********************************************************************************************************************
 * USB Configuration Descriptor for FS mode
 **********************************************************************************************************************/
#if defined(__GNUC__)
static volatile const unsigned char ux_cfg_descriptor_full_speed[] BSP_PLACE_IN_SECTION_V2(".usb_config_desc_fs") BSP_ALIGN_VARIABLE_V2(1)
#else /* __ICCARM__ */
#pragma section = ".usb_config_desc_fs" 1
__root static const unsigned char ux_cfg_descriptor_full_speed[] BSP_PLACE_IN_SECTION_V2(".usb_config_desc_fs")
#endif
=
{
    0x09, /* 0 bLength */
    UX_CONFIGURATION_DESCRIPTOR_ITEM, /* 1 bDescriptorType */
    (uint8_t)(USB_CFG_DESCRIPTOR_wTotalLength), /* 2 wTotalLength : This will be calculated at run-time. */
    (uint8_t)(USB_CFG_DESCRIPTOR_wTotalLength >> 8), /* 3 wTotalLength : This will be calculated at run-time. */
    (uint8_t)(USB_CFG_DESCRIPTOR_bNumInterfaces), /* 4 bNumInterfaces */
    0x01, /* 5 bConfigurationValue : Fixed to 1 since only one configuration is supported. */
    0x00, /* 6 iConfiguration */
    0x80 | (1 << 6) | (0 << 5), /* 7 bmAttributes */
    150, /* 8 bMaxPower */
};

#if defined(UX_DCD_SYNERY_USE_USBHS)
/***********************************************************************************************************************
 * USB Configuration Descriptor for HS mode
 **********************************************************************************************************************/
#if defined(__GNUC__)
static volatile const unsigned char ux_cfg_descriptor_high_speed[] BSP_PLACE_IN_SECTION_V2(".usb_config_desc_hs") BSP_ALIGN_VARIABLE_V2(1)
#else /* __ICCARM__ */
#pragma section = ".usb_config_desc_hs" 1
__root static const unsigned char ux_cfg_descriptor_high_speed[] BSP_PLACE_IN_SECTION_V2(".usb_config_desc_hs")
#endif
=
{
    0x09, /* 0 bLength */
    UX_CONFIGURATION_DESCRIPTOR_ITEM, /* 1 bDescriptorType */
    (uint8_t)(USB_CFG_DESCRIPTOR_wTotalLength), /* 2 wTotalLength : This will be calculated at run-time. */
    (uint8_t)(USB_CFG_DESCRIPTOR_wTotalLength >> 8), /* 3 wTotalLength : This will be calculated at run-time. */
    (uint8_t)(USB_CFG_DESCRIPTOR_bNumInterfaces), /* 4 bNumInterfaces */
    0x01, /* 5 bConfigurationValue : Fixed to 1 since only one configuration is supported. */
    0x00, /* 6 iConfiguration */
    0x80 | (1 << 6) | (0 << 5), /* 7 bmAttributes */
    150, /* 8 bMaxPower */
};
#endif

/***********************************************************************************************************************
 * USB Language Framework (Default setting for US English)
 **********************************************************************************************************************/
#if(0 == 0) /* Create USB Language Framework with default setting if not defined by user. */
static volatile const UCHAR language_id_framework_default[] =
{ (uint8_t) (0x0409), /* Supported Language Code */
  (uint8_t) (0x0409 >> 8) /* US English as the default */
};
#endif

/***********************************************************************************************************************
 * Function to get size of USB String Descriptor
 **********************************************************************************************************************/
#if (0)
extern const UCHAR NULL[];
static ULONG ux_device_string_descriptor_size_get(void)
{
    ULONG size = 0;
    UCHAR * ptr = (UCHAR *)NULL;
    if(NULL != ptr)
    {
        for(INT i = 0; i < 0; i++)
        {
            ptr = ptr + 3; /* bLength at byte offset 3 */
            /* Counts bLength + Language code(2bytes) + bLength(1byte) */
            size = size + *ptr + 4;
            ptr = ptr + (*ptr) + 1;
        }
    }
    return size;
}
#endif

#if (0)
extern const UCHAR NULL[];
#endif
/*******************************************************************************************************************//**
 * @brief      This is a weak example initialization error function.  It should be overridden by defining a user  function 
 *             with the prototype below.
 *             - void ux_device_err_callback(void * p_instance, void * p_data)
 *
 * @param[in]  p_instance arguments used to identify which instance caused the error and p_data Callback arguments used to identify what error caused the callback.
 **********************************************************************************************************************/
void ux_device_err_callback_internal(void *p_instance, void *p_data);
void ux_device_err_callback_internal(void *p_instance, void *p_data)
{
    /** Suppress compiler warning for not using parameters. */
    SSP_PARAMETER_NOT_USED (p_instance);
    SSP_PARAMETER_NOT_USED (p_data);

    /** An error has occurred. Please check function arguments for more information. */
    BSP_CFG_HANDLE_UNRECOVERABLE_ERROR (0);
}
/*******************************************************************************************************************//**
 * @brief     Initialization function that the user can choose to have called automatically during thread entry.
 *            The user can call this function at a later time if desired using the prototype below.
 *            - void ux_device_init0(void)
 **********************************************************************************************************************/
void ux_device_init0(void)
{

    UINT status_ux_device_init;

    /** Calculate the size of USBX String Framework. */
    ULONG string_framework_size;
    UCHAR *p_string_framework;

#if(0) /* Check if the number of index in USBX String Framework is greater than zero. */
    {
        p_string_framework = (UCHAR *)NULL;
        string_framework_size = ux_device_string_descriptor_size_get();
    }
#else
    {
        p_string_framework = NULL;
        string_framework_size = 0;
    }
#endif

    /** Calculate the size of USB Language Framework. */
    ULONG language_framework_size;
    UCHAR *p_language_framework;

#if(0) /* Check if the number of index in USB Language Framework is greater than zero. */
    {
        p_language_framework = (UCHAR *)NULL;
        language_framework_size = 0 * 2;
    }
#else
    {
        p_language_framework = (UCHAR *) language_id_framework_default;
        language_framework_size = 2;
    }
#endif

    /** Initialize the USB Device stack. */
#ifdef __GNUC__
    status_ux_device_init = ux_device_stack_initialize(
#if defined(UX_DCD_SYNERY_USE_USBHS)
            (UCHAR *)device_framework_high_speed, ((uint32_t)&__usb_descriptor_end_hs - (uint32_t)&__usb_dev_descriptor_start_hs),
#else /* If USB DCD runs in FS mode, USB Device Descriptor for HS mode is not required. */
            NULL, 0,
#endif
            (UCHAR *)device_framework_full_speed, ((uint32_t)&__usb_descriptor_end_fs - (uint32_t)&__usb_dev_descriptor_start_fs),
            p_string_framework, string_framework_size,
            p_language_framework, language_framework_size, UX_NULL);
#else /* __ICCARM__ */

    uint8_t *p_device_framework;
#if defined(UX_DCD_SYNERY_USE_USBHS)
    p_device_framework = (uint8_t *)device_framework_high_speed_ram;
    memcpy (p_device_framework, device_framework_high_speed, sizeof(device_framework_high_speed));
    p_device_framework = p_device_framework + sizeof(device_framework_high_speed);
    memcpy (p_device_framework, ux_cfg_descriptor_high_speed, sizeof(ux_cfg_descriptor_high_speed));
    p_device_framework = p_device_framework + sizeof(ux_cfg_descriptor_high_speed);
    memcpy (p_device_framework, (uint8_t *)__section_begin(".usb_interface_desc_hs"), USB_TOTAL_LENGTH);
#endif

    /* Creates Device Descriptor Frameworks copying Descriptors from ROM. */
    p_device_framework = (uint8_t *) device_framework_full_speed_ram;
    memcpy (p_device_framework, device_framework_full_speed, sizeof(device_framework_full_speed));
    p_device_framework = p_device_framework + sizeof(device_framework_full_speed);
    /* Copy Configuration Descriptor */
    memcpy (p_device_framework, ux_cfg_descriptor_full_speed, sizeof(ux_cfg_descriptor_full_speed));
    p_device_framework = p_device_framework + sizeof(ux_cfg_descriptor_full_speed);
    /* Copy Interface Descriptors */
    memcpy (p_device_framework, (uint8_t *) __section_begin (".usb_interface_desc_fs"), USB_TOTAL_LENGTH);

    status_ux_device_init = ux_device_stack_initialize (
#if defined(UX_DCD_SYNERY_USE_USBHS)
            (UCHAR *)device_framework_high_speed_ram, (0x12 + 0x0a + USB_CFG_DESCRIPTOR_wTotalLength),
#else /* If USB DCD runs in FS mode, USB Device Descriptor for HS mode is not required. */
            NULL,
            0,
#endif
            (UCHAR *) device_framework_full_speed_ram,
            (0x12 + USB_CFG_DESCRIPTOR_wTotalLength), p_string_framework, string_framework_size, p_language_framework,
            language_framework_size, UX_NULL);
#endif
    if (UX_SUCCESS != status_ux_device_init)
    {
        ux_device_err_callback (NULL, &status_ux_device_init);
    }

}
static void g_ux_device_class_storage_setup(void);
/***********************************************************************************************************************
 * USBX Mass Storage Class Parameter Setup Function.
 **********************************************************************************************************************/
static UX_SLAVE_CLASS_STORAGE_PARAMETER g_ux_device_class_storage_parameter;
void g_ux_device_class_storage_setup(void)
{
    UINT status;

#if (1 == 1)
    /* Auto setup for a simple media storage configuration with single Logical Unit Number (LUN). */
    /* Stores the number of LUN in this device storage instance.  */
    g_ux_device_class_storage_parameter.ux_slave_class_storage_parameter_number_lun = 1;
    g_ux_device_class_storage_parameter.ux_slave_class_storage_parameter_lun[0].ux_slave_class_storage_media_last_lba =
            15118335;
    g_ux_device_class_storage_parameter.ux_slave_class_storage_parameter_lun[0].ux_slave_class_storage_media_block_length =
            512;
    g_ux_device_class_storage_parameter.ux_slave_class_storage_parameter_lun[0].ux_slave_class_storage_media_type = 0;
    g_ux_device_class_storage_parameter.ux_slave_class_storage_parameter_lun[0].ux_slave_class_storage_media_removable_flag =
            0x80;
    g_ux_device_class_storage_parameter.ux_slave_class_storage_parameter_lun[0].ux_slave_class_storage_media_read =
            ux_device_msc_media_read;
    g_ux_device_class_storage_parameter.ux_slave_class_storage_parameter_lun[0].ux_slave_class_storage_media_write =
            ux_device_msc_media_write;
    g_ux_device_class_storage_parameter.ux_slave_class_storage_parameter_lun[0].ux_slave_class_storage_media_status =
            ux_device_msc_media_status;
#else
    /* Manual setup for a media storage(s) configuration. Calls back the user manual setup function. */
    ux_device_class_storage_user_setup(&g_ux_device_class_storage_parameter);
#endif

    /* Register user callback functions.  */
    g_ux_device_class_storage_parameter.ux_slave_class_storage_instance_activate = NULL;
    g_ux_device_class_storage_parameter.ux_slave_class_storage_instance_deactivate = NULL;
    g_ux_device_class_storage_parameter.ux_slave_class_storage_parameter_vendor_id = (UCHAR *) "NULL";
    g_ux_device_class_storage_parameter.ux_slave_class_storage_parameter_product_id = (UCHAR *) "NULL";
    g_ux_device_class_storage_parameter.ux_slave_class_storage_parameter_product_rev = (UCHAR *) "NULL";
    g_ux_device_class_storage_parameter.ux_slave_class_storage_parameter_product_serial = (UCHAR *) "NULL";

    /* Initializes the device storage class. The class is connected with interface 0 on configuration 1. */
    status = _ux_device_stack_class_register (_ux_system_slave_class_storage_name, ux_device_class_storage_entry, 1,
                                              0x00, (VOID *) &g_ux_device_class_storage_parameter);
    assert (status == UX_SUCCESS);

    /* Counts up the number of Interfaces. */
    g_usbx_number_of_interface_count = (uint8_t) (g_usbx_number_of_interface_count + 1);
}
/*******************************************************************************************************************//**
 * @brief     Initialization function that the user can choose to have called automatically during thread entry.
 *            The user can call this function at a later time if desired using the prototype below.
 *            - void ux_device_class_storage_init0(void)
 **********************************************************************************************************************/
void ux_device_class_storage_init0(void)
{
    /* Setups USB Mass Storage Class to create the instance (if required) */
    g_ux_device_class_storage_setup ();

    /* USB DCD initialization (Only executed once after all the class registration completed). */
    if (g_usbx_number_of_interface_count == USB_NUMBER_OF_INTERFACE)
    {
        /* Initializes USBX Device Control Driver */
        ux_dcd_initialize ();
    }
}
const elc_instance_t g_elc =
{ .p_api = &g_elc_on_elc, .p_cfg = NULL };
/* Instance structure to use this module. */
const fmi_instance_t g_fmi =
{ .p_api = &g_fmi_on_fmi };
const ioport_instance_t g_ioport =
{ .p_api = &g_ioport_on_ioport, .p_cfg = NULL };
const cgc_instance_t g_cgc =
{ .p_api = &g_cgc_on_cgc, .p_cfg = NULL };
ssp_err_t SF_EL_FX_Get_MEDIA_Info(sf_block_media_instance_t *p_block_media, uint32_t *sector_size,
        uint32_t *sector_count)
{
    ssp_err_t ret_val = SSP_SUCCESS;

    ret_val = p_block_media->p_api->open (p_block_media->p_ctrl, p_block_media->p_cfg);
    if (ret_val != SSP_SUCCESS)
    {
        return ret_val;
    }

    /* Get actual sector size from media. */
    ret_val = p_block_media->p_api->ioctl (p_block_media->p_ctrl, SSP_COMMAND_GET_SECTOR_SIZE, sector_size);
    if (ret_val != SSP_SUCCESS)
    {
        return ret_val;
    }

    /* Get actual sector count from media. */
    ret_val = p_block_media->p_api->ioctl (p_block_media->p_ctrl, SSP_COMMAND_GET_SECTOR_COUNT, sector_count);
    if (ret_val != SSP_SUCCESS)
    {
        return ret_val;
    }

    /* Close driver.  */
    ret_val = p_block_media->p_api->close (p_block_media->p_ctrl);
    return ret_val;
}
void g_common_init(void)
{
    /** Call initialization function if user has selected to do so. */
#if FX_COMMON_INITIALIZE
    fx_common_init0 ();
#endif
    /** Call initialization function if user has selected to do so. */
#if SF_EL_FX_AUTO_INIT_sd_fx_media
    fx_media_init0();
#endif

    /** Call initialization function if user has selected to do so. */
#if (0)
    ux_common_init0();
#endif
    /** Call initialization function if user has selected to do so. */
#if (0)
    ux_device_init0();
#endif
    /** Call initialization function if user has selected to do so */
#if (0)
    ux_device_class_storage_init0();
#endif
}
