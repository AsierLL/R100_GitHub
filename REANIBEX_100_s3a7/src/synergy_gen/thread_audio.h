/* generated thread header file - do not edit */
#ifndef THREAD_AUDIO_H_
#define THREAD_AUDIO_H_
#include "bsp_api.h"
#include "tx_api.h"
#include "hal_data.h"
#ifdef __cplusplus
extern "C" void thread_audio_entry(void);
#else
extern void thread_audio_entry(void);
#endif
#include "r_dmac.h"
#include "r_transfer_api.h"
#include "r_gpt.h"
#include "r_timer_api.h"
#include "r_dac.h"
#include "r_dac_api.h"
#ifdef __cplusplus
extern "C"
{
#endif
/* Transfer on DMAC Instance. */
extern const transfer_instance_t dma_audio;
#ifndef dma_audio_callback
void dma_audio_callback(transfer_callback_args_t *p_args);
#endif
/** Timer on GPT Instance. */
extern const timer_instance_t timer_audio;
#ifndef NULL
void NULL(timer_callback_args_t *p_args);
#endif
/** DAC on DAC Instance. */
extern const dac_instance_t dac_audio;
extern TX_SEMAPHORE audio_semaphore;
extern TX_EVENT_FLAGS_GROUP g_events_Audio;
extern TX_MUTEX usd_mutex;
#ifdef __cplusplus
} /* extern "C" */
#endif
#endif /* THREAD_AUDIO_H_ */
