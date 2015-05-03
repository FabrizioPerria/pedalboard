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


/******************************************************
 STD INCLUDES
******************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <pthread.h>
#include <arpa/inet.h>
#include <netinet/in.h>
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
#define MAXPROXIMITY 127
#define PROXIMITY_OFFSET 0
#define MAXUINT 65535

/********************************************************
 TYPE DEFINITIONS
 ********************************************************/

typedef enum {FALSE,TRUE} boolean;

/********************************************************
 GLOBAL VARIABLES
 ********************************************************/

pthread_mutex_t lock;				//mutex used to synchronize sending messages
int udpSocket;						//socket ID
boolean closeThread=TRUE;			//flag to trigger the end of the Proximity sensor's thread
struct sockaddr_in simulatorAddr;   //IP address of the simulator(PC) 
pthread_t th;						//ID of the Proximity sensor's thread
boolean isLockInit = FALSE;
/*********************************************************
 FUNCTIONS
 **********************************************************/

//Send the measures detected by the Proximity Sensor
void *sendProxMeasures(void *pa)
{
        char buf[BUFLEN];
        int slen = sizeof(simulatorAddr);
	    unsigned long tmp = 0;
#ifdef DEBUG
        printf("Inside Thread\n");
#endif
	    SSD1306_fillPixels((3*SSD1306_WIDTH/4),0,SSD1306_WIDTH,SSD1306_MAXROW - 3);
	    SSD1306_updateScreen();
        if(testVCNL4000() < 0){
#ifdef DEBUG
                printf("VCNL4000 error\n");
#endif
		        return;
	    }else{
                while(!closeThread){
                        tmp=((getProximityMeasure()*MAXPROXIMITY)/MAXUINT)-PROXIMITY_OFFSET;
            
                        if (tmp > 110){
								tmp = 110;
						}
					    sprintf(buf,"%d",tmp);
	                    usleep(100);
                        if(pthread_mutex_trylock(&lock) == 0){
						        sendto(udpSocket,buf,strlen(buf),0,(struct sockaddr *)&simulatorAddr,slen);
                                pthread_mutex_unlock(&lock);
						}
                }        
        }
}

//Handle the exit from the program
void ctrlC()
{
#ifdef DEBUG
        printf("\n********************\nClosing\n********************\n");
#endif
        if(closeThread == FALSE){
                closeThread=TRUE;
                pthread_join(th,NULL);
        }
        if(isLockInit == TRUE){
                pthread_mutex_destroy(&lock);
	}
        if(udpSocket > 0){
                int slen=sizeof(simulatorAddr);
                char buf[7]=ENDING_PEDALBOARD;
                sendto(udpSocket,buf,strlen(buf),0,(struct sockaddr *)&simulatorAddr,slen);
                close(udpSocket);
        }
        int tmp= GPIO_PIN_LEVEL & GPIO_BIT_SET(4);
        SSD1306_turnOFF();
        I2C_close();
        GPIO_unmap();
	    if(tmp != 0){
			    system("reboot");
	     
	    }else{
                system("poweroff");
        }
        exit(0);
}

/**********************************************************************
 MAIN
 **********************************************************************/
int main()
{
	    boolean change=FALSE;
        struct timeval selectExceed;
        fd_set readSet;
        int slen=sizeof(simulatorAddr);
        struct sockaddr_in si_me;
        char buf[BUFLEN];
        char ip[BUFLEN];    
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
		FILE *fp=popen("hostname -I","r");
		udpSocket = 0;

		if(fp == NULL){
#ifdef DEBUG
                printf("Impossible to get the IP\nSHUTDOWN\n");
#endif
				kill(getpid(),SIGINT);
        }
    
		while(fgets(ip,BUFLEN,fp)!=NULL){
			    if(ip[0]==0x20)						//blank IP means no IP
				        continue;
			    sprintf(ip,"%s %d",ip, PORT);
			    SSD1306_writeMessage(ip,0,6);		//Yellow screen
			    copyImage(splashScreen,0,0,128,6);	//blue screen
		}
    
		SSD1306_updateScreen();
    
		pclose(fp);

		GPIO_map();
		GPIO_setInput(4);
		GPIO_setInput(17);
		GPIO_setInput(27);
		GPIO_setInput(22);
		GPIO_setInput(10);
		GPIO_setInput(9);

#ifdef DEBUG
        printf("Creating Socket\n");
#endif
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
#ifdef DEBUG
        printf("Waiting for PC connection\n");
#endif
		memset(buf,0,BUFLEN);
		int size=recvfrom(udpSocket, buf, BUFLEN, 0, (struct sockaddr *)&simulatorAddr, &slen);
		if(!strcmp(buf,STARTING_PEDALBOARD)){
			    sendto(udpSocket,buf,size,0,(struct sockaddr *)&simulatorAddr,slen);
		} else {
		        kill(getpid(),SIGINT);
		}
#ifdef DEBUG
        printf("PC: %s:%d\n",inet_ntoa(simulatorAddr.sin_addr), ntohs(simulatorAddr.sin_port));
#endif
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
                SSD1306_fillPixels(0,0,(1*SSD1306_WIDTH/4),SSD1306_MAXROW - 3);
        }
	
	    change=TRUE;
        while(1){
	            memset(buf,0,BUFLEN);
			    if((!GPIO4_is_set) && ((GPIO_PIN_LEVEL & GPIO_BIT_SET(4))!=0)){
#ifdef DEBUG
				        printf("(!GPIO4_is_set) && ((GPIO_PIN_LEVEL & GPIO_BIT_SET(4))!=0)\n");
#endif
						strncpy(buf,START_DAW,BUFLEN);
						if(pthread_mutex_trylock(&lock) == 0){
						        sendto(udpSocket,buf,strlen(buf),0,(struct sockaddr*)&simulatorAddr,slen);
						        pthread_mutex_unlock(&lock);
						}
						GPIO4_is_set = TRUE;
			    } else if ((GPIO4_is_set) && ((GPIO_PIN_LEVEL & GPIO_BIT_SET(4))==0)){
#ifdef DEBUG
				        printf("(GPIO4_is_set) && ((GPIO_PIN_LEVEL & GPIO_BIT_SET(4))==0)\n");
#endif
						strncpy(buf,STOP_DAW,BUFLEN);
						if(pthread_mutex_trylock(&lock) == 0){
						        sendto(udpSocket,buf,strlen(buf),0,(struct sockaddr *)&simulatorAddr,slen);
						        pthread_mutex_unlock(&lock);
						}
						GPIO4_is_set = FALSE;
				}
        
                if((!GPIO17_is_set) && ((GPIO_PIN_LEVEL & GPIO_BIT_SET(17))!=0)){
#ifdef DEBUG
                        printf("(!GPIO17_is_set) && ((GPIO_PIN_LEVEL & GPIO_BIT_SET(17))!=0)\n");
#endif
                        closeThread=FALSE;
                        strncpy(buf ,START_EXPR_PEDAL,BUFLEN);
                        if(pthread_mutex_trylock(&lock) == 0){
						        sendto(udpSocket,buf,strlen(buf),0,(struct sockaddr *)&simulatorAddr,slen);
                                pthread_mutex_unlock(&lock);
						}
                        pthread_create(&th,NULL,&sendProxMeasures,NULL);
                        GPIO17_is_set=TRUE;
                } else if((GPIO17_is_set) && ((GPIO_PIN_LEVEL & GPIO_BIT_SET(17))==0)){
#ifdef DEBUG
                        printf("(GPIO17_is_set) && ((GPIO_PIN_LEVEL & GPIO_BIT_SET(17))==0)\n");
#endif
						change=TRUE;
						SSD1306_emptyPixels((3*SSD1306_WIDTH/4),0,SSD1306_WIDTH,SSD1306_MAXROW - 3);
						closeThread=TRUE;
						strncpy(buf,STOP_EXPR_PEDAL,BUFLEN);
						if(pthread_mutex_trylock(&lock) == 0){
						        sendto(udpSocket,buf,strlen(buf),0,(struct sockaddr *)&simulatorAddr,slen);
						        pthread_mutex_unlock(&lock);
						}
						GPIO17_is_set=FALSE;
			    }
        
                if((!GPIO27_is_set)&&((GPIO_PIN_LEVEL & GPIO_BIT_SET(27))!=0)){
#ifdef DEBUG
                        printf("(!GPIO27_is_set) && ((GPIO_PIN_LEVEL & GPIO_BIT_SET(27))!=0)\n");
#endif
                        change=TRUE;
						SSD1306_fillPixels((2*SSD1306_WIDTH/4),0,(3*SSD1306_WIDTH/4),SSD1306_MAXROW - 3);
						strncpy(buf,START_REVERBER,BUFLEN);
						if(pthread_mutex_trylock(&lock) == 0){
						        sendto(udpSocket,buf,strlen(buf),0,(struct sockaddr *)&simulatorAddr,slen);
						        pthread_mutex_unlock(&lock);
						}
						GPIO27_is_set = TRUE;
                } else if ((GPIO27_is_set) && ((GPIO_PIN_LEVEL & GPIO_BIT_SET(27))==0)){
#ifdef DEBUG
                        printf("(GPIO27_is_set) && ((GPIO_PIN_LEVEL & GPIO_BIT_SET(27))==0)\n");
#endif
						change=TRUE;
						SSD1306_emptyPixels((2*SSD1306_WIDTH/4),0,(3*SSD1306_WIDTH/4),SSD1306_MAXROW - 3);
						strncpy(buf,STOP_REVERBER,BUFLEN);
						if(pthread_mutex_lock(&lock) == 0){
						        sendto(udpSocket,buf,strlen(buf),0,(struct sockaddr *)&simulatorAddr,slen);
						        pthread_mutex_unlock(&lock);
						}
						GPIO27_is_set = FALSE;
                }

                if((!GPIO22_is_set)&&((GPIO_PIN_LEVEL & GPIO_BIT_SET(22))!=0)){
#ifdef DEBUG
                        printf("(!GPIO22_is_set) && ((GPIO_PIN_LEVEL & GPIO_BIT_SET(22))!=0)\n");
#endif
                        change=TRUE;
			            SSD1306_fillPixels((1*SSD1306_WIDTH/4),0,(2*SSD1306_WIDTH/4),SSD1306_MAXROW - 3);
                        strncpy(buf,START_CHORUS,BUFLEN);
					    if(pthread_mutex_trylock(&lock) == 0){
                                sendto(udpSocket,buf,strlen(buf),0,(struct sockaddr *)&simulatorAddr,slen);
                                pthread_mutex_unlock(&lock);
						}
                        GPIO22_is_set = TRUE;
                } else if ((GPIO22_is_set) && ((GPIO_PIN_LEVEL & GPIO_BIT_SET(22))==0)){
#ifdef DEBUG
                        printf("(GPIO22_is_set) && ((GPIO_PIN_LEVEL & GPIO_BIT_SET(22))==0)\n");
#endif
			            change=TRUE;
			            SSD1306_emptyPixels((1*SSD1306_WIDTH/4),0,2*SSD1306_WIDTH/4,SSD1306_MAXROW - 3);
                        strncpy(buf,STOP_CHORUS,BUFLEN);
                        if(pthread_mutex_trylock(&lock) == 0){
						        sendto(udpSocket,buf,strlen(buf),0,(struct sockaddr *)&simulatorAddr,slen);
                                pthread_mutex_unlock(&lock);
						}
                        GPIO22_is_set = FALSE;
                }
				FD_ZERO(&readSet);
				FD_SET(udpSocket,&readSet);
				if((GPIO_PIN_LEVEL & GPIO_BIT_SET(10))==0 || select(FD_SETSIZE,&readSet,NULL,NULL,&selectExceed) > 0){
					    kill(getpid(),SIGINT);
				}
        
				if((!GPIO9_is_set)&&((GPIO_PIN_LEVEL & GPIO_BIT_SET(9))!=0)){
#ifdef DEBUG
                        printf("(!GPIO9_is_set) && ((GPIO_PIN_LEVEL & GPIO_BIT_SET(9))!=0)\n");
#endif
						change=TRUE;
						SSD1306_fillPixels(0,0,1*SSD1306_WIDTH/4,SSD1306_MAXROW - 3);
						strncpy(buf,START_DISTORTION,BUFLEN);
						if(pthread_mutex_trylock(&lock) == 0){
						        sendto(udpSocket,buf,strlen(buf),0,(struct sockaddr *)&simulatorAddr,slen);
						        pthread_mutex_unlock(&lock);
						}
						GPIO9_is_set = TRUE;
                } else if ((GPIO9_is_set) && ((GPIO_PIN_LEVEL & GPIO_BIT_SET(9))==0)){
#ifdef DEBUG
                        printf("(GPIO9_is_set) && ((GPIO_PIN_LEVEL & GPIO_BIT_SET(9))==0)\n");
#endif
						change=TRUE;
						SSD1306_emptyPixels(0,0,1*SSD1306_WIDTH/4,SSD1306_MAXROW - 3);
						strncpy(buf,STOP_DISTORTION,BUFLEN);
						if(pthread_mutex_trylock(&lock) == 0){
						        sendto(udpSocket,buf,strlen(buf),0,(struct sockaddr *)&simulatorAddr,slen);
						        pthread_mutex_unlock(&lock);
						}
						GPIO9_is_set = FALSE;
                }
		        if(change == TRUE){
		                SSD1306_updateScreen();
		                change=FALSE;
		        }
                usleep(100);
        }
	
        return 0;
}
