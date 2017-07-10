//Display SSD1306
/*
PROTOCOL

Write on the display

APP     |START|Device Address|Write bit|   |Control Byte|   |Data/command|   |Control Byte|   |Data/command|   |  . . .  |Stop bit|
DISP                                   |ACK|            |ACK|            |ACK|            |ACK|            |ACK| . . .
*/

#ifndef __DISPLAY_LIB_H
#define __DISPLAY_LIB_H

#include "I2C_lib.h"
#include "fonts.h"
#include "debug_enable.h"
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <linux/i2c-dev.h>
#include <errno.h>
#include <sys/ioctl.h>

#define SSD1306_WIDTH									128
#define SSD1306_HEIGHT									64
#define SSD1306_FBSIZE									1024									//128x8
#define SSD1306_MAXROW									8							
#define DISPLAY_ADDRESS                                 0x3C
#define DISPLAY_CONTROL_COMMAND_SINGLE                  0x80                                    // next byte will be command
#define DISPLAY_CONTROL_COMMAND_CONTINUOUS              0x00                                    // next byte will be command
#define DISPLAY_CONTROL_DATA                            0x40                                    // next byte will be data

/*******************************************************************************************************************************************************************/
//Fundamental command table with DISPLAY_CONTROL_COMMAND (control byte = 0)
#define DISPLAY_COMMAND_CONTRAST                        0x81                                    //1 to 256 as data after the command (after RESET should be 0x7F)

#define DISPLAY_ON_CURRENT_RAM                          0xA4                                    //display RAM content (default after RESET)
#define DISPLAY_ON_IGNORE_RAM                           0xA5                                    //ignore RAM content

#define DISPLAY_NORMAL_MODE                             0xA6                                    //0 in RAM means OFF....1 in RAM means ON (default after RESET)
#define DISPLAY_INVERSE_MODE                            0xA7                                    //0 in RAM means ON.....1 in RAM means OFF

#define DISPLAY_OFF		                                0xAE                                    //set display OFF (sleep mode) (default after RESET)
#define DISPLAY_ON                                      0xAF                                    //set display ON in normal mode

/******************************************************************************************************************************************************************/
//Scrolling command table - Continuous Horizontal scroll setup (control byte = 0)
#define DISPLAY_RIGHT_HORIZONTAL_SCROLL                 0x26   
#define DISPLAY_LEFT_HORIZONTAL_SCROLL                  0x27
/*After(control byte = 0):
	- send 0x0 as dummy byte
	- send the start page address (0x0 to 0x7)
	- send the time interval between each scroll (0x0 to 0x7)
	- send the end page address (0x0 to 0x7)
	- send 0x0 as dummy byte
	- send 0xFF as dummy byte
*/
/******************************************************************************************************************************************************************/
//Scrolling command table - Continuous Vertical and Horizontal scroll setup (control byte = 0)
#define DISPLAY_RIGHT_VERTICAL_HORIZONTAL_SCROLL        0x29   
#define DISPLAY_LEFT_VERTICAL_HORIZONTAL_SCROLL         0x2A  
/*After(control byte = 0):
	- send 0x0 as dummy byte
	- send the start page address (0x0 to 0x7)
	- send the time interval between each scroll (0x0 to 0x7)
	- send the end page address (0x0 to 0x7)
	- send the vertical scrolling offset (0x0 to 0x3F) 
*/

/******************************************************************************************************************************************************************/
//Scrolling command table - scroll enable (control byte = 0)
#define DISPLAY_STOP_SCROLL                              0x2E       //stops the scroll commanded by 0x26, 0x27,0x29 or 0x2A - after the stop, the RAM needs to be rewritten
#define DISPLAY_START_SCROLL                             0x2F       //enables the scroll configured in 0x26, 0x27, 0x29, 0x2A (in this order) 
                                                                    //if you do more than one setup before the start command, only the last one will be executed
                                                                    //example:  0x26 0x2A 0x2F the DISPLAY_LEFT_VERTICAL_HORIZONTAL_SCROLL will be started and the right horizontal ignored

/*****************************************************************************************************************************************************************/
//Scrolling command table - vertical scroll area (control byte = 0)
#define DISPLAY_VERTICAL_SCROLL_AREA                     0xA3
/*After(control byte = 0):
	- send the number of rows in the top fixed area (0x0 to 0x3F) (after RESET is 0)
	- send the number of rows in the scroll area (0x0 to 7F) (after RESET is 64)
*/

/*****************************************************************************************************************************************************************/
//Addressing Settings (control byte = 0)
#define DISPLAY_PAGE_ADDRESSING_MODE_LOWER_NIBBLE(n)    (n < 0x10 ? n : 0)
#define DISPLAY_PAGE_ADDRESSING_MODE_HIGHER_NIBBLE(n)   ((n < 0x20)&&(n >= 0x10) ? n : 0x10)
#define DISPLAY_PAGE_ADDRESS(n)							((n < 0xB7)&&(n >= 0xB0) ? 0xB0 + n : 0xB0)
/*Page Addressing mode:
  - 0x00 to 0x0F   Sets the lower nibble of the column start address register for page addressing mode (After RESET is 0x0)
  - 0x10 to 0x1F   Sets the higher nibble of the column start address register for page addressing mode (After RESET is 0x10)
  ***************************************************************************************************************************
  - 0xB0 to 0xB7   page start address
  ***************************************************************************************************************************
*/
#define DISPLAY_SET_MEMORY_ADDRESSING_MODE               0x20
/*After(Control byte = 0)
    - 00b horizontal addressing mode
	- 01b vertical addressing mode
	- 10b page addressing mode (default after RESET)
	- 11b INVALID
*/

#define DISPLAY_SET_COLUMN_ADDRESS                       0x21  //(Horizontal or vertical addressing mode)
/*After(Control Byte = 0):
    - Column start address (0x0 to 0x7F)  (
	- Column end address (0x0 to 0x7F)
*/

#define DISPLAY_PAGE_HORIZ_VERT_MODE                     0x22   //(Horizontal or vertical addressing mode)
/*After(Control Byte=0):
    - page start address (0x0 to 0x7) (default 0x0 after RESET)
	- page end address (0x0 to 0x7)   (default 0x7 after RESET)
*/

/*****************************************************************************************************************************************************************/
// Hardware Configuration (control byte = 0)
#define DISPLAY_START_LINE(n)                           ((n >= 0x40)&&(n<0x80) ? n : 0x40)
/*Set Display start Line:
  - 0x40 to 0x7F (after RESET the start line is the first so the default value is 0x40 - the max value corresponds to line 63)
*/

#define DISPLAY_SEGMENT0_REMAP_ADDR0                     0xA0
#define DISPLAY_SEGMENT0_REMAP_ADDR127                   0xA1

#define DISPLAY_SET_MUX_RATIO                            0xA8
/*After (Control Byte=0):
    -0x10 to 0x3F	(from 16MUX to 64MUX......default 64MUX after RESET)
*/

#define DISPLAY_COM_OUT_DIRECTION_NORMAL                 0xC0       //scan from COM0 to COM(MUX_RATIO-1)
#define DISPLAY_COM_OUT_DIRECTION_REMAPPED               0xC8       //scan from COM(MUX_RATIO-1) to COM0

#define DISPLAY_SET_OFFSET                               0xD3       //set vertical shift by COM0
/*After (control Byte = 0):
    - 0x00 to 0x7F (after RESET the default value is 0)
*/

#define DISPLAY_COM_PINS_CONFIG                          0xDA
/*After (control byte=0):
    - 0x02   sequential configuration with COM left/right disabled
	- 0x12   alternative configuration with COM left/right disabled (default after RESET)
	- 0x22   sequential configuration with COM left/right enabled
	- 0x32   alternative configuration with COM left/right enabled
*/
/*****************************************************************************************************************************************************************/
//Timing settings (control byte=0)
#define DISPLAY_SET_CLOCK_RATIO_OSC_FREQ                 0xD5
/*After (control byte=0):
    -4 MSB (default 0x08) frequency oscillator
	-4 LSB (default 0x00) divide ratio = value + 1  default ratio=1 with A[3:0] = 0x00
	N.B.:  register= MSB + LSB
*/

#define DISPLAY_PRECHARGE_PERIOD                         0xD9
/*After (control byte=0):
    - 4 LSB (default 0x02 - 0x00 invalid)    phase 1: pixels discharges from the previous image
    - 4 MSB (default 0x02 - 0x00 invalid)    phase 2: pixel are driven to charge until right value
	N.B.: register = MSB+LSB
*/

#define DISPLAY_VCOM                                     0xDB
/*After (control byte = 0):
    - 0x00 (0.65 * Vcc)
	- 0x20 (0.77 * Vcc) (default value after RESET)
	- 0x30 (0.83 * Vcc)
*/

#define DISPLAY_NOP                                      0xE3


/******************************************************************************
 Prototypes
 *******************************************************************************/
void SSD1306_updateScreen();
void SSD1306_writeCmd(unsigned char);
int SSD1306_setup();
void SSD1306_turnOFF();
void SSD1306_writeChar(unsigned char,int,int);
void SSD1306_writeMessage(char *,int,int);
void copyImage(const unsigned char *image,int xStart,int yStart,int xEnd,int yEnd);
void SSD1306_cleanBlue();
void SSD1306_cleanYellow();
void SSD1306_cleanAll();
void SSD1306_fillPixels(int,int,int,int);    
void SSD1306_emptyPixels(int,int,int,int);
#endif

