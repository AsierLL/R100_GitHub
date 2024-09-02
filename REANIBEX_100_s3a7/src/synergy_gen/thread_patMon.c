/* generated thread source file - do not edit */
#include "thread_patMon.h"

TX_THREAD thread_patMon;
void thread_patMon_create(void);
static void thread_patMon_func(ULONG thread_input);
static uint8_t thread_patMon_stack[2048] BSP_PLACE_IN_SECTION_V2(".stack.thread_patMon") BSP_ALIGN_VARIABLE_V2(BSP_STACK_ALIGNMENT);
void tx_startup_err_callback(void *p_instance, void *p_data);
void tx_startup_common_init(void);
#if (7) != BSP_IRQ_DISABLED
#if !defined(SSP_SUPPRESS_ISR_g_external_irq) && !defined(SSP_SUPPRESS_ISR_ICU8)
SSP_VECTOR_DEFINE( icu_irq_isr, ICU, IRQ8);
#endif
#endif
static icu_instance_ctrl_t g_external_irq_ctrl;
static const external_irq_cfg_t g_external_irq_cfg =
{ .channel = 8, .trigger = EXTERNAL_IRQ_TRIG_FALLING, .filter_enable = false, .pclk_div = EXTERNAL_IRQ_PCLK_DIV_BY_64,
  .autostart = false, .p_callback = g_irq8_callback, .p_context = &g_external_irq, .p_extend = NULL, .irq_ipl = (7), };
/* Instance structure to use this module. */
const external_irq_instance_t g_external_irq =
{ .p_ctrl = &g_external_irq_ctrl, .p_cfg = &g_external_irq_cfg, .p_api = &g_external_irq_on_icu };
dac_instance_ctrl_t g_dac0_ctrl;
const dac_cfg_t g_dac0_cfg =
{ .channel = 0, .ad_da_synchronized = false, .data_format = DAC_DATA_FORMAT_FLUSH_LEFT, .output_amplifier_enabled =
          false,
  .p_extend = NULL };
/* Instance structure to use this module. */
const dac_instance_t g_dac0 =
{ .p_ctrl = &g_dac0_ctrl, .p_cfg = &g_dac0_cfg, .p_api = &g_dac_on_dac };
/** Get driver cfg from bus and use all same settings except slave address and addressing mode. */
const spi_cfg_t spi_patmon_spi_cfg =
{ .channel = spi_bus0_CHANNEL, .operating_mode = spi_bus0_OPERATING_MODE, .clk_phase = SPI_CLK_PHASE_EDGE_EVEN,
  .clk_polarity = SPI_CLK_POLARITY_LOW, .mode_fault = spi_bus0_MODE_FAULT, .bit_order = spi_bus0_BIT_ORDER, .bitrate =
          spi_bus0_BIT_RATE,
  .p_transfer_tx = spi_bus0_P_TRANSFER_TX, .p_transfer_rx = spi_bus0_P_TRANSFER_RX, .p_callback = spi_bus0_P_CALLBACK,
  .p_context = spi_bus0_P_CONTEXT, .rxi_ipl = spi_bus0_RXI_IPL, .txi_ipl = spi_bus0_TXI_IPL,
  .tei_ipl = spi_bus0_TEI_IPL, .eri_ipl = spi_bus0_ERI_IPL, .p_extend = spi_bus0_P_EXTEND, };

sf_spi_instance_ctrl_t spi_patmon_ctrl =
{ .p_lower_lvl_ctrl = &g_spi0_ctrl, };

const sf_spi_cfg_t spi_patmon_cfg =
{ .p_bus = (sf_spi_bus_t *) &spi_bus0, .chip_select = IOPORT_PORT_01_PIN_03, .chip_select_level_active =
          IOPORT_LEVEL_LOW,
  .p_lower_lvl_cfg = &spi_patmon_spi_cfg, };

/* Instance structure to use this module. */
const sf_spi_instance_t spi_patmon =
{ .p_ctrl = &spi_patmon_ctrl, .p_cfg = &spi_patmon_cfg, .p_api = &g_sf_spi_on_sf_spi };
TX_EVENT_FLAGS_GROUP g_events_PatMon;
extern bool g_ssp_common_initialized;
extern uint32_t g_ssp_common_thread_count;
extern TX_SEMAPHORE g_ssp_common_initialized_semaphore;

void thread_patMon_create(void)
{
    /* Increment count so we will know the number of ISDE created threads. */
    g_ssp_common_thread_count++;

    /* Initialize each kernel object. */
    UINT err_g_events_PatMon;
    err_g_events_PatMon = tx_event_flags_create (&g_events_PatMon, (CHAR *) "Events_PatMon");
    if (TX_SUCCESS != err_g_events_PatMon)
    {
        tx_startup_err_callback (&g_events_PatMon, 0);
    }

    UINT err;
    err = tx_thread_create (&thread_patMon, (CHAR *) "Thread PatMon", thread_patMon_func, (ULONG) NULL,
                            &thread_patMon_stack, 2048, 4, 4, 1, TX_DONT_START);
    if (TX_SUCCESS != err)
    {
        tx_startup_err_callback (&thread_patMon, 0);
    }
}

static void thread_patMon_func(ULONG thread_input)
{
    /* Not currently using thread_input. */
    SSP_PARAMETER_NOT_USED (thread_input);

    /* Initialize common components */
    tx_startup_common_init ();

    /* Initialize each module instance. */

    /* Enter user code for this thread. */
    thread_patMon_entry ();
}
