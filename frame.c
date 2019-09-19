/*
 * frame.c
 *
 *  Created on: 2018��1��24��
 *      Author: zhao
 */

#include"frame.h"

volatile uint8_t uart_continue_cnt = 0;

/****************************************************************************
FUNCTION        : UartDataPro
DESCRIPTION     : UART���մ����뷴��
INPUT           : None
OUTPUT          : None
NOTICE          : �жϻ���,MAIN�ж�
DATE            : 2018/09/18
 *****************************************************************************/
void UartDataPro(void)
{
    int8_t  rx_length = 0;
    uint8_t data_length = 0;
    uint8_t sum_check = 0;
    uint8_t err_flag = 0;   // ͨѶ֡���ݳ���Χ����Ҫ��Ӧʱ��1

    if((uint8_t)(sys_10ms_cnt - uart_continue_cnt) >= 100)
    {   // ����1Sû����ȷͨѶ����ֹ�ŵ�
        uart_continue_cnt = sys_10ms_cnt;
        g_sys_state.val.dch_on_flag = FALSE;      // �����ã�ȥ��1S��ͨѶ������߼� blust 20180927
        comm_led_state = led_comm_normol;
    }

    if(g_sys_state.val.receive_ok_flag == TRUE)
    {
        g_sys_state.val.receive_ok_flag = FALSE;
        rx_length = uart_Read();
        if((rx_length <= 0)||(rx_length > UART_TX_RX_SIZE))
        {   // ���ݳ��ȴ���ֱ����������飬�����Ӻ���
            memset(UartRxBuffer, 0, sizeof(UartRxBuffer));
            return;
        }

        if((UartRxBuffer[0] == COMM_SOF_B)&&(UartRxBuffer[1] == COMM_ADDR_1)&&(UartRxBuffer[2] == COMM_ADDR_2))
        {   // ֡ͷ���ַ��ȷ
            data_length = UartRxBuffer[5];
            if(rx_length < (data_length + 8))
            {   // ���ݳ��Ȳ��㣬ֱ����������飬�����Ӻ���
                memset(UartRxBuffer, 0, sizeof(UartRxBuffer));
                return;
            }
            else
            {   // ��ȡͨѶ֡����
                rx_length = data_length + 8;
            }
            sum_check = SumCalc(UartRxBuffer, rx_length - 2);
            if((UartRxBuffer[6 + data_length] == sum_check)&&(UartRxBuffer[7 + data_length] == COMM_EOF_B))
            {   // �ۼӺ���ȷ��֡β��ȷ
                uart_continue_cnt = sys_10ms_cnt;   // ��ͨѶ��ʱ��ʱ����֡ͷ֡β���ۼӺ;���ȷʱ�Ž���
                switch(UartRxBuffer[3])
                {
                case COMM_INFO_CMD1:    // �豸��Ϣָ��1
                    if(UartRxBuffer[4] == COMM_INFO_CMD2)
                    {
                        TxSysInfoProc();
                    }
                    else if(UartRxBuffer[4] == COMM_SN_CMD2)
                    {   // �豸���к�
                        TxSysSNProc();
                    }
                    break;

                case COMM_VOL_CMD1:     // ��ѹ��Ϣָ��1
                    if(UartRxBuffer[4] == COMM_VOL_CMD2)
                    {
                        TxCellsVolProc();
                    }
                    break;

                case COMM_STATE_CMD1:   // �������¶ȡ�����������״̬��Ϣָ��1
                    if(UartRxBuffer[4] == COMM_CUR_TEMP_CMD2)
                    {   // ���� �¶�
                        TxCurTempProc();
                    }
                    else if(UartRxBuffer[4] == COMM_SOC_ERR_CMD2)
                    {   // ���� ����״̬
                        TxStateProc();
                    }
                    break;

                case COMM_LED_OUT_CMD1:     // ֹͣ���硢LED����ָ��1
                    if(UartRxBuffer[4] == COMM_DIS_OUT_CMD2)
                    {   // ֹͣ���������ر�����ʱ�޸�
                        switch(UartRxBuffer[6])
                        {
                        case 0:     // ֹͣ����
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
                        case 0x10:  // ���ر���ʱ��Ϊ1.5h
                            g_sys_state.val.unload_long_flag = TRUE;
                            break;
                        case 0x11:  // ���ر���ʱ��Ϊ5S
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
                    {   // LED����
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

// ָʾ�ƿ���ָ����Ӧ
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

// �����͹���״̬��Ϣ����
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
    {   // ���״̬
        if(
            (g_sys_flags.val.chg_temp_low_protect_flag == TRUE)   // �����±���
          ||(g_sys_flags.val.chg_temp_high_protect_flag == TRUE)  // �����±���
        )
        {
            UartTxBuffer[9] = 0x01;
        }
        else if(g_sys_flags.val.chg_oc_protect_flag == TRUE)        // ������
        {
            UartTxBuffer[9] = 0x02;
        }
        else if(g_sys_state.val.chg_full_ledoff_flag == TRUE)       // �䱥֮����5min���֮���״̬
        {
            UartTxBuffer[9] = 0xF0;
        }
        else if(g_sys_state.val.chg_5h_err_flag == TRUE)        // ���ʱ�䳬��5H
        {
            UartTxBuffer[9] = 0xF1;
        }
        else
        {
            UartTxBuffer[9] = 0x0A;     // �������״̬
        }
    }
    else
    {
        UartTxBuffer[9] = 0x1A;     // �����ŵ�״̬
    }

    UartTxBuffer[10] = show_full_cap >> 8;
    UartTxBuffer[11] = show_full_cap;
    UartTxBuffer[12] = g_sys_cap.val.bat_cycle_cnt >> 8;
    UartTxBuffer[13] = g_sys_cap.val.bat_cycle_cnt;

    UartTxBuffer[14] = SumCalc(UartTxBuffer, 14);

    UartTxBuffer[15] = COMM_SOF_B;
    UART_Transmit(16);
}

// �豸���кŷ���
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
    for(i=0;i<10;i++)   // �豸���к�
    {
        UartTxBuffer[8+i]  = Serial_No[i];
    }

    UartTxBuffer[18] = SumCalc(UartTxBuffer, 18);

    UartTxBuffer[19] = COMM_SOF_B;
    UART_Transmit(20);
}

// �豸��Ϣ����
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
    UartTxBuffer[10]  = (version >> 8) + 0x30; // �汾�Ÿ��ֽ�
    UartTxBuffer[11] = version + 0x30; // �汾�ŵ��ֽ�

    UartTxBuffer[12] = SumCalc(UartTxBuffer, 12);

    UartTxBuffer[13] = COMM_SOF_B;
    UART_Transmit(14);
}

// �������¶���Ϣ����
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

// ��ѹ��Ϣ����
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
DESCRIPTION     : ͨѶ֡�ۼӺͼ���
INPUT           : ����  ����
OUTPUT          : �ۼӺ�
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



