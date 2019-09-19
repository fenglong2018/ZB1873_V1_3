/*
 * frame.h
 *
 *  Created on: 2018��1��24��
 *      Author: zhao
 */

#ifndef FRAME_H_
#define FRAME_H_

#include"Global.h"


#define COMM_SOF_B  0xF1
#define COMM_EOF_B  0xF2
#define COMM_ADDR_1 0xE0
#define COMM_ADDR_2 0x01

#define COMM_INFO_CMD1      0x10    // �豸��Ϣָ��1
#define COMM_INFO_CMD2      0x03    // �豸��Ϣָ��2
#define COMM_SN_CMD2        0x05    // �豸���к�ָ��2
#define COMM_VOL_CMD1       0x20    // ��ѹ��Ϣָ��1
#define COMM_VOL_CMD2       0x04    // ��ѹ��Ϣָ��2
#define COMM_STATE_CMD1     0x50    // �������¶ȡ�����������״̬��Ϣָ��1
#define COMM_CUR_TEMP_CMD2  0x02    // �������¶���Ϣָ��2
#define COMM_SOC_ERR_CMD2   0x04    // ����������״̬��Ϣָ��2
#define COMM_LED_OUT_CMD1   0x90    // ֹͣ���硢LED����ָ��1
#define COMM_DIS_OUT_CMD2   0x01    // ֹͣ����ָ��2
#define COMM_LED_CTRL_CMD2  0x02    // ָʾ�ƿ���ָ��2

extern volatile uint8_t uart_continue_cnt;

void UartDataPro(void);

void TxSysLedProc(void);
void TxStateProc(void);
void TxSysSNProc(void);
void TxSysInfoProc(void);
void TxCurTempProc(void);
void TxCellsVolProc(void);
uint8_t SumCalc(uint8_t *data_arr, uint8_t data_len);

#endif /* FRAME_H_ */

