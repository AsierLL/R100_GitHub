/* generated thread header file - do not edit */
#ifndef THREAD_DEFIBRILLATOR_H_
#define THREAD_DEFIBRILLATOR_H_
#include "bsp_api.h"
#include "tx_api.h"
#include "hal_data.h"
#ifdef __cplusplus
extern "C" void thread_defibrillator_entry(void);
#else
extern void thread_defibrillator_entry(void);
#endif
#include "r_gpt.h"
#include "r_timer_api.h"
#include "r_adc.h"
#include "r_adc_api.h"
#ifdef __cplusplus
extern "C"
{
#endif
/** Timer on GPT Instance. */
extern const timer_instance_t gpt_shock_L;
#ifndef GPT_Shock_Callback
ssp_err_t GPT_Shock_Callback(timer_callback_args_t *p_args);
#endif
/** Timer on GPT Instance. */
extern const timer_instance_t gpt_shock_H;
#ifndef NULL
void NULL(timer_callback_args_t *p_args);
#endif
/** ADC on ADC Instance. */
extern const adc_instance_t adc_defib;
#ifndef NULL
void NULL(adc_callback_args_t *p_args);
#endif
/** Timer on GPT Instance. */
extern const timer_instance_t gpt_ovch;
#ifndef NULL
void NULL(timer_callback_args_t *p_args);
#endif
/** Timer on GPT Instance. */
extern const timer_instance_t gpt_charger;
#ifndef NULL
void NULL(timer_callback_args_t *p_args);
#endif
extern TX_QUEUE queue_def;
#ifdef __cplusplus
} /* extern "C" */
#endif
#endif /* THREAD_DEFIBRILLATOR_H_ */
