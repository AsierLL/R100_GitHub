/***********************************************************************************************************************
 * Copyright [2015-2017] Renesas Electronics Corporation and/or its licensors. All Rights Reserved.
 * 
 * This file is part of Renesas SynergyTM Software Package (SSP)
 *
 * The contents of this file (the "contents") are proprietary and confidential to Renesas Electronics Corporation
 * and/or its licensors ("Renesas") and subject to statutory and contractual protections.
 *
 * This file is subject to a Renesas SSP license agreement. Unless otherwise agreed in an SSP license agreement with
 * Renesas: 1) you may not use, copy, modify, distribute, display, or perform the contents; 2) you may not use any name
 * or mark of Renesas for advertising or publicity purposes or in connection with your use of the contents; 3) RENESAS
 * MAKES NO WARRANTY OR REPRESENTATIONS ABOUT THE SUITABILITY OF THE CONTENTS FOR ANY PURPOSE; THE CONTENTS ARE PROVIDED
 * "AS IS" WITHOUT ANY EXPRESS OR IMPLIED WARRANTY, INCLUDING THE IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS FOR A
 * PARTICULAR PURPOSE, AND NON-INFRINGEMENT; AND 4) RENESAS SHALL NOT BE LIABLE FOR ANY DIRECT, INDIRECT, SPECIAL, OR
 * CONSEQUENTIAL DAMAGES, INCLUDING DAMAGES RESULTING FROM LOSS OF USE, DATA, OR PROJECTS, WHETHER IN AN ACTION OF
 * CONTRACT OR TORT, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THE CONTENTS. Third-party contents
 * included in this file may be subject to different terms.
 **********************************************************************************************************************/
/***********************************************************************************************************************
* File Name    : bsp_analog.h
* Description  : Analog pin connections available on this MCU.
***********************************************************************************************************************/

#ifndef BSP_ANALOG_H_
#define BSP_ANALOG_H_

/*******************************************************************************************************************//**
 * @ingroup BSP_MCU_S3A7
 * @defgroup BSP_MCU_ANALOG Analog Connections
 *
 * This group contains a list of enumerations that can be used with the @ref ANALOG_CONNECT_API.
 *
 * @{
 **********************************************************************************************************************/

/***********************************************************************************************************************
 * Includes   <System Includes> , "Project Includes"
 **********************************************************************************************************************/
#include "../all/bsp_common_analog.h"

/***********************************************************************************************************************
Macro definitions
***********************************************************************************************************************/

/***********************************************************************************************************************
Typedef definitions
***********************************************************************************************************************/

/***********************************************************************************************************************
Exported global variables
***********************************************************************************************************************/

/***********************************************************************************************************************
Exported global functions (to be accessed by other files)
***********************************************************************************************************************/
/** List of analog connections that can be made on S3A7
 * @note This list may change based on device. This list is for S3A7.
 * */
typedef enum e_analog_connect
{
    /* Connections for ACMPHS channel 0 VCMP input. */
    /* AN004 = P004 */
    /** Connect ACMPHS0 IVCMP to PORT0 P004. */
    ANALOG_CONNECT_ACMPHS0_IVCMP_TO_PORT0_P004      = ANALOG_CONNECT_DEFINE(ACMPHS, 0, CMPSEL0, IVCMP0, FLAG_CLEAR),
    /* AN007 = P007 */
    /** Connect ACMPHS0 IVCMP to PORT0 P007. */
    ANALOG_CONNECT_ACMPHS0_IVCMP_TO_PORT0_P007      = ANALOG_CONNECT_DEFINE(ACMPHS, 0, CMPSEL0, IVCMP1, FLAG_CLEAR),
    /* AN015 = P015 */
    /** Connect ACMPHS0 IVCMP to PORT0 P015. */
    ANALOG_CONNECT_ACMPHS0_IVCMP_TO_PORT0_P015      = ANALOG_CONNECT_DEFINE(ACMPHS, 0, CMPSEL0, IVCMP2, FLAG_CLEAR),
    /* ANALOG0_VREF is the internal reference voltage. */
    /** Connect ACMPHS0 IVCMP to ANALOG0 VREF. */
    ANALOG_CONNECT_ACMPHS0_IVCMP_TO_ANALOG0_VREF    = ANALOG_CONNECT_DEFINE(ACMPHS, 0, CMPSEL0, IVCMP3, FLAG_SET),

    /* Connections for ACMPHS channel 0 VREF input. */
    /* AN005 = P005 */
    /** Connect ACMPHS0 IVREF to PORT0 P005. */
    ANALOG_CONNECT_ACMPHS0_IVREF_TO_PORT0_P005      = ANALOG_CONNECT_DEFINE(ACMPHS, 0, CMPSEL1, IVREF0, FLAG_CLEAR),
    /* AN006 = P006 */
    /** Connect ACMPHS0 IVREF to PORT0 P006. */
    ANALOG_CONNECT_ACMPHS0_IVREF_TO_PORT0_P006      = ANALOG_CONNECT_DEFINE(ACMPHS, 0, CMPSEL1, IVREF1, FLAG_CLEAR),
    /* AN014 = P014 */
    /** Connect ACMPHS0 IVREF to PORT0 P014. */
    ANALOG_CONNECT_ACMPHS0_IVREF_TO_PORT0_P014      = ANALOG_CONNECT_DEFINE(ACMPHS, 0, CMPSEL1, IVREF2, FLAG_CLEAR),
    /* ANALOG0_VREF is the internal reference voltage. */
    /** Connect ACMPHS0 IVREF to ANALOG0 VREF. */
    ANALOG_CONNECT_ACMPHS0_IVREF_TO_ANALOG0_VREF    = ANALOG_CONNECT_DEFINE(ACMPHS, 0, CMPSEL1, IVREF3, FLAG_SET),

    /* Connections for ACMPHS channel 1 VCMP input. */
    /* AN000 = P000 */
    /** Connect ACMPHS1 IVCMP to PORT0 P000. */
    ANALOG_CONNECT_ACMPHS1_IVCMP_TO_PORT0_P000      = ANALOG_CONNECT_DEFINE(ACMPHS, 1, CMPSEL0, IVCMP0, FLAG_CLEAR),
    /* AN001 = P001 */
    /** Connect ACMPHS1 IVCMP to PORT0 P001. */
    ANALOG_CONNECT_ACMPHS1_IVCMP_TO_PORT0_P001      = ANALOG_CONNECT_DEFINE(ACMPHS, 1, CMPSEL0, IVCMP1, FLAG_CLEAR),
    /* AN002 = P002 */
    /** Connect ACMPHS1 IVCMP to PORT0 P002. */
    ANALOG_CONNECT_ACMPHS1_IVCMP_TO_PORT0_P002      = ANALOG_CONNECT_DEFINE(ACMPHS, 1, CMPSEL0, IVCMP2, FLAG_CLEAR),
    /* AN003 = P003 */
    /** Connect ACMPHS1 IVCMP to PORT0 P003. */
    ANALOG_CONNECT_ACMPHS1_IVCMP_TO_PORT0_P003      = ANALOG_CONNECT_DEFINE(ACMPHS, 1, CMPSEL0, IVCMP3, FLAG_CLEAR),
    /* AN007 = P007 */
    /** Connect ACMPHS1 IVCMP to PORT0 P007. */
    ANALOG_CONNECT_ACMPHS1_IVCMP_TO_PORT0_P007      = ANALOG_CONNECT_DEFINE(ACMPHS, 1, CMPSEL0, IVCMP4, FLAG_CLEAR),
    /* AN015 = P015 */
    /** Connect ACMPHS1 IVCMP to PORT0 P015. */
    ANALOG_CONNECT_ACMPHS1_IVCMP_TO_PORT0_P015      = ANALOG_CONNECT_DEFINE(ACMPHS, 1, CMPSEL0, IVCMP5, FLAG_CLEAR),

    /* Connections for ACMPHS channel 1 VREF input. */
    /* AN000 = P000 */
    /** Connect ACMPHS1 IVREF to PORT0 P000. */
    ANALOG_CONNECT_ACMPHS1_IVREF_TO_PORT0_P000      = ANALOG_CONNECT_DEFINE(ACMPHS, 1, CMPSEL1, IVREF0, FLAG_CLEAR),
    /* AN001 = P001 */
    /** Connect ACMPHS1 IVREF to PORT0 P001. */
    ANALOG_CONNECT_ACMPHS1_IVREF_TO_PORT0_P001      = ANALOG_CONNECT_DEFINE(ACMPHS, 1, CMPSEL1, IVREF1, FLAG_CLEAR),
    /* AN002 = P002 */
    /** Connect ACMPHS1 IVREF to PORT0 P002. */
    ANALOG_CONNECT_ACMPHS1_IVREF_TO_PORT0_P002      = ANALOG_CONNECT_DEFINE(ACMPHS, 1, CMPSEL1, IVREF2, FLAG_CLEAR),
    /* AN003 = P003 */
    /** Connect ACMPHS1 IVREF to PORT0 P003. */
    ANALOG_CONNECT_ACMPHS1_IVREF_TO_PORT0_P003      = ANALOG_CONNECT_DEFINE(ACMPHS, 1, CMPSEL1, IVREF3, FLAG_CLEAR),
    /* AN006 = P006 */
    /** Connect ACMPHS1 IVREF to PORT0 P006. */
    ANALOG_CONNECT_ACMPHS1_IVREF_TO_PORT0_P006      = ANALOG_CONNECT_DEFINE(ACMPHS, 1, CMPSEL1, IVREF4, FLAG_CLEAR),
    /* AN014 = P014 */
    /** Connect ACMPHS1 IVREF to PORT0 P014. */
    ANALOG_CONNECT_ACMPHS1_IVREF_TO_PORT0_P014      = ANALOG_CONNECT_DEFINE(ACMPHS, 1, CMPSEL1, IVREF5, FLAG_CLEAR),

    /* Connections for ACMPLP channel 0 VREF input. */
    /* ANALOG0_VREF is the internal reference voltage. */
    /** Connect ACMPLP0 IVREF to ANALOG0 VREF. */
    ANALOG_CONNECT_ACMPLP0_IVREF_TO_ANALOG0_VREF    = ANALOG_CONNECT_DEFINE(ACMPLP, 0, COMPMDR, C0VRF, FLAG_CLEAR),
    /* CMPREF0 = P101 */
    /** Connect ACMPLP0 IVREF to PORT1 P101. */
    ANALOG_CONNECT_ACMPLP0_IVREF_TO_PORT1_P101      = ANALOG_CONNECT_DEFINE(ACMPLP, 0, COMPMDR, CLEAR_C0VRF, FLAG_CLEAR),

    /* Connections for ACMPLP channel 1 VREF input. */
    /* ANALOG0_VREF is the internal reference voltage. */
    /** Connect ACMPLP1 IVREF to ANALOG0 VREF. */
    ANALOG_CONNECT_ACMPLP1_IVREF_TO_ANALOG0_VREF    = ANALOG_CONNECT_DEFINE(ACMPLP, 0, COMPMDR, C1VRF, FLAG_CLEAR),
    /* CMPREF1 = P103 */
    /** Connect ACMPLP1 IVREF to PORT1 P103. */
    ANALOG_CONNECT_ACMPLP1_IVREF_TO_PORT1_P103      = ANALOG_CONNECT_DEFINE(ACMPLP, 0, COMPMDR, CLEAR_C1VRF, FLAG_CLEAR),

} analog_connect_t;

/** @} (end defgroup BSP_MCU_ANALOG) */

#endif /* BSP_ANALOG_H_ */
