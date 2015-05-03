// The proximity sensor used for the projects is the VCNL4000
/*
PROTOCOL

Write configuration on a register

APP     |START|Device Address|Write bit|   |Address of the register|   |Data configuration|   |Stop bit|
VCNL                                   |ACK|                       |ACK|                  |ACK|

Read data from a register

APP     |START|Device Address|Write bit|   |Address of the register|   |STOP bit|START|Device Address|Read bit|        |ACK|STOP bit|
VCNL                                   |ACK|                       |ACK|                                      |ACK|DATA|
*/

#ifndef __PROXIMITY_LIB_H
#define __PROXIMITY_LIB_H

#include "I2C_lib.h"
#define PROXIMITY_ADDRESS   0x13
#define PROXIMITY_REGISTERS 0x80
    #define PROXIMITY_CMD                               PROXIMITY_REGISTERS + 0x0
        #define PROXIMITY_CMD_AMBIENT_LIGHT_DATA_READY  1 << 6      //(read only)
        #define PROXIMITY_CMD_DATA_READY                1 << 5      //(read only)
        #define PROXIMITY_CMD_AMBIENT_LIGHT_REQUEST     1 << 4
        #define PROXIMITY_CMD_REQUEST                   1 << 3
    #define PROXIMITY_VERSION                           PROXIMITY_REGISTERS + 0x1
    #define PROXIMITY_LED_CURRENT                       PROXIMITY_REGISTERS + 0x3
    #define PROXIMITY_AMBIENT_PARAM                     PROXIMITY_REGISTERS + 0x4
        #define PROXIMITY_AMBIENT_PARAM_CONT_CONVERSION 1 << 7
        #define PROXIMITY_AMBIENT_PARAM_COMPENSATION    1 << 3
    #define PROXIMITY_AMBIENT_DATA_MSB                  PROXIMITY_REGISTERS + 0x5
    #define PROXIMITY_AMBIENT_DATA_LSB                  PROXIMITY_REGISTERS + 0x6
    #define PROXIMITY_DATA_MSB                          PROXIMITY_REGISTERS + 0x7
    #define PROXIMITY_DATA_LSB                          PROXIMITY_REGISTERS + 0x8
    #define PROXIMITY_FREQUENCY                         PROXIMITY_REGISTERS + 0x9
    #define PROXIMITY_TIME_ADJUST                       PROXIMITY_REGISTERS + 0xa


    void VCNL4000_writeCmd(unsigned char,unsigned char);
    char VCNL4000_readCmd(unsigned char);
    void setupProximity();
	void setupAmbientLight();
    int testVCNL4000();
    int getProximityMeasure();
    int getAmbientLightMeasure();

#endif
