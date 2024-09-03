/*
 * CU_header.h
 *
 *  Created on: 7 mar. 2024
 *      Author: asier
 */

#ifndef CUNIT_CU_HEADER_H_
#define CUNIT_CU_HEADER_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "CUnit.h"
#include "Basic.h"
#include "Automated.h"
#include "time.h"
#include "sysMon_Battery.h"
#include "device_init.h"

typedef enum{
    CU_ELECTRODES_DATA_NAME                 = 0,
    CU_ELECTRODES_DATA_EXPIRATION_DATE      = 1,
    CU_ELECTRODES_DATA_EVENT_ID             = 2,
}CU_ELECTRODES_DATA_PROPERTIES;

typedef enum{
    CU_SUCCESS                              = 0,

    CU_ERR_POINTER_IS_NULL                  = 1,
    CU_ERR_CLOCK_START                      = 2,
    CU_ERR_SYSTEM_CLOCK_SET                 = 3,

    CU_ERR_NO_CHANGE_REGISTERED             = 4,
    CU_ERR_CHANGES_DONE                     = 5,

    CU_ERR_FLAG_ERROR_IN_AUDIO              = 6,
    CU_ERR_AUDIO_MAX_ID                     = 7,
    CU_ERR_NO_MESSAGE_TRUE                  = 8,
    CU_ERR_IGNORED_MESSAGES                 = 9,
    CU_ERR_AUDIO_QUEUE_FULL                 = 10,

    CU_ERR_GPT_OVCH_PERIOD_SET              = 11,
    CU_ERR_GPT_OVCH_DUTY_CYCLE_SET          = 12,
    CU_ERR_eDEFIB_STATE_IN_ERROR            = 13,
    CU_ERR_eDEFIB_STATE_OUT_OF_SERVICE      = 14,
    CU_ERR_RELAY_HV_SHOCK_HIGH              = 15,
    CU_ERR_HV_OVC_REARM_HIGH                = 16,
    CU_ERR_HV_OVC_REARM_LOW                 = 17,
    CU_ERR_GPT_SHOCK_H_OPEN                 = 18,
    CU_ERR_GPT_SHOCK_H_START                = 19,

    CU_ERR_MAX_INT32                        = 20,
    CU_ERR_MIN_INT32                        = 21,
    CU_ERR_PARAM_NULL                       = 22,

    CU_ERR_IS_SAT_ERROR                     = 23,
    CU_ERR_BATTERY_REPLACE_CHARGE           = 24,
    CU_ERR_BATTERY_LOW_CHARGE               = 25,
    CU_ERR_COVER_OPEN                       = 26,
    CU_ERR_COVER_CLOSE                      = 27,

    CU_eDEFIB_STATE_IN_ERROR                = 28,
    CU_eDEFIB_STATE_OUT_OF_SERVICE          = 29,

    PRUEBA2                                 = 200,



}CU_ERRORS;

#endif /* CUNIT_CU_HEADER_H_ */
