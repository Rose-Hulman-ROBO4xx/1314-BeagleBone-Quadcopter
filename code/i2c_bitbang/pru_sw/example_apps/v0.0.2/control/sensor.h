#ifndef __ITG_SENSOR_HH__
#define __ITG_SENSOR_HH__

class ITG4200;
class Adxl345;
class Sensor
{
public:
	ITG4200 Gyro;
	Adxl345 Acc;
	Sensor();
	~Sensor();
	// return true if init OK
	// init accel, gyro, mag, and baro
	bool Init();
		
	// calibrate the sensors
	bool Calibrate();
	

};

#endif
