/*
 * Copyright (c) 2023, Texas Instruments Incorporated - http://www.ti.com
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * *  Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *
 * *  Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * *  Neither the name of Texas Instruments Incorporated nor the names of
 *    its contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

/*
 *  ============ ti_msp_dl_config.h =============
 *  Configured MSPM0 DriverLib module declarations
 *
 *  DO NOT EDIT - This file is generated for the MSPM0G350X
 *  by the SysConfig tool.
 */
#ifndef ti_msp_dl_config_h
#define ti_msp_dl_config_h

#define CONFIG_MSPM0G350X
#define CONFIG_MSPM0G3507

#if defined(__ti_version__) || defined(__TI_COMPILER_VERSION__)
#define SYSCONFIG_WEAK __attribute__((weak))
#elif defined(__IAR_SYSTEMS_ICC__)
#define SYSCONFIG_WEAK __weak
#elif defined(__GNUC__)
#define SYSCONFIG_WEAK __attribute__((weak))
#endif

#include <ti/devices/msp/msp.h>
#include <ti/driverlib/driverlib.h>
#include <ti/driverlib/m0p/dl_core.h>

#ifdef __cplusplus
extern "C" {
#endif

/*
 *  ======== SYSCFG_DL_init ========
 *  Perform all required MSP DL initialization
 *
 *  This function should be called once at a point before any use of
 *  MSP DL.
 */


/* clang-format off */

#define POWER_STARTUP_DELAY                                                (16)


#define CPUCLK_FREQ                                                     32000000



/* Defines for PWM_TURRET */
#define PWM_TURRET_INST                                                    TIMA0
#define PWM_TURRET_INST_IRQHandler                              TIMA0_IRQHandler
#define PWM_TURRET_INST_INT_IRQN                                (TIMA0_INT_IRQn)
#define PWM_TURRET_INST_CLK_FREQ                                          500000
/* GPIO defines for channel 0 */
#define GPIO_PWM_TURRET_C0_PORT                                            GPIOB
#define GPIO_PWM_TURRET_C0_PIN                                     DL_GPIO_PIN_8
#define GPIO_PWM_TURRET_C0_IOMUX                                 (IOMUX_PINCM25)
#define GPIO_PWM_TURRET_C0_IOMUX_FUNC                IOMUX_PINCM25_PF_TIMA0_CCP0
#define GPIO_PWM_TURRET_C0_IDX                               DL_TIMER_CC_0_INDEX
/* GPIO defines for channel 1 */
#define GPIO_PWM_TURRET_C1_PORT                                            GPIOB
#define GPIO_PWM_TURRET_C1_PIN                                    DL_GPIO_PIN_12
#define GPIO_PWM_TURRET_C1_IOMUX                                 (IOMUX_PINCM29)
#define GPIO_PWM_TURRET_C1_IOMUX_FUNC                IOMUX_PINCM29_PF_TIMA0_CCP1
#define GPIO_PWM_TURRET_C1_IDX                               DL_TIMER_CC_1_INDEX



/* Defines for TIMER_MOTION */
#define TIMER_MOTION_INST                                                (TIMA1)
#define TIMER_MOTION_INST_IRQHandler                            TIMA1_IRQHandler
#define TIMER_MOTION_INST_INT_IRQN                              (TIMA1_INT_IRQn)
#define TIMER_MOTION_INST_LOAD_VALUE                                     (9999U)



/* clang-format on */

void SYSCFG_DL_init(void);
void SYSCFG_DL_initPower(void);
void SYSCFG_DL_GPIO_init(void);
void SYSCFG_DL_SYSCTL_init(void);
void SYSCFG_DL_PWM_TURRET_init(void);
void SYSCFG_DL_TIMER_MOTION_init(void);

bool SYSCFG_DL_saveConfiguration(void);
bool SYSCFG_DL_restoreConfiguration(void);

#ifdef __cplusplus
}
#endif

#endif /* ti_msp_dl_config_h */
