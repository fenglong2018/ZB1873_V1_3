/*
 * Protect.h
 *
 *  Created on: 2017年1月23日
 *      Author: chenjiawei
 */

#ifndef PROTECT_H_
#define PROTECT_H_
#include"Global.h"

#define CELLS_NUM  6

#define CUR_K 100

#define USER_RSNS   1       // RSNS值，过流、短路档位选择
#define SCDDelay  SCD_DELAY_400us   //blust 20171226
#define SCDThresh  SCD_THRESH_133mV_67mV //RSNS=0   200/2 100   SCD_THRESH_178mV_89mV 		//blust 20171226
#define OCDDelay  OCD_DELAY_640ms		//blust 20171226
#define OCDThresh  OCD_THRESH_33mV_17mV//OCD_THRESH_61mV_31mV //RSNS=0  31A		//blust 20171226 30
#define OVDelay  OV_DELAY_1s
#define UVDelay  UV_DELAY_1s

// 充电温度保护
#define TEMP_H_CHG_P   470      //47
#define TEMP_H_CHG_R   420      //42
#define TEMP_L_CHG_P   30       //3
#define TEMP_L_CHG_R   50       //5

// NTC短路/断路保护
#define NTC_CLOSE_TMP   1000    // 100
#define NTC_OPEN_TMP    (-250)  // -25
//放电温度保护
#define TEMP_H_DCH_P   770      // 77
#define TEMP_H_DCH_R   570      // 57
#define TEMP_L_DCH_P   (-170)   //-17
#define TEMP_L_DCH_R   (-140)   //-14

#define OV_CELL_V  4200
#define OV_CELL_RV 4100

#define SOFT_OV_CELL_V  4170		//zhao
#define SOFT_OV_SHUT   5000

#define UV_CELL_V  2500
#define UV_CELL_RV 3300

#define SOFT_UV_CELL_V  2700
#define SHUT_DOWN_VOLT 2450

#define DIFF_CELL_V  600

#define VBAT_OV_startup   24600		//blust 20171226   zhao 起充保护
#define VBAT_OV_startup_R 24500		//blust 20171226

#define VCHG_OV_V   32500   // 充电器过压错误值

#define VBAT_UV_START_UP   16800//
#define VBAT_START_UP_R    17000//2018.2.26 整体开机保护

#define CHG_OC_CUR   2000           //blust 20171226
#define DCH_OC_CUR  -28000		//blust 20171226
#define DCH_OC_CUR2 -33000      //blust 20171226

#define CHG_STATE_CUR 100	// 由40改为100 blust 20180108
#define DCH_STATE_CUR -100	// 由-50改为-100 blust 20180108

typedef union
{
    struct
    {
        unsigned chg_state_flag                                	: 1;    // 充电状态
        unsigned dch_state_flag                                	: 1;    // 放电状态
        unsigned dch_on_flag                                    : 1;    // 允许放电标志（受控于TRIG和通讯）
        unsigned charger_on_flag                                : 1;    // 充电器在位状态
        unsigned start_vbat_ov_flag                             : 1;    // 开机整机过压起充保护     // 开机起充保护采用充饱标志代替，blust 20181128
        unsigned start_vbat_uv_flag                             : 1;    // 开机整机欠压保护
        unsigned vbat_chg_full_flag                             : 1;    // 电池包充满
        unsigned I2C_error_flag						            : 1;    // I2C通讯故障

        unsigned chg_5h_err_flag                         	    : 1;    // 充电5H故障标志
        unsigned vbat_update_OK     						    : 1;    // VBAT电压已更新为BQ采样值
        unsigned receive_ok_flag                                : 1;    // 通讯接收完成标志
        unsigned unload_long_flag                               : 1;    // 轻载长延时标志位
        unsigned trig_isr_flag                                  : 1;    // TRIG中断打开标志
        unsigned trig_on_flag                                   : 1;    // TRIG在位标志
        unsigned chg_full_ledoff_flag                      		: 1;    // 满电五分钟灭灯标志 blust 20181106
        unsigned NTC_open_close_flag                            : 1;    // NTC短路/断路标志 blust 20181202

        unsigned chg_on_checking_flag                           : 1;    // 充电器在位延时判断的100ms时的状态

    }val;
    uint32_t VAL;

}__SYS_STATE;

typedef union
{
    struct
    {
        unsigned chg_temp_low_protect_flag	   					: 1;    // 充电低温保护标志
        unsigned chg_temp_high_protect_flag                     : 1;    // 充电高温保护标志
        unsigned dch_temp_low_protect_flag     					: 1;    // 放电低温保护标志
        unsigned dch_temp_high_protect_flag     				: 1;    // 放电高温保护标志
        unsigned soft_vcell_high_protect_flag                   : 1;    // 软件单节过压保护标志
        unsigned vcell_high_protect_flag 						: 1;    // 硬件单节过压保护标志
        unsigned soft_vcell_low_protect_flag					: 1;    // 软件单节欠压保护标志
        unsigned vcell_low_protect_flag							: 1;    // 硬件单节欠压保护标志

        unsigned dch_oc_protect_flag						    : 1;    // 放电过流1保护标志
        unsigned dch_oc2_protect_flag							: 1;    // 放电过流2保护标志
        unsigned chg_oc_protect_flag							: 1;    // 充电过流保护标志
        unsigned dch_sc_protect_flag							: 1;    // 放电短路保护标志
        unsigned vcell_diff_protect_flag                       	: 1;    // 电芯压差过大标志
        unsigned vcell_high_err_flag							: 1;    // 电芯电压超高错误标志
        unsigned vcell_low_err_flag                             : 1;    // 电芯电压过低错误标志
        unsigned chg_val_high_err_flag                          : 1;    // 充电器电压过高错误标志
        //new by ljc
    }val;
    uint16_t VAL;
}__SYS_FLAGS;

//NEW BY LJC
typedef union
{
    struct
    {
        unsigned   chg_cap_revise_flag                          : 1;    // 充电过压修正容量标志位
        unsigned   first_cap_update_flag                        : 1;    // 首次上电容量更新标志
        unsigned   full_cap_update_flag                         : 1;    // 未使用
        unsigned   re_cap_rate_adjust_flag                      : 1;    // 未使用
        unsigned   re_cap_rate_adjust_flag2                     : 1;    // 未使用
        unsigned   re_write_flag                                : 1;    // 未使用
        unsigned   flash_write_error_flag                       : 1;    // 未使用
        unsigned   vbat_full_flag                               : 1;    // 未使用
    }val;
    uint8_t VAL;
}__CAP_FLAGS;

typedef union
{
    struct{
        uint16_t  vbat;
        uint16_t  vchg_ad_cp;
        uint16_t  vchg_ad_cb;
        uint16_t  vchg_val_cb;
        int32_t   cur;
        uint16_t vcell[CELLS_NUM];
        uint16_t max_cell_volt;
        uint16_t min_cell_volt;
        int16_t  temperature;
    }val;
}__SYS_SIGNAL;

//变量申明
extern volatile __SYS_FLAGS  g_sys_flags;
extern volatile __CAP_FLAGS  g_cap_flags;
extern volatile __SYS_SIGNAL g_sys_signal;
extern volatile __SYS_STATE  g_sys_state;


#endif /* PROTECT_H_ */
