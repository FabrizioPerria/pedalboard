#include "DISPLAY_lib.h"
#include <pthread.h>
/********************************************************************************
 Framebuffer structure
 to be static so only the DISPLAY_lib functions can access to it
 *********************************************************************************/
static unsigned char frameBuffer[SSD1306_FBSIZE];

extern pthread_mutex_t i2cMutex;

#ifdef DEBUG
static void printFrameBuffer()
{
	int i,j;
	for(i=0;i<8;i++){
		for(j=0;j<128;j++){
			printf("%02x ",frameBuffer[(i*128)+j]);
                }
                printf("\n");
        }
}
#endif


/************************************************************************************
 Setting up the SSD1306 registers
 ************************************************************************************/
int SSD1306_setup()
{
        int i;

        if (I2C_setup() < 0) {
        	return -1;
        }
        SSD1306_writeCmd(DISPLAY_OFF);
	SSD1306_writeCmd(DISPLAY_SET_CLOCK_RATIO_OSC_FREQ);
	SSD1306_writeCmd(0x80);					//suggested value
	SSD1306_writeCmd(DISPLAY_SET_MUX_RATIO);
	SSD1306_writeCmd(0x3F);					//suggested value
	SSD1306_writeCmd(DISPLAY_SET_OFFSET);
	SSD1306_writeCmd(0x00);					//suggested value
	SSD1306_writeCmd(DISPLAY_START_LINE(0));		//suggested value
	SSD1306_writeCmd(0x8D);
	SSD1306_writeCmd(0x14);					//suggested value
	SSD1306_writeCmd(DISPLAY_SEGMENT0_REMAP_ADDR0);
	SSD1306_writeCmd(DISPLAY_COM_OUT_DIRECTION_NORMAL);
	SSD1306_writeCmd(DISPLAY_COM_PINS_CONFIG);
	SSD1306_writeCmd(0x12);					//suggested value
	SSD1306_writeCmd(DISPLAY_COMMAND_CONTRAST);
	SSD1306_writeCmd(0xCF);					//suggested value
	SSD1306_writeCmd(DISPLAY_PRECHARGE_PERIOD);
	SSD1306_writeCmd(0xF1);					//suggested value
	SSD1306_writeCmd(DISPLAY_VCOM);
	SSD1306_writeCmd(0x40);					//suggested value
	SSD1306_writeCmd(DISPLAY_ON_CURRENT_RAM);
        SSD1306_writeCmd(DISPLAY_NORMAL_MODE);
        SSD1306_writeCmd(DISPLAY_SET_MEMORY_ADDRESSING_MODE);
        SSD1306_writeCmd(0x00);                                 // Horizontal mode
        SSD1306_writeCmd(DISPLAY_ON);                           // Display ON
 	SSD1306_writeCmd(DISPLAY_PAGE_ADDRESS(0));
	SSD1306_writeCmd(DISPLAY_PAGE_ADDRESSING_MODE_LOWER_NIBBLE(0));
	SSD1306_writeCmd(DISPLAY_PAGE_ADDRESSING_MODE_HIGHER_NIBBLE(0x10));

        usleep(100);
}

/***********************************************************************************
 Clean up the framebuffer and turn off the screen
 **********************************************************************************/
void SSD1306_turnOFF()
{
        SSD1306_cleanAll();
	SSD1306_writeCmd(DISPLAY_OFF);
}

/************************************************************************************
 Ask the writechar function to store a string in the frame buffer
 ************************************************************************************/
void SSD1306_writeMessage(char *buffer,int x,int y)
{
	int i=0;
	for(i=0;i<strlen(buffer);i++){
	     SSD1306_writeChar(buffer[i],x+(FONT_WIDTH*i),y);
	}
}

/***********************************************************************************
 WriteChar searches for each character the 8x8 hexcode and it stores the matrix on the framebuffer
 *******************************************************************************************/
void SSD1306_writeChar(unsigned char dat,int x,int y)
{
	    int k=0;
	    if(dat==0x0a || dat == 0x0d){            // ignore CR or LF
		        return;
	    }
	    if(dat == 0x0){                          //NULL byte handling (the font i'm using starts from the 0x20 of the ascii code, so from 0 to 0x1F the non-pritable characters are not considered)
		        frameBuffer[(y*128)+x+k]=0x0;
	    }else{
		   	for(k=0;k < FONT_WIDTH;k++){
                	    	frameBuffer[(y*128)+x+k]=font[8*(dat-32)+k];
        	    	}
	    }
}

/************************************************************************************************************************
 Copy a portion of an image(defined as a bitmap hex code based) starting from the coord [xStart,yStart] to [xEnd,yEnd] nto the frame buffer
 ************************************************************************************************************************/
void copyImage(const unsigned char *image,int xStart,int yStart,int xEnd,int yEnd)
{
	    int i,j;
	    for(i=xStart;i<xEnd;i++){
		        for(j=yStart;j<yEnd;j++){
			            frameBuffer[(j*128)+i]=image[(j*128)+i];
		        }
	    }
}

/***************************************************************************************************************************
 Wrapper to send a command over I2C
 *************************************************************************************************************************/
void SSD1306_writeCmd(unsigned char cmd)
{
	pthread_mutex_lock(&i2cMutex);
        I2C_writeCmd(DISPLAY_ADDRESS,DISPLAY_CONTROL_COMMAND_SINGLE,cmd);
	pthread_mutex_unlock(&i2cMutex);
}

/**************************************************************************************************************************
 Send the framebuffer to the screen over I2C bus
 *****************************************************************************************************************************/
void SSD1306_updateScreen()
{
	    unsigned char buffer[2];
	    int i=0;
        SSD1306_writeCmd(DISPLAY_PAGE_ADDRESS(0));
        SSD1306_writeCmd(DISPLAY_PAGE_ADDRESSING_MODE_LOWER_NIBBLE(0));
        SSD1306_writeCmd(DISPLAY_PAGE_ADDRESSING_MODE_HIGHER_NIBBLE(0x10));
        for(i=0;i<SSD1306_FBSIZE;i++){
                I2C_writeCmd(DISPLAY_ADDRESS,DISPLAY_CONTROL_DATA,frameBuffer[i]);
	}
}

/****************************************************************************************************************************
 Fill the space inside the rectangle defined by paramenters
 *****************************************************************************************************************************/
void SSD1306_fillPixels(int xStart,int yStart,int xEnd,int yEnd)
{
            int i,j;
            for(i=xStart+4;i<xEnd-4;i++){
                        for(j=yStart;j<yEnd;j++){
                                    frameBuffer[(j*128)+i]=0xFF;
                        }
            }
}

/*****************************************************************************************************************************
 Empty the pixels inside the rectangle defined by parameters
 *****************************************************************************************************************************/
void SSD1306_emptyPixels(int xStart,int yStart,int xEnd,int yEnd)
{
            int i,j;
            for(i=xStart+4;i<xEnd-4;i++){
                        for(j=yStart;j<yEnd;j++){
                                    frameBuffer[(j*128)+i]=0;
                        }
            }
}

void SSD1306_cleanBlue()
{
	    int i,j;
	    for(i=0;i<128;i++){
		        for(j=0;j<6;j++){
			            SSD1306_writeChar(0x0,i,j);     //send 0 to all
                	    frameBuffer[(j*128) + i]=0;
		        }
		}
	    SSD1306_updateScreen();
}

void SSD1306_cleanYellow()
{
	    int i,j;
	    for(i=0;i<128;i++){
		        for(j=6;j<8;j++){
			            SSD1306_writeChar(0x0,i,j);     //send 0 to all
			            frameBuffer[(j*128)+i]=0;
		        }
	    }
	    SSD1306_updateScreen();
}

void SSD1306_cleanAll()
{
	    SSD1306_cleanBlue();
	    SSD1306_cleanYellow();
}
