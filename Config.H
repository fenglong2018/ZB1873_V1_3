/*
 * Config.H
 *
 *  Created on: 2013-12-18
 *      Author: Administrator
 */

#ifndef CONFIG_H_
#define CONFIG_H_

#include"Global.h"

#define TRIG_IN       (P2IN&BIT2)
//#define CHG_IN        (P2IN&BIT1)

//#define CPU_F ((double)8000000)
#define delay_cycles(x)	__delay_cycles(x)
#define delay_us(x) __delay_cycles((long)x *8)
#define delay_ms(x) __delay_cycles((long)x*8000) //__delay_cycles((long)x * 1000);__delay_cycles((long)x << 10)

#define EN_VBAT_CHK()     {P3OUT |=  BIT1;}
#define DIS_VBAT_CHK()    {P3OUT &= ~BIT1;}

#define EN_VCHG_CHK()     {P3OUT |=  BIT2;}
#define DIS_VCHG_CHK()    {P3OUT &= ~BIT2;}

#define DIS_LED_RED()     {P3OUT &= ~BIT6;}
#define EN_LED_RED()      {P3OUT |=  BIT6;}
#define DEF_LED_RED()     {P3OUT ^=  BIT6;}

void InitIO(void);

void WDT_RST_1S(void);
void STOP_WDT(void);


#endif /* CONFIG_H_ */
