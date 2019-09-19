/*
 * CapProc.h
 *
 *  Created on: 2015-5-28
 *      Author: Administrator
 */

#ifndef CAPPROC_H_
#define CAPPROC_H_
#include "Config.H"
#include "Global.h"
#define CAP_MS 250  	//����ɨ��ʱ��
#define CAP_CNT_VAL 14400   //1mah  20S

#define BAT_NORMAL_CAPACITY_MIN_AD 1800
#define BAT_NORMAL_CAP 2257     // �����˱���ֵ 143mAh
#define BAT_LEARNNING_CAPACITY_MAX_AD 2500   //10%

#define VBAT_FULL_VOLT 41000LL//4100*CELLS_NUM


typedef union
{
    struct{
        uint16_t  	full_cap;   // ������    // ��ʱΪ�̶�ֵ
        uint16_t   	bat_cap;    // �������   // ��ʱδ��
        int32_t     cap_val;    // ������       // ��ʹ����������ֵ����洢��FLASH���洢ʱתΪ��ֵ�洢
        int32_t     cap_val2;   // ������ 2  ���γ��������ۼ�ֵ   // ��ʱδ��

        uint16_t    cycle_cap_val;	//ѭ��������������������ѭ����������洢��FLASH
        uint16_t    cycle_dch_cap_val;	//   // ��ʱδ��

        uint8_t     show_cap_rate;  // ��ʾ����ֵ �����������չʾ���û���SOCֵ����������ʣ�������ٷֱ���һ���������̣�������Ч�仯����洢��FLASH
        uint8_t     re_cap_rate;    // ʣ�������ٷֱ�   // SOCr ��(CAPa - CAPu - CAPf)/CAPa�������
        uint8_t     re_cap_rate2;   // ���ǰ�ľ���soc   // ��ʱδ��
        uint8_t     re_cap_rate3;   // ���ǰ�ľ���soc ���ҵ�ѹС��3500mv   // ��ʱδ��

        int32_t  	cap_cnt;
        uint16_t    bat_cycle_cnt;  // ѭ����������洢��FLASH
        uint16_t    dch_cycle_cnt;   // ��ʱδ��
    }val;
}__SYS_CAP;

extern volatile uint8_t old_show_cap_rate;
extern volatile uint8_t old_re_cap_rate;
extern uint8_t cap_write_cnt;
extern volatile __SYS_CAP    g_sys_cap;

extern void InitSoc(void);
extern void Cap_Proc(signed long cur);
extern void NormalCapacityProc(void);
extern void BatCycleProc(void);
extern void Cap_Revise(void);

uint16_t VbatToSoc(uint16_t vbat_val,const uint16_t *p);

#endif /* CAPPROC_H_ */
