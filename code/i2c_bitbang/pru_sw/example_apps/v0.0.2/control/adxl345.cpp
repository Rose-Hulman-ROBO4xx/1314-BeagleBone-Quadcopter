// Adxl345.cpp
// Adxl345 accelerator 

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <linux/i2c-dev.h>
#include <sys/ioctl.h>
#include <iostream>;
#include <sys/mman.h>;
#include "adxl345.h"
#include "prussdrv.h"
#include <pruss_intc_mapping.h>


using namespace std;


// init Adxl345
bool Adxl345::Init() {
	prussdrv_map_prumem(PRUSS0_SHARED_DATARAM, &sharedMem);
	return true;

}


Adxl345::Adxl345()
{
	x = 0;
	y = 0;
	z = 0;
}
Adxl345::~Adxl345()
{

}
// return true if Calibrate OK
bool Adxl345::Calibrate()
{
	FILE * calib_file = fopen("./cal.txt", "r");
	printf("Calibrating accelerometer\n");
	if (calib_file == NULL){
		printf("Please create the cal.txt\n");
		return false;
	} else{
		double temp;
		fscanf(calib_file, "%f,%f,%f,%f,%f,%f\n", &m_xOffset, &m_yOffset, &m_zOffset, &temp, &temp, &temp);
		fclose(calib_file);
	}


	printf("X offset: %f Y offset: %f Z offset: %f\n", m_xOffset, m_yOffset, m_zOffset);
	return true;

}
// return true if data been updated correctly.
bool Adxl345::UpdateData()
{
	x = (signed short)(*(((int *)sharedMem)+2+2048));
	y = (signed short)(*(((int *)sharedMem)+3+2048));
	z = (signed short)(*(((int *)sharedMem)+4+2048));
	printf("%d %d %d\n", x,y,z);

	return true;

}

