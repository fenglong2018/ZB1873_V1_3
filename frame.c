/*
 * frame.c
 *
 *  Created on: 2018年1月24日
 *      Author: zhao
 */

#include"frame.h"

volatile uint8_t uart_continue_cnt = 0;

/****************************************************************************
FUNCTION        : UartDataPro
DESCRIPTION     : UART接收处理与反馈
INPUT           : None
OUTPUT          : None
NOTICE          : 中断缓存,MAIN判断
DATE            : 2018/09/18
 *****************************************************************************/
void UartDataPro(void)
{
    int8_t  rx_length = 0;
    uint8_t data_length = 0;
    uint8_t sum_check = 0;
    uint8_t err_flag = 0;   // 通讯帧数据超范围不需要响应时置1

    if((uint8_t)(sys_10ms_cnt - uart_continue_cnt) >= 100)
    {   // 超过1S没有正确通讯，禁止放电
        uart_continue_cnt = sys_10ms_cnt;
        g_sys_state.val.dch_on_flag = FALSE;      // 测试用，去除1S无通讯关输出逻辑 blust 20180927
        comm_led_state = led_comm_normol;
    }

    if(g_sys_state.val.receive_ok_flag == TRUE)
    {
        g_sys_state.val.receive_ok_flag = FALSE;
        rx_length = uart_Read();
        if((rx_length <= 0)||(rx_length > UART_TX_RX_SIZE))
        {   // 数据长度错误，直接清接收数组，跳出子函数
            memset(UartRxBuffer, 0, sizeof(UartRxBuffer));
            return;
        }

        if((UartRxBuffer[0] == COMM_SOF_B)&&(UartRxBuffer[1] == COMM_ADDR_1)&&(UartRxBuffer[2] == COMM_ADDR_2))
        {   // 帧头与地址正确
            data_length = UartRxBuffer[5];
            if(rx_length < (data_length + 8))
            {   // 数据长度不足，直接清接收数组，跳出子函数
                memset(UartRxBuffer, 0, sizeof(UartRxBuffer));
                return;
            }
            else
            {   // 截取通讯帧长度
                rx_length = data_length + 8;
            }
            sum_check = SumCalc(UartRxBuffer, rx_length - 2);
            if((UartRxBuffer[6 + data_length] == sum_check)&&(UartRxBuffer[7 + data_length] == COMM_EOF_B))
            {   // 累加和正确，帧尾正确
                uart_continue_cnt = sys_10ms_cnt;   // 清通讯超时延时，在帧头帧尾和累加和均正确时才进行
                switch(UartRxBuffer[3])
                {
                case COMM_INFO_CMD1:    // 设备信息指令1
                    if(UartRxBuffer[4] == COMM_INFO_CMD2)
                    {
                        TxSysInfoProc();
                    }
                    else if(UartRxBuffer[4] == COMM_SN_CMD2)
                    {   // 设备序列号
                        TxSysSNProc();
                    }
                    break;

                case COMM_VOL_CMD1:     // 电压信息指令1
                    if(UartRxBuffer[4] == COMM_VOL_CMD2)
                    {
                        TxCellsVolProc();
                    }
                    break;

                case COMM_STATE_CMD1:   // 电流、温度、电量、工作状态信息指令1
                    if(UartRxBuffer[4] == COMM_CUR_TEMP_CMD2)
                    {   // 电流 温度
                        TxCurTempProc();
                    }
                    else if(UartRxBuffer[4] == COMM_SOC_ERR_CMD2)
                    {   // 电量 工作状态
                        TxStateProc();
                    }
                    break;

                case COMM_LED_OUT_CMD1:     // 停止供电、LED控制指令1
                    if(UartRxBuffer[4] == COMM_DIS_OUT_CMD2)
                    {   // 停止供电与轻载保护延时修改
                        switch(UartRxBuffer[6])
                        {
                        case 0:     // 停止供电
                            if(g_sys_state.val.charger_on_flag == FALSE)
                            {
                                comm_led_state = led_comm_normol;
                                g_sys_state.val.dch_on_flag = FALSE;
                            }
                            else
                            {
                                err_flag = 1;
                            }
                            break;
                        case 0x10:  // 轻载保护时间为1.5h
                            g_sys_state.val.unload_long_flag = TRUE;
                            break;
                        case 0x11:  // 轻载保护时间为5S
                            g_sys_state.val.unload_long_flag = FALSE;
                            break;
                        default:
                            err_flag = 1;
                            break;
                        }
                        if(err_flag == 0)
                        {
                            TxSysLedProc();
                        }
                    }
                    else if(UartRxBuffer[4] == COMM_LED_CTRL_CMD2)
                    {   // LED控制
                        switch(UartRxBuffer[6])
                        {
                        case 0xAA:
                            comm_led_state = led_comm_normol;
                            break;
                        case 0:
                            comm_led_state = led_comm_off;
                            break;
                        case 1:
                            comm_led_state = led_comm_alarm1;
                            break;
                        case 2:
                            comm_led_state = led_comm_alarm2;
                            break;
                        default:
                            err_flag = 1;
                            break;
                        }
                        if(err_flag == 0)
                        {
                            TxSysLedProc();
                        }
                    }
                    break;

                default:
                    break;
                }
            }
        }
        memset(UartRxBuffer, 0, sizeof(UartRxBuffer));
        return;
    }
}

// 指示灯控制指令响应
void TxSysLedProc(void)
{
    //start
    UartTxBuffer[0] = COMM_EOF_B;
    //addr
    UartTxBuffer[1] = COMM_ADDR_2;
    UartTxBuffer[2] = COMM_ADDR_1;
    //cmd
    UartTxBuffer[3] = UartRxBuffer[3];
    UartTxBuffer[4] = UartRxBuffer[4];
    //date
    UartTxBuffer[5] = 1;
    UartTxBuffer[6] = UartRxBuffer[6];

    UartTxBuffer[7] = SumCalc(UartTxBuffer, 7);

    UartTxBuffer[8] = COMM_SOF_B;
    UART_Transmit(9);
}

// 电量和工作状态信息发送
void TxStateProc(void)
{
    uint16_t temp = 0;
    uint16_t show_full_cap = g_sys_cap.val.full_cap + 143;
    //start
    UartTxBuffer[0] = COMM_EOF_B;
    //addr
    UartTxBuffer[1] = COMM_ADDR_2;
    UartTxBuffer[2] = COMM_ADDR_1;
    //cmd
    UartTxBuffer[3] = COMM_STATE_CMD1;
    UartTxBuffer[4] = COMM_SOC_ERR_CMD2;
    //date
    UartTxBuffer[5] = 8;
    UartTxBuffer[6] = g_sys_cap.val.show_cap_rate;
    temp = (uint32_t)g_sys_cap.val.show_cap_rate * show_full_cap / 100;
    UartTxBuffer[7] = temp >> 8;
    UartTxBuffer[8] = temp;
    if(g_sys_state.val.charger_on_flag == TRUE)
    {   // 充电状态
        if(
            (g_sys_flags.val.chg_temp_low_protect_flag == TRUE)   // 充电低温保护
          ||(g_sys_flags.val.chg_temp_high_protect_flag == TRUE)  // 充电高温保护
        )
        {
            UartTxBuffer[9] = 0x01;
        }
        else if(g_sys_flags.val.chg_oc_protect_flag == TRUE)        // 充电过流
        {
            UartTxBuffer[9] = 0x02;
        }
        else if(g_sys_state.val.chg_full_ledoff_flag == TRUE)       // 充饱之后常亮5min灭灯之后的状态
        {
            UartTxBuffer[9] = 0xF0;
        }
        else if(g_sys_state.val.chg_5h_err_flag == TRUE)        // 充电时间超过5H
        {
            UartTxBuffer[9] = 0xF1;
        }
        else
        {
            UartTxBuffer[9] = 0x0A;     // 正常充电状态
        }
    }
    else
    {
        UartTxBuffer[9] = 0x1A;     // 正常放电状态
    }

    UartTxBuffer[10] = show_full_cap >> 8;
    UartTxBuffer[11] = show_full_cap;
    UartTxBuffer[12] = g_sys_cap.val.bat_cycle_cnt >> 8;
    UartTxBuffer[13] = g_sys_cap.val.bat_cycle_cnt;

    UartTxBuffer[14] = SumCalc(UartTxBuffer, 14);

    UartTxBuffer[15] = COMM_SOF_B;
    UART_Transmit(16);
}

// 设备序列号发送
void TxSysSNProc(void)
{
    uint8_t Serial_No[10] = {0};
    uint8_t i;
    for(i=0;i<10;i++)
    {
        Serial_No[i] = Flash_Read_Byte((char *)SysSNStart, i);
    }
    //start
    UartTxBuffer[0] = COMM_EOF_B;
    //addr
    UartTxBuffer[1] = COMM_ADDR_2;
    UartTxBuffer[2] = COMM_ADDR_1;
    //cmd
    UartTxBuffer[3] = COMM_INFO_CMD1;
    UartTxBuffer[4] = COMM_SN_CMD2;
    //date
    UartTxBuffer[5] = 12;
    UartTxBuffer[6] = '2';
    UartTxBuffer[7] = '0';
    for(i=0;i<10;i++)   // 设备序列号
    {
        UartTxBuffer[8+i]  = Serial_No[i];
    }

    UartTxBuffer[18] = SumCalc(UartTxBuffer, 18);

    UartTxBuffer[19] = COMM_SOF_B;
    UART_Transmit(20);
}

// 设备信息发送
void TxSysInfoProc(void)
{
    //start
    UartTxBuffer[0] = COMM_EOF_B;
    //addr
    UartTxBuffer[1] = COMM_ADDR_2;
    UartTxBuffer[2] = COMM_ADDR_1;
    //cmd
    UartTxBuffer[3] = COMM_INFO_CMD1;
    UartTxBuffer[4] = COMM_INFO_CMD2;
    //date
    UartTxBuffer[5]  = 6;
    UartTxBuffer[6]  = 0x53; // S
    UartTxBuffer[7]  = 0x43; // C
    UartTxBuffer[8]  = 0x55; // U
    UartTxBuffer[9]  = 0x44; // D
    UartTxBuffer[10]  = (version >> 8) + 0x30; // 版本号高字节
    UartTxBuffer[11] = version + 0x30; // 版本号低字节

    UartTxBuffer[12] = SumCalc(UartTxBuffer, 12);

    UartTxBuffer[13] = COMM_SOF_B;
    UART_Transmit(14);
}

// 电流、温度信息发送
void TxCurTempProc(void)
{
    uint16_t temp = 0;
    //start
    UartTxBuffer[0] = COMM_EOF_B;
    //addr
    UartTxBuffer[1] = COMM_ADDR_2;
    UartTxBuffer[2] = COMM_ADDR_1;
    //cmd
    UartTxBuffer[3] = COMM_STATE_CMD1;
    UartTxBuffer[4] = COMM_CUR_TEMP_CMD2;
    //date
    UartTxBuffer[5] = 4;
    temp = g_sys_signal.val.cur;
    UartTxBuffer[6] = temp >> 8;
    UartTxBuffer[7] = temp;
    temp = g_sys_signal.val.temperature + 2730;
    UartTxBuffer[8] = temp >> 8;
    UartTxBuffer[9] = temp;

    UartTxBuffer[10] = SumCalc(UartTxBuffer, 10);

    UartTxBuffer[11] = COMM_SOF_B;
    UART_Transmit(12);
}

// 电压信息发送
void TxCellsVolProc(void)
{
    uint8_t i = 0;
    //start
    UartTxBuffer[0] = COMM_EOF_B;
    //addr
    UartTxBuffer[1] = COMM_ADDR_2;
    UartTxBuffer[2] = COMM_ADDR_1;
    //cmd
    UartTxBuffer[3] = COMM_VOL_CMD1;
    UartTxBuffer[4] = COMM_VOL_CMD2;
    //date
    UartTxBuffer[5] = 16;
    UartTxBuffer[6] = 0x00;
    UartTxBuffer[7] = 0x00;
    UartTxBuffer[8] = g_sys_signal.val.vbat >> 8;
    UartTxBuffer[9] = g_sys_signal.val.vbat;
    for(i = 0; i < CELLS_NUM; i ++)
    {
        UartTxBuffer[10 + i*2] = g_sys_signal.val.vcell[i] >> 8;
        UartTxBuffer[11 + i*2] = g_sys_signal.val.vcell[i];
    }

    UartTxBuffer[22] = SumCalc(UartTxBuffer, 22);

    UartTxBuffer[23] = COMM_SOF_B;
    UART_Transmit(24);
}

/****************************************************************************
FUNCTION        : SumCalc
DESCRIPTION     : 通讯帧累加和计算
INPUT           : 数组  长度
OUTPUT          : 累加和
NOTICE          :
DATE            : 2018/09/18
 *****************************************************************************/
uint8_t SumCalc(uint8_t *data_arr, uint8_t data_len)
{
    uint8_t sum_result = 0;
    uint8_t i;
    for (i = 1; i < data_len; i ++)
    {
        sum_result += data_arr[i];
    }
    return sum_result;
}



