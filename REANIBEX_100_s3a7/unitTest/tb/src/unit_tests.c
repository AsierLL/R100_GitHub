/*
 * unit_tests.c
 *
 *  Created on: 30 may. 2018
 *      Author: SABIN
 */
#ifdef UNIT_TESTS

#include "unit_tests.h"
#include "types_basic.h"


void Run_All_Unit_Test_Suites (void)
{
    const uint32_t test_mask = ALL_TESTS;

    if(test_mask & KEYPAD_TESTS)
    {
        Run_Keypad_Unit_Test_Suite();
    }

    if(test_mask & I2C_TESTS)
    {
        //Run_I2C_Unit_Test_Suite();
    }
}

#endif
