/*
 * I2C.c
 *
 *  Created on: 2017Äê7ÔÂ31ÈÕ
 *      Author: xxy
 */

#include "I2C.h"


int I2CSendBytes(unsigned char I2CSlaveAddress, unsigned char *DataBuffer, unsigned int ByteCount, unsigned int *SentByte)
{
    unsigned long int DelayCounter = 0;
    unsigned int NumberOfBytesSent = 0;
    unsigned char *DataPointer;

    UCB0CTL0 |= UCMST;
    UCB0I2CSA = I2CSlaveAddress;

    DataPointer = DataBuffer;

    UCB0CTL1 |= UCTR; //data in transmit direction
    UCB0CTL1 |= UCTXSTT; //Generate Start Condition
    //Send Start Byte
    while(!(IFG2 & UCB0TXIFG))  //if UCTXSTT != 0, wait here
    {
        DelayCounter ++;
        if (DelayCounter > DELAY_LIMIT)
        {
            break;
        }
    }

    if (DelayCounter >= DELAY_LIMIT)   //check if NACK condition occurred
    {
        *SentByte = NumberOfBytesSent;
        UCB0CTL1 |= UCTXSTP;

        g_sys_state.val.I2C_error_flag = TRUE;
        return -1;
    }

    for(NumberOfBytesSent = 0; NumberOfBytesSent < ByteCount; NumberOfBytesSent++)
    {
        UCB0TXBUF= *DataPointer;
        DelayCounter = 0;

        while((DelayCounter < DELAY_LIMIT) && (!(IFG2 & UCB0TXIFG) || (UCB0CTL1 & UCTXSTT)))	//check if the byte has been sent
        {
            DelayCounter ++;
        }

        if (DelayCounter >= DELAY_LIMIT)	//check if NACK condition occurred
        {
            *SentByte = NumberOfBytesSent;
            UCB0CTL1 |= UCTXSTP;				//send stop condition
            return -1;
        }
        DataPointer ++;
    }

    UCB0CTL1 |= UCTXSTP;		//send stop bit
    DelayCounter = 0;

    while((DelayCounter < DELAY_LIMIT) && (UCB0CTL1 & UCTXSTP))
    {
        DelayCounter ++;
    }

    *SentByte =  NumberOfBytesSent;
    if (DelayCounter >= DELAY_LIMIT)	//check if NACK condition occurred
    {
        UCB0CTL1 |= UCSWRST;
        return -1;
    }
    else
    {
        return 0;
    }
}

int I2CReadBytes(unsigned char I2CSlaveAddress, unsigned char *DataBuffer, unsigned int ExpectedByteNumber, unsigned int *NumberOfReceivedBytes)
{
    unsigned long int DelayCounter = 0;
    unsigned char *DataPointer;
    unsigned int *NumberOfReceivedBytesPointer;

    NumberOfReceivedBytesPointer = NumberOfReceivedBytes;
    *NumberOfReceivedBytesPointer = 0;

    UCB0CTL0 |= UCMST;
    DataPointer = DataBuffer;
    UCB0I2CSA = I2CSlaveAddress;

    UCB0CTL1 &= ~(UCTR); //data in receive direction
    UCB0CTL1 |= UCTXSTT; //Generate Start Condition

    while(UCB0CTL1 & UCTXSTT)  //if UCTXSTT != 0, wait here
    {
        DelayCounter ++;
        if (DelayCounter >= DELAY_LIMIT)
        {
            break;
        }
    }

    if (DelayCounter >= DELAY_LIMIT || UCB0STAT & UCNACKIFG)   //check if NACK condition occurred
    {
        return -1;
    }

    for(*NumberOfReceivedBytesPointer = 0; *NumberOfReceivedBytesPointer < ExpectedByteNumber; (*NumberOfReceivedBytesPointer)++)
    {
        if(*NumberOfReceivedBytesPointer + 1 == ExpectedByteNumber)
        {
            UCB0CTL1 |= UCTXSTP;
        }
        DelayCounter = 0;

        while((DelayCounter < DELAY_LIMIT) && (!(IFG2 & UCB0RXIFG)))
        {
            DelayCounter ++;
        }
        if(DelayCounter >= DELAY_LIMIT)
        {
            UCB0CTL1 |= UCSWRST;   //if I2C overtime condition occurred, reset I2C engine
            return -1;
        }

        *DataPointer = UCB0RXBUF;
        DataPointer ++;
    }

    DelayCounter = 0;
    while((DelayCounter < DELAY_LIMIT) && (UCB0CTL1 & UCTXSTP))
    {
        DelayCounter ++;
    }

    if(DelayCounter >= DELAY_LIMIT)
    {
        UCB0CTL1 |= UCSWRST;
        return -1;
    }
    return 0;
}

int I2CSendByte(unsigned char I2CSlaveAddress, unsigned char data)
{
    unsigned long int DelayCounter = 0;

    UCB0CTL0 |= UCMST;
    UCB0I2CSA = I2CSlaveAddress;

    UCB0CTL1 |= UCTR; //data in transmit direction
    UCB0CTL1 |= UCTXSTT; //Generate Start Condition
    //Send Start Byte
    while(!(IFG2 & UCB0TXIFG))  //if UCB0TXIFG != 0, wait here
    {
        DelayCounter ++;
        if (DelayCounter >= DELAY_LIMIT)
        {
            break;
        }
    }

    if (DelayCounter >= DELAY_LIMIT)
    {
        return -1;
    }

    UCB0TXBUF = data;				// send the data
    DelayCounter = 0;

    while((DelayCounter < DELAY_LIMIT) && (!(IFG2 & UCB0TXIFG)))
    {
        DelayCounter ++;
    }

    if (DelayCounter >= DELAY_LIMIT)
    {
        return -1;
    }

    UCB0CTL1 |= UCTXSTP;			//send stop bit
    DelayCounter = 0;

    while((DelayCounter < DELAY_LIMIT) && (UCB0CTL1 & UCTXSTP))
    {
        DelayCounter ++;
    }

    if (DelayCounter >= DELAY_LIMIT)	//check if NACK condition occurred
    {
        return -1;
    }
    else
    {
        return 0;
    }
}

void I2CInitialise(void)
{
    P1SEL |= BIT6 + BIT7;                     // Assign I2C pins to USCI_B0, P1.6 for SCL and P1.7 for SDA
    P1SEL2|= BIT6 + BIT7;                     // Assign I2C pins to USCI_B0, P1.6 for SCL and P1.7 for SDA

    ADC10AE0 &= 0x3F;
    CAPD &= 0x3F;

    UCB0CTL1 |= UCSWRST + UCSSEL_2;                      // Enable SW reset, hold USCI logic in reset state
    UCB0CTL0 = UCMODE_3 + UCSYNC;			//set to I2C mode, sync=1
    UCB0BR0 = 128;
    UCB0BR1 = 0;

    UCB0I2CIE = 0;
    IE2 &= ~(UCB0TXIE + UCB0RXIE);	//disable interrupts

    UCB0CTL1 |= UCSSEL_1;
    UCB0CTL1 &= ~UCSWRST;
}

void I2CClear(void)
{
    P1SEL  &= ~(BIT6+BIT7);
    P1SEL2 &= ~(BIT6+BIT7);
    P1DIR  |=  (BIT6+BIT7);
    P1OUT  &= ~(BIT6+BIT7);
    delay_us(10);
    P1OUT  |=  (BIT6+BIT7);
    delay_us(10);
    P1OUT &= ~(BIT6+BIT7);
    delay_us(10);
    P1OUT |=  (BIT6+BIT7);
    delay_us(10);
    P1OUT &= ~(BIT6+BIT7);
    delay_us(10);
    P1OUT |=  (BIT6+BIT7);
    delay_us(10);

    UCB0CTL1=0;
    UCB0CTL0=0;
}

