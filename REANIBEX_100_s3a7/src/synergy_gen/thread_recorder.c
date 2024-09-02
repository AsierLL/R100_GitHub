/* generated thread source file - do not edit */
#include "thread_recorder.h"

TX_THREAD thread_recorder;
void thread_recorder_create(void);
static void thread_recorder_func(ULONG thread_input);
static uint8_t thread_recorder_stack[4096] BSP_PLACE_IN_SECTION_V2(".stack.thread_recorder") BSP_ALIGN_VARIABLE_V2(BSP_STACK_ALIGNMENT);
void tx_startup_err_callback(void *p_instance, void *p_data);
void tx_startup_common_init(void);
TX_QUEUE queue_recorder;
static uint8_t queue_memory_queue_recorder[20];
extern bool g_ssp_common_initialized;
extern uint32_t g_ssp_common_thread_count;
extern TX_SEMAPHORE g_ssp_common_initialized_semaphore;

void thread_recorder_create(void)
{
    /* Increment count so we will know the number of ISDE created threads. */
    g_ssp_common_thread_count++;

    /* Initialize each kernel object. */
    UINT err_queue_recorder;
    err_queue_recorder = tx_queue_create (&queue_recorder, (CHAR *) "Queue Recorder", 1, &queue_memory_queue_recorder,
                                          sizeof(queue_memory_queue_recorder));
    if (TX_SUCCESS != err_queue_recorder)
    {
        tx_startup_err_callback (&queue_recorder, 0);
    }

    UINT err;
    err = tx_thread_create (&thread_recorder, (CHAR *) "Thread Recorder", thread_recorder_func, (ULONG) NULL,
                            &thread_recorder_stack, 4096, 10, 10, 1, TX_DONT_START);
    if (TX_SUCCESS != err)
    {
        tx_startup_err_callback (&thread_recorder, 0);
    }
}

static void thread_recorder_func(ULONG thread_input)
{
    /* Not currently using thread_input. */
    SSP_PARAMETER_NOT_USED (thread_input);

    /* Initialize common components */
    tx_startup_common_init ();

    /* Initialize each module instance. */

    /* Enter user code for this thread. */
    thread_recorder_entry ();
}
