/* generated HAL source file - do not edit */
#include "hal_data.h"
#if (4) != BSP_IRQ_DISABLED
#if !defined(SSP_SUPPRESS_ISR_g_flash0) && !defined(SSP_SUPPRESS_ISR_FLASH)
SSP_VECTOR_DEFINE( fcu_frdyi_isr, FCU, FRDYI);
#endif
#endif
flash_lp_instance_ctrl_t g_flash0_ctrl;
const flash_cfg_t g_flash0_cfg =
{ .data_flash_bgo = false, .p_callback = NULL, .p_context = &g_flash0, .irq_ipl = (4),

};
/* Instance structure to use this module. */
const flash_instance_t g_flash0 =
{ .p_ctrl = &g_flash0_ctrl, .p_cfg = &g_flash0_cfg, .p_api = &g_flash_on_flash_lp };
static iwdt_instance_ctrl_t iWDT_ctrl;

static const wdt_cfg_t iWDT_cfg =
{ .p_callback = NULL, };

/* Instance structure to use this module. */
const wdt_instance_t iWDT =
{ .p_ctrl = &iWDT_ctrl, .p_cfg = &iWDT_cfg, .p_api = &g_wdt_on_iwdt };
#if (BSP_IRQ_DISABLED) != BSP_IRQ_DISABLED
#if !defined(SSP_SUPPRESS_ISR_g_transfer3) && !defined(SSP_SUPPRESS_ISR_DTCELC_EVENT_SCI2_RXI)
#define DTC_ACTIVATION_SRC_ELC_EVENT_SCI2_RXI
#if defined(DTC_ACTIVATION_SRC_ELC_EVENT_ELC_SOFTWARE_EVENT_0) && !defined(DTC_VECTOR_DEFINED_SOFTWARE_EVENT_0)
SSP_VECTOR_DEFINE(elc_software_event_isr, ELC, SOFTWARE_EVENT_0);
#define DTC_VECTOR_DEFINED_SOFTWARE_EVENT_0
#endif
#if defined(DTC_ACTIVATION_SRC_ELC_EVENT_ELC_SOFTWARE_EVENT_1) && !defined(DTC_VECTOR_DEFINED_SOFTWARE_EVENT_1)
SSP_VECTOR_DEFINE(elc_software_event_isr, ELC, SOFTWARE_EVENT_1);
#define DTC_VECTOR_DEFINED_SOFTWARE_EVENT_1
#endif
#endif
#endif

dtc_instance_ctrl_t g_transfer3_ctrl;
transfer_info_t g_transfer3_info =
{ .dest_addr_mode = TRANSFER_ADDR_MODE_INCREMENTED, .repeat_area = TRANSFER_REPEAT_AREA_DESTINATION, .irq =
          TRANSFER_IRQ_END,
  .chain_mode = TRANSFER_CHAIN_MODE_DISABLED, .src_addr_mode = TRANSFER_ADDR_MODE_FIXED, .size = TRANSFER_SIZE_1_BYTE,
  .mode = TRANSFER_MODE_NORMAL, .p_dest = (void *) NULL, .p_src = (void const *) NULL, .num_blocks = 0, .length = 0, };
const transfer_cfg_t g_transfer3_cfg =
{ .p_info = &g_transfer3_info, .activation_source = ELC_EVENT_SCI2_RXI, .auto_enable = false, .p_callback = NULL,
  .p_context = &g_transfer3, .irq_ipl = (BSP_IRQ_DISABLED) };
/* Instance structure to use this module. */
const transfer_instance_t g_transfer3 =
{ .p_ctrl = &g_transfer3_ctrl, .p_cfg = &g_transfer3_cfg, .p_api = &g_transfer_on_dtc };
#if (BSP_IRQ_DISABLED) != BSP_IRQ_DISABLED
#if !defined(SSP_SUPPRESS_ISR_g_transfer2) && !defined(SSP_SUPPRESS_ISR_DTCELC_EVENT_SCI2_TXI)
#define DTC_ACTIVATION_SRC_ELC_EVENT_SCI2_TXI
#if defined(DTC_ACTIVATION_SRC_ELC_EVENT_ELC_SOFTWARE_EVENT_0) && !defined(DTC_VECTOR_DEFINED_SOFTWARE_EVENT_0)
SSP_VECTOR_DEFINE(elc_software_event_isr, ELC, SOFTWARE_EVENT_0);
#define DTC_VECTOR_DEFINED_SOFTWARE_EVENT_0
#endif
#if defined(DTC_ACTIVATION_SRC_ELC_EVENT_ELC_SOFTWARE_EVENT_1) && !defined(DTC_VECTOR_DEFINED_SOFTWARE_EVENT_1)
SSP_VECTOR_DEFINE(elc_software_event_isr, ELC, SOFTWARE_EVENT_1);
#define DTC_VECTOR_DEFINED_SOFTWARE_EVENT_1
#endif
#endif
#endif

dtc_instance_ctrl_t g_transfer2_ctrl;
transfer_info_t g_transfer2_info =
{ .dest_addr_mode = TRANSFER_ADDR_MODE_FIXED, .repeat_area = TRANSFER_REPEAT_AREA_SOURCE, .irq = TRANSFER_IRQ_END,
  .chain_mode = TRANSFER_CHAIN_MODE_DISABLED, .src_addr_mode = TRANSFER_ADDR_MODE_INCREMENTED, .size =
          TRANSFER_SIZE_1_BYTE,
  .mode = TRANSFER_MODE_NORMAL, .p_dest = (void *) NULL, .p_src = (void const *) NULL, .num_blocks = 0, .length = 0, };
const transfer_cfg_t g_transfer2_cfg =
{ .p_info = &g_transfer2_info, .activation_source = ELC_EVENT_SCI2_TXI, .auto_enable = false, .p_callback = NULL,
  .p_context = &g_transfer2, .irq_ipl = (BSP_IRQ_DISABLED) };
/* Instance structure to use this module. */
const transfer_instance_t g_transfer2 =
{ .p_ctrl = &g_transfer2_ctrl, .p_cfg = &g_transfer2_cfg, .p_api = &g_transfer_on_dtc };
#if SCI_UART_CFG_RX_ENABLE
#if (13) != BSP_IRQ_DISABLED
#if !defined(SSP_SUPPRESS_ISR_commUART) && !defined(SSP_SUPPRESS_ISR_SCI2)
SSP_VECTOR_DEFINE_CHAN(sci_uart_rxi_isr, SCI, RXI, 2);
#endif
#endif
#endif
#if SCI_UART_CFG_TX_ENABLE
#if (13) != BSP_IRQ_DISABLED
#if !defined(SSP_SUPPRESS_ISR_commUART) && !defined(SSP_SUPPRESS_ISR_SCI2)
SSP_VECTOR_DEFINE_CHAN(sci_uart_txi_isr, SCI, TXI, 2);
#endif
#endif
#if (13) != BSP_IRQ_DISABLED
#if !defined(SSP_SUPPRESS_ISR_commUART) && !defined(SSP_SUPPRESS_ISR_SCI2)
SSP_VECTOR_DEFINE_CHAN(sci_uart_tei_isr, SCI, TEI, 2);
#endif
#endif
#endif
#if SCI_UART_CFG_RX_ENABLE
#if (BSP_IRQ_DISABLED) != BSP_IRQ_DISABLED
#if !defined(SSP_SUPPRESS_ISR_commUART) && !defined(SSP_SUPPRESS_ISR_SCI2)
SSP_VECTOR_DEFINE_CHAN(sci_uart_eri_isr, SCI, ERI, 2);
#endif
#endif
#endif
sci_uart_instance_ctrl_t commUART_ctrl;

/** UART extended configuration for UARTonSCI HAL driver */
const uart_on_sci_cfg_t commUART_cfg_extend =
{ .clk_src = SCI_CLK_SRC_INT, .baudclk_out = false, .rx_edge_start = true, .noisecancel_en = false, .p_extpin_ctrl =
          NULL,
  .bitrate_modulation = true, .rx_fifo_trigger = SCI_UART_RX_FIFO_TRIGGER_MAX, .baud_rate_error_x_1000 = (uint32_t) (
          2.0 * 1000),
  .uart_comm_mode = UART_MODE_RS232, .uart_rs485_mode = UART_RS485_HD, .rs485_de_pin = IOPORT_PORT_09_PIN_14, };

/** UART interface configuration */
const uart_cfg_t commUART_cfg =
{ .channel = 2, .baud_rate = 9600, .data_bits = UART_DATA_BITS_8, .parity = UART_PARITY_OFF, .stop_bits =
          UART_STOP_BITS_1,
  .ctsrts_en = false, .p_callback = comm_uart_callback, .p_context = &commUART, .p_extend = &commUART_cfg_extend,
#define SYNERGY_NOT_DEFINED (1)                        
#if (SYNERGY_NOT_DEFINED == g_transfer2)
  .p_transfer_tx = NULL,
#else
  .p_transfer_tx = &g_transfer2,
#endif            
#if (SYNERGY_NOT_DEFINED == g_transfer3)
  .p_transfer_rx = NULL,
#else
  .p_transfer_rx = &g_transfer3,
#endif   
#undef SYNERGY_NOT_DEFINED            
  .rxi_ipl = (13),
  .txi_ipl = (13), .tei_ipl = (13), .eri_ipl = (BSP_IRQ_DISABLED), };

/* Instance structure to use this module. */
const uart_instance_t commUART =
{ .p_ctrl = &commUART_ctrl, .p_cfg = &commUART_cfg, .p_api = &g_uart_on_sci };
#if (14) != BSP_IRQ_DISABLED
#if !defined(SSP_SUPPRESS_ISR_on_off_irq) && !defined(SSP_SUPPRESS_ISR_ICU1)
SSP_VECTOR_DEFINE( icu_irq_isr, ICU, IRQ1);
#endif
#endif
static icu_instance_ctrl_t on_off_irq_ctrl;
static const external_irq_cfg_t on_off_irq_cfg =
{ .channel = 1, .trigger = EXTERNAL_IRQ_TRIG_FALLING, .filter_enable = false, .pclk_div = EXTERNAL_IRQ_PCLK_DIV_BY_64,
  .autostart = true, .p_callback = irq_callback, .p_context = &on_off_irq, .p_extend = NULL, .irq_ipl = (14), };
/* Instance structure to use this module. */
const external_irq_instance_t on_off_irq =
{ .p_ctrl = &on_off_irq_ctrl, .p_cfg = &on_off_irq_cfg, .p_api = &g_external_irq_on_icu };
#if (14) != BSP_IRQ_DISABLED
#if !defined(SSP_SUPPRESS_ISR_cover_open_irq) && !defined(SSP_SUPPRESS_ISR_ICU0)
SSP_VECTOR_DEFINE( icu_irq_isr, ICU, IRQ0);
#endif
#endif
static icu_instance_ctrl_t cover_open_irq_ctrl;
static const external_irq_cfg_t cover_open_irq_cfg =
{ .channel = 0, .trigger = EXTERNAL_IRQ_TRIG_FALLING, .filter_enable = false, .pclk_div = EXTERNAL_IRQ_PCLK_DIV_BY_64,
  .autostart = true, .p_callback = irq_callback, .p_context = &cover_open_irq, .p_extend = NULL, .irq_ipl = (14), };
/* Instance structure to use this module. */
const external_irq_instance_t cover_open_irq =
{ .p_ctrl = &cover_open_irq_ctrl, .p_cfg = &cover_open_irq_cfg, .p_api = &g_external_irq_on_icu };
#if (BSP_IRQ_DISABLED) != BSP_IRQ_DISABLED
#if !defined(SSP_SUPPRESS_ISR_iRTC) && !defined(SSP_SUPPRESS_ISR_RTC)
SSP_VECTOR_DEFINE(rtc_alarm_isr, RTC, ALARM);
#endif
#endif
#if (BSP_IRQ_DISABLED) != BSP_IRQ_DISABLED
#if !defined(SSP_SUPPRESS_ISR_iRTC) && !defined(SSP_SUPPRESS_ISR_RTC)
SSP_VECTOR_DEFINE(rtc_period_isr, RTC, PERIOD);
#endif
#endif
#if (15) != BSP_IRQ_DISABLED
#if !defined(SSP_SUPPRESS_ISR_iRTC) && !defined(SSP_SUPPRESS_ISR_RTC)
SSP_VECTOR_DEFINE( rtc_carry_isr, RTC, CARRY);
#endif
#endif
rtc_instance_ctrl_t iRTC_ctrl;
const rtc_cfg_t iRTC_cfg =
{ .clock_source = RTC_CLOCK_SOURCE_LOCO, .hw_cfg = true, .error_adjustment_value = 0, .error_adjustment_type =
          RTC_ERROR_ADJUSTMENT_NONE,
  .p_callback = NULL, .p_context = &iRTC, .alarm_ipl = (BSP_IRQ_DISABLED), .periodic_ipl = (BSP_IRQ_DISABLED),
  .carry_ipl = (15), };
/* Instance structure to use this module. */
const rtc_instance_t iRTC =
{ .p_ctrl = &iRTC_ctrl, .p_cfg = &iRTC_cfg, .p_api = &g_rtc_on_rtc };
#if (BSP_IRQ_DISABLED) != BSP_IRQ_DISABLED
#if !defined(SSP_SUPPRESS_ISR_g_transfer1) && !defined(SSP_SUPPRESS_ISR_DTCELC_EVENT_SCI3_RXI)
#define DTC_ACTIVATION_SRC_ELC_EVENT_SCI3_RXI
#if defined(DTC_ACTIVATION_SRC_ELC_EVENT_ELC_SOFTWARE_EVENT_0) && !defined(DTC_VECTOR_DEFINED_SOFTWARE_EVENT_0)
SSP_VECTOR_DEFINE(elc_software_event_isr, ELC, SOFTWARE_EVENT_0);
#define DTC_VECTOR_DEFINED_SOFTWARE_EVENT_0
#endif
#if defined(DTC_ACTIVATION_SRC_ELC_EVENT_ELC_SOFTWARE_EVENT_1) && !defined(DTC_VECTOR_DEFINED_SOFTWARE_EVENT_1)
SSP_VECTOR_DEFINE(elc_software_event_isr, ELC, SOFTWARE_EVENT_1);
#define DTC_VECTOR_DEFINED_SOFTWARE_EVENT_1
#endif
#endif
#endif

dtc_instance_ctrl_t g_transfer1_ctrl;
transfer_info_t g_transfer1_info =
{ .dest_addr_mode = TRANSFER_ADDR_MODE_INCREMENTED, .repeat_area = TRANSFER_REPEAT_AREA_DESTINATION, .irq =
          TRANSFER_IRQ_END,
  .chain_mode = TRANSFER_CHAIN_MODE_DISABLED, .src_addr_mode = TRANSFER_ADDR_MODE_FIXED, .size = TRANSFER_SIZE_1_BYTE,
  .mode = TRANSFER_MODE_NORMAL, .p_dest = (void *) NULL, .p_src = (void const *) NULL, .num_blocks = 0, .length = 0, };
const transfer_cfg_t g_transfer1_cfg =
{ .p_info = &g_transfer1_info, .activation_source = ELC_EVENT_SCI3_RXI, .auto_enable = false, .p_callback = NULL,
  .p_context = &g_transfer1, .irq_ipl = (BSP_IRQ_DISABLED) };
/* Instance structure to use this module. */
const transfer_instance_t g_transfer1 =
{ .p_ctrl = &g_transfer1_ctrl, .p_cfg = &g_transfer1_cfg, .p_api = &g_transfer_on_dtc };
#if (BSP_IRQ_DISABLED) != BSP_IRQ_DISABLED
#if !defined(SSP_SUPPRESS_ISR_g_transfer0) && !defined(SSP_SUPPRESS_ISR_DTCELC_EVENT_SCI3_TXI)
#define DTC_ACTIVATION_SRC_ELC_EVENT_SCI3_TXI
#if defined(DTC_ACTIVATION_SRC_ELC_EVENT_ELC_SOFTWARE_EVENT_0) && !defined(DTC_VECTOR_DEFINED_SOFTWARE_EVENT_0)
SSP_VECTOR_DEFINE(elc_software_event_isr, ELC, SOFTWARE_EVENT_0);
#define DTC_VECTOR_DEFINED_SOFTWARE_EVENT_0
#endif
#if defined(DTC_ACTIVATION_SRC_ELC_EVENT_ELC_SOFTWARE_EVENT_1) && !defined(DTC_VECTOR_DEFINED_SOFTWARE_EVENT_1)
SSP_VECTOR_DEFINE(elc_software_event_isr, ELC, SOFTWARE_EVENT_1);
#define DTC_VECTOR_DEFINED_SOFTWARE_EVENT_1
#endif
#endif
#endif

dtc_instance_ctrl_t g_transfer0_ctrl;
transfer_info_t g_transfer0_info =
{ .dest_addr_mode = TRANSFER_ADDR_MODE_FIXED, .repeat_area = TRANSFER_REPEAT_AREA_SOURCE, .irq = TRANSFER_IRQ_END,
  .chain_mode = TRANSFER_CHAIN_MODE_DISABLED, .src_addr_mode = TRANSFER_ADDR_MODE_INCREMENTED, .size =
          TRANSFER_SIZE_1_BYTE,
  .mode = TRANSFER_MODE_NORMAL, .p_dest = (void *) NULL, .p_src = (void const *) NULL, .num_blocks = 0, .length = 0, };
const transfer_cfg_t g_transfer0_cfg =
{ .p_info = &g_transfer0_info, .activation_source = ELC_EVENT_SCI3_TXI, .auto_enable = false, .p_callback = NULL,
  .p_context = &g_transfer0, .irq_ipl = (BSP_IRQ_DISABLED) };
/* Instance structure to use this module. */
const transfer_instance_t g_transfer0 =
{ .p_ctrl = &g_transfer0_ctrl, .p_cfg = &g_transfer0_cfg, .p_api = &g_transfer_on_dtc };
#if SCI_UART_CFG_RX_ENABLE
#if (12) != BSP_IRQ_DISABLED
#if !defined(SSP_SUPPRESS_ISR_traceUART) && !defined(SSP_SUPPRESS_ISR_SCI3)
SSP_VECTOR_DEFINE_CHAN(sci_uart_rxi_isr, SCI, RXI, 3);
#endif
#endif
#endif
#if SCI_UART_CFG_TX_ENABLE
#if (12) != BSP_IRQ_DISABLED
#if !defined(SSP_SUPPRESS_ISR_traceUART) && !defined(SSP_SUPPRESS_ISR_SCI3)
SSP_VECTOR_DEFINE_CHAN(sci_uart_txi_isr, SCI, TXI, 3);
#endif
#endif
#if (12) != BSP_IRQ_DISABLED
#if !defined(SSP_SUPPRESS_ISR_traceUART) && !defined(SSP_SUPPRESS_ISR_SCI3)
SSP_VECTOR_DEFINE_CHAN(sci_uart_tei_isr, SCI, TEI, 3);
#endif
#endif
#endif
#if SCI_UART_CFG_RX_ENABLE
#if (BSP_IRQ_DISABLED) != BSP_IRQ_DISABLED
#if !defined(SSP_SUPPRESS_ISR_traceUART) && !defined(SSP_SUPPRESS_ISR_SCI3)
SSP_VECTOR_DEFINE_CHAN(sci_uart_eri_isr, SCI, ERI, 3);
#endif
#endif
#endif
sci_uart_instance_ctrl_t traceUART_ctrl;

/** UART extended configuration for UARTonSCI HAL driver */
const uart_on_sci_cfg_t traceUART_cfg_extend =
{ .clk_src = SCI_CLK_SRC_INT, .baudclk_out = false, .rx_edge_start = true, .noisecancel_en = false, .p_extpin_ctrl =
          NULL,
  .bitrate_modulation = true, .rx_fifo_trigger = SCI_UART_RX_FIFO_TRIGGER_1, .baud_rate_error_x_1000 = (uint32_t) (
          2.0 * 1000),
  .uart_comm_mode = UART_MODE_RS232, .uart_rs485_mode = UART_RS485_HD, .rs485_de_pin = IOPORT_PORT_09_PIN_14, };

/** UART interface configuration */
const uart_cfg_t traceUART_cfg =
{ .channel = 3, .baud_rate = 250000, .data_bits = UART_DATA_BITS_8, .parity = UART_PARITY_OFF, .stop_bits =
          UART_STOP_BITS_1,
  .ctsrts_en = false, .p_callback = trace_uart_callback, .p_context = &traceUART, .p_extend = &traceUART_cfg_extend,
#define SYNERGY_NOT_DEFINED (1)                        
#if (SYNERGY_NOT_DEFINED == g_transfer0)
  .p_transfer_tx = NULL,
#else
  .p_transfer_tx = &g_transfer0,
#endif            
#if (SYNERGY_NOT_DEFINED == g_transfer1)
  .p_transfer_rx = NULL,
#else
  .p_transfer_rx = &g_transfer1,
#endif   
#undef SYNERGY_NOT_DEFINED            
  .rxi_ipl = (12),
  .txi_ipl = (12), .tei_ipl = (12), .eri_ipl = (BSP_IRQ_DISABLED), };

/* Instance structure to use this module. */
const uart_instance_t traceUART =
{ .p_ctrl = &traceUART_ctrl, .p_cfg = &traceUART_cfg, .p_api = &g_uart_on_sci };
void g_hal_init(void)
{
    g_common_init ();
}
