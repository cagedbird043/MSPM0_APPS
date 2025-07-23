/*
 * Copyright (c) 2023, Texas Instruments Incorporated
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

#include "ti_msp_dl_config.h"

// 中断服务程序 (ISR) 和主循环 (main loop) 之间的通信标志
volatile bool gTimerUpdate = false;

int main(void)
{
    // SYSCFG_DL_init() 会初始化所有在 SysConfig 中配置的外设
    SYSCFG_DL_init();

    // 手动启动定时器
    DL_TimerG_startCounter(TIMER_0_INST);

    // 启用 TIMG0 的中断
    NVIC_EnableIRQ(TIMER_0_INST_INT_IRQN);

    // 初始化标志位
    gTimerUpdate = false;

    while (1) {
        // 高效地等待中断的发生
        while (gTimerUpdate == false) {
            // 在此期间 CPU 是空闲的
        }

        // 定时器中断已发生，清除标志位
        gTimerUpdate = false;

        // 翻转 USER_LED_1 的电平
        DL_GPIO_togglePins(GPIO_GRP_0_PORT, GPIO_GRP_0_USER_LED_1_PIN);
    }
}

// =============================================================================
// 定时器中断服务程序 (ISR)
// **重要：函数名必须与 ti_msp_dl_config.h 中的定义完全一致！**
// =============================================================================
void TIMER_0_INST_IRQHandler(void)
{
    switch (DL_TimerG_getPendingInterrupt(TIMER_0_INST)) {
    case DL_TIMER_IIDX_ZERO:
        // 500ms 时间到，设置标志位，通知主循环
        gTimerUpdate = true;
        break;
    default:
        break;
    }
}