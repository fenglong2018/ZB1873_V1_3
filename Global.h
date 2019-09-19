/*
 * Global.h
 *
 *  Created on: 2017Äê1ÔÂ23ÈÕ
 *      Author: chenjiawei
 */

#ifndef GLOBAL_H_
#define GLOBAL_H_

#include <msp430.h>
#include <stdbool.h>
#include <math.h>
#include <float.h>
#include <stdlib.h>
#include <stdint.h>
#include "uart.h"
#include "Config.h"
#include "I2C.h"
#include "BQ76920.H"
#include "Protect.H"
#include "CapProc.h"
#include "timer.h"
#include "SampProc.h"
#include "frame.h"
#include "LED.h"
#include "Flash.h"
#include "ADC.h"

typedef enum _BOOL { FALSE = 0, TRUE } BOOL;

extern const uint16_t version;

#endif /* GLOBAL_H_ */
