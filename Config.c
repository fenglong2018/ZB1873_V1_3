/*
 * Config.c
 *
 *  Created on: 2014-10-31
 *      Author: Administrator
 */
#include "Config.H"


void InitIO(void)
{
    P1DIR &= ~(BIT0 | BIT3 | BIT4 | BIT5);     // CHG_AD、VBAT_AD配置为输入
    P2DIR &= ~(BIT0 | BIT2);  // ALERT、TRIG检测配置为输入
    P3DIR |=  0xFF;

    P1REN = 0x00;
    P1OUT = 0x00;  //
    P3OUT = 0x00;  //

    EN_VBAT_CHK();

    P2SEL &= ~BIT2;
    P2IES |= BIT2;      // 中断  P2.2下降沿触发
    P2IFG &= ~BIT2;     //清除IO口中断标志
//    P2IE  |= BIT2;      // 中断使能
}

void WDT_RST_1S(void)
{
    WDTCTL = WDT_ARST_1000;
}

void STOP_WDT(void)
{
    WDTCTL = WDTPW | WDTHOLD;	// Stop watchdog timer
}

