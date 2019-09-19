
/*
 * uart.c
 *
 *  Created on: 2018年1月24日
 *      Author: blust
 */

#include"uart.h"

uint8_t UartTxBuffer[UART_TX_RX_SIZE];    // 发送数组
uint8_t UartRxBuffer[UART_TX_RX_SIZE];    // 接收数组

uint8_t UartRxCircleBuffer[UART_RX_BUFF_SIZE];    // 接收环形缓冲区
volatile uint8_t uart_rx_head = 0;    // 环形缓冲区头，缓冲区头尾重合有两种情况，一种是接收溢出，头覆盖了尾；一种是缓冲区空，所有数据都被读取处理了
volatile uint8_t uart_rx_tail = 0;    // 环形缓冲区尾
volatile uint8_t uart_tx_num = 0;     // 需发送数据长度
volatile uint8_t uart_tx_cnt = 0;     // 发送数据计数


void Uart_Init(void) //ACLK=32.678K
{
    //Select the IO's function to uart.
    P1DIR  |=  BIT2;             //P1.2  UART_TX
    P1DIR  &= ~BIT1;             //P1.1  UART_RX
    P1SEL  |=  BIT1;        //select P1.1 as UART port
    P1SEL2 |=  BIT1;
    //Set the bus as uart.
    UCA0CTL1 |= UCSWRST;         //reset UART module,as well as enable UART module
    UCA0CTL1 |= UCSSEL1; // CLK = SMCLK  8MHz
    UCA0BR0   = 0x41; // 8MHz/9600 = 833.33   833 = 0x0341
    UCA0BR1   = 0x03;
    UCA0MCTL  = UCBRS1; // UCBRSx = 0.33×8=2.64取整
    UCA0CTL1 &= ~UCSWRST;        //UART reset end
    IE2 |= UCA0RXIE ;  //Open the UART receive interrupt

    DIS_TX_PIN();       // TX引脚置低
}

int8_t uart_Read(void)
{
    uint8_t i = 0;

    if(uart_rx_tail == uart_rx_head)
    {
        return -1;
    }

    do{
        if(i >= UART_TX_RX_SIZE)
        {
            return -1;
        }
        UartRxBuffer[i++] = UartRxCircleBuffer[uart_rx_tail++];
        if (uart_rx_tail >= UART_RX_BUFF_SIZE)
        {
            uart_rx_tail = 0;
        }
    }
    while(uart_rx_tail != uart_rx_head);

    return i;
}

#pragma vector=USCIAB0RX_VECTOR
__interrupt void USCI0RX_ISR(void)   //接收中断
{
    timer_comm_stick = 0;

    UartRxCircleBuffer[uart_rx_head ++] = UCA0RXBUF;
    if (uart_rx_head >= UART_RX_BUFF_SIZE)
    {
        uart_rx_head = 0;
    }
}

void UART_Transmit(uint8_t length)
{
    if((length == 0)||(length > UART_TX_RX_SIZE))
    {
        return;
    }

    IE2 &= ~UCA0RXIE;   // 关闭接收中断
    EN_TX_PIN();        // 发送引脚置高
    P1SEL  |=  BIT2;        //select P1.2 as UART port
    P1SEL2 |=  BIT2;
    IFG2 &= ~UCA0TXIFG; // 清发送中断标志
    IE2 |= UCA0TXIE;    // 打开发送中断
    delay_ms(1);
    uart_tx_cnt = 0;
    uart_tx_num = length;
    UCA0TXBUF = UartTxBuffer[uart_tx_cnt ++];
}

void uart_transmit_isr(void)
{
    if(uart_tx_cnt < uart_tx_num)
    {
        UCA0TXBUF = UartTxBuffer[uart_tx_cnt ++];
    }
    else
    {
        memset(UartTxBuffer, 0, sizeof(UartTxBuffer));
        IFG2 &= ~UCA0TXIFG;  // 清发送中断标志
        IE2  &= ~UCA0TXIE;   // 关闭发送中断
        IFG2 &= ~UCA0RXIFG;  // 清接收中断标志
        IE2  |= UCA0RXIE;    // 开接收中断
    }
}

#pragma vector=USCIAB0TX_VECTOR
__interrupt void USCI0TX_ISR(void)
{
    timer_TX_stick = 0;
    if(UART_TXIF)
    {
        uart_transmit_isr();
    }
}

