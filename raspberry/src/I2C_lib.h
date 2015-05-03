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

#ifndef __I2C_LIB_H
#define __I2C_LIB_H
/*************************************************************************
 includes section
 ***************************************************************************/
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <linux/i2c-dev.h>
#include <errno.h>
#include <sys/ioctl.h>
#include <string.h>
#include "debug_enable.h"

/***************************************************************************
 PROTOTYPES
 ****************************************************************************/
int I2C_setup();
void I2C_close();
void I2C_writeCmd(unsigned char,unsigned char,unsigned char);
char I2C_readCmd(unsigned char,unsigned char);

#endif
