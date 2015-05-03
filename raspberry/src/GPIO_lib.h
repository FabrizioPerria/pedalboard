/*
This file is part of Jack_pedalboard.

    Jack_pedalboard is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    Jack_pedalboard is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with Jack_pedalboard.  If not, see <http://www.gnu.org/licenses/>.

*/


#ifndef __GPIO_LIB_H
#define __GPIO_LIB_H

#include "peripheral.h"
#include "debug_enable.h"

/**********************************************************************
 Memory addresses of the GPIO registers
 *************************************************************************/
#define GPIO_BASE                               BCM2835_BASE + 0x200000
#define GPIO_PIN_LEVEL                          *(gpio.addr + 13)
#define GPIO_PIN_DETECT_EVENT_STATUS            *(gpio.addr + 16)
#define GPIO_RISING_EDGE_ENABLE                 *(gpio.addr + 19)
#define GPIO_FALLING_EDGE_ENABLE                *(gpio.addr + 22)
#define GPIO_HIGH_DETECT_ENABLE                 *(gpio.addr + 25)
#define GPIO_LOW_DETECT ENABLE                  *(gpio.addr + 28)
#define GPIO_RISING_DETECT                      *(gpio.addr + 31)
#define GPIO_FALLING_DETECT                     *(gpio.addr + 34)

#define GPIO_BIT_SET(n)                         1 << n


extern peripheral gpio;

/************************************************************************
 PROTOTYPES
 ************************************************************************/
int GPIO_map();
void GPIO_unmap();

void GPIO_setInput(int pin);
void GPIO_setOutput(int pin);

void GPIO_high(int pin);
void GPIO_low(int pin);

#endif
