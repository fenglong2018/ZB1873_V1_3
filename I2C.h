/*
 * I2C.h
 *
 *  Created on: 2017Äê7ÔÂ31ÈÕ
 *      Author: xxy
 */

#ifndef I2C_H_
#define I2C_H_

#include "Config.H"
#define DELAY_LIMIT  2000

int I2CReadBytes(unsigned char I2CSlaveAddress, unsigned char *DataBuffer, unsigned int ExpectedByteNumber, unsigned int *NumberOfReceivedBytes);
int I2CSendBytes(unsigned char I2CSlaveAddress, unsigned char *DataBuffer, unsigned int ByteCount, unsigned int *SentByte);

void I2CInitialise(void);
void I2CClear(void);

#endif /* I2C_H_ */
