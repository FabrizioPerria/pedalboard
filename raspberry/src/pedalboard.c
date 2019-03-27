/******************************************************
  STD INCLUDES
 ******************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <signal.h>
#include <pthread.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <netdb.h>
#include <ifaddrs.h>
#include <memory.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/socket.h>
#include <unistd.h>
#include <limits.h>

/*******************************************************
  CUSTOM INCLUDES
 *******************************************************/

#include "GPIO_lib.h"
#include "PROXIMITY_lib.h"
#include "I2C_lib.h"
#include "DISPLAY_lib.h"
#include "guitar.h"
#include "debug_enable.h"
#include "messages.h"
/*******************************************************
  DEFINES
 *******************************************************/

#define BUFLEN 25
#define PORT 9999
#define MAXPROXIMITY 65535
#define PROXIMITY_DIVIDER 650
#define PROXIMITY_LOW_LEVEL 0
#define PROXIMITY_OFFSET 8
#define MAXUINT 65535

/********************************************************
  TYPE DEFINITIONS
 ********************************************************/

typedef enum {FALSE,TRUE} boolean;
typedef enum {NOT_SET, MESSAGE_ON, MESSAGE_OFF} GPIO_STATUS;

/********************************************************
  GLOBAL VARIABLES
 ********************************************************/

pthread_mutex_t lock;				//mutex used to synchronize sending messages
pthread_mutex_t i2cMutex;			//mutex between vcnl4000 and ssd1306 for i2c bus concurrency
int udpSocket;						//socket ID
boolean closeThread=TRUE;			//flag to trigger the end of the Proximity sensor's thread
struct sockaddr_in simulatorAddr;   //IP address of the simulator(PC) 
pthread_t th;						//ID of the Proximity sensor's thread
boolean isLockInit = FALSE;

/*********************************************************
  FUNCTIONS
 **********************************************************/

GPIO_STATUS checkButton(boolean* GPIO_is_set, int GPIO_num, int GUI_N, char* message_on, char* message_off)
{
	char buf[BUFLEN];
	if((!(*GPIO_is_set)) && ((GPIO_PIN_LEVEL & GPIO_BIT_SET(GPIO_num))!=0)){
		strncpy(buf,message_on,BUFLEN);
		if(pthread_mutex_trylock(&lock) == 0){
			sendto(udpSocket,buf,strlen(buf),0,(struct sockaddr*)&simulatorAddr,sizeof(simulatorAddr));
			pthread_mutex_unlock(&lock);
		}
		if (GUI_N >= 0)
			SSD1306_fillPixels((GUI_N*SSD1306_WIDTH/4),0,((1+GUI_N)*SSD1306_WIDTH/4),SSD1306_MAXROW - 3);
		*GPIO_is_set = TRUE;
		return MESSAGE_ON;
	} else if ((*GPIO_is_set) && ((GPIO_PIN_LEVEL & GPIO_BIT_SET(GPIO_num))==0)){
		strncpy(buf,message_off,BUFLEN);
		if(pthread_mutex_trylock(&lock) == 0){
			sendto(udpSocket,buf,strlen(buf),0,(struct sockaddr *)&simulatorAddr,sizeof(simulatorAddr));
			pthread_mutex_unlock(&lock);
		}
		if (GUI_N >= 0)
			SSD1306_emptyPixels((GUI_N*SSD1306_WIDTH/4),0,((1+GUI_N)*SSD1306_WIDTH/4),SSD1306_MAXROW - 3);
		*GPIO_is_set = FALSE;
		return MESSAGE_OFF;
	}
	return NOT_SET;
}

//Send the measures detected by the Proximity Sensor
void *sendProxMeasures(void *pa)
{
	char buf[BUFLEN];
	int slen = sizeof(simulatorAddr);
	unsigned long tmp = 0;

	SSD1306_fillPixels((3*SSD1306_WIDTH/4),0,SSD1306_WIDTH,SSD1306_MAXROW - 3);
	SSD1306_updateScreen();
	while(!closeThread){
		if(pthread_mutex_trylock(&i2cMutex) == 0){
			tmp= getProximityMeasure();
			pthread_mutex_unlock(&i2cMutex);
#ifdef DEBUG
			printf("PROXIMITY: %d\n", tmp);
#endif
			sprintf(buf,"%d",tmp);
			usleep(1000);
			if(pthread_mutex_trylock(&lock) == 0){
				sendto(udpSocket,buf,strlen(buf),0,(struct sockaddr *)&simulatorAddr,slen);
				pthread_mutex_unlock(&lock);
			}
		}
	}
}

int checkProximityPedal(boolean* GPIO_is_set, int GPIO_num, int GUI_N)
{
	GPIO_STATUS res = checkButton(GPIO_is_set, GPIO_num, GUI_N, START_EXPR_PEDAL, STOP_EXPR_PEDAL);
	if (res) {
		closeThread ^= 1;

		if (res == MESSAGE_ON)
			pthread_create(&th, NULL, &sendProxMeasures, NULL);

		if (res == MESSAGE_OFF)
			pthread_join(th, NULL);
	}
	return res;
}

//Handle the exit from the program
void ctrlC()
{
	if(closeThread == FALSE){
		closeThread=TRUE;
		pthread_join(th,NULL);
	}
	if(isLockInit == TRUE){
		pthread_mutex_destroy(&lock);
	}
	SSD1306_turnOFF();
	I2C_close();
	if(udpSocket > 0){
		int slen=sizeof(simulatorAddr);
		char buf[7]=ENDING_PEDALBOARD;
		sendto(udpSocket,buf,strlen(buf),0,(struct sockaddr *)&simulatorAddr,slen);
		close(udpSocket);
		int tmp= GPIO_PIN_LEVEL & GPIO_BIT_SET(4);
		GPIO_unmap();
		if(tmp){
			system("reboot");
		}else{
			system("poweroff");
		}
	}
	//The disconnection has been requested by the computer
	exit(0);
}

/**********************************************************************
  MAIN
 **********************************************************************/
int main()
{
	int change;
	struct timeval selectExceed;
	fd_set readSet;
	int slen=sizeof(simulatorAddr);
	struct sockaddr_in si_me;
	struct ifaddrs *ifaddr,*ifa;
	char buf[BUFLEN];
	char ip[BUFLEN];
	char port[4];
	boolean GPIO4_is_set = TRUE;
	boolean GPIO17_is_set = FALSE;
	boolean GPIO27_is_set = FALSE;
	boolean GPIO22_is_set = TRUE;
	boolean GPIO10_is_set = FALSE;
	boolean GPIO9_is_set = FALSE;

	selectExceed.tv_sec=0;
	selectExceed.tv_usec=50;
	signal(SIGINT,ctrlC);

	//display initialization
	SSD1306_setup();
	SSD1306_cleanAll();
	SSD1306_updateScreen();
	usleep(100000);

	//network initialization
	if (getifaddrs(&ifaddr) == -1){
		return -1;
	}
	for (ifa = ifaddr; ifa != NULL; ifa=ifa->ifa_next){
		if (ifa->ifa_addr == NULL) continue;
		getnameinfo(ifa->ifa_addr,sizeof(struct sockaddr_in),ip,BUFLEN,NULL,0,NI_NUMERICHOST);
		if(!strcmp(ifa->ifa_name,"wlan0") && ifa->ifa_addr->sa_family == AF_INET){
			SSD1306_writeMessage(ip,0,6);
			sprintf(port, "%d", PORT);
			SSD1306_writeMessage(port,0,7);
			copyImage(splashScreen,0,0,128,6);
			break;
		}
	}
	SSD1306_updateScreen();

	freeifaddrs(ifaddr);

	GPIO_map();
	GPIO_setInput(4);
	GPIO_setInput(17);
	GPIO_setInput(27);
	GPIO_setInput(22);
	GPIO_setInput(10);
	GPIO_setInput(9);

	if ((udpSocket=socket(AF_INET, SOCK_DGRAM,IPPROTO_UDP))==-1){
		return -1;
	}
	memset((char *) &si_me, 0, sizeof(si_me));
	si_me.sin_family = AF_INET;
	si_me.sin_port = htons(PORT);
	si_me.sin_addr.s_addr = htonl(INADDR_ANY);
	if (bind(udpSocket,(struct sockaddr*)&si_me, sizeof(si_me))==-1){
		return -1;
	}

	memset(buf,0,BUFLEN);
	int size=recvfrom(udpSocket, buf, BUFLEN, 0, (struct sockaddr *)&simulatorAddr, &slen);
	if(!strcmp(buf,STARTING_PEDALBOARD)){
		sendto(udpSocket,buf,size,0,(struct sockaddr *)&simulatorAddr,slen);
	} else {
		kill(getpid(),SIGINT);
	}

	SSD1306_cleanYellow();
	SSD1306_writeMessage("CONNECTED",28,6);

	if (pthread_mutex_init(&lock, NULL) != 0){
		return -1;
	}
	isLockInit = TRUE;
	setupProximity();
	SSD1306_cleanBlue();
	if(GPIO_PIN_LEVEL & GPIO_BIT_SET(17)!=0){
		SSD1306_fillPixels((3*SSD1306_WIDTH/4),0,SSD1306_WIDTH-32,SSD1306_MAXROW - 3);
	}
	if(GPIO_PIN_LEVEL & GPIO_BIT_SET(27)==0){
		SSD1306_fillPixels((2*SSD1306_WIDTH/4),0,(3*SSD1306_WIDTH/4),SSD1306_MAXROW - 3);
	}
	if(GPIO_PIN_LEVEL & GPIO_BIT_SET(22)!=0){
		SSD1306_fillPixels((1*SSD1306_WIDTH/4),0,(2*SSD1306_WIDTH/4),SSD1306_MAXROW - 3);
	}
	if(GPIO_PIN_LEVEL & GPIO_BIT_SET(9)==0){
		SSD1306_fillPixels((0*SSD1306_WIDTH/4),0,(1*SSD1306_WIDTH/4),SSD1306_MAXROW - 3);
	}

	while(1){
		change = 0;
		change += checkButton(&GPIO4_is_set, 4, -1, START_DAW, STOP_DAW);
		change += checkProximityPedal(&GPIO17_is_set, 17, 3);
		change += checkButton(&GPIO27_is_set, 27, 2, START_REVERBER, STOP_REVERBER);
		change += checkButton(&GPIO22_is_set, 22, 1, START_CHORUS, STOP_CHORUS);
		change += checkButton(&GPIO9_is_set, 9, 0, START_DISTORTION, STOP_DISTORTION);

		FD_ZERO(&readSet);
		FD_SET(udpSocket,&readSet);
		if((GPIO_PIN_LEVEL & GPIO_BIT_SET(10))==0){			
			kill(getpid(),SIGINT);
		}

		if(select(FD_SETSIZE,&readSet,NULL,NULL,&selectExceed) > 0){
			close(udpSocket);
			udpSocket = 0;
			kill(getpid(),SIGINT);
		}


		if(change){
			SSD1306_updateScreen();
		}
		usleep(200);
	}

	return 0;
}
