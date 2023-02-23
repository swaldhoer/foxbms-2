/**
 *
 * @copyright &copy; 2010 - 2023, Fraunhofer-Gesellschaft zur Foerderung der angewandten Forschung e.V.
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice, this
 *    list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 *
 * 3. Neither the name of the copyright holder nor the names of its
 *    contributors may be used to endorse or promote products derived from
 *    this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * We kindly request you to use one or more of the following phrases to refer to
 * foxBMS in your hardware, software, documentation or advertising materials:
 *
 * - &Prime;This product uses parts of foxBMS&reg;&Prime;
 * - &Prime;This product includes parts of foxBMS&reg;&Prime;
 * - &Prime;This product is derived from foxBMS&reg;&Prime;
 *
 */

/**
 * @file    fstartup.c
 * @author  foxBMS Team
 * @date    2020-07-09 (date of creation)
 * @updated 2023-02-23 (date of last update)
 * @version v1.5.1
 * @ingroup GENERAL
 * @prefix  STU
 *
 * @brief   Startup code
 *
 * @details This file contains startup code mostly identical to TI code.
 *          Function "_c_int00" is extracted from file "HL_sys_startup.c" and
 *          function "STU_GetResetSourceWithoutFlagReset" is taken from file
 *          "HL_system.c". Both files are generated by TI HALCoGen under the
 *          following license:
 *
 *          Copyright (C) 2009-2018 Texas Instruments Incorporated - www.ti.com
 *
 *          Redistribution and use in source and binary forms, with or without
 *          modification, are permitted provided that the following conditions
 *          are met:
 *
 *          Redistributions of source code must retain the above copyright
 *          notice, this list of conditions and the following disclaimer.
 *
 *          Redistributions in binary form must reproduce the above copyright
 *          notice, this list of conditions and the following disclaimer in the
 *          documentation and/or other materials provided with the
 *          distribution.
 *
 *          Neither the name of Texas Instruments Incorporated nor the names of
 *          its contributors may be used to endorse or promote products derived
 *          from this software without specific prior written permission.
 *
 *          THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 *          "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 *          LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 *          A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 *          OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 *          SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 *          LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 *          DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 *          THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 *          (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 *          OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 */

/*========== Includes =======================================================*/
/* clang-format off */
/* keep include order as suggested by TI HALCoGen */
#include "HL_sys_common.h"
#include "HL_system.h"
#include "HL_sys_vim.h"
#include "HL_sys_core.h"
#include "HL_esm.h"
#include "HL_sys_mpu.h"
#include "HL_errata_SSWF021_45.h"

#include "fassert.h"
#include "fstartup.h"
#include "main.h"

#include <stdlib.h>
/* clang-format on */

#include <stdint.h>

/*========== Macros and Definitions =========================================*/
/** number of retries for the PLL to come up */
#define STU_PLL_RETRIES (5u)

/*========== Static Constant and Variable Definitions =======================*/

/*========== Extern Constant and Variable Definitions =======================*/

/*========== Static Function Prototypes =====================================*/
/**
 * @brief   Handler for a failed PLL lock
 * @details If the PLL can not be locked the, this function shall be called to
 *          ensure that the application no further starts.
 * @return  This function never returns */
static void STU_HandlePllLockFail(void);

/**
 * @brief   Get reset flag
 * @details Get reset source without reseting respective the flag in SYSESR
 *          register
 * @return  returns reset reason
 */
static resetSource_t STU_GetResetSourceWithoutFlagReset(void);

/*========== Static Function Implementations ================================*/

void STU_HandlePllLockFail(void) {
    FAS_ASSERT(FAS_TRAP);
}
resetSource_t STU_GetResetSourceWithoutFlagReset(void) {
    register resetSource_t rst_source;

    if ((SYS_EXCEPTION & (uint32)POWERON_RESET) != 0U) {
        /* power-on reset condition */
        rst_source = POWERON_RESET;
    } else if ((SYS_EXCEPTION & (uint32)EXT_RESET) != 0U) {
        /*** Check for other causes of EXT_RESET that would take precedence **/
        if ((SYS_EXCEPTION & (uint32)OSC_FAILURE_RESET) != 0U) {
            /* Reset caused due to oscillator failure. Add user code here to handle oscillator failure */
            rst_source = OSC_FAILURE_RESET;
        } else if ((SYS_EXCEPTION & (uint32)WATCHDOG_RESET) != 0U) {
            /* Reset caused due watchdog violation */
            rst_source = WATCHDOG_RESET;
        } else if ((SYS_EXCEPTION & (uint32)WATCHDOG2_RESET) != 0U) {
            /* Reset caused due watchdog violation */
            rst_source = WATCHDOG2_RESET;
        } else if ((SYS_EXCEPTION & (uint32)SW_RESET) != 0U) {
            /* Reset caused due to software reset. */
            rst_source = SW_RESET;
        } else {
            /* Reset caused due to External reset. */
            rst_source = EXT_RESET;
        }
    } else if ((SYS_EXCEPTION & (uint32)DEBUG_RESET) != 0U) {
        /* Reset caused due Debug reset request */
        rst_source = DEBUG_RESET;
    } else if ((SYS_EXCEPTION & (uint32)CPU0_RESET) != 0U) {
        /* Reset caused due to CPU0 reset. CPU reset can be caused by CPU self-test completion, or by toggling the "CPU RESET" bit of the CPU Reset Control Register. */
        rst_source = CPU0_RESET;
    } else {
        /* No_reset occurred. */
        rst_source = NO_RESET;
    }
    return rst_source;
}

/*========== Extern Function Implementations ================================*/
/** system entry point */
#pragma CODE_STATE(_c_int00, 32)
#pragma INTERRUPT(_c_int00, RESET)
/* SourceId : STARTUP_SourceId_001 */
/* DesignId : STARTUP_DesignId_001 */
/* Requirements : HL_CONQ_STARTUP_SR1 */
void _c_int00(void) {
    register resetSource_t rstSrc;

    /* Initialize Core Registers to avoid CCM Error */
    _coreInitRegisters_();

    /* Initialize Stack Pointers */
    _coreInitStackPointer_();

    /* Reset handler: the following instructions read from the system exception status register
     * to identify the cause of the CPU reset.
     */
    /* Changed in comparison to TI _c_int00 implementation. Readout flags but do NOT clear them! */
    rstSrc = STU_GetResetSourceWithoutFlagReset();
    switch (rstSrc) {
        case POWERON_RESET:
            /* Initialize L2RAM to avoid ECC errors right after power on */
            _memInit_();

            /* Add condition to check whether PLL can be started successfully */
            if (_errata_SSWF021_45_both_plls(STU_PLL_RETRIES) != 0U) {
                /* Put system in a safe state */
                STU_HandlePllLockFail();
            }

        case DEBUG_RESET:
        case EXT_RESET:

            /* Initialize L2RAM to avoid ECC errors right after power on */
            if (rstSrc != POWERON_RESET) {
                _memInit_();
            }

            /* Enable CPU Event Export */
            /* This allows the CPU to signal any single-bit or double-bit errors detected
         * by its ECC logic for accesses to program flash or data RAM.
         */
            _coreEnableEventBusExport_();

            /* Check if there were ESM group3 errors during power-up.
         * These could occur during eFuse auto-load or during reads from flash OTP
         * during power-up. Device operation is not reliable and not recommended
         * in this case. */
            if ((esmREG->SR1[2]) != 0U) {
                esmGroup3Notification(esmREG, esmREG->SR1[2]);
            }

            /* Initialize System - Clock, Flash settings with Efuse self check */
            systemInit();

            /* Enable IRQ offset via Vic controller */
            _coreEnableIrqVicOffset_();

            /* Initialize VIM table */
            vimInit();

            /* Configure system response to error conditions signaled to the ESM group1 */
            /* This function can be configured from the ESM tab of HALCoGen */
            esmInit();
            break;

        case OSC_FAILURE_RESET:
            break;

        case WATCHDOG_RESET:
        case WATCHDOG2_RESET:

            break;

        case CPU0_RESET:
            /* Enable CPU Event Export */
            /* This allows the CPU to signal any single-bit or double-bit errors detected
         * by its ECC logic for accesses to program flash or data RAM.
         */
            _coreEnableEventBusExport_();
            break;

        case SW_RESET:
            break;

        default:
            break;
    }

    _mpuInit_();

    /* initialize global variable and constructors */
    __TI_auto_init();

    /* call the application */
    main();

    /* AXIVION Next Codeline Style MisraC2012-21.8: exit is called as in generated code by TI */
    exit(0);
}

/*========== Externalized Static Function Implementations (Unit Test) =======*/
#ifdef UNITY_UNIT_TEST
#endif
