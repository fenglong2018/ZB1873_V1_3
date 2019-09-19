/*
 * LED.c
 *
 *  Created on: 2018��1��30��
 *      Author: zhao
 */
#include "LED.h"


__comm_led_state comm_led_state = led_comm_normol;
_led_task_  led_task_state = led_off;
_led_breath_state_enum led_breath_state = led_state_down;
uint16_t led_blue_pwm = 0;

//���еƶ��ر�
void LED_state0(void)
{
    DIS_LED_RED();
    led_blue_pwm = 0;
    led_breath_state = led_state_up;
    pwm_4k_start(0);
}

// ���ƺ��� Ƶ��2.5S ���Ϩ��
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
            if((led_blue_pwm == 0)||(led_blue_pwm > 5000))  // ����5000��Ϊ�˷�ֹ���������
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

// ���Ƴ��� ���Ϩ��
void LED_state2(void)
{
    DIS_LED_RED();
    led_blue_pwm = 500;
    led_breath_state = led_state_down;
    pwm_4k_start(500);
}

// 2018-10-16 ���ͻ������Ϊ����Ϩ�� ��Ƴ���
void LED_state3(void)
{
    led_blue_pwm = 0;
    led_breath_state = led_state_up;
    pwm_4k_start(0);
    EN_LED_RED();
}

// ����Ϩ��  �����˸ Ƶ��0.25S��/0.25S��
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

// ����Ϩ��  �����˸ Ƶ��0.15S��/0.15S��
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

// ����Ϩ��  �����˸ Ƶ��1S��/1S��
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
    {   // ������������������ѹ����ƿ���6S�����
        led_task_state = error_charge_OV_CV;
        return ;
    }

    if(g_sys_state.val.charger_on_flag == TRUE)
    {   // �������λ�����ڳ��״̬
        if(g_sys_flags.val.vcell_diff_protect_flag == TRUE)     // ��оѹ����󣬺�ƿ���6S
        {
            led_task_state = error_charge_OV_CV;
        }
        else if(g_sys_state.val.chg_5h_err_flag == TRUE)    // ��糬��5H�����
        {
            led_task_state = led_off;
        }
        else if((g_sys_state.val.vbat_chg_full_flag == TRUE)&&(g_sys_cap.val.show_cap_rate == 100))    // ������   // ������䱣�����ó䱥��־���棬blust 20181128
        {   // �����磬���Ƴ���5min����ƣ����ǲ����粻����� blust 20181106
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
        {   // ����У����ƺ���
            led_task_state = charge_vbat_going;
        }
        return ;
    }

    if(
      (g_sys_flags.val.dch_oc_protect_flag == TRUE)     // һ���ŵ��������
    ||(g_sys_flags.val.dch_oc2_protect_flag == TRUE)    // �����ŵ��������
    ||(g_sys_flags.val.dch_sc_protect_flag == TRUE)     // �ŵ��·����
    ||(g_sys_flags.val.dch_temp_low_protect_flag == TRUE)   // �ŵ���±���
    ||(g_sys_flags.val.dch_temp_high_protect_flag == TRUE)  // �ŵ���±���
    ||(g_sys_flags.val.vcell_high_err_flag == TRUE)     // ��о����ѹ����
    ||(g_sys_flags.val.vcell_low_err_flag == TRUE)      // ��о����ѹ����
    ||(g_sys_state.val.NTC_open_close_flag == TRUE)     // NTC��·/��·����
    )
    {   // ��ƿ���6S�����
        led_task_state = error_discharge_TV_CV;
        return ;
    }

    if(
      (g_sys_state.val.start_vbat_uv_flag == TRUE)  // ��������Ƿѹ����
    ||(g_sys_flags.val.soft_vcell_low_protect_flag == TRUE) // ���Ƿѹ����
    ||(g_sys_flags.val.vcell_low_protect_flag == TRUE)      // Ӳ��Ƿѹ����
    )
    {   // LED�����˸��1S��1S�����Σ�����6S��
        led_task_state = error_discharge_UV;
        return ;
    }

    if(g_sys_state.val.dch_on_flag == TRUE)
    {   // ����ŵ�״̬
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
 * LED_state1 ���ƺ���  Ƶ��1S ���Ϩ��                          // ���ʱ����
 * LED_state2 ���Ƴ���  ���Ϩ��                                       // �������ߵ����ŵ�ʱ����
 * LED_state3 ����Ϩ��  ��Ƴ���                                       // �͵����ŵ�ʱ����
 * LED_state4 ����Ϩ��  �����˸ Ƶ��0.25S��/0.25S�� // ����ʱ����
 * LED_state5 ����Ϩ��  �����˸ Ƶ��0.15S��/0.15S�� // ͨѶ���Ƹ澯1ʱ����
 * LED_state6 ����Ϩ��  �����˸ Ƶ��1S��/1S��             // ͨѶ���Ƹ澯2ʱ����
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
    // ���ģʽ
    case charge_vbat_going:LED_state1();break;
    case charge_vbat_full :LED_state2() ;break;
    // �ŵ�ģʽ
    case discharge_high_soc:LED_state2();break;
    case discharge_low_soc:LED_state3();break;
    // ����
    case error_charge_OV_CV:LED_state4();break;
    case error_discharge_TV_CV:LED_state4();break;
    case error_discharge_UV:LED_state6();break;
    // ͨѶ����
    case comm_led_alarm1:LED_state5();break;
    case comm_led_alarm2:LED_state6();break;
    default: led_task_state = led_off;LED_state0();break;
    }
}

