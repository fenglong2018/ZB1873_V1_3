/*
 * timer.c
 *
 *  Created on: Aug 25, 2015
 *      Author: a0220433
 */
#include "timer.H"

volatile uint16_t sys_1s_cnt = 0;
volatile uint16_t sys_250ms_cnt = 0;
volatile uint16_t sys_1min_cnt = 0;
volatile uint16_t sys_10ms_cnt = 0;

volatile uint8_t timer0_250ms_stick;
volatile uint8_t timer0_1s_stick;
volatile uint8_t timer0_1min_stick;

volatile uint8_t timer_comm_stick = 0;
volatile uint8_t timer_TX_stick = 0;

//定时器初始化
void timer0_A0_init(void)
{
    P3DIR  |=  BIT5;  // PWM 输出配置 P3.5
    P3SEL  |=  BIT5;
    P3SEL2 &= ~BIT5;
    P3OUT  |=  BIT5;

    TA0CCTL0 = CCIE;                             // CCR0 interrupt enabled
    TA0CCR0 = 500 - 1;	//设置进入定时器中断的时间							// 0.25ms
    TA0CTL = TASSEL_2 + MC_1  + ID_2;                  // SMCLK divided by 4, continuous,
}

void timer1_A0_init(void)
{
    TA1CCTL0 = CCIE;                             // CCR0 interrupt enabled
    //CCR0 interrupt enabled用到CCR0定时中断的时侯必须记得让CCR0中断使能
    TA1CCR0 = 10000 - 1; //设置进入定时器中断的时间                          // 10ms
    TA1CTL = TASSEL_2 + MC_1  + ID_3;                  // SMCLK divided by 8, continuous,
}

#pragma vector=TIMER0_A0_VECTOR
__interrupt void Timer0_A0 (void)
{
    if(timer_comm_stick >= 20)
    {   // 超过5ms没有接收到新数据，认为接收到完整帧，进行通讯数据处理与恢复
        timer_comm_stick = 0;
        if(uart_rx_tail != uart_rx_head)
        {   // 缓冲区内有新数据，置接收完成标志位
            g_sys_state.val.receive_ok_flag = TRUE;
        }
    }
    else
    {
        timer_comm_stick ++;
    }

    if(timer_TX_stick >= 6)
    {   // 超过1.5ms没有进发送中断，认为数据发送完成，关闭发送中断，拉低TX引脚，释放总线
        if(timer_TX_stick < 10)
        {
            timer_TX_stick = 20;
            P1SEL  &= ~BIT2;        //select P1.2 as I/O port
            P1SEL2 &= ~BIT2;
            DIS_TX_PIN();        // TX引脚置低
        }
    }
    else
    {
        timer_TX_stick ++;
    }
}

#pragma vector=TIMER1_A0_VECTOR
__interrupt void Timer1_A0 (void)
{
    sys_10ms_cnt ++;
    timer0_250ms_stick ++;
    if(timer0_250ms_stick >= 25)
    {
        timer0_250ms_stick = 0;

        sys_250ms_cnt ++;
        timer0_1s_stick ++;
        timer0_1min_stick ++;

        if(timer0_1min_stick >= 240)  // 1min
        {
            timer0_1min_stick = 0;
            sys_1min_cnt ++;
        }

        if(timer0_1s_stick >= 4)//1S
        {
            timer0_1s_stick = 0;
            sys_1s_cnt ++;
        }
    }
}

//PWM配置 LED_BLUE
void pwm_4k_start(uint16_t temp)
{
    if(temp == 0)
    {
        TACCTL1 = OUTMOD_5;     // PWM输出，复位
    }
    else if(temp < 500)
    {
        TACCR1 = temp;
        TACCTL1 = OUTMOD_7;     // PWM输出，复位/置位 模式
    }
    else
    {
        TACCTL1 = OUTMOD_1;     // PWM输出，置位
    }
}


