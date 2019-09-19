/*
 * Protect.h
 *
 *  Created on: 2017��1��23��
 *      Author: chenjiawei
 */

#ifndef PROTECT_H_
#define PROTECT_H_
#include"Global.h"

#define CELLS_NUM  6

#define CUR_K 100

#define USER_RSNS   1       // RSNSֵ����������·��λѡ��
#define SCDDelay  SCD_DELAY_400us   //blust 20171226
#define SCDThresh  SCD_THRESH_133mV_67mV //RSNS=0   200/2 100   SCD_THRESH_178mV_89mV 		//blust 20171226
#define OCDDelay  OCD_DELAY_640ms		//blust 20171226
#define OCDThresh  OCD_THRESH_33mV_17mV//OCD_THRESH_61mV_31mV //RSNS=0  31A		//blust 20171226 30
#define OVDelay  OV_DELAY_1s
#define UVDelay  UV_DELAY_1s

// ����¶ȱ���
#define TEMP_H_CHG_P   470      //47
#define TEMP_H_CHG_R   420      //42
#define TEMP_L_CHG_P   30       //3
#define TEMP_L_CHG_R   50       //5

// NTC��·/��·����
#define NTC_CLOSE_TMP   1000    // 100
#define NTC_OPEN_TMP    (-250)  // -25
//�ŵ��¶ȱ���
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

#define VBAT_OV_startup   24600		//blust 20171226   zhao ��䱣��
#define VBAT_OV_startup_R 24500		//blust 20171226

#define VCHG_OV_V   32500   // �������ѹ����ֵ

#define VBAT_UV_START_UP   16800//
#define VBAT_START_UP_R    17000//2018.2.26 ���忪������

#define CHG_OC_CUR   2000           //blust 20171226
#define DCH_OC_CUR  -28000		//blust 20171226
#define DCH_OC_CUR2 -33000      //blust 20171226

#define CHG_STATE_CUR 100	// ��40��Ϊ100 blust 20180108
#define DCH_STATE_CUR -100	// ��-50��Ϊ-100 blust 20180108

typedef union
{
    struct
    {
        unsigned chg_state_flag                                	: 1;    // ���״̬
        unsigned dch_state_flag                                	: 1;    // �ŵ�״̬
        unsigned dch_on_flag                                    : 1;    // ����ŵ��־���ܿ���TRIG��ͨѶ��
        unsigned charger_on_flag                                : 1;    // �������λ״̬
        unsigned start_vbat_ov_flag                             : 1;    // ����������ѹ��䱣��     // ������䱣�����ó䱥��־���棬blust 20181128
        unsigned start_vbat_uv_flag                             : 1;    // ��������Ƿѹ����
        unsigned vbat_chg_full_flag                             : 1;    // ��ذ�����
        unsigned I2C_error_flag						            : 1;    // I2CͨѶ����

        unsigned chg_5h_err_flag                         	    : 1;    // ���5H���ϱ�־
        unsigned vbat_update_OK     						    : 1;    // VBAT��ѹ�Ѹ���ΪBQ����ֵ
        unsigned receive_ok_flag                                : 1;    // ͨѶ������ɱ�־
        unsigned unload_long_flag                               : 1;    // ���س���ʱ��־λ
        unsigned trig_isr_flag                                  : 1;    // TRIG�жϴ򿪱�־
        unsigned trig_on_flag                                   : 1;    // TRIG��λ��־
        unsigned chg_full_ledoff_flag                      		: 1;    // �����������Ʊ�־ blust 20181106
        unsigned NTC_open_close_flag                            : 1;    // NTC��·/��·��־ blust 20181202

        unsigned chg_on_checking_flag                           : 1;    // �������λ��ʱ�жϵ�100msʱ��״̬

    }val;
    uint32_t VAL;

}__SYS_STATE;

typedef union
{
    struct
    {
        unsigned chg_temp_low_protect_flag	   					: 1;    // �����±�����־
        unsigned chg_temp_high_protect_flag                     : 1;    // �����±�����־
        unsigned dch_temp_low_protect_flag     					: 1;    // �ŵ���±�����־
        unsigned dch_temp_high_protect_flag     				: 1;    // �ŵ���±�����־
        unsigned soft_vcell_high_protect_flag                   : 1;    // ������ڹ�ѹ������־
        unsigned vcell_high_protect_flag 						: 1;    // Ӳ�����ڹ�ѹ������־
        unsigned soft_vcell_low_protect_flag					: 1;    // �������Ƿѹ������־
        unsigned vcell_low_protect_flag							: 1;    // Ӳ������Ƿѹ������־

        unsigned dch_oc_protect_flag						    : 1;    // �ŵ����1������־
        unsigned dch_oc2_protect_flag							: 1;    // �ŵ����2������־
        unsigned chg_oc_protect_flag							: 1;    // ������������־
        unsigned dch_sc_protect_flag							: 1;    // �ŵ��·������־
        unsigned vcell_diff_protect_flag                       	: 1;    // ��оѹ������־
        unsigned vcell_high_err_flag							: 1;    // ��о��ѹ���ߴ����־
        unsigned vcell_low_err_flag                             : 1;    // ��о��ѹ���ʹ����־
        unsigned chg_val_high_err_flag                          : 1;    // �������ѹ���ߴ����־
        //new by ljc
    }val;
    uint16_t VAL;
}__SYS_FLAGS;

//NEW BY LJC
typedef union
{
    struct
    {
        unsigned   chg_cap_revise_flag                          : 1;    // ����ѹ����������־λ
        unsigned   first_cap_update_flag                        : 1;    // �״��ϵ��������±�־
        unsigned   full_cap_update_flag                         : 1;    // δʹ��
        unsigned   re_cap_rate_adjust_flag                      : 1;    // δʹ��
        unsigned   re_cap_rate_adjust_flag2                     : 1;    // δʹ��
        unsigned   re_write_flag                                : 1;    // δʹ��
        unsigned   flash_write_error_flag                       : 1;    // δʹ��
        unsigned   vbat_full_flag                               : 1;    // δʹ��
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

//��������
extern volatile __SYS_FLAGS  g_sys_flags;
extern volatile __CAP_FLAGS  g_cap_flags;
extern volatile __SYS_SIGNAL g_sys_signal;
extern volatile __SYS_STATE  g_sys_state;


#endif /* PROTECT_H_ */
