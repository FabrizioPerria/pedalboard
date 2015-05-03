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


#ifndef __PERIPHERAL_H
#define __PERIPHERAL_H

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>

#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>

#define BCM2835_BASE 0x20000000
#define BLOCK_SIZE 4*1024

typedef struct peripheral {
    int memoryFILE;
    void *map;
    volatile unsigned int *addr;
}peripheral;

#endif
