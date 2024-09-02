/*
 * Keypad_tests.c
 *
 *  Created on: 29 may. 2018
 *      Author: SABIN
 */
#include "unity.h"
#include "unit_tests.h"
#include "Keypad.h"
#include "types_app.h"
#include "hal_data.h"

/**
 *
 * @param
 * @return
 */
static void Test_Led_On_Onboard_Leds (void)
{
    ioport_level_t led_level = LED_VALUE_OFF;

    Led_On (LED_ONOFF);
    Led_On (LED_SHOCK);
    Led_On (LED_PATYPE);
    Led_On (LED_LIFE_BEAT);

    g_ioport.p_api->pinRead (LED_ONOFF, &led_level);
    TEST_ASSERT_EQUAL_INT(LED_VALUE_ON,led_level);
    led_level = LED_VALUE_OFF;
    g_ioport.p_api->pinRead (LED_SHOCK, &led_level);
    TEST_ASSERT_EQUAL_INT(LED_VALUE_ON,led_level);
    led_level = LED_VALUE_OFF;
    g_ioport.p_api->pinRead (LED_PATYPE, &led_level);
    TEST_ASSERT_EQUAL_INT(LED_VALUE_ON,led_level);
    g_ioport.p_api->pinRead (LED_LIFE_BEAT, &led_level);
    TEST_ASSERT_EQUAL_INT(LED_VALUE_ON,led_level);
}

static void Test_Led_Off_Onboard_Leds (void)
{
    ioport_level_t led_level = LED_VALUE_ON;

    Led_Off (LED_ONOFF);
    Led_Off (LED_SHOCK);
    Led_Off (LED_PATYPE);
    Led_Off (LED_LIFE_BEAT);

    g_ioport.p_api->pinRead (LED_ONOFF, &led_level);
    TEST_ASSERT_EQUAL_INT(LED_VALUE_OFF,led_level);
    led_level = LED_VALUE_ON;
    g_ioport.p_api->pinRead (LED_SHOCK, &led_level);
    TEST_ASSERT_EQUAL_INT(LED_VALUE_OFF,led_level);
    led_level = LED_VALUE_ON;
    g_ioport.p_api->pinRead (LED_PATYPE, &led_level);
    TEST_ASSERT_EQUAL_INT(LED_VALUE_OFF,led_level);
    g_ioport.p_api->pinRead (LED_LIFE_BEAT, &led_level);
    TEST_ASSERT_EQUAL_INT(LED_VALUE_OFF,led_level);
}

static void Test_Led_Toggle_Onboard_Leds (void)
{
    ioport_level_t led_level = LED_VALUE_ON;

    Led_Off (LED_ONOFF);
    Led_On (LED_SHOCK);
    Led_Off (LED_PATYPE);
    Led_On (LED_LIFE_BEAT);

    Led_Toggle (LED_ONOFF);
    Led_Toggle (LED_SHOCK);
    Led_Toggle (LED_PATYPE);
    Led_Toggle (LED_LIFE_BEAT);

    g_ioport.p_api->pinRead (LED_ONOFF, &led_level);
    TEST_ASSERT_EQUAL_INT(LED_VALUE_ON,led_level);

    g_ioport.p_api->pinRead (LED_SHOCK, &led_level);
    TEST_ASSERT_EQUAL_INT(LED_VALUE_OFF,led_level);

    g_ioport.p_api->pinRead (LED_PATYPE, &led_level);
    TEST_ASSERT_EQUAL_INT(LED_VALUE_ON,led_level);

    g_ioport.p_api->pinRead (LED_LIFE_BEAT, &led_level);
    TEST_ASSERT_EQUAL_INT(LED_VALUE_OFF,led_level);
}



/**
 *
 * @param
 * @return
 */
void Run_Keypad_Unit_Test_Suite (void)
{
    //Comm_Test_Trace("#TI: APP Layer AED Unit Test Suite not available");
    //Comm_Test_Trace("#TI: App AED Unit Test_Suite Begin");
    UNITY_BEGIN();
    RUN_TEST(Test_Led_On_Onboard_Leds);
    RUN_TEST(Test_Led_Off_Onboard_Leds);
    RUN_TEST(Test_Led_Toggle_Onboard_Leds);
    UNITY_END();
    //Comm_Test_Trace("#TI: App AED Unit Test_Suite End");
}
