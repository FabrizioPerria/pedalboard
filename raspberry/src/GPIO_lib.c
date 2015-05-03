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


#include "GPIO_lib.h"

peripheral gpio;

/****************************************************************************************************************
 map GPIO registers into the RAM memory
 *****************************************************************************************************************/
int GPIO_map()
{
        if((gpio.memoryFILE=open("/dev/mem",O_RDWR|O_SYNC)) < 0){
#ifdef DEBUG
                printf("ERROR: memory file cannot be open\n");
#endif
                return -1;
        }

        gpio.map=mmap(
                NULL,                       //let the kernel choose where to place the mapping
                BLOCK_SIZE,                 //the mapping is initialized with 4*1024 size
                PROT_READ|PROT_WRITE,       //protection of the pages(read and write permission on registers)
                MAP_SHARED,                 //map shared with other processes
                gpio.memoryFILE,              //file to map
                GPIO_BASE                //physical address of the map
                );

        if(gpio.map == MAP_FAILED){
#ifdef DEBUG
                printf("ERROR: memory file cannot be mapped\n");
#endif
                return -1;
        }

        gpio.addr=(volatile unsigned int*)gpio.map;

        return 0;
}

/******************************************************************************
 Unmap and close the /dev/mem file
 *******************************************************************************/
void GPIO_unmap()
{
        munmap(gpio.map, BLOCK_SIZE);
        close(gpio.memoryFILE);
}

void GPIO_setInput(int pin)
{
        *(gpio.addr+(pin/10)) &= ~(7<<((pin%10)*3));
}

void GPIO_setOutput(int pin)
{
        GPIO_setInput(pin);
        *(gpio.addr+(pin/10))|=1<<((pin%10)*3);
}

void GPIO_high(int pin)
{
        *(gpio.addr+7)=GPIO_BIT_SET(pin);
}

void GPIO_low(int pin)
{
        *(gpio.addr+10)=GPIO_BIT_SET(pin);
}