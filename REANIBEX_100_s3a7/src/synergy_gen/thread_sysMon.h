/* generated thread header file - do not edit */
#ifndef THREAD_SYSMON_H_
#define THREAD_SYSMON_H_
#include "bsp_api.h"
#include "tx_api.h"
#include "hal_data.h"
#ifdef __cplusplus
extern "C" void thread_sysMon_entry(void);
#else
extern void thread_sysMon_entry(void);
#endif
#ifdef __cplusplus
extern "C"
{
#endif
extern TX_SEMAPHORE i2c_2_semaphore;
extern TX_SEMAPHORE i2c_1_semaphore;
#ifdef __cplusplus
} /* extern "C" */
#endif
#endif /* THREAD_SYSMON_H_ */
