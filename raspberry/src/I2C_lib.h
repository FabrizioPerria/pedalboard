
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
