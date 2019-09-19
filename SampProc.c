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

// BQ数据更新
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
        Temp_Get();      // 获取温度值
    }
}

// 电压信息处理
void VoltProc(void)
{
    static uint8_t vcell_high_cnt = 0;
    static uint8_t vcell_low_cnt = 0;
    static uint8_t vcell_diff_cnt = 0;
    static uint8_t vcell_high_err_cnt = 0;
    static uint8_t vcell_low_err_cnt = 0;

    // 过压
    if(Registers.SysStatus.StatusBit.OV == 1)
    {   // 硬件过压标志
        g_sys_flags.val.vcell_high_protect_flag = TRUE;
        g_sys_state.val.vbat_chg_full_flag = TRUE;
    }
    if(g_sys_signal.val.max_cell_volt > SOFT_OV_CELL_V)
    {   // 软件过压标志
        if((uint8_t)(sys_10ms_cnt - vcell_high_cnt) >= 100)
        {   // 软件过压延时
            vcell_high_cnt = sys_10ms_cnt;
            g_sys_flags.val.soft_vcell_high_protect_flag = TRUE;
            g_cap_flags.val.chg_cap_revise_flag = FALSE; // 过压清除充电容量修正标志位
            g_sys_state.val.vbat_chg_full_flag = TRUE;
        }
    }
    else
    {
        vcell_high_cnt = sys_10ms_cnt;
        if(g_sys_signal.val.max_cell_volt < OV_CELL_RV)
        {   // 过压恢复
            g_sys_flags.val.soft_vcell_high_protect_flag = FALSE;
            g_sys_state.val.vbat_chg_full_flag = FALSE; // 单节最高电压低于4.1V，清除充饱标志位 blust 20181114

            if(g_sys_flags.val.vcell_high_protect_flag == TRUE)
            {
                g_sys_flags.val.vcell_high_protect_flag = FALSE;
                Registers.SysStatus.StatusBit.OV = 1;
                I2CWriteBlockWithCRC(BQMAXIMO, SYS_STAT, &(Registers.SysStatus.StatusByte), 1);
            }
        }
    }

    // 欠压
    if(Registers.SysStatus.StatusBit.UV == 1)
    {   // 硬件欠压标志
        g_sys_flags.val.vcell_low_protect_flag = TRUE;
    }
    if(g_sys_signal.val.min_cell_volt < SOFT_UV_CELL_V)
    {   // 软件欠压标志
        if((uint8_t)(sys_10ms_cnt - vcell_low_cnt) >= 50)
        {   // 软件欠压延时
            vcell_low_cnt = sys_10ms_cnt;
            g_sys_flags.val.soft_vcell_low_protect_flag = TRUE;
        }
    }
    else
    {
        vcell_low_cnt = sys_10ms_cnt;
        if(g_sys_state.val.chg_state_flag == TRUE)   // 进入充电状态（有充电电流）才会解除欠压标志 blust 20181201
        {   // 欠压恢复
            g_sys_flags.val.soft_vcell_low_protect_flag = FALSE;
            if(g_sys_flags.val.vcell_low_protect_flag == TRUE)
            {
                g_sys_flags.val.vcell_low_protect_flag = FALSE;
                Registers.SysStatus.StatusBit.UV = 1;
                I2CWriteBlockWithCRC(BQMAXIMO, SYS_STAT, &(Registers.SysStatus.StatusByte), 1);
            }
        }
    }

    // 电芯电压异常
    if(g_sys_signal.val.max_cell_volt - g_sys_signal.val.min_cell_volt > DIFF_CELL_V)
    {   // 电芯压差过大
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
    {   // 电芯电压过高
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
    {   // 电芯电压过低
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

    // 整包电压
    if(g_sys_state.val.dch_state_flag == TRUE)
    {
        g_sys_state.val.start_vbat_uv_flag = FALSE;
    }
    else
    {   // 开机保护
        if(g_sys_signal.val.vbat < VBAT_UV_START_UP)
        {
            g_sys_state.val.start_vbat_uv_flag = TRUE;
        }
        else if(g_sys_signal.val.vbat > VBAT_START_UP_R)
        {
            g_sys_state.val.start_vbat_uv_flag = FALSE;
        }
    }

    // 开机起充保护采用充饱标志代替，blust 20181128
//    if(g_sys_state.val.chg_state_flag == TRUE)
//    {
//        g_sys_state.val.start_vbat_ov_flag = FALSE;
//    }
//    else
//    {   // 起充保护,电池包充满标志存在时才有起充保护，否则起充保护无作用。 blust 20181114
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

// 电流信息处理
void CurProc(void)
{
    static uint8_t cur_chg_state_cnt = 0;
    static uint8_t cur_chg_oc_cnt = 0;
    static uint8_t cur_dch_state_cnt = 0;
    static uint8_t cur_dch_oc1_cnt = 0;

    if(g_sys_signal.val.cur > CHG_STATE_CUR)
    {   // 充电
        if((uint8_t)(sys_10ms_cnt - cur_chg_state_cnt) >= 25)
        {   // 充电状态标志
            cur_chg_state_cnt = sys_10ms_cnt;
            g_sys_state.val.chg_state_flag = TRUE;
        }

        if(g_sys_signal.val.cur > CHG_OC_CUR)
        {   // 充电过流
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
    {   // 放电
        if((uint8_t)(sys_10ms_cnt - cur_dch_state_cnt) >= 25)
        {   // 放电状态标志
            cur_dch_state_cnt = sys_10ms_cnt;
            g_sys_state.val.dch_state_flag = TRUE;
        }

        if(g_sys_signal.val.cur < DCH_OC_CUR)
        {   // 放电过流1
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
    {   // 放电过流2
        g_sys_flags.val.dch_oc2_protect_flag = TRUE;
    }
    if(Registers.SysStatus.StatusBit.SCD == 1)
    {   // 放电短路
        g_sys_flags.val.dch_sc_protect_flag = TRUE;
    }
}

// 温度信息处理
void TempProc(void)
{
    static uint8_t temp_high_dch_cnt = 0;
    static uint8_t temp_low_dch_cnt = 0;
    static uint8_t temp_high_chg_cnt = 0;
    static uint8_t temp_low_chg_cnt = 0;

    if((g_sys_signal.val.temperature > NTC_CLOSE_TMP)||(g_sys_signal.val.temperature < NTC_OPEN_TMP))
    {   // NTC短路/断路判断 blust 20181202
        g_sys_state.val.NTC_open_close_flag = TRUE;
    }

    if(g_sys_signal.val.temperature > TEMP_H_DCH_P)
    {   // 放电高温保护
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
        {   // 放电高温恢复
            g_sys_flags.val.dch_temp_high_protect_flag = FALSE;
        }
    }

    if(g_sys_signal.val.temperature < TEMP_L_DCH_P)
    {   // 放电低温保护
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
        {   // 放电低温恢复
            g_sys_flags.val.dch_temp_low_protect_flag = FALSE;
        }
    }

    if(g_sys_signal.val.temperature > TEMP_H_CHG_P)
    {   // 充电高温保护
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
        {   // 充电高温恢复
            g_sys_flags.val.chg_temp_high_protect_flag = FALSE;
        }
    }

    if(g_sys_signal.val.temperature < TEMP_L_CHG_P)
    {   // 充电低温保护
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
        {   // 充电低温恢复
            g_sys_flags.val.chg_temp_low_protect_flag = FALSE;
        }
    }
}

// 系统状态信息处理
void SysInfoProc(void)
{
    VoltProc();
    CurProc();
    TempProc();
}

// 保护动作处理
void SysCtrlProc(void)
{
    uint8_t old_sysctrl = 0;

    old_sysctrl = Registers.SysCtrl2.SysCtrl2Byte;

    if(
      (g_sys_state.val.charger_on_flag == FALSE)    // 充电器未在位
    ||(g_sys_state.val.vbat_chg_full_flag == TRUE)  // 开机起充保护   // 开机起充保护采用充饱标志代替，blust 20181128
    ||(g_sys_state.val.chg_5h_err_flag == TRUE)  // 充电超过5H故障
    ||(g_sys_state.val.NTC_open_close_flag == TRUE)     // NTC短路/断路故障
    ||(g_sys_flags.val.chg_temp_low_protect_flag == TRUE)   // 充电低温保护
    ||(g_sys_flags.val.chg_temp_high_protect_flag == TRUE)  // 充电高温保护
    ||(g_sys_flags.val.soft_vcell_high_protect_flag == TRUE)    // 软件过压
    ||(g_sys_flags.val.vcell_high_protect_flag == TRUE)     // 硬件过压
    ||(g_sys_flags.val.chg_oc_protect_flag == TRUE)     // 充电过流
    ||(g_sys_flags.val.chg_val_high_err_flag == TRUE)     // 充电器过压
    ||(g_sys_flags.val.vcell_diff_protect_flag == TRUE)     // 电芯压差过大
    ||(g_sys_flags.val.vcell_high_err_flag == TRUE)     // 电芯超高压错误
    ||(g_sys_flags.val.vcell_low_err_flag == TRUE)      // 电芯超低压错误
    )
    {
        Registers.SysCtrl2.SysCtrl2Bit.CHG_ON = 0;    //shutdown the mosfet
    }
    else
    {
        Registers.SysCtrl2.SysCtrl2Bit.CHG_ON = 1;    //open the mosfet
    }

    if(
      (g_sys_state.val.dch_on_flag == FALSE)    // 不允许放电
    ||(g_sys_state.val.start_vbat_uv_flag == TRUE)  // 开机整机欠压保护
    ||(g_sys_state.val.NTC_open_close_flag == TRUE)     // NTC短路/断路故障
    ||(g_sys_flags.val.dch_temp_low_protect_flag == TRUE)   // 放电低温保护
    ||(g_sys_flags.val.dch_temp_high_protect_flag == TRUE)  // 放电高温保护
    ||(g_sys_flags.val.soft_vcell_low_protect_flag == TRUE) // 软件欠压保护
    ||(g_sys_flags.val.vcell_low_protect_flag == TRUE)      // 硬件欠压保护
    ||(g_sys_flags.val.dch_oc_protect_flag == TRUE)     // 一级放电过流保护
    ||(g_sys_flags.val.dch_oc2_protect_flag == TRUE)    // 二级放电过流保护
    ||(g_sys_flags.val.dch_sc_protect_flag == TRUE)     // 放电短路保护
//    ||(g_sys_flags.val.vcell_diff_protect_flag == TRUE)     // 电芯压差过大     // 放电不进行压差保护判断
    ||(g_sys_flags.val.vcell_high_err_flag == TRUE)     // 电芯超高压错误
    ||(g_sys_flags.val.vcell_low_err_flag == TRUE)      // 电芯超低压错误
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

// shutdown逻辑处理
void ShutDownProc(void)
{
    static uint8_t shutdown_5S_cnt = 0;
    static uint8_t shutdown_1_5h_cnt = 0;   // 轻载掉电1.5h延时计数
    static uint8_t shutdown_6S_cnt = 0;
    static uint8_t shutdown_30S_cnt = 0;
    static uint16_t shutdown_5min_cnt = 0;
    static uint16_t shutdown_2h_cnt = 0;
    static uint16_t shutdown_5h_cnt = 0;

//    if()
//    {   // 立即掉电
//        BqShutDown();
//    }

    if((g_sys_state.val.dch_on_flag == TRUE)&&(g_sys_signal.val.cur > -500)&&(g_sys_state.val.charger_on_flag == FALSE))
    {   // 轻载掉电，轻载掉电逻辑增加条件：充电器未在位时才进行轻载保护动作 blust 20181106
        if(g_sys_state.val.unload_long_flag == TRUE)
        {   // 接收到延长轻载掉电时间指令，轻载1.5h掉电 blust 20181009
            if((uint8_t)(sys_1min_cnt - shutdown_1_5h_cnt) >= 90)
            {
                shutdown_1_5h_cnt = sys_1min_cnt;
                __disable_interrupt();
                Registers.SysCtrl2.SysCtrl2Bit.CHG_ON = 0;  // 轻载保护掉电之前先关闭充放电管，1S以后再掉电  blust 20181129
                Registers.SysCtrl2.SysCtrl2Bit.DSG_ON = 0;
                I2CWriteBlockWithCRC(BQMAXIMO, SYS_CTRL2, &(Registers.SysCtrl2.SysCtrl2Byte), 1);
                LED_state0();
                delay_ms(1000);

                BqShutDown();
            }
        }
        else
        {   // 其余情况，轻载5S掉电
            if((uint8_t)(sys_250ms_cnt - shutdown_5S_cnt) >= 20)
            {
                shutdown_5S_cnt = sys_250ms_cnt;
                __disable_interrupt();
                Registers.SysCtrl2.SysCtrl2Bit.CHG_ON = 0;  // 轻载保护掉电之前先关闭充放电管，1S以后再掉电  blust 20181129
                Registers.SysCtrl2.SysCtrl2Bit.DSG_ON = 0;
                I2CWriteBlockWithCRC(BQMAXIMO, SYS_CTRL2, &(Registers.SysCtrl2.SysCtrl2Byte), 1);
                LED_state0();
                delay_ms(1000);

                BqShutDown();     // 测试程序，去除轻载掉电逻辑 blust 20180927
            }
        }
    }
    else
    {
        shutdown_5S_cnt = sys_250ms_cnt;
        shutdown_1_5h_cnt = sys_1min_cnt;
    }

    if(
        (g_sys_flags.val.dch_oc_protect_flag == TRUE)     // 一级放电过流保护
      ||(g_sys_flags.val.dch_oc2_protect_flag == TRUE)    // 二级放电过流保护
      ||(g_sys_flags.val.dch_sc_protect_flag == TRUE)     // 放电短路保护
      ||(g_sys_flags.val.chg_oc_protect_flag == TRUE)     // 充电过流
      ||(g_sys_flags.val.chg_val_high_err_flag == TRUE)     // 充电器过压
      ||(g_sys_state.val.start_vbat_uv_flag == TRUE)  // 开机整机欠压保护
      ||((g_sys_flags.val.dch_temp_low_protect_flag == TRUE)&&(g_sys_state.val.charger_on_flag == FALSE))   // 放电低温保护且充电器未在位时
      ||((g_sys_flags.val.dch_temp_high_protect_flag == TRUE)&&(g_sys_state.val.charger_on_flag == FALSE))  // 放电高温保护且充电器未在位时
      ||(g_sys_flags.val.soft_vcell_low_protect_flag == TRUE) // 软件欠压保护
      ||(g_sys_flags.val.vcell_low_protect_flag == TRUE)      // 硬件欠压保护
      ||((g_sys_flags.val.vcell_diff_protect_flag == TRUE)&&(g_sys_state.val.charger_on_flag == TRUE))     // 电芯压差过大且充电器在位时
      ||(g_sys_flags.val.vcell_high_err_flag == TRUE)     // 电芯超高压错误
      ||(g_sys_flags.val.vcell_low_err_flag == TRUE)      // 电芯超低压错误
      ||(g_sys_state.val.NTC_open_close_flag == TRUE)     // NTC短路/断路故障
    )
    {   // 故障，红灯快闪6S掉电
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
    {   // 输出被禁止且(充电器未在位，或者充电器在位但已经充饱5min后灭灯)，关闭输出，灭灯， 30S掉电
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

    if((g_sys_state.val.vbat_chg_full_flag == TRUE)    // 将软件过压标志和硬件过压标志改为充饱标志 blust 20181114
     &&(g_sys_cap.val.show_cap_rate == 100)         // SOC已经更新到100%，再开始5min常亮计时 blust 20181205
     &&(g_sys_state.val.charger_on_flag == TRUE))   // 充电器在位
    {   // 充满电或起充保护，充电器在位时，蓝灯常亮5min，掉电 充电器移除后，灭灯，30S掉电  // 开机起充保护采用充饱标志代替，blust 20181128
        if((sys_1s_cnt - shutdown_5min_cnt) >= 300)
        {
            shutdown_5min_cnt = sys_1s_cnt;
            g_sys_state.val.chg_full_ledoff_flag = TRUE;    // 充满电后，常亮5分钟灭灯 系统不掉电 blust 20181106
//            BqShutDown();
        }
    }
    else
    {
        g_sys_state.val.chg_full_ledoff_flag = FALSE;
        shutdown_5min_cnt = sys_1s_cnt;
    }

    if((
       (g_sys_flags.val.chg_temp_low_protect_flag == TRUE)   // 充电低温保护
     ||(g_sys_flags.val.chg_temp_high_protect_flag == TRUE)  // 充电高温保护
     )&&(g_sys_state.val.charger_on_flag == TRUE))   // 充电器在位
    {   // 充电温度保护，2h内未恢复，掉电
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
    {   // 充电过程（有充电电流），超过5h，直接灭灯。移除充电器解除该标志位
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
        {   // 移除充电器，解除充电5H故障标志
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
    //异常清除
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

