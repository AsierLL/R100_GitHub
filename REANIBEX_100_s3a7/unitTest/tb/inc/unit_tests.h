/******************************************************************************
* Name      : R700C                                                           *
* Copyright : Osatu S. Coop                                                   *
* Compiler  : Renesas C/C++ compiler for SH2                                  *
* Target    : Renesas SH-2                                                    *
******************************************************************************/

/*!
* @file        unit_tests.h
* @brief       Prototypse for Unit Tests Suites
* @version     0.0
* @date        2017-01-03
* @author      JAA (jarribas@bexencardio.com)
* @warning     JAA  2017-01-03        V0.0      .- First edition
*
*
*/
/******************************************************************************
**Includes
*/
#include "types_basic.h"

#ifndef UNIT_TEST_H  // Entry, use file only if it's not already included.
#define UNIT_TEST_H

/******************************************************************************
** Defines
*/

#define     KEYPAD_TESTS       (0x00000001)      ///< Execute Keypad.c Unit Test Suite
#define     I2C_TESTS          (0x00000002)      ///< Execute XXXXXX.c Unit Test Suite
#define     X01_TESTS          (0x00000004)      ///< Execute XXXXXX.c Unit Test Suite
#define     X02_TESTS          (0x00000008)      ///< Execute XXXXXX.c Unit Test Suite
#define     X03_TESTS          (0x00000010)      ///< Execute XXXXXX.c Unit Test Suite
#define     X04_TESTS          (0x00000020)      ///< Execute XXXXXX.c Unit Test Suite
#define     X05_TESTS          (0x00000040)      ///< Execute XXXXXX.c Unit Test Suite
#define     X06_TESTS          (0x00000080)      ///< Execute XXXXXX.c Unit Test Suite
#define     X07_TESTS          (0x00000100)      ///< Execute XXXXXX.c Unit Test Suite
#define     X08_TESTS          (0x00000200)      ///< Execute XXXXXX.c Unit Test Suite
#define     X09_TESTS          (0x00000400)      ///< Execute XXXXXX.c Unit Test Suite
#define     X10_TESTS          (0x00000800)      ///< Execute XXXXXX.c Unit Test Suite
#define     X11_TESTS          (0x00010000)      ///< Execute XXXXXX.c Unit Test Suite

#define     ALL_TESTS          (0xFFFFFFFF)      ///< Execute All Unit Test Suites

/******************************************************************************
** Typedefs
*/

/******************************************************************************
** Globals
*/

/******************************************************************************
** Prototypes
*/

void_t Run_All_Unit_Test_Suites (void_t);

/*====================================================================
************************ SRC UNIT TEST SUITES ************************
=====================================================================*/
void_t Run_Keypad_Unit_Test_Suite(void_t);
//void_t Run_XXXXXX_Unit_Test_Suite(void_t);
#endif  /*UNIT_TEST_H*/

/*
** $Log$
**
** end of unit_test.h
******************************************************************************/
