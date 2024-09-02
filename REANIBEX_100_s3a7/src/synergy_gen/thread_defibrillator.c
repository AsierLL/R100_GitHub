/* generated thread source file - do not edit */
#include "thread_defibrillator.h"

TX_THREAD thread_defibrillator;
void thread_defibrillator_create(void);
static void thread_defibrillator_func(ULONG thread_input);
static uint8_t thread_defibrillator_stack[1024] BSP_PLACE_IN_SECTION_V2(".stack.thread_defibrillator") BSP_ALIGN_VARIABLE_V2(BSP_STACK_ALIGNMENT);
void tx_startup_err_callback(void *p_instance, void *p_data);
void tx_startup_common_init(void);
#if (10) != BSP_IRQ_DISABLED
#if !defined(SSP_SUPPRESS_ISR_gpt_shock_L) && !defined(SSP_SUPPRESS_ISR_GPT8)
SSP_VECTOR_DEFINE_CHAN(gpt_counter_overflow_isr, GPT, COUNTER_OVERFLOW, 8);
#endif
#endif
static gpt_instance_ctrl_t gpt_shock_L_ctrl;
static const timer_on_gpt_cfg_t gpt_shock_L_extend =
{ .gtioca =
{ .output_enabled = true, .stop_level = GPT_PIN_LEVEL_LOW },
  .gtiocb =
  { .output_enabled = true, .stop_level = GPT_PIN_LEVEL_LOW },
  .shortest_pwm_signal = GPT_SHORTEST_LEVEL_OFF, };
static const timer_cfg_t gpt_shock_L_cfg =
{ .mode = TIMER_MODE_ONE_SHOT, .period = 20000, .unit = TIMER_UNIT_PERIOD_USEC, .duty_cycle = 0, .duty_cycle_unit =
          TIMER_PWM_UNIT_RAW_COUNTS,
  .channel = 8, .autostart = false, .p_callback = GPT_Shock_Callback, .p_context = &gpt_shock_L, .p_extend =
          &gpt_shock_L_extend,
  .irq_ipl = (10), };
/* Instance structure to use this module. */
const timer_instance_t gpt_shock_L =
{ .p_ctrl = &gpt_shock_L_ctrl, .p_cfg = &gpt_shock_L_cfg, .p_api = &g_timer_on_gpt };
#if (BSP_IRQ_DISABLED) != BSP_IRQ_DISABLED
#if !defined(SSP_SUPPRESS_ISR_gpt_shock_H) && !defined(SSP_SUPPRESS_ISR_GPT1)
SSP_VECTOR_DEFINE_CHAN(gpt_counter_overflow_isr, GPT, COUNTER_OVERFLOW, 1);
#endif
#endif
static gpt_instance_ctrl_t gpt_shock_H_ctrl;
static const timer_on_gpt_cfg_t gpt_shock_H_extend =
{ .gtioca =
{ .output_enabled = true, .stop_level = GPT_PIN_LEVEL_LOW },
  .gtiocb =
  { .output_enabled = true, .stop_level = GPT_PIN_LEVEL_LOW },
  .shortest_pwm_signal = GPT_SHORTEST_LEVEL_OFF, };
static const timer_cfg_t gpt_shock_H_cfg =
{ .mode = TIMER_MODE_PWM, .period = 120, .unit = TIMER_UNIT_FREQUENCY_KHZ, .duty_cycle = 50, .duty_cycle_unit =
          TIMER_PWM_UNIT_PERCENT,
  .channel = 1, .autostart = false, .p_callback = NULL, .p_context = &gpt_shock_H, .p_extend = &gpt_shock_H_extend,
  .irq_ipl = (BSP_IRQ_DISABLED), };
/* Instance structure to use this module. */
const timer_instance_t gpt_shock_H =
{ .p_ctrl = &gpt_shock_H_ctrl, .p_cfg = &gpt_shock_H_cfg, .p_api = &g_timer_on_gpt };
#if (BSP_IRQ_DISABLED) != BSP_IRQ_DISABLED
#if !defined(SSP_SUPPRESS_ISR_adc_defib) && !defined(SSP_SUPPRESS_ISR_ADC0)
SSP_VECTOR_DEFINE_CHAN(adc_scan_end_isr, ADC, SCAN_END, 0);
#endif
#endif
#if (BSP_IRQ_DISABLED) != BSP_IRQ_DISABLED
#if !defined(SSP_SUPPRESS_ISR_adc_defib) && !defined(SSP_SUPPRESS_ISR_ADC0)
SSP_VECTOR_DEFINE_CHAN(adc_scan_end_b_isr, ADC, SCAN_END_B, 0);
#endif
#endif
adc_instance_ctrl_t adc_defib_ctrl;
const adc_cfg_t adc_defib_cfg =
{ .unit = 0, .mode = ADC_MODE_SINGLE_SCAN, .resolution = ADC_RESOLUTION_12_BIT, .alignment = ADC_ALIGNMENT_RIGHT,
  .add_average_count = ADC_ADD_AVERAGE_FOUR, .clearing = ADC_CLEAR_AFTER_READ_OFF, .trigger = ADC_TRIGGER_SOFTWARE,
  .trigger_group_b = ADC_TRIGGER_SYNC_ELC, .p_callback = NULL, .p_context = &adc_defib, .scan_end_ipl =
          (BSP_IRQ_DISABLED),
  .scan_end_b_ipl = (BSP_IRQ_DISABLED), .calib_adc_skip = true, };
const adc_channel_cfg_t adc_defib_channel_cfg =
{ .scan_mask = (uint32_t) (
        ((uint64_t) ADC_MASK_CHANNEL_0) | ((uint64_t) ADC_MASK_CHANNEL_1) | ((uint64_t) ADC_MASK_CHANNEL_2)
                | ((uint64_t) ADC_MASK_CHANNEL_3) | ((uint64_t) ADC_MASK_CHANNEL_4) | ((uint64_t) ADC_MASK_CHANNEL_5)
                | ((uint64_t) 0) | ((uint64_t) 0) | ((uint64_t) 0) | ((uint64_t) 0) | ((uint64_t) 0) | ((uint64_t) 0)
                | ((uint64_t) 0) | ((uint64_t) 0) | ((uint64_t) 0) | ((uint64_t) 0) | ((uint64_t) 0) | ((uint64_t) 0)
                | ((uint64_t) 0) | ((uint64_t) 0) | ((uint64_t) 0) | ((uint64_t) 0) | ((uint64_t) 0) | ((uint64_t) 0)
                | ((uint64_t) 0) | ((uint64_t) 0) | ((uint64_t) 0) | ((uint64_t) 0) | ((uint64_t) 0) | (0)),
  /** Group B channel mask is right shifted by 32 at the end to form the proper mask */
  .scan_mask_group_b = (uint32_t) (
          (((uint64_t) ADC_MASK_CHANNEL_0) | ((uint64_t) ADC_MASK_CHANNEL_1) | ((uint64_t) ADC_MASK_CHANNEL_2)
                  | ((uint64_t) ADC_MASK_CHANNEL_3) | ((uint64_t) ADC_MASK_CHANNEL_4) | ((uint64_t) ADC_MASK_CHANNEL_5)
                  | ((uint64_t) 0) | ((uint64_t) 0) | ((uint64_t) 0) | ((uint64_t) 0) | ((uint64_t) 0) | ((uint64_t) 0)
                  | ((uint64_t) 0) | ((uint64_t) 0) | ((uint64_t) 0) | ((uint64_t) 0) | ((uint64_t) 0) | ((uint64_t) 0)
                  | ((uint64_t) 0) | ((uint64_t) 0) | ((uint64_t) 0) | ((uint64_t) 0) | ((uint64_t) 0) | ((uint64_t) 0)
                  | ((uint64_t) 0) | ((uint64_t) 0) | ((uint64_t) 0) | ((uint64_t) 0) | ((uint64_t) 0) | (0)) >> 32),
  .priority_group_a = ADC_GROUP_A_PRIORITY_OFF, .add_mask = (uint32_t) (
          (0) | (ADC_MASK_CHANNEL_1) | (0) | (ADC_MASK_CHANNEL_3) | (ADC_MASK_CHANNEL_4) | (ADC_MASK_CHANNEL_5) | (0)
                  | (0) | (0) | (0) | (0) | (0) | (0) | (0) | (0) | (0) | (0) | (0) | (0) | (0) | (0) | (0) | (0) | (0)
                  | (0) | (0) | (0) | (0) | (0) | (0)),
  .sample_hold_mask = (uint32_t) ((0) | (0) | (0)), .sample_hold_states = 24, };
/* Instance structure to use this module. */
const adc_instance_t adc_defib =
{ .p_ctrl = &adc_defib_ctrl, .p_cfg = &adc_defib_cfg, .p_channel_cfg = &adc_defib_channel_cfg, .p_api = &g_adc_on_adc };
#if (BSP_IRQ_DISABLED) != BSP_IRQ_DISABLED
#if !defined(SSP_SUPPRESS_ISR_gpt_ovch) && !defined(SSP_SUPPRESS_ISR_GPT3)
SSP_VECTOR_DEFINE_CHAN(gpt_counter_overflow_isr, GPT, COUNTER_OVERFLOW, 3);
#endif
#endif
static gpt_instance_ctrl_t gpt_ovch_ctrl;
static const timer_on_gpt_cfg_t gpt_ovch_extend =
{ .gtioca =
{ .output_enabled = true, .stop_level = GPT_PIN_LEVEL_LOW },
  .gtiocb =
  { .output_enabled = false, .stop_level = GPT_PIN_LEVEL_LOW },
  .shortest_pwm_signal = GPT_SHORTEST_LEVEL_OFF, };
static const timer_cfg_t gpt_ovch_cfg =
{ .mode = TIMER_MODE_PWM, .period = 100, .unit = TIMER_UNIT_PERIOD_USEC, .duty_cycle = 50, .duty_cycle_unit =
          TIMER_PWM_UNIT_PERCENT,
  .channel = 3, .autostart = false, .p_callback = NULL, .p_context = &gpt_ovch, .p_extend = &gpt_ovch_extend, .irq_ipl =
          (BSP_IRQ_DISABLED), };
/* Instance structure to use this module. */
const timer_instance_t gpt_ovch =
{ .p_ctrl = &gpt_ovch_ctrl, .p_cfg = &gpt_ovch_cfg, .p_api = &g_timer_on_gpt };
#if (BSP_IRQ_DISABLED) != BSP_IRQ_DISABLED
#if !defined(SSP_SUPPRESS_ISR_gpt_charger) && !defined(SSP_SUPPRESS_ISR_GPT7)
SSP_VECTOR_DEFINE_CHAN(gpt_counter_overflow_isr, GPT, COUNTER_OVERFLOW, 7);
#endif
#endif
static gpt_instance_ctrl_t gpt_charger_ctrl;
static const timer_on_gpt_cfg_t gpt_charger_extend =
{ .gtioca =
{ .output_enabled = true, .stop_level = GPT_PIN_LEVEL_LOW },
  .gtiocb =
  { .output_enabled = true, .stop_level = GPT_PIN_LEVEL_LOW },
  .shortest_pwm_signal = GPT_SHORTEST_LEVEL_OFF, };
static const timer_cfg_t gpt_charger_cfg =
{ .mode = TIMER_MODE_PWM, .period = 100, .unit = TIMER_UNIT_PERIOD_USEC, .duty_cycle = 5, .duty_cycle_unit =
          TIMER_PWM_UNIT_PERCENT,
  .channel = 7, .autostart = false, .p_callback = NULL, .p_context = &gpt_charger, .p_extend = &gpt_charger_extend,
  .irq_ipl = (BSP_IRQ_DISABLED), };
/* Instance structure to use this module. */
const timer_instance_t gpt_charger =
{ .p_ctrl = &gpt_charger_ctrl, .p_cfg = &gpt_charger_cfg, .p_api = &g_timer_on_gpt };
TX_QUEUE queue_def;
static uint8_t queue_memory_queue_def[20];
extern bool g_ssp_common_initialized;
extern uint32_t g_ssp_common_thread_count;
extern TX_SEMAPHORE g_ssp_common_initialized_semaphore;

void thread_defibrillator_create(void)
{
    /* Increment count so we will know the number of ISDE created threads. */
    g_ssp_common_thread_count++;

    /* Initialize each kernel object. */
    UINT err_queue_def;
    err_queue_def = tx_queue_create (&queue_def, (CHAR *) "Queue Defibrillator", 1, &queue_memory_queue_def,
                                     sizeof(queue_memory_queue_def));
    if (TX_SUCCESS != err_queue_def)
    {
        tx_startup_err_callback (&queue_def, 0);
    }

    UINT err;
    err = tx_thread_create (&thread_defibrillator, (CHAR *) "Thread Defibrillator", thread_defibrillator_func,
                            (ULONG) NULL, &thread_defibrillator_stack, 1024, 14, 14, 1, TX_DONT_START);
    if (TX_SUCCESS != err)
    {
        tx_startup_err_callback (&thread_defibrillator, 0);
    }
}

static void thread_defibrillator_func(ULONG thread_input)
{
    /* Not currently using thread_input. */
    SSP_PARAMETER_NOT_USED (thread_input);

    /* Initialize common components */
    tx_startup_common_init ();

    /* Initialize each module instance. */

    /* Enter user code for this thread. */
    thread_defibrillator_entry ();
}
