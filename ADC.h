/*
 * ADC.h
 *
 *  Created on: 2018年1月29日
 *      Author: zhao
 */

#ifndef ADC_H_
#define ADC_H_

#include "Config.H"
#include "until.h"
#define CONVERSION_NUM 10

#define MODE_2_5    0
#define MODE_3_3    1
//#define TEMP_DEGREE_RANGE   130     // 温度表数据个数
//#define TEMP_OFFSET_VAL     30      // 温度表偏移量
#define MCU_TEMP_DEGREE_RANGE 140LL
#define MCU_TEMP_OFFSET_VAL 30LL

#define VCHG_CP_LOW     507 //26V     // C+ P-减法电路AD值下限，低于此值认为无充电器
#define VCHG_CP_HIGH    636 //32.5V     // C+ P-减法电路AD值上限，高于此值认为充电器过压
#define VCHG_CB_LOW     624 //26V     // C+ B-分压电路AD值下限，低于此值认为无充电器
#define VCHG_CB_HIGH    782 //32.5V     // C+ B-分压电路AD值上限，高于此值认为充电器过压

extern void Vbat_volt(void);
extern void Vchg_volt(void);
extern void Temp_Get(void);


#endif /* ADC_H_ */
