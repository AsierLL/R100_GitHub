/* generated thread source file - do not edit */
#include "thread_audio.h"

TX_THREAD thread_audio;
void thread_audio_create(void);
static void thread_audio_func(ULONG thread_input);
static uint8_t thread_audio_stack[2048] BSP_PLACE_IN_SECTION_V2(".stack.thread_audio") BSP_ALIGN_VARIABLE_V2(BSP_STACK_ALIGNMENT);
void tx_startup_err_callback(void *p_instance, void *p_data);
void tx_startup_common_init(void);
#if (14) != BSP_IRQ_DISABLED
#if !defined(SSP_SUPPRESS_ISR_dma_audio) && !defined(SSP_SUPPRESS_ISR_DMACELC_EVENT_GPT0_COUNTER_OVERFLOW)
SSP_VECTOR_DEFINE_CHAN(dmac_int_isr, DMAC, INT, 1);
#endif
#endif
dmac_instance_ctrl_t dma_audio_ctrl;
transfer_info_t dma_audio_info =
{ .dest_addr_mode = TRANSFER_ADDR_MODE_FIXED, .repeat_area = TRANSFER_REPEAT_AREA_SOURCE, .irq = TRANSFER_IRQ_EACH,
  .chain_mode = TRANSFER_CHAIN_MODE_DISABLED, .src_addr_mode = TRANSFER_ADDR_MODE_INCREMENTED, .size =
          TRANSFER_SIZE_2_BYTE,
  .mode = TRANSFER_MODE_NORMAL, .p_dest = (void *) &R_DAC->DADRn[0], .p_src = (void const *) NULL, .num_blocks = 0,
  .length = 0, };
const transfer_on_dmac_cfg_t dma_audio_extend =
{ .channel = 1, };
const transfer_cfg_t dma_audio_cfg =
{ .p_info = &dma_audio_info, .activation_source = ELC_EVENT_GPT0_COUNTER_OVERFLOW, .auto_enable = false, .p_callback =
          dma_audio_callback,
  .p_context = &dma_audio, .irq_ipl = (14), .p_extend = &dma_audio_extend, };
/* Instance structure to use this module. */
const transfer_instance_t dma_audio =
{ .p_ctrl = &dma_audio_ctrl, .p_cfg = &dma_audio_cfg, .p_api = &g_transfer_on_dmac };
#if (14) != BSP_IRQ_DISABLED
#if !defined(SSP_SUPPRESS_ISR_timer_audio) && !defined(SSP_SUPPRESS_ISR_GPT0)
SSP_VECTOR_DEFINE_CHAN(gpt_counter_overflow_isr, GPT, COUNTER_OVERFLOW, 0);
#endif
#endif
static gpt_instance_ctrl_t timer_audio_ctrl;
static const timer_on_gpt_cfg_t timer_audio_extend =
{ .gtioca =
{ .output_enabled = false, .stop_level = GPT_PIN_LEVEL_LOW },
  .gtiocb =
  { .output_enabled = false, .stop_level = GPT_PIN_LEVEL_LOW },
  .shortest_pwm_signal = GPT_SHORTEST_LEVEL_OFF, };
static const timer_cfg_t timer_audio_cfg =
{ .mode = TIMER_MODE_PERIODIC, .period = 11025, .unit = TIMER_UNIT_FREQUENCY_HZ, .duty_cycle = 50, .duty_cycle_unit =
          TIMER_PWM_UNIT_RAW_COUNTS,
  .channel = 0, .autostart = true, .p_callback = NULL, .p_context = &timer_audio, .p_extend = &timer_audio_extend,
  .irq_ipl = (14), };
/* Instance structure to use this module. */
const timer_instance_t timer_audio =
{ .p_ctrl = &timer_audio_ctrl, .p_cfg = &timer_audio_cfg, .p_api = &g_timer_on_gpt };
dac_instance_ctrl_t dac_audio_ctrl;
const dac_cfg_t dac_audio_cfg =
{ .channel = 0, .ad_da_synchronized = false, .data_format = DAC_DATA_FORMAT_FLUSH_LEFT, .output_amplifier_enabled =
          false,
  .p_extend = NULL };
/* Instance structure to use this module. */
const dac_instance_t dac_audio =
{ .p_ctrl = &dac_audio_ctrl, .p_cfg = &dac_audio_cfg, .p_api = &g_dac_on_dac };
TX_SEMAPHORE audio_semaphore;
TX_EVENT_FLAGS_GROUP g_events_Audio;
TX_MUTEX usd_mutex;
extern bool g_ssp_common_initialized;
extern uint32_t g_ssp_common_thread_count;
extern TX_SEMAPHORE g_ssp_common_initialized_semaphore;

void thread_audio_create(void)
{
    /* Increment count so we will know the number of ISDE created threads. */
    g_ssp_common_thread_count++;

    /* Initialize each kernel object. */
    UINT err_audio_semaphore;
    err_audio_semaphore = tx_semaphore_create (&audio_semaphore, (CHAR *) "Audio Semaphore", 0);
    if (TX_SUCCESS != err_audio_semaphore)
    {
        tx_startup_err_callback (&audio_semaphore, 0);
    }
    UINT err_g_events_Audio;
    err_g_events_Audio = tx_event_flags_create (&g_events_Audio, (CHAR *) "Events Audio");
    if (TX_SUCCESS != err_g_events_Audio)
    {
        tx_startup_err_callback (&g_events_Audio, 0);
    }
    UINT err_usd_mutex;
    err_usd_mutex = tx_mutex_create (&usd_mutex, (CHAR *) "uSD Mutex", TX_NO_INHERIT);
    if (TX_SUCCESS != err_usd_mutex)
    {
        tx_startup_err_callback (&usd_mutex, 0);
    }

    UINT err;
    err = tx_thread_create (&thread_audio, (CHAR *) "Thread Audio", thread_audio_func, (ULONG) NULL,
                            &thread_audio_stack, 2048, 9, 9, 1, TX_DONT_START);
    if (TX_SUCCESS != err)
    {
        tx_startup_err_callback (&thread_audio, 0);
    }
}

static void thread_audio_func(ULONG thread_input)
{
    /* Not currently using thread_input. */
    SSP_PARAMETER_NOT_USED (thread_input);

    /* Initialize common components */
    tx_startup_common_init ();

    /* Initialize each module instance. */

    /* Enter user code for this thread. */
    thread_audio_entry ();
}
