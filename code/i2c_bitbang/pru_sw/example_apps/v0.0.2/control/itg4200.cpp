// itg3200.cpp
// demo of read itg3200 gyro

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <linux/i2c-dev.h>
#include <sys/ioctl.h>
#include <iostream>;
#include <sys/mman.h>
#include "itg4200.h"
#include "prussdrv.h"
#include <pruss_intc_mapping.h>

using namespace std;

char buf[8];


// init itg3200
bool ITG4200::Init() {
	//basically just need to get a pointer to PRU shared ram
	
	prussdrv_map_prumem(PRUSS0_SHARED_DATARAM, &sharedMem);

	return true;
}


ITG4200::ITG4200()
{
	x = 0;
	y = 0;
	z = 0;
}
ITG4200::~ITG4200()
{

}
// return true if Calibrate OK
bool ITG4200::Calibrate()
{
	//TODO: load our calibration data
	printf("calibrating gyro\n");
	FILE * calib_file = fopen("./cal.txt", "r");
	if (calib_file == NULL){
		printf("Please create the cal.txt\n");
		return false;
	} else{
		double temp;
		fscanf(calib_file, "%f,%f,%f,%f,%f,%f", &temp, &temp, &temp, &m_xOffset, &m_yOffset, &m_zOffset);
		fclose(calib_file);
	}

	printf("X offset: %f Y offset: %f Z offset: %f\n", m_xOffset, m_yOffset, m_zOffset);
	return true;

}
// return true if data been updated correctly.
bool ITG4200::UpdateData()
{
	x = (signed short)(*(((int *)sharedMem)+5+2048));
	y = (signed short)(*(((int *)sharedMem)+6+2048));
	z = (signed short)(*(((int *)sharedMem)+7+2048));
	printf("%d %d %d\n", x,y,z);
	return true;

}

