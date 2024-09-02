/* generated thread source file - do not edit */
#include "thread_core.h"

TX_THREAD thread_core;
void thread_core_create(void);
static void thread_core_func(ULONG thread_input);
static uint8_t thread_core_stack[2048] BSP_PLACE_IN_SECTION_V2(".stack.thread_core") BSP_ALIGN_VARIABLE_V2(BSP_STACK_ALIGNMENT);
void tx_startup_err_callback(void *p_instance, void *p_data);
void tx_startup_common_init(void);
TX_SEMAPHORE trace_semaphore;
TX_QUEUE queue_core;
static uint8_t queue_memory_queue_core[20];
TX_EVENT_FLAGS_GROUP g_cdcacm_activate_event_flags0;
TX_SEMAPHORE comm_semaphore;
extern bool g_ssp_common_initialized;
extern uint32_t g_ssp_common_thread_count;
extern TX_SEMAPHORE g_ssp_common_initialized_semaphore;

void thread_core_create(void)
{
    /* Increment count so we will know the number of ISDE created threads. */
    g_ssp_common_thread_count++;

    /* Initialize each kernel object. */
    UINT err_trace_semaphore;
    err_trace_semaphore = tx_semaphore_create (&trace_semaphore, (CHAR *) "Trace Semaphore", 0);
    if (TX_SUCCESS != err_trace_semaphore)
    {
        tx_startup_err_callback (&trace_semaphore, 0);
    }
    UINT err_queue_core;
    err_queue_core = tx_queue_create (&queue_core, (CHAR *) "Queue Core", 1, &queue_memory_queue_core,
                                      sizeof(queue_memory_queue_core));
    if (TX_SUCCESS != err_queue_core)
    {
        tx_startup_err_callback (&queue_core, 0);
    }
    UINT err_g_cdcacm_activate_event_flags0;
    err_g_cdcacm_activate_event_flags0 = tx_event_flags_create (&g_cdcacm_activate_event_flags0,
                                                                (CHAR *) "USB Device Connected");
    if (TX_SUCCESS != err_g_cdcacm_activate_event_flags0)
    {
        tx_startup_err_callback (&g_cdcacm_activate_event_flags0, 0);
    }
    UINT err_comm_semaphore;
    err_comm_semaphore = tx_semaphore_create (&comm_semaphore, (CHAR *) "Comm Semaphore", 0);
    if (TX_SUCCESS != err_comm_semaphore)
    {
        tx_startup_err_callback (&comm_semaphore, 0);
    }

    UINT err;
    err = tx_thread_create (&thread_core, (CHAR *) "Thread Core", thread_core_func, (ULONG) NULL, &thread_core_stack,
                            2048, 6, 6, 1, TX_DONT_START);
    if (TX_SUCCESS != err)
    {
        tx_startup_err_callback (&thread_core, 0);
    }
}

static void thread_core_func(ULONG thread_input)
{
    /* Not currently using thread_input. */
    SSP_PARAMETER_NOT_USED (thread_input);

    /* Initialize common components */
    tx_startup_common_init ();

    /* Initialize each module instance. */

    /* Enter user code for this thread. */
    thread_core_entry ();
}
