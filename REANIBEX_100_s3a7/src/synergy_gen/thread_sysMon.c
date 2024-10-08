/* generated thread source file - do not edit */
#include "thread_sysMon.h"

TX_THREAD thread_sysMon;
void thread_sysMon_create(void);
static void thread_sysMon_func(ULONG thread_input);
static uint8_t thread_sysMon_stack[5600] BSP_PLACE_IN_SECTION_V2(".stack.thread_sysMon") BSP_ALIGN_VARIABLE_V2(BSP_STACK_ALIGNMENT);
void tx_startup_err_callback(void *p_instance, void *p_data);
void tx_startup_common_init(void);
TX_SEMAPHORE i2c_2_semaphore;
TX_SEMAPHORE i2c_1_semaphore;
extern bool g_ssp_common_initialized;
extern uint32_t g_ssp_common_thread_count;
extern TX_SEMAPHORE g_ssp_common_initialized_semaphore;

void thread_sysMon_create(void)
{
    /* Increment count so we will know the number of ISDE created threads. */
    g_ssp_common_thread_count++;

    /* Initialize each kernel object. */
    UINT err_i2c_2_semaphore;
    err_i2c_2_semaphore = tx_semaphore_create (&i2c_2_semaphore, (CHAR *) "i2c_2_semaphore", 0);
    if (TX_SUCCESS != err_i2c_2_semaphore)
    {
        tx_startup_err_callback (&i2c_2_semaphore, 0);
    }
    UINT err_i2c_1_semaphore;
    err_i2c_1_semaphore = tx_semaphore_create (&i2c_1_semaphore, (CHAR *) "i2c_1_semaphore", 1);
    if (TX_SUCCESS != err_i2c_1_semaphore)
    {
        tx_startup_err_callback (&i2c_1_semaphore, 0);
    }
/*
    UINT err;
    err = tx_thread_create (&thread_sysMon, (CHAR *) "Thread SysMon", thread_sysMon_func, (ULONG) NULL,
                            &thread_sysMon_stack, 5600, 20, 20, 1, TX_AUTO_START);
    if (TX_SUCCESS != err)
    {
        tx_startup_err_callback (&thread_sysMon, 0);
    }
*/
    thread_sysMon_func(NULL);
}

static void thread_sysMon_func(ULONG thread_input)
{
    /* Not currently using thread_input. */
    SSP_PARAMETER_NOT_USED (thread_input);

    /* Initialize common components */
    tx_startup_common_init ();

    /* Initialize each module instance. */

    /* Enter user code for this thread. */
    thread_sysMon_entry ();
}
