// gpio.cpp 
// class for GPIO on expansion board, can read input and set output
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <linux/i2c-dev.h>
#include <sys/ioctl.h>
#include <iostream>;
using namespace std;

#include "gpio.h"

GPIO::GPIO()
{
	//map= new int[10];
	map[0] = 38;	// p8 - 3, gpmc_ad6, input
	map[1] = 39;	// p8 - 4, gpmc_ad7, output
	map[2] = 34;	// p8 - 5, gpmc_ad2, input
	map[3] = 35; 	// p8 - 6, gpmc_ad3, output
	map[4] = 30;	// p9 - 11, gpmc_wait0, output
	map[5] = 60;	// p9 - 12, gpmc_ben1, output
	map[6] = 31; 	// p9 - 13, gpmc_wpn, output
	map[7] = 50;	// p9 - 14, gpmc_a2, output
	map[8] = 33; 	// p9 - 15, gpmc_a0, output
	map[9] = 51; 	// p9 - 16, gpmc_a3, output
}

GPIO::~GPIO()
{

}
// read input from gpio
// id the id of the input
bool	GPIO::ReadInput(int id)
{
	bool bRtn = false;
	if(id == 0 || id == 2)	// check if it is input
	{
		char  filename[256];
		sprintf(filename, "/sys/class/gpio/gpio%d/value", map[id]);
		int file = open(filename, O_RDWR);
		if(file < 0)
		{
			cout << "fail to open gpio " << map[id] << endl;
		}
		char buf[1];
		if(read(file, buf, 1) > 0)
		{	
			if(buf[0] == '1')
			{
				bRtn = true;
			}
		}
		close(file);	
	}
	return bRtn;
}

// set output of the gpio
// id, the id the of output
void 	GPIO::SetOutput(int id, bool high)
{
	if(id == 0 || id == 2)	// check if it is input
	{
		return;	
	}
	char  filename[256];
	sprintf(filename, "/sys/class/gpio/gpio%d/value", map[id]);
	int file = open(filename, O_RDWR);
	if(file < 0)
	{
		cout << "fail to open gpio " << map[id] << endl;
	}
	char buf[1];
	buf[0] = high? '1' : '0';
	if(write(file, buf, 1) > 0)
	{	
		// correct
	}
	close(file);	
}

// set the gpio output to flash status
void 	GPIO::SetFlash(int id, int rate)
{
	// not implement yet0
}
