/*
 * CU_main.c
 *
 *  Created on: 7 mar. 2024
 *      Author: asier
 */

/*
 * CU_main.c
 *
 *  Created on: 24 ene. 2024
 *      Author: asier
 */

#include "CU_header.h"

int CUnit_main(void);

int CUnit_main(void){

    int CU_device_info_SN;

    // Define the run mode for the basic interface
    // Verbose mode - maximum output of run details
        CU_device_info_SN = CU_getDevice_info();
        CU_BasicRunMode mode = CU_BRM_VERBOSE;

    // Define error action
    // Runs should be continued when an error condition occurs (if possible)
        CU_ErrorAction error_action = CUEA_IGNORE;

    // Initialize the framework test registry
        if (CU_initialize_registry()) {
            printf("Initialization of Test Registry failed.\n");
        }
        else
        {
            //CU_test_sysMon();
            //CU_test_audio();
            //CU_test_defibrillator();
            //CU_test_core();
            //CU_test_patMon();

            CU_test_comm();

            // Set the basic run mode, which controls the output during test
            CU_basic_set_mode(mode);

            // Set the error action
            CU_set_error_action(error_action);

            // Run all tests in all registered suites
            //printf("Tests completed with return value %d.\n",

            CU_basic_run_tests();
            //CU_automated_run_tests();

            // Clean up and release memory used by the framework
            CU_cleanup_registry();

        }

return 0;
}

