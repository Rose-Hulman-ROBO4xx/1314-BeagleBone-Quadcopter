// Sensor.cpp
// demo of read 10 dof sendor board

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <linux/i2c-dev.h>
#include <sys/ioctl.h>
#include <iostream>;
#include "itg4200.h"
#include "adxl345.h"
#include "sensor.h"

using namespace std;



// init itg3200
bool Sensor::Init() {
	Gyro.Init();
	Acc.Init();
	return true;
}


Sensor::Sensor()
{
}
Sensor::~Sensor()
{

}

// return true if init OK
bool Sensor::Calibrate()
{
	cout << "Calibrate sensors" << endl;
	Gyro.Calibrate();
	Acc.Calibrate();
	return true;

}

