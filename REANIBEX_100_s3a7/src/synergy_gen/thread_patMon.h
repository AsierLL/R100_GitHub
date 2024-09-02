/* generated thread header file - do not edit */
#ifndef THREAD_PATMON_H_
#define THREAD_PATMON_H_
#include "bsp_api.h"
#include "tx_api.h"
#include "hal_data.h"
#ifdef __cplusplus
extern "C" void thread_patMon_entry(void);
#else
extern void thread_patMon_entry(void);
#endif
#include "r_icu.h"
#include "r_external_irq_api.h"
#include "r_dac.h"
#include "r_dac_api.h"
#include "sf_spi.h"
#include "sf_spi_api.h"
#ifdef __cplusplus
extern "C"
{
#endif
/* External IRQ on ICU Instance. */
extern const external_irq_instance_t g_external_irq;
#ifndef g_irq8_callback
void g_irq8_callback(external_irq_callback_args_t *p_args);
#endif
/** DAC on DAC Instance. */
extern const dac_instance_t g_dac0;
/* SF SPI on SF SPI Instance. */
extern const sf_spi_instance_t spi_patmon;
extern TX_EVENT_FLAGS_GROUP g_events_PatMon;
#ifdef __cplusplus
} /* extern "C" */
#endif
#endif /* THREAD_PATMON_H_ */
