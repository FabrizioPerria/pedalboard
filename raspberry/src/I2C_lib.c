#include "I2C_lib.h"
#include <pthread.h>

/*****************************************************************************
  Static variables
 ******************************************************************************/
static int i2c_fd = -1;                         //file descriptor of the i2c device file 
static unsigned char currentSlave =0x0;         //slave currently addressed
static pthread_mutex_t i2c_lock;				//lock for the I2C bus

/******************************************************************************
  Opening device file of the i2c linux device driver and initializing the mutex
 *******************************************************************************/
int I2C_setup()
{
	if(i2c_fd >= 0){
#ifdef DEBUG
		printf("File /dev/i2c-1 already opened\n");
#endif
		return 0;					//the file is already opened so we skip the initialization (it's not an error!!)
	}

	i2c_fd  = open("/dev/i2c-1", O_RDWR);

	if (i2c_fd  < 0) {
#ifdef DEBUG
		printf("File /dev/i2c-1 not existing\n");
#endif
		return -1;
	}

	if (pthread_mutex_init(&i2c_lock,NULL) != 0){
#ifdef DEBUG
		printf("Impossible to initialize the i2c mutex\n");
#endif
		close(i2c_fd);
		i2c_fd=-1;
		return -1;
	}

	return 0;
}

/********************************************************************
  Release the i2c and mutex resources before closing
 *******************************************************************/
void I2C_close()
{
	pthread_mutex_destroy(&i2c_lock);
	close(i2c_fd);
	i2c_fd=-1;
}

/************************************************************************
  Send a command through i2c bus
 *************************************************************************/
void I2C_writeCmd(unsigned char slaveAddr, unsigned char reg,unsigned char cmd)
{
	if(pthread_mutex_trylock(&i2c_lock) == 0){
		if(currentSlave!=slaveAddr){
			if (ioctl(i2c_fd, I2C_SLAVE_FORCE, slaveAddr) < 0) {
				pthread_mutex_unlock(&i2c_lock);
				return;
			} else {
				currentSlave = slaveAddr;
			}
		}

		unsigned char buf[2];

		buf[0]=reg;
		buf[1]=cmd;
		write(i2c_fd,buf,2);
		pthread_mutex_unlock(&i2c_lock);
	}
	//usleep(100);
}

/************************************************************************
  Request data through i2c bus
 *************************************************************************/
char I2C_readCmd(unsigned char slaveAddr,unsigned char reg)
{
	if(pthread_mutex_trylock(&i2c_lock) == 0){
		if(currentSlave!=slaveAddr){
			if (ioctl(i2c_fd, I2C_SLAVE, slaveAddr) < 0) {
				pthread_mutex_unlock(&i2c_lock);
				return 0xFF;						//return -1
			} else {
				currentSlave = slaveAddr;
			}
		}
		unsigned char a[1];
		a[0]=reg;
		write(i2c_fd,&a[0],1);
		usleep(500);
		read(i2c_fd,a,1);
		pthread_mutex_unlock(&i2c_lock);
		return a[0];
	}
	return 0x0;
}
