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
#define CAP_MS 250  	//容量扫描时间
#define CAP_CNT_VAL 14400   //1mah  20S

#define BAT_NORMAL_CAPACITY_MIN_AD 1800
#define BAT_NORMAL_CAP 2257     // 修正了保留值 143mAh
#define BAT_LEARNNING_CAPACITY_MAX_AD 2500   //10%

#define VBAT_FULL_VOLT 41000LL//4100*CELLS_NUM


typedef union
{
    struct{
        uint16_t  	full_cap;   // 满容量    // 暂时为固定值
        uint16_t   	bat_cap;    // 电池容量   // 暂时未用
        int32_t     cap_val;    // 容量池       // 已使用容量，负值，需存储到FLASH，存储时转为正值存储
        int32_t     cap_val2;   // 容量池 2  单次充电的容量累计值   // 暂时未用

        uint16_t    cycle_cap_val;	//循环次数容量，用来计算循环次数，需存储到FLASH
        uint16_t    cycle_dch_cap_val;	//   // 暂时未用

        uint8_t     show_cap_rate;  // 显示容量值 用来存放最终展示给用户的SOC值，与真正的剩余容量百分比有一个趋近过程，并不等效变化。需存储到FLASH
        uint8_t     re_cap_rate;    // 剩余容量百分比   // SOCr 由(CAPa - CAPu - CAPf)/CAPa计算而来
        uint8_t     re_cap_rate2;   // 充电前的静置soc   // 暂时未用
        uint8_t     re_cap_rate3;   // 充电前的静置soc 并且电压小于3500mv   // 暂时未用

        int32_t  	cap_cnt;
        uint16_t    bat_cycle_cnt;  // 循环次数，需存储到FLASH
        uint16_t    dch_cycle_cnt;   // 暂时未用
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
