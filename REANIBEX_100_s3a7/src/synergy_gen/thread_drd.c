/* generated thread source file - do not edit */
#include "thread_drd.h"

TX_THREAD thread_drd;
void thread_drd_create(void);
static void thread_drd_func(ULONG thread_input);
static uint8_t thread_drd_stack[1024] BSP_PLACE_IN_SECTION_V2(".stack.thread_drd") BSP_ALIGN_VARIABLE_V2(BSP_STACK_ALIGNMENT);
void tx_startup_err_callback(void *p_instance, void *p_data);
void tx_startup_common_init(void);
TX_QUEUE queue_drd;
static uint8_t queue_memory_queue_drd[20];
TX_EVENT_FLAGS_GROUP ev_drd_diag_ready;
extern bool g_ssp_common_initialized;
extern uint32_t g_ssp_common_thread_count;
extern TX_SEMAPHORE g_ssp_common_initialized_semaphore;

void thread_drd_create(void)
{
    /* Increment count so we will know the number of ISDE created threads. */
    g_ssp_common_thread_count++;

    /* Initialize each kernel object. */
    UINT err_queue_drd;
    err_queue_drd = tx_queue_create (&queue_drd, (CHAR *) "Queue DRD", 1, &queue_memory_queue_drd,
                                     sizeof(queue_memory_queue_drd));
    if (TX_SUCCESS != err_queue_drd)
    {
        tx_startup_err_callback (&queue_drd, 0);
    }
    UINT err_ev_drd_diag_ready;
    err_ev_drd_diag_ready = tx_event_flags_create (&ev_drd_diag_ready, (CHAR *) "DRD Diagnostic Ready");
    if (TX_SUCCESS != err_ev_drd_diag_ready)
    {
        tx_startup_err_callback (&ev_drd_diag_ready, 0);
    }

    UINT err;
    err = tx_thread_create (&thread_drd, (CHAR *) "Thread DRD", thread_drd_func, (ULONG) NULL, &thread_drd_stack, 1024,
                            8, 8, 1, TX_DONT_START);
    if (TX_SUCCESS != err)
    {
        tx_startup_err_callback (&thread_drd, 0);
    }
}

static void thread_drd_func(ULONG thread_input)
{
    /* Not currently using thread_input. */
    SSP_PARAMETER_NOT_USED (thread_input);

    /* Initialize common components */
    tx_startup_common_init ();

    /* Initialize each module instance. */

    /* Enter user code for this thread. */
    thread_drd_entry ();
}
