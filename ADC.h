/*
 * ADC.h
 *
 *  Created on: 2018��1��29��
 *      Author: zhao
 */

#ifndef ADC_H_
#define ADC_H_

#include "Config.H"
#include "until.h"
#define CONVERSION_NUM 10

#define MODE_2_5    0
#define MODE_3_3    1
//#define TEMP_DEGREE_RANGE   130     // �¶ȱ����ݸ���
//#define TEMP_OFFSET_VAL     30      // �¶ȱ�ƫ����
#define MCU_TEMP_DEGREE_RANGE 140LL
#define MCU_TEMP_OFFSET_VAL 30LL

#define VCHG_CP_LOW     507 //26V     // C+ P-������·ADֵ���ޣ����ڴ�ֵ��Ϊ�޳����
#define VCHG_CP_HIGH    636 //32.5V     // C+ P-������·ADֵ���ޣ����ڴ�ֵ��Ϊ�������ѹ
#define VCHG_CB_LOW     624 //26V     // C+ B-��ѹ��·ADֵ���ޣ����ڴ�ֵ��Ϊ�޳����
#define VCHG_CB_HIGH    782 //32.5V     // C+ B-��ѹ��·ADֵ���ޣ����ڴ�ֵ��Ϊ�������ѹ

extern void Vbat_volt(void);
extern void Vchg_volt(void);
extern void Temp_Get(void);


#endif /* ADC_H_ */
