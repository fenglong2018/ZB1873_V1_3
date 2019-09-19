/*
 * uart.h
 *
 *  Created on: 2018��1��24��
 *      Author: blust
 */

#ifndef UART_H_
#define UART_H_

#include <msp430.h>
#include "stdint.h"
#include <stdio.h>
#include <string.h>
#include "Config.h"


#define UART_RX_BUFF_SIZE   64  // ���ջ��λ���������

#define UART_TX_RX_SIZE   30  // ���ͽ������鳤��
extern uint8_t UartTxBuffer[UART_TX_RX_SIZE];     // ��������
extern uint8_t UartRxBuffer[UART_TX_RX_SIZE];     // ��������

extern volatile uint8_t uart_rx_head;    // ���λ�����ͷ
extern volatile uint8_t uart_rx_tail;    // ���λ�����β

#define UART_TXIF  (IFG2&UCA0TXIFG)
#define UART_RXIF  (IFG2&UCA0RXIFG)

#define EN_TX_PIN()     {P1OUT |=  BIT2;}
#define DIS_TX_PIN()    {P1OUT &= ~BIT2;}

extern void Uart_Init(void);
extern int8_t uart_Read(void);
extern void UART_Transmit(uint8_t length);


#endif /* UART_H_ */
