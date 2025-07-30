#include "ti_msp_dl_config.h"

// SysTick延时函数部分，虽然在这个测试程序里用不到，但保留着无害
volatile uint32_t g_systick_delay_counter = 0;
void SysTick_Handler(void)
{
    if (g_systick_delay_counter > 0)
        g_systick_delay_counter--;
}
void delay_ms(uint32_t ms)
{
    g_systick_delay_counter = ms;
    while (g_systick_delay_counter > 0)
        ;
}
void systick_init(void)
{
#define SYSCLK_FREQUENCY 32000000
    uint32_t ticks = SYSCLK_FREQUENCY / 1000;
    SysTick->LOAD = ticks - 1;
    SysTick->VAL = 0;
    SysTick->CTRL = SysTick_CTRL_CLKSOURCE_Msk | SysTick_CTRL_TICKINT_Msk | SysTick_CTRL_ENABLE_Msk;
}

// =================================================================
// 舵机控制部分
// =================================================================

// --- 俯仰舵机 (Pitch, 180度, 连接到 PB12, 由 Channel 1 控制) ---
#define PITCH_PULSE_LEFT (600)
#define PITCH_PULSE_CENTER (1550)
#define PITCH_PULSE_RIGHT (2530)
#define PITCH_SERVO_CHANNEL (DL_TIMER_CC_1_INDEX) // 明确指定通道1

void set_pitch_angle(uint8_t angle) // 0-180
{
    uint16_t pulse;
    if (angle > 180)
        angle = 180;

    if (angle <= 90) {
        pulse = PITCH_PULSE_LEFT + (uint16_t)(((float)(PITCH_PULSE_CENTER - PITCH_PULSE_LEFT) * angle) / 90.0f);
    } else {
        pulse = PITCH_PULSE_CENTER + (uint16_t)(((float)(PITCH_PULSE_RIGHT - PITCH_PULSE_CENTER) * (angle - 90)) / 90.0f);
    }
    // 假设你的PWM实例名叫 PWM_TURRET (炮台)
    DL_Timer_setCaptureCompareValue(PWM_TURRET_INST, pulse, PITCH_SERVO_CHANNEL);
}

// --- 方位舵机 (Yaw, 270度, 连接到 PB8, 由 Channel 0 控制) ---
// ▼▼▼ 你需要通过实验找到并填入这三个值 ▼▼▼
#define YAW_PULSE_LEFT (500) // 假设值
#define YAW_PULSE_CENTER (1520) // 假设值
#define YAW_PULSE_RIGHT (2650) // 假设值
// ▲▲▲▲▲▲▲▲▲▲▲▲▲▲▲▲▲▲▲▲▲▲▲▲▲▲▲▲▲▲▲▲▲▲
#define YAW_SERVO_CHANNEL (DL_TIMER_CC_0_INDEX) // 明确指定通道0

void set_yaw_angle(uint16_t angle) // 0-270
{
    uint16_t pulse;
    if (angle > 270)
        angle = 270;

    // 使用简单的线性映射，或者也可以用更精确的三点校准
    pulse = YAW_PULSE_LEFT + (uint16_t)(((float)(YAW_PULSE_RIGHT - YAW_PULSE_LEFT) * angle) / 270.0f);

    // 同样，假设PWM实例名叫 PWM_TURRET
    DL_Timer_setCaptureCompareValue(PWM_TURRET_INST, pulse, YAW_SERVO_CHANNEL);
}

// =================================================================
// 主函数 main() - 演示组合运动
// =================================================================

int main(void)
{
    SYSCFG_DL_init();
    systick_init();

    // 只需要启动一次总的定时器
    // 假设你在SysConfig里给唯一的PWM实例起的名字是 PWM_TURRET
    DL_Timer_startCounter(PWM_TURRET_INST);

    // 启动后，让炮台归位到“正前方，水平”姿态
    set_yaw_angle(135); // 方位舵机的中心是135度
    set_pitch_angle(90); // 俯仰舵机的中心是90度
    delay_ms(1500);

    while (1) {
        // 让俯仰舵机上下摆动
        for (uint8_t pitch_angle = 45; pitch_angle <= 135; pitch_angle += 5) {
            set_pitch_angle(pitch_angle);
            delay_ms(50);
        }
        for (uint8_t pitch_angle = 135; pitch_angle > 45; pitch_angle -= 5) {
            set_pitch_angle(pitch_angle);
            delay_ms(50);
        }

        // 让方位舵机左右摆动
        for (uint16_t yaw_angle = 90; yaw_angle <= 180; yaw_angle += 5) {
            set_yaw_angle(yaw_angle);
            delay_ms(50);
        }
        for (uint16_t yaw_angle = 180; yaw_angle > 90; yaw_angle -= 5) {
            set_yaw_angle(yaw_angle);
            delay_ms(50);
        }
    }
}