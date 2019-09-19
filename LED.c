/*
 * LED.c
 *
 *  Created on: 2018年1月30日
 *      Author: zhao
 */
#include "LED.h"


__comm_led_state comm_led_state = led_comm_normol;
_led_task_  led_task_state = led_off;
_led_breath_state_enum led_breath_state = led_state_down;
uint16_t led_blue_pwm = 0;

//所有灯都关闭
void LED_state0(void)
{
    DIS_LED_RED();
    led_blue_pwm = 0;
    led_breath_state = led_state_up;
    pwm_4k_start(0);
}

// 蓝灯呼吸 频率2.5S 红灯熄灭
void LED_state1(void)
{
    static uint8_t state1_tick = 0;
    static uint8_t led_state_cnt = 0;
    DIS_LED_RED();
    if ((uint8_t)(sys_10ms_cnt - state1_tick) >= 1)
    {
        state1_tick = sys_10ms_cnt;

        if(led_breath_state == led_state_up)
        {
            if(led_blue_pwm >= 500)
            {
                led_breath_state = led_state_high;
                led_state_cnt = 0;
            }
            else
            {
                led_blue_pwm += 5;
            }
        }
        else if(led_breath_state == led_state_high)
        {
            led_blue_pwm = 500;
            led_state_cnt ++;
            if(led_state_cnt >= 25)
            {
                led_breath_state = led_state_down;
                led_state_cnt = 0;
            }
        }
        else if(led_breath_state == led_state_down)
        {
            if((led_blue_pwm == 0)||(led_blue_pwm > 5000))  // 大于5000是为了防止多减造成溢出
            {
                led_breath_state = led_state_low;
                led_state_cnt = 0;
            }
            else
            {
                led_blue_pwm -= 5;
            }
        }
        else if(led_breath_state == led_state_low)
        {
            led_blue_pwm = 0;
            led_state_cnt ++;
            if(led_state_cnt >= 25)
            {
                led_breath_state = led_state_up;
                led_state_cnt = 0;
            }
        }
        pwm_4k_start(led_blue_pwm);
    }
}

// 蓝灯常亮 红灯熄灭
void LED_state2(void)
{
    DIS_LED_RED();
    led_blue_pwm = 500;
    led_breath_state = led_state_down;
    pwm_4k_start(500);
}

// 2018-10-16 按客户需求改为蓝灯熄灭 红灯常亮
void LED_state3(void)
{
    led_blue_pwm = 0;
    led_breath_state = led_state_up;
    pwm_4k_start(0);
    EN_LED_RED();
}

// 蓝灯熄灭  红灯闪烁 频率0.25S亮/0.25S灭
void LED_state4(void)
{
    static uint8_t state4_tick = 0;

    led_blue_pwm = 0;
    led_breath_state = led_state_up;
    pwm_4k_start(0);
    if ((uint8_t)(sys_250ms_cnt - state4_tick) >= 1)
    {
        state4_tick = sys_250ms_cnt;
        DEF_LED_RED();
    }
}

// 蓝灯熄灭  红灯闪烁 频率0.15S亮/0.15S灭
void LED_state5(void)
{
    static uint8_t state5_tick = 0;

    led_blue_pwm = 0;
    led_breath_state = led_state_up;
    pwm_4k_start(0);
    if ((uint8_t)(sys_10ms_cnt - state5_tick) >= 15)
    {
        state5_tick = sys_10ms_cnt;
        DEF_LED_RED();
    }
}

// 蓝灯熄灭  红灯闪烁 频率1S亮/1S灭
void LED_state6(void)
{
    static uint8_t state6_tick = 0;

    led_blue_pwm = 0;
    led_breath_state = led_state_up;
    pwm_4k_start(0);
    if ((uint8_t)(sys_250ms_cnt - state6_tick) >= 4)
    {
        state6_tick = sys_250ms_cnt;
        DEF_LED_RED();
    }
}

void LED_ctrl(void)
{
    if((g_sys_flags.val.chg_oc_protect_flag == TRUE)||(g_sys_flags.val.chg_val_high_err_flag == TRUE))
    {   // 充电过流保护或充电器过压，红灯快闪6S后掉电
        led_task_state = error_charge_OV_CV;
        return ;
    }

    if(g_sys_state.val.charger_on_flag == TRUE)
    {   // 充电器在位，处于充电状态
        if(g_sys_flags.val.vcell_diff_protect_flag == TRUE)     // 电芯压差过大，红灯快闪6S
        {
            led_task_state = error_charge_OV_CV;
        }
        else if(g_sys_state.val.chg_5h_err_flag == TRUE)    // 充电超过5H，灭灯
        {
            led_task_state = led_off;
        }
        else if((g_sys_state.val.vbat_chg_full_flag == TRUE)&&(g_sys_cap.val.show_cap_rate == 100))    // 充满电   // 开机起充保护采用充饱标志代替，blust 20181128
        {   // 充满电，蓝灯常亮5min后灭灯，但是不掉电不关输出 blust 20181106
            if(g_sys_state.val.chg_full_ledoff_flag == TRUE)
            {
                led_task_state = led_off;
            }
            else
            {
                led_task_state = charge_vbat_full;
            }
        }
        else
        {   // 充电中，蓝灯呼吸
            led_task_state = charge_vbat_going;
        }
        return ;
    }

    if(
      (g_sys_flags.val.dch_oc_protect_flag == TRUE)     // 一级放电过流保护
    ||(g_sys_flags.val.dch_oc2_protect_flag == TRUE)    // 二级放电过流保护
    ||(g_sys_flags.val.dch_sc_protect_flag == TRUE)     // 放电短路保护
    ||(g_sys_flags.val.dch_temp_low_protect_flag == TRUE)   // 放电低温保护
    ||(g_sys_flags.val.dch_temp_high_protect_flag == TRUE)  // 放电高温保护
    ||(g_sys_flags.val.vcell_high_err_flag == TRUE)     // 电芯超高压错误
    ||(g_sys_flags.val.vcell_low_err_flag == TRUE)      // 电芯超低压错误
    ||(g_sys_state.val.NTC_open_close_flag == TRUE)     // NTC短路/断路故障
    )
    {   // 红灯快闪6S后掉电
        led_task_state = error_discharge_TV_CV;
        return ;
    }

    if(
      (g_sys_state.val.start_vbat_uv_flag == TRUE)  // 开机整机欠压保护
    ||(g_sys_flags.val.soft_vcell_low_protect_flag == TRUE) // 软件欠压保护
    ||(g_sys_flags.val.vcell_low_protect_flag == TRUE)      // 硬件欠压保护
    )
    {   // LED红灯闪烁（1S亮1S灭）三次（共计6S）
        led_task_state = error_discharge_UV;
        return ;
    }

    if(g_sys_state.val.dch_on_flag == TRUE)
    {   // 允许放电状态
        if(g_sys_cap.val.show_cap_rate > 20)
        {
            led_task_state = discharge_high_soc;
        }
        else
        {
            led_task_state = discharge_low_soc;
        }

        switch(comm_led_state)
        {
        case led_comm_normol: break;
        case led_comm_off: led_task_state = led_off; break;
        case led_comm_alarm1: led_task_state = comm_led_alarm1; break;
        case led_comm_alarm2: led_task_state = comm_led_alarm2; break;
        default: comm_led_state = led_comm_normol; break;
        }
    }
    else
    {
        led_task_state = led_off;
    }
}

/*
 * LED_state1 蓝灯呼吸  频率1S 红灯熄灭                          // 充电时运行
 * LED_state2 蓝灯常亮  红灯熄灭                                       // 充满电或高电量放电时运行
 * LED_state3 蓝灯熄灭  红灯常亮                                       // 低电量放电时运行
 * LED_state4 蓝灯熄灭  红灯闪烁 频率0.25S亮/0.25S灭 // 故障时运行
 * LED_state5 蓝灯熄灭  红灯闪烁 频率0.15S亮/0.15S灭 // 通讯控制告警1时运行
 * LED_state6 蓝灯熄灭  红灯闪烁 频率1S亮/1S灭             // 通讯控制告警2时运行
 */
void LED_task(void)
{
    if(g_sys_state.val.chg_on_checking_flag == FALSE)
    {
        LED_ctrl();
    }

    switch(led_task_state)
    {
    case led_off:LED_state0();break;
    // 充电模式
    case charge_vbat_going:LED_state1();break;
    case charge_vbat_full :LED_state2() ;break;
    // 放电模式
    case discharge_high_soc:LED_state2();break;
    case discharge_low_soc:LED_state3();break;
    // 故障
    case error_charge_OV_CV:LED_state4();break;
    case error_discharge_TV_CV:LED_state4();break;
    case error_discharge_UV:LED_state6();break;
    // 通讯控制
    case comm_led_alarm1:LED_state5();break;
    case comm_led_alarm2:LED_state6();break;
    default: led_task_state = led_off;LED_state0();break;
    }
}

