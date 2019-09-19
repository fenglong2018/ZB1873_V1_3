/*
 * SampProc.c
 *
 *  Created on: 2014-7-18
 *      Author: Administrator
 */
#include "Protect.h"
#include "SampProc.h"

volatile __SYS_FLAGS  g_sys_flags;
volatile __CAP_FLAGS  g_cap_flags;
volatile __SYS_SIGNAL g_sys_signal;
volatile __SYS_STATE  g_sys_state;

// BQ���ݸ���
void Update_data(void)
{
    static uint8_t state_125mS_tick = 0;

    if((uint8_t)(sys_10ms_cnt - state_125mS_tick) >= 12)
    {
        state_125mS_tick = sys_10ms_cnt;

        UpdateStatFromBq();
        if(g_sys_state.val.I2C_error_flag == TRUE)
        {
            g_sys_state.val.I2C_error_flag = FALSE;
            ReConfigBq();
        }
        UpdateVoltageFromBqMaximo();
        if(g_sys_state.val.I2C_error_flag == TRUE)
        {
            g_sys_state.val.I2C_error_flag = FALSE;
            ReConfigBq();
        }
        UpdateCurFromBq();
        if(g_sys_state.val.I2C_error_flag == TRUE)
        {
            g_sys_state.val.I2C_error_flag = FALSE;
            ReConfigBq();
        }
        Temp_Get();      // ��ȡ�¶�ֵ
    }
}

// ��ѹ��Ϣ����
void VoltProc(void)
{
    static uint8_t vcell_high_cnt = 0;
    static uint8_t vcell_low_cnt = 0;
    static uint8_t vcell_diff_cnt = 0;
    static uint8_t vcell_high_err_cnt = 0;
    static uint8_t vcell_low_err_cnt = 0;

    // ��ѹ
    if(Registers.SysStatus.StatusBit.OV == 1)
    {   // Ӳ����ѹ��־
        g_sys_flags.val.vcell_high_protect_flag = TRUE;
        g_sys_state.val.vbat_chg_full_flag = TRUE;
    }
    if(g_sys_signal.val.max_cell_volt > SOFT_OV_CELL_V)
    {   // �����ѹ��־
        if((uint8_t)(sys_10ms_cnt - vcell_high_cnt) >= 100)
        {   // �����ѹ��ʱ
            vcell_high_cnt = sys_10ms_cnt;
            g_sys_flags.val.soft_vcell_high_protect_flag = TRUE;
            g_cap_flags.val.chg_cap_revise_flag = FALSE; // ��ѹ����������������־λ
            g_sys_state.val.vbat_chg_full_flag = TRUE;
        }
    }
    else
    {
        vcell_high_cnt = sys_10ms_cnt;
        if(g_sys_signal.val.max_cell_volt < OV_CELL_RV)
        {   // ��ѹ�ָ�
            g_sys_flags.val.soft_vcell_high_protect_flag = FALSE;
            g_sys_state.val.vbat_chg_full_flag = FALSE; // ������ߵ�ѹ����4.1V������䱥��־λ blust 20181114

            if(g_sys_flags.val.vcell_high_protect_flag == TRUE)
            {
                g_sys_flags.val.vcell_high_protect_flag = FALSE;
                Registers.SysStatus.StatusBit.OV = 1;
                I2CWriteBlockWithCRC(BQMAXIMO, SYS_STAT, &(Registers.SysStatus.StatusByte), 1);
            }
        }
    }

    // Ƿѹ
    if(Registers.SysStatus.StatusBit.UV == 1)
    {   // Ӳ��Ƿѹ��־
        g_sys_flags.val.vcell_low_protect_flag = TRUE;
    }
    if(g_sys_signal.val.min_cell_volt < SOFT_UV_CELL_V)
    {   // ���Ƿѹ��־
        if((uint8_t)(sys_10ms_cnt - vcell_low_cnt) >= 50)
        {   // ���Ƿѹ��ʱ
            vcell_low_cnt = sys_10ms_cnt;
            g_sys_flags.val.soft_vcell_low_protect_flag = TRUE;
        }
    }
    else
    {
        vcell_low_cnt = sys_10ms_cnt;
        if(g_sys_state.val.chg_state_flag == TRUE)   // ������״̬���г��������Ż���Ƿѹ��־ blust 20181201
        {   // Ƿѹ�ָ�
            g_sys_flags.val.soft_vcell_low_protect_flag = FALSE;
            if(g_sys_flags.val.vcell_low_protect_flag == TRUE)
            {
                g_sys_flags.val.vcell_low_protect_flag = FALSE;
                Registers.SysStatus.StatusBit.UV = 1;
                I2CWriteBlockWithCRC(BQMAXIMO, SYS_STAT, &(Registers.SysStatus.StatusByte), 1);
            }
        }
    }

    // ��о��ѹ�쳣
    if(g_sys_signal.val.max_cell_volt - g_sys_signal.val.min_cell_volt > DIFF_CELL_V)
    {   // ��оѹ�����
        if((uint8_t)(sys_10ms_cnt - vcell_diff_cnt) >= 100)
        {
            vcell_diff_cnt = sys_10ms_cnt;
            g_sys_flags.val.vcell_diff_protect_flag = TRUE;
        }
    }
    else
    {
        vcell_diff_cnt = sys_10ms_cnt;
        g_sys_flags.val.vcell_diff_protect_flag = FALSE;
    }

    if(g_sys_signal.val.max_cell_volt > 5000)
    {   // ��о��ѹ����
        if((uint8_t)(sys_10ms_cnt - vcell_high_err_cnt) >= 100)
        {
            vcell_high_err_cnt = sys_10ms_cnt;
            g_sys_flags.val.vcell_high_err_flag = TRUE;
        }
    }
    else
    {
        vcell_high_err_cnt = sys_10ms_cnt;
        g_sys_flags.val.vcell_high_err_flag = FALSE;
    }

    if(g_sys_signal.val.min_cell_volt < 1600)
    {   // ��о��ѹ����
        if((uint8_t)(sys_10ms_cnt - vcell_low_err_cnt) >= 100)
        {
            vcell_low_err_cnt = sys_10ms_cnt;
            g_sys_flags.val.vcell_low_err_flag = TRUE;
        }
    }
    else
    {
        vcell_low_err_cnt = sys_10ms_cnt;
        g_sys_flags.val.vcell_low_err_flag = FALSE;
    }

    // ������ѹ
    if(g_sys_state.val.dch_state_flag == TRUE)
    {
        g_sys_state.val.start_vbat_uv_flag = FALSE;
    }
    else
    {   // ��������
        if(g_sys_signal.val.vbat < VBAT_UV_START_UP)
        {
            g_sys_state.val.start_vbat_uv_flag = TRUE;
        }
        else if(g_sys_signal.val.vbat > VBAT_START_UP_R)
        {
            g_sys_state.val.start_vbat_uv_flag = FALSE;
        }
    }

    // ������䱣�����ó䱥��־���棬blust 20181128
//    if(g_sys_state.val.chg_state_flag == TRUE)
//    {
//        g_sys_state.val.start_vbat_ov_flag = FALSE;
//    }
//    else
//    {   // ��䱣��,��ذ�������־����ʱ������䱣����������䱣�������á� blust 20181114
//        if((g_sys_signal.val.vbat > VBAT_OV_startup)&&(g_sys_state.val.vbat_chg_full_flag == TRUE))
//        {
//            g_sys_state.val.start_vbat_ov_flag = TRUE;
//        }
//        else if(g_sys_signal.val.vbat < VBAT_OV_startup_R)
//        {
//            g_sys_state.val.start_vbat_ov_flag = FALSE;
//        }
//    }
}

// ������Ϣ����
void CurProc(void)
{
    static uint8_t cur_chg_state_cnt = 0;
    static uint8_t cur_chg_oc_cnt = 0;
    static uint8_t cur_dch_state_cnt = 0;
    static uint8_t cur_dch_oc1_cnt = 0;

    if(g_sys_signal.val.cur > CHG_STATE_CUR)
    {   // ���
        if((uint8_t)(sys_10ms_cnt - cur_chg_state_cnt) >= 25)
        {   // ���״̬��־
            cur_chg_state_cnt = sys_10ms_cnt;
            g_sys_state.val.chg_state_flag = TRUE;
        }

        if(g_sys_signal.val.cur > CHG_OC_CUR)
        {   // ������
            if((uint8_t)(sys_10ms_cnt - cur_chg_oc_cnt) >= 100)
            {
                cur_chg_oc_cnt = sys_10ms_cnt;
                g_sys_flags.val.chg_oc_protect_flag = TRUE;
            }
        }
        else
        {
            cur_chg_oc_cnt = sys_10ms_cnt;
        }
    }
    else
    {
        g_sys_state.val.chg_state_flag = FALSE;
        cur_chg_state_cnt = sys_10ms_cnt;
        cur_chg_oc_cnt = sys_10ms_cnt;
    }

    if(g_sys_signal.val.cur < DCH_STATE_CUR)
    {   // �ŵ�
        if((uint8_t)(sys_10ms_cnt - cur_dch_state_cnt) >= 25)
        {   // �ŵ�״̬��־
            cur_dch_state_cnt = sys_10ms_cnt;
            g_sys_state.val.dch_state_flag = TRUE;
        }

        if(g_sys_signal.val.cur < DCH_OC_CUR)
        {   // �ŵ����1
            if((uint8_t)(sys_10ms_cnt - cur_dch_oc1_cnt) >= 100)
            {
                cur_dch_oc1_cnt = sys_10ms_cnt;
                g_sys_flags.val.dch_oc_protect_flag = TRUE;
            }
        }
        else
        {
            cur_dch_oc1_cnt = sys_10ms_cnt;
        }
    }
    else
    {
        g_sys_state.val.dch_state_flag = FALSE;
        cur_dch_state_cnt = sys_10ms_cnt;
        cur_dch_oc1_cnt = sys_10ms_cnt;
    }

    if(Registers.SysStatus.StatusBit.OCD == 1)
    {   // �ŵ����2
        g_sys_flags.val.dch_oc2_protect_flag = TRUE;
    }
    if(Registers.SysStatus.StatusBit.SCD == 1)
    {   // �ŵ��·
        g_sys_flags.val.dch_sc_protect_flag = TRUE;
    }
}

// �¶���Ϣ����
void TempProc(void)
{
    static uint8_t temp_high_dch_cnt = 0;
    static uint8_t temp_low_dch_cnt = 0;
    static uint8_t temp_high_chg_cnt = 0;
    static uint8_t temp_low_chg_cnt = 0;

    if((g_sys_signal.val.temperature > NTC_CLOSE_TMP)||(g_sys_signal.val.temperature < NTC_OPEN_TMP))
    {   // NTC��·/��·�ж� blust 20181202
        g_sys_state.val.NTC_open_close_flag = TRUE;
    }

    if(g_sys_signal.val.temperature > TEMP_H_DCH_P)
    {   // �ŵ���±���
        if((uint8_t)(sys_10ms_cnt - temp_high_dch_cnt) >= 100)
        {
            temp_high_dch_cnt = sys_10ms_cnt;
            g_sys_flags.val.dch_temp_high_protect_flag = TRUE;
        }
    }
    else
    {
        temp_high_dch_cnt = sys_10ms_cnt;
        if(g_sys_signal.val.temperature < TEMP_H_DCH_R)
        {   // �ŵ���»ָ�
            g_sys_flags.val.dch_temp_high_protect_flag = FALSE;
        }
    }

    if(g_sys_signal.val.temperature < TEMP_L_DCH_P)
    {   // �ŵ���±���
        if((uint8_t)(sys_10ms_cnt - temp_low_dch_cnt) >= 100)
        {
            temp_low_dch_cnt = sys_10ms_cnt;
            g_sys_flags.val.dch_temp_low_protect_flag = TRUE;
        }
    }
    else
    {
        temp_low_dch_cnt = sys_10ms_cnt;
        if(g_sys_signal.val.temperature > TEMP_L_DCH_R)
        {   // �ŵ���»ָ�
            g_sys_flags.val.dch_temp_low_protect_flag = FALSE;
        }
    }

    if(g_sys_signal.val.temperature > TEMP_H_CHG_P)
    {   // �����±���
        if((uint8_t)(sys_10ms_cnt - temp_high_chg_cnt) >= 100)
        {
            temp_high_chg_cnt = sys_10ms_cnt;
            g_sys_flags.val.chg_temp_high_protect_flag = TRUE;
        }
    }
    else
    {
        temp_high_chg_cnt = sys_10ms_cnt;
        if(g_sys_signal.val.temperature < TEMP_H_CHG_R)
        {   // �����»ָ�
            g_sys_flags.val.chg_temp_high_protect_flag = FALSE;
        }
    }

    if(g_sys_signal.val.temperature < TEMP_L_CHG_P)
    {   // �����±���
        if((uint8_t)(sys_10ms_cnt - temp_low_chg_cnt) >= 100)
        {
            temp_low_chg_cnt = sys_10ms_cnt;
            g_sys_flags.val.chg_temp_low_protect_flag = TRUE;
        }
    }
    else
    {
        temp_low_chg_cnt = sys_10ms_cnt;
        if(g_sys_signal.val.temperature > TEMP_L_CHG_R)
        {   // �����»ָ�
            g_sys_flags.val.chg_temp_low_protect_flag = FALSE;
        }
    }
}

// ϵͳ״̬��Ϣ����
void SysInfoProc(void)
{
    VoltProc();
    CurProc();
    TempProc();
}

// ������������
void SysCtrlProc(void)
{
    uint8_t old_sysctrl = 0;

    old_sysctrl = Registers.SysCtrl2.SysCtrl2Byte;

    if(
      (g_sys_state.val.charger_on_flag == FALSE)    // �����δ��λ
    ||(g_sys_state.val.vbat_chg_full_flag == TRUE)  // ������䱣��   // ������䱣�����ó䱥��־���棬blust 20181128
    ||(g_sys_state.val.chg_5h_err_flag == TRUE)  // ��糬��5H����
    ||(g_sys_state.val.NTC_open_close_flag == TRUE)     // NTC��·/��·����
    ||(g_sys_flags.val.chg_temp_low_protect_flag == TRUE)   // �����±���
    ||(g_sys_flags.val.chg_temp_high_protect_flag == TRUE)  // �����±���
    ||(g_sys_flags.val.soft_vcell_high_protect_flag == TRUE)    // �����ѹ
    ||(g_sys_flags.val.vcell_high_protect_flag == TRUE)     // Ӳ����ѹ
    ||(g_sys_flags.val.chg_oc_protect_flag == TRUE)     // ������
    ||(g_sys_flags.val.chg_val_high_err_flag == TRUE)     // �������ѹ
    ||(g_sys_flags.val.vcell_diff_protect_flag == TRUE)     // ��оѹ�����
    ||(g_sys_flags.val.vcell_high_err_flag == TRUE)     // ��о����ѹ����
    ||(g_sys_flags.val.vcell_low_err_flag == TRUE)      // ��о����ѹ����
    )
    {
        Registers.SysCtrl2.SysCtrl2Bit.CHG_ON = 0;    //shutdown the mosfet
    }
    else
    {
        Registers.SysCtrl2.SysCtrl2Bit.CHG_ON = 1;    //open the mosfet
    }

    if(
      (g_sys_state.val.dch_on_flag == FALSE)    // ������ŵ�
    ||(g_sys_state.val.start_vbat_uv_flag == TRUE)  // ��������Ƿѹ����
    ||(g_sys_state.val.NTC_open_close_flag == TRUE)     // NTC��·/��·����
    ||(g_sys_flags.val.dch_temp_low_protect_flag == TRUE)   // �ŵ���±���
    ||(g_sys_flags.val.dch_temp_high_protect_flag == TRUE)  // �ŵ���±���
    ||(g_sys_flags.val.soft_vcell_low_protect_flag == TRUE) // ���Ƿѹ����
    ||(g_sys_flags.val.vcell_low_protect_flag == TRUE)      // Ӳ��Ƿѹ����
    ||(g_sys_flags.val.dch_oc_protect_flag == TRUE)     // һ���ŵ��������
    ||(g_sys_flags.val.dch_oc2_protect_flag == TRUE)    // �����ŵ��������
    ||(g_sys_flags.val.dch_sc_protect_flag == TRUE)     // �ŵ��·����
//    ||(g_sys_flags.val.vcell_diff_protect_flag == TRUE)     // ��оѹ�����     // �ŵ粻����ѹ����ж�
    ||(g_sys_flags.val.vcell_high_err_flag == TRUE)     // ��о����ѹ����
    ||(g_sys_flags.val.vcell_low_err_flag == TRUE)      // ��о����ѹ����
    )
    {
        Registers.SysCtrl2.SysCtrl2Bit.DSG_ON = 0;
    }
    else
    {
        Registers.SysCtrl2.SysCtrl2Bit.DSG_ON = 1;
    }

    if(old_sysctrl != Registers.SysCtrl2.SysCtrl2Byte)
    {
        __disable_interrupt();
        I2CWriteBlockWithCRC(BQMAXIMO, SYS_CTRL2, &(Registers.SysCtrl2.SysCtrl2Byte), 1);
        __enable_interrupt();
    }
}

// shutdown�߼�����
void ShutDownProc(void)
{
    static uint8_t shutdown_5S_cnt = 0;
    static uint8_t shutdown_1_5h_cnt = 0;   // ���ص���1.5h��ʱ����
    static uint8_t shutdown_6S_cnt = 0;
    static uint8_t shutdown_30S_cnt = 0;
    static uint16_t shutdown_5min_cnt = 0;
    static uint16_t shutdown_2h_cnt = 0;
    static uint16_t shutdown_5h_cnt = 0;

//    if()
//    {   // ��������
//        BqShutDown();
//    }

    if((g_sys_state.val.dch_on_flag == TRUE)&&(g_sys_signal.val.cur > -500)&&(g_sys_state.val.charger_on_flag == FALSE))
    {   // ���ص��磬���ص����߼����������������δ��λʱ�Ž������ر������� blust 20181106
        if(g_sys_state.val.unload_long_flag == TRUE)
        {   // ���յ��ӳ����ص���ʱ��ָ�����1.5h���� blust 20181009
            if((uint8_t)(sys_1min_cnt - shutdown_1_5h_cnt) >= 90)
            {
                shutdown_1_5h_cnt = sys_1min_cnt;
                __disable_interrupt();
                Registers.SysCtrl2.SysCtrl2Bit.CHG_ON = 0;  // ���ر�������֮ǰ�ȹرճ�ŵ�ܣ�1S�Ժ��ٵ���  blust 20181129
                Registers.SysCtrl2.SysCtrl2Bit.DSG_ON = 0;
                I2CWriteBlockWithCRC(BQMAXIMO, SYS_CTRL2, &(Registers.SysCtrl2.SysCtrl2Byte), 1);
                LED_state0();
                delay_ms(1000);

                BqShutDown();
            }
        }
        else
        {   // �������������5S����
            if((uint8_t)(sys_250ms_cnt - shutdown_5S_cnt) >= 20)
            {
                shutdown_5S_cnt = sys_250ms_cnt;
                __disable_interrupt();
                Registers.SysCtrl2.SysCtrl2Bit.CHG_ON = 0;  // ���ر�������֮ǰ�ȹرճ�ŵ�ܣ�1S�Ժ��ٵ���  blust 20181129
                Registers.SysCtrl2.SysCtrl2Bit.DSG_ON = 0;
                I2CWriteBlockWithCRC(BQMAXIMO, SYS_CTRL2, &(Registers.SysCtrl2.SysCtrl2Byte), 1);
                LED_state0();
                delay_ms(1000);

                BqShutDown();     // ���Գ���ȥ�����ص����߼� blust 20180927
            }
        }
    }
    else
    {
        shutdown_5S_cnt = sys_250ms_cnt;
        shutdown_1_5h_cnt = sys_1min_cnt;
    }

    if(
        (g_sys_flags.val.dch_oc_protect_flag == TRUE)     // һ���ŵ��������
      ||(g_sys_flags.val.dch_oc2_protect_flag == TRUE)    // �����ŵ��������
      ||(g_sys_flags.val.dch_sc_protect_flag == TRUE)     // �ŵ��·����
      ||(g_sys_flags.val.chg_oc_protect_flag == TRUE)     // ������
      ||(g_sys_flags.val.chg_val_high_err_flag == TRUE)     // �������ѹ
      ||(g_sys_state.val.start_vbat_uv_flag == TRUE)  // ��������Ƿѹ����
      ||((g_sys_flags.val.dch_temp_low_protect_flag == TRUE)&&(g_sys_state.val.charger_on_flag == FALSE))   // �ŵ���±����ҳ����δ��λʱ
      ||((g_sys_flags.val.dch_temp_high_protect_flag == TRUE)&&(g_sys_state.val.charger_on_flag == FALSE))  // �ŵ���±����ҳ����δ��λʱ
      ||(g_sys_flags.val.soft_vcell_low_protect_flag == TRUE) // ���Ƿѹ����
      ||(g_sys_flags.val.vcell_low_protect_flag == TRUE)      // Ӳ��Ƿѹ����
      ||((g_sys_flags.val.vcell_diff_protect_flag == TRUE)&&(g_sys_state.val.charger_on_flag == TRUE))     // ��оѹ������ҳ������λʱ
      ||(g_sys_flags.val.vcell_high_err_flag == TRUE)     // ��о����ѹ����
      ||(g_sys_flags.val.vcell_low_err_flag == TRUE)      // ��о����ѹ����
      ||(g_sys_state.val.NTC_open_close_flag == TRUE)     // NTC��·/��·����
    )
    {   // ���ϣ���ƿ���6S����
        if((uint8_t)(sys_250ms_cnt - shutdown_6S_cnt) >= 24)
        {
            shutdown_6S_cnt = sys_250ms_cnt;
            BqShutDown();
        }
    }
    else
    {
        shutdown_6S_cnt = sys_250ms_cnt;
    }

    if(
        (g_sys_state.val.dch_on_flag == FALSE)
      &&(  (g_sys_state.val.charger_on_flag == FALSE)
         ||((g_sys_state.val.charger_on_flag == TRUE)&&(g_sys_state.val.chg_full_ledoff_flag == TRUE))
        )
      )
    {   // �������ֹ��(�����δ��λ�����߳������λ���Ѿ��䱥5min�����)���ر��������ƣ� 30S����
        if((uint8_t)(sys_1s_cnt - shutdown_30S_cnt) >= 30)
        {
            shutdown_30S_cnt = sys_1s_cnt;
            Cap_Revise();
            BqShutDown();
        }
    }
    else
    {
        shutdown_30S_cnt = sys_1s_cnt;
    }

    if((g_sys_state.val.vbat_chg_full_flag == TRUE)    // �������ѹ��־��Ӳ����ѹ��־��Ϊ�䱥��־ blust 20181114
     &&(g_sys_cap.val.show_cap_rate == 100)         // SOC�Ѿ����µ�100%���ٿ�ʼ5min������ʱ blust 20181205
     &&(g_sys_state.val.charger_on_flag == TRUE))   // �������λ
    {   // ���������䱣�����������λʱ�����Ƴ���5min������ ������Ƴ�����ƣ�30S����  // ������䱣�����ó䱥��־���棬blust 20181128
        if((sys_1s_cnt - shutdown_5min_cnt) >= 300)
        {
            shutdown_5min_cnt = sys_1s_cnt;
            g_sys_state.val.chg_full_ledoff_flag = TRUE;    // ������󣬳���5������� ϵͳ������ blust 20181106
//            BqShutDown();
        }
    }
    else
    {
        g_sys_state.val.chg_full_ledoff_flag = FALSE;
        shutdown_5min_cnt = sys_1s_cnt;
    }

    if((
       (g_sys_flags.val.chg_temp_low_protect_flag == TRUE)   // �����±���
     ||(g_sys_flags.val.chg_temp_high_protect_flag == TRUE)  // �����±���
     )&&(g_sys_state.val.charger_on_flag == TRUE))   // �������λ
    {   // ����¶ȱ�����2h��δ�ָ�������
        if((sys_1min_cnt - shutdown_2h_cnt) >= 120)
        {
            shutdown_2h_cnt = sys_1min_cnt;
            BqShutDown();
        }
    }
    else
    {
        shutdown_2h_cnt = sys_1min_cnt;
    }

    if(g_sys_state.val.chg_state_flag == TRUE)
    {   // �����̣��г�������������5h��ֱ����ơ��Ƴ����������ñ�־λ
        if((sys_1min_cnt - shutdown_5h_cnt) >= 300)
        {
            shutdown_5h_cnt = sys_1min_cnt;
            g_sys_state.val.chg_5h_err_flag = TRUE;
//            BqShutDown();
        }
    }
    else
    {
        shutdown_5h_cnt = sys_1min_cnt;
        if(g_sys_state.val.charger_on_flag == FALSE)
        {   // �Ƴ��������������5H���ϱ�־
            g_sys_state.val.chg_5h_err_flag = FALSE;
        }
    }
}

/***************************************
 * @Description:clear the BQ DEVICE_XREADY and OVRD_ALERT flag
 *
 * @Parameter:
 *
 ***************************************/
void BQErrorClear(void)
{
    uint8_t Status_Byte = 0;
    //�쳣���
    if(1 == Registers.SysStatus.StatusBit.DEVICE_XREADY)
    {
        Status_Byte = 0x20;
        I2CWriteBlockWithCRC(BQMAXIMO, SYS_STAT, &(Status_Byte), 1);
        I2CReadBlockWithCRC(BQMAXIMO, SYS_STAT, &(Registers.SysStatus.StatusByte), 1);
    }

    if(1 == Registers.SysStatus.StatusBit.OVRD_ALERT)
    {
        Status_Byte = 0x10;
        I2CWriteBlockWithCRC(BQMAXIMO, SYS_STAT, &(Status_Byte), 1);
        I2CReadBlockWithCRC(BQMAXIMO, SYS_STAT, &(Registers.SysStatus.StatusByte), 1);
    }

    if(0 == Registers.SysCtrl1.SysCtrl1Bit.TEMP_SEL)
    {
        Registers.SysCtrl1.SysCtrl1Bit.TEMP_SEL = 1;

        I2CWriteBlockWithCRC(BQMAXIMO, SYS_CTRL1, &(Registers.SysCtrl1.SysCtrl1Byte), 1);
        I2CReadBlockWithCRC(BQMAXIMO, SYS_CTRL1, &(Registers.SysCtrl1.SysCtrl1Byte), 1);
    }
}

