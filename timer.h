/*
 * timer.h
 *
 *  Created on: Aug 25, 2015
 *      Author: a0220433
 */

#ifndef SRC_TIMER_H_
#define SRC_TIMER_H_

#include"Global.h"
extern volatile uint16_t sys_1s_cnt;
extern volatile uint16_t sys_250ms_cnt;
extern volatile uint16_t sys_1min_cnt;
extern volatile uint16_t sys_10ms_cnt;

extern volatile uint8_t timer_comm_stick;
extern volatile uint8_t timer_TX_stick;

void timer0_A0_init(void);
void timer1_A0_init(void);
void pwm_4k_start(uint16_t temp);

#endif /* SRC_TIMER_H_ */
