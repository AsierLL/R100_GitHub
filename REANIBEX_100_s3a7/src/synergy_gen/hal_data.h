/* generated HAL header file - do not edit */
#ifndef HAL_DATA_H_
#define HAL_DATA_H_
#include <stdint.h>
#include "bsp_api.h"
#include "common_data.h"
#include "r_flash_lp.h"
#include "r_flash_api.h"
#include "r_iwdt.h"
#include "r_wdt_api.h"
#include "r_dtc.h"
#include "r_transfer_api.h"
#include "r_sci_uart.h"
#include "r_uart_api.h"
#include "r_icu.h"
#include "r_external_irq_api.h"
#include "r_rtc.h"
#include "r_rtc_api.h"
#ifdef __cplusplus
extern "C"
{
#endif
/* Flash on Flash LP Instance. */
extern const flash_instance_t g_flash0;
#ifndef NULL
void NULL(flash_callback_args_t *p_args);
#endif
/** WDT on IWDT Instance. */
extern const wdt_instance_t iWDT;
#ifndef NULL
void NULL(wdt_callback_args_t *p_args);
#endif
/* Transfer on DTC Instance. */
extern const transfer_instance_t g_transfer3;
#ifndef NULL
void NULL(transfer_callback_args_t *p_args);
#endif
/* Transfer on DTC Instance. */
extern const transfer_instance_t g_transfer2;
#ifndef NULL
void NULL(transfer_callback_args_t *p_args);
#endif
/** UART on SCI Instance. */
extern const uart_instance_t commUART;
#ifdef NULL
#else
extern void NULL(uint32_t channel, uint32_t level);
#endif
#ifndef comm_uart_callback
void comm_uart_callback(uart_callback_args_t *p_args);
#endif
/* External IRQ on ICU Instance. */
extern const external_irq_instance_t on_off_irq;
#ifndef irq_callback
void irq_callback(external_irq_callback_args_t *p_args);
#endif
/* External IRQ on ICU Instance. */
extern const external_irq_instance_t cover_open_irq;
#ifndef irq_callback
void irq_callback(external_irq_callback_args_t *p_args);
#endif
/** RTC on RTC Instance. */
extern const rtc_instance_t iRTC;
#ifndef NULL
void NULL(rtc_callback_args_t *p_args);
#endif
/* Transfer on DTC Instance. */
extern const transfer_instance_t g_transfer1;
#ifndef NULL
void NULL(transfer_callback_args_t *p_args);
#endif
/* Transfer on DTC Instance. */
extern const transfer_instance_t g_transfer0;
#ifndef NULL
void NULL(transfer_callback_args_t *p_args);
#endif
/** UART on SCI Instance. */
extern const uart_instance_t traceUART;
#ifdef NULL
#else
extern void NULL(uint32_t channel, uint32_t level);
#endif
#ifndef trace_uart_callback
void trace_uart_callback(uart_callback_args_t *p_args);
#endif
void hal_entry(void);
void g_hal_init(void);
#ifdef __cplusplus
} /* extern "C" */
#endif
#endif /* HAL_DATA_H_ */
