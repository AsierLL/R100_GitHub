/* generated thread header file - do not edit */
#ifndef THREAD_DRD_H_
#define THREAD_DRD_H_
#include "bsp_api.h"
#include "tx_api.h"
#include "hal_data.h"
#ifdef __cplusplus
extern "C" void thread_drd_entry(void);
#else
extern void thread_drd_entry(void);
#endif
#ifdef __cplusplus
extern "C"
{
#endif
extern TX_QUEUE queue_drd;
extern TX_EVENT_FLAGS_GROUP ev_drd_diag_ready;
#ifdef __cplusplus
} /* extern "C" */
#endif
#endif /* THREAD_DRD_H_ */
