/*
 * LED.h
 *
 *  Created on: 2018年1月30日
 *      Author: zhao
 */

#ifndef LED_H_
#define LED_H_
#include "Config.h"
#include "timer.h"

typedef enum
{
    led_comm_normol = 0,
    led_comm_off    = 1,
    led_comm_alarm1 = 2,
    led_comm_alarm2 = 3,
}__comm_led_state;

typedef enum
{
    led_off           = 0,
    //充电模式
    charge_vbat_going = 1,
    charge_vbat_full = 2,
    //放电模式
    discharge_high_soc,
    discharge_low_soc,
    //故障
    error_charge_OV_CV,
    error_discharge_TV_CV,
    error_discharge_UV,
    // 通讯控制
    comm_led_alarm1,
    comm_led_alarm2,
}_led_task_;

typedef enum
{
    led_state_up = 0,   // 向上更新
    led_state_high = 1, // 保持最亮
    led_state_down ,    // 向下更新
    led_state_low ,     // 保持熄灭
}_led_breath_state_enum;

extern _led_task_   led_task_state;
extern __comm_led_state comm_led_state;
extern _led_breath_state_enum led_breath_state;

void LED_state0(void);
void LED_state1(void);
void LED_state2(void);
void LED_state3(void);
void LED_state4(void);
void LED_state5(void);
void LED_state6(void);
void LED_task(void);

#endif /* LED_H_ */
