
struct SystemVaribles
{
	long currentTime;
	 long previousTime;
	 short cycleTime;     // this is the number in micro second to achieve a full loop, it can differ a little and is taken into account in the PID loop
	 short calibratingA;  // the calibration is done is the main loop. Calibrating decreases at each cycle down to 0, then we enter in a normal mode.
	 short calibratingG;
	 unsigned char  armed;
	 short acc_1G;             // this is the 1G measured acceleration
	 short  acc_25deg;
	 short  headFreeModeHold;
	 short  gyroADC[3],accADC[3],accSmooth[3],magADC[3];
	 short  heading,magHold;
	 unsigned char  vbat;               // battery voltage in 0.1V steps
	 unsigned char  rcOptions[CHECKBOXITEMS];
	 long  BaroAlt;
	 long  EstAlt;             // in cm
	 short  BaroPID;
	 long  AltHold;
	 short  errorAltitudeI;
	 short  debug[4];
	  
	//for log
	 long cycleTimeMax;       // highest ever cycle timen
	 long cycleTimeMin;   // lowest ever cycle timen
	 long powerMax;           // highest ever current

	 short i2c_errors_count;
	 short  annex650_overrun_count;

	// **********************
	//Automatic ACC Offset Calibration
	// **********************
	 unsigned short InflightcalibratingA;
	 unsigned short AccInflightCalibrationArmed;
	 unsigned short AccInflightCalibrationMeasurementDone;
	 unsigned short AccInflightCalibrationSavetoEEProm;
	 unsigned short AccInflightCalibrationActive;

	// **********************
	// power meter
	// **********************
	#define PMOTOR_SUM 8                     // index into pMeter[] for sum
	 long pMeter[PMOTOR_SUM + 1];  //we use [0:7] for eight motors,one extra for sum
	 unsigned char pMeterV;                  // dummy to satisfy the paramStruct logic in ConfigurationLoop()
	 long pAlarm;                  // we scale the eeprom value from [0:255] to this value we can directly compare to the sum in pMeter[6]
	 unsigned long powerTrigger1;        // trigger for alarm based on power consumption
	 unsigned short powerValue;          // last known current
	 unsigned short intPowerMeterSum, intPowerTrigger1;

	// **********************
	// telemetry
	// **********************
	 unsigned  char telemetry;
	 unsigned  char telemetry_auto;
	 short failsafeCnt;
	 short failsafeEvents;
	 short rcData[8];    // interval [1000;2000]
	 short rcCommand[4]; // interval [1000;2000] for THROTTLE and [-500;+500] for ROLL/PITCH/YAW 
	 short lookupRX[6]; //  lookup table for expo & RC rate
	char rcFrameComplete; //for serial rc receiver Spektrum


	// **************
	// gyro+acc IMU
	// **************
	 short gyroData[3];
	 short gyroZero[3];
	short angle[2];//   = {0,0};  // absolute angle inclination in multiple of 0.1 degree    180 deg = 1800
	 long  smallAngle25;

	// *************************
	// motor and servo functions
	// *************************
	short axisPID[3];
	short motor[8];
	unsigned char dynP8[3], dynD8[3];
	int gyroRead;	// reading count of gyro
	int gyroFail;	// failure reading of gyro	 
	int AccRead;	// reading count of accelarator
	int AccFail;	// failure reading of accelarator	 
	unsigned long framerate;	
	
	unsigned long calibratedAccTime;	
} ;

struct GPS_Common_Variables
{
// **********************
// GPS common variables
// **********************
  long  GPS_coord[2];
  long  GPS_home[2];
  long  GPS_hold[2];
  unsigned char  GPS_numSat;
  unsigned short GPS_distanceToHome;                          // distance to home in meters
  short  GPS_directionToHome;                         // direction to home in degrees
  unsigned short GPS_altitude,GPS_speed;                      // altitude in 0.1m and speed in 0.1m/s
  unsigned char  GPS_update;                              // it's a binary toogle to distinct a GPS position update
  short  GPS_angle[2];                      // it's the angles that must be applied for GPS correction
  unsigned short GPS_ground_course;                       // degrees*10
  unsigned char  GPS_Present;                             // Checksum from Gps serial
  unsigned char  GPS_Enable;

  // The desired bank towards North (Positive) or South (Negative) : latitude
  // The desired bank towards East (Positive) or West (Negative)   : longitude
  short  nav[2];
  short  nav_rated[2];    //Adding a rate controller to the navigation to make it smoother
};


struct flags_struct 
{
	unsigned char OK_TO_ARM :1 ;
	unsigned char ARMED :1 ;
	unsigned char I2C_INIT_DONE :1 ; // For i2c gps we have to now when i2c init is done, so we can update parameters to the i2cgps from eeprom (at startup it is done in setup())
	unsigned char ACC_CALIBRATED :1 ;
	unsigned char NUNCHUKDATA :1 ;
	unsigned char ACC_MODE :1 ;
	unsigned char MAG_MODE :1 ;
	unsigned char BARO_MODE :1 ;
	unsigned char GPS_HOME_MODE :1 ;
	unsigned char GPS_HOLD_MODE :1 ;
	unsigned char HEADFREE_MODE :1 ;
	unsigned char PASSTHRU_MODE :1 ;
	unsigned char GPS_FIX :1 ;
	unsigned char GPS_FIX_HOME :1 ;
	unsigned char SMALL_ANGLES_25 :1 ;
	unsigned char CALIBRATE_MAG :1 ;
} ;


class Config
{
public:
	unsigned char checkNewConf;
	unsigned char P8[PIDITEMS], I8[PIDITEMS], D8[PIDITEMS];
	unsigned char rcRate8;
	unsigned char rcExpo8;
	unsigned char rollPitchRate;
	unsigned char yawRate;
	unsigned char dynThrPID;
	unsigned char thrMid8;
	unsigned char thrExpo8;
	unsigned char accZero[3];
	unsigned char magZero[3];
	unsigned char angleTrim[2];
	unsigned char activate[CHECKBOXITEMS];
	unsigned char powerTrigger1;
	
	// load from file
	bool LoadFromFile(char * filename);
	// save config to file
	bool SaveToFile(char * filename);
	// load default setting
	void LoadDefault();
	// dump Config
	void Dump();
	
	// construct
	Config();
	
} ;



#define  uint32_t unsigned long
#define uint8_t char
#define uint16_t unsigned short

#define  int32_t long