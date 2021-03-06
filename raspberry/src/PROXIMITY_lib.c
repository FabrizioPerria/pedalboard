#include "PROXIMITY_lib.h"
/*****************************************************************************************************
  Setting up the proximity measurements feature of VCNL4000 with the values suggested by the datasheet
 ******************************************************************************************************/
void setupProximity()
{
	if(I2C_setup() < 0){
		return;
	}

	VCNL4000_writeCmd(PROXIMITY_LED_CURRENT,0x14);
	VCNL4000_writeCmd(PROXIMITY_FREQUENCY,0x2);
	VCNL4000_writeCmd(PROXIMITY_TIME_ADJUST,0x81);
}

/*****************************************************************************************************
  Setting up the ambient light measurements feature of VCNL4000 with the values suggested by the datasheet
 ******************************************************************************************************/
void setupAmbientLight()
{
	if(I2C_setup() < 0){
		return;
	}

	VCNL4000_writeCmd(PROXIMITY_AMBIENT_PARAM, PROXIMITY_AMBIENT_PARAM_CONT_CONVERSION|PROXIMITY_AMBIENT_PARAM_COMPENSATION);
}

/************************************************************************************************************
  Testing the presence of the VCNL4000 in the IIC bus
 ********************************************************************************************************/
int testVCNL4000()
{
	char c = VCNL4000_readCmd(PROXIMITY_VERSION);
	if(c==0x11)
		return 0;
	else
		return -1;
}

/************************************************************************************************
  Wrapper to send a command through the I2C bus
 *************************************************************************************************/
void VCNL4000_writeCmd(unsigned char reg,unsigned char cmd)
{
	I2C_writeCmd(PROXIMITY_ADDRESS,reg,cmd);
}

/*********************************************************************************
  Wrapper to read data from the I2C device
 *********************************************************************************/
char VCNL4000_readCmd(unsigned char reg)
{
	return I2C_readCmd(PROXIMITY_ADDRESS,reg);
}


int getProximityMeasure()
{
	char c = 0;

	VCNL4000_writeCmd(PROXIMITY_CMD,PROXIMITY_CMD_REQUEST);
	usleep(500);

	c=VCNL4000_readCmd(PROXIMITY_DATA_MSB);

	int result= (int)c << 8;

	c=VCNL4000_readCmd(PROXIMITY_DATA_LSB);

	result |= (int)c;

	return result;
}

int getAmbientLightMeasure()
{
	char c = 0;

	VCNL4000_writeCmd(PROXIMITY_CMD,PROXIMITY_CMD_AMBIENT_LIGHT_REQUEST);

	usleep(500);
	c=VCNL4000_readCmd(PROXIMITY_AMBIENT_DATA_MSB);

	int result= (int)c << 8;

	c=VCNL4000_readCmd(PROXIMITY_AMBIENT_DATA_LSB);

	result |= (int)c;
	return result;
}
