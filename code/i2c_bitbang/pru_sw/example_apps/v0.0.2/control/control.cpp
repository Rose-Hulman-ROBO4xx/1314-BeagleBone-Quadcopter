// control.cpp
#include "config.h"
#include "def.h"
#include <math.h>
#include <vector>
#include <string.h>
#include <stdio.h>
#include <iostream>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <termios.h>
#include <errno.h>
#include <time.h>
#include <pthread.h>


#define BAUDRATE B115200
#define SERIALPORT "/dev/ttyO2"
using namespace std;

#include "typedef.h"


#include "dumpData.h"
#include "itg4200.h"
#include "adxl345.h"
#include "sensor.h"
#include "PruProxy.h"
#include "misc.h"

static SystemVaribles sysInfo;

static GPS_Common_Variables gpsVars;

static flags_struct f;

static Config conf;


void initSysVari(SystemVaribles &s)
{
	s.currentTime = 0;
	 s.previousTime = 0;
	 s.cycleTime = 0;     // this is the number in micro second to achieve a full loop, it can differ a little and is taken into account in the PID loop
	 s.calibratingA = 0;  // the calibration is done is the main loop. Calibrating decreases at each cycle down to 0, then we enter in a normal mode.
	 s.calibratingG;
	 s.armed = 0;
	 s.BaroPID = 0;
	 s.errorAltitudeI = 0;
	  
	//for log
	 s.cycleTimeMax = 0;       // highest ever cycle timen
	 s.cycleTimeMin = 65535;   // lowest ever cycle timen
	 s.powerMax = 0;           // highest ever current

	 s.i2c_errors_count = 0;
	 s.annex650_overrun_count = 0;

	// **********************
	//Automatic ACC Offset Calibration
	// **********************
	 s.InflightcalibratingA = 0;
	 s.AccInflightCalibrationMeasurementDone = 0;
	 s.AccInflightCalibrationSavetoEEProm = 0;
	 s.AccInflightCalibrationActive = 0;
	s.acc_1G = 265;
	 s.powerTrigger1 = 0;        // trigger for alarm based on power consumption
	 s.powerValue = 0;          // last known current

	// **********************
	// telemetry
	// **********************
	 s.telemetry = 0;
	 s.telemetry_auto = 0;
	 s.failsafeCnt = 0;
	 s.failsafeEvents = 0;


	// **************
	// gyro+acc IMU
	// **************
	for(int i =0; i < 3;i ++)
	{
		 s.gyroData[i] = 0;
		 s.gyroZero[i] = 0;
	}
	s.angle[0] = 0;
	s.angle[1] = 0;   // absolute angle inclination in multiple of 0.1 degree    180 deg = 1800
	 s.smallAngle25 = 1;
	 s.gyroRead = 0;	// reading count of gyro
	 s.gyroFail = 0;	// failure reading of gyro	 
	 s.AccRead = 0;	// reading count of accelarator
	s.AccFail = 0;	// failure reading of accelarator	

}

// init the gps variables
void InitGpsVars(GPS_Common_Variables& g)
{
  g.GPS_update = 0;                              // it's a binary toogle to distinct a GPS position update
  g.GPS_angle[0] = 0; g.GPS_angle[1] = 0;                      // it's the angles that must be applied for GPS correction
  g.GPS_ground_course = 0;                       // degrees*10
  g.GPS_Present = 0;                             // Checksum from Gps serial
  g.GPS_Enable  = 0;
}

// ******************
// rc functions
// ******************
#define MINCHECK 1050
#define MAXCHECK 1950

void *SensorThread( void * );

Sensor sensor;
static PruProxy Pru;
void blinkLED(uint32_t num, uint32_t wait,uint32_t repeat) {
  // uint8_t i,r;
  // for (r=0;r<repeat;r++) {
    // for(i=0;i<num;i++) {
      // LEDPIN_TOGGLE; //switch LEDPIN state
      // BUZZERPIN_ON;
      // delay(wait);
      // BUZZERPIN_OFF;
    // }
    // delay(60);
  // }
}


unsigned int min(unsigned int a, unsigned int b)
{
	if (a < b)
		return a;
	return b;
}


#define QUADX
 #define MULTITYPE 3
  



void setup() 
{
	initStopTimer();
	if(conf.LoadFromFile(SETTINGFILE))
	{
		cout << "Load  Quad Setting" << endl;
	}
	else
	{
		conf.LoadDefault();
		cout << "Fail to load Quad Setting" << endl << "Load with Default value" << endl;
		conf.SaveToFile(SETTINGFILE);
	}
	conf.Dump();
	if(sensor.Init())
	{
		cout << "sensor init success." << endl;
	}
	else
	{
		cout << "sensor init failed. " << endl;
	}
	
	
	sysInfo.armed=0;
	
	for(int i=0;i<6;i++)
	{
		sysInfo.lookupRX[i] = (2500 + conf.rcExpo8*(i*i-25)) * i * (long)conf.rcRate8/2500;
	}	
  // SerialOpen(0,115200);
  // LEDPIN_PINMODE;
  // POWERPIN_PINMODE;
  // BUZZERPIN_PINMODE;
  // STABLEPIN_PINMODE;
  // POWERPIN_OFF;
  // initOutput();
  // readEEPROM();
  // checkFirstTime();
  // configureReceiver();
  // initSensors();
  sysInfo.previousTime = micros();
  sysInfo.calibratingG = 400;

}


void writeMotors()
{
	Pru.Output1 = sysInfo.motor[0]*200;
	Pru.Output2 = sysInfo.motor[1]*200;
	Pru.Output3 = sysInfo.motor[2]*200;
	Pru.Output4 = sysInfo.motor[3]*200;
	Pru.UpdateOutput();
}



 // #define PIDMIX(X,Y,Z) rcCommand[THROTTLE] + axisPID[ROLL]*X*20 + axisPID[PITCH]*Y*20 + YAW_DIRECTION * axisPID[YAW]*Z
int PIDMIX(int X, int Y, int Z)
{
	return sysInfo.rcCommand[THROTTLE] + sysInfo.axisPID[ROLL]*X + sysInfo.axisPID[PITCH]*Y + YAW_DIRECTION * sysInfo.axisPID[YAW]*Z;
}
  
void mixTable() 
{
	sysInfo.motor[0] = PIDMIX(-1,+1,-1); //REAR_R
	sysInfo.motor[1] = PIDMIX(-1,-1,+1); //FRONT_R
	sysInfo.motor[2] = PIDMIX(+1,+1,+1); //REAR_L
	sysInfo.motor[3] = PIDMIX(+1,-1,-1); //FRONT_L
	//cout << "throttle " << rcCommand[THROTTLE] << " " << axisPID[ROLL] << " " << axisPID[PITCH]<< " " << YAW_DIRECTION * axisPID[YAW] << endl;
	int maxMotor=sysInfo.motor[0];
	for(int i=1;i< NUMBER_MOTOR;i++)
	{
		if (sysInfo.motor[i]>maxMotor) 
		{	
			maxMotor=sysInfo.motor[i];
		}
	}
	for (int i = 0; i < NUMBER_MOTOR; i++) 
	{
		if (maxMotor > MAXTHROTTLE) // this is a way to still have good gyro corrections if at least one motor reaches its max.
		{
			sysInfo.motor[i] -= maxMotor - MAXTHROTTLE;
		}
		sysInfo.motor[i] = constrain(sysInfo.motor[i], MINTHROTTLE, MAXTHROTTLE);    
		if ((sysInfo.rcData[THROTTLE]) < MINCHECK)
		{
			#ifndef MOTOR_STOP
			sysInfo.motor[i] = MINTHROTTLE;
			#else
			sysInfo.motor[i] = MINCOMMAND;
			#endif
		}
		if (sysInfo.armed == 0)
		{
			sysInfo.motor[i] = MINCOMMAND;
		}
	}
}


void annexCode() { //this code is excetuted at each loop and won't interfere with control loop if it lasts less than 650 microseconds
  static uint32_t buzzerTime,calibratedAccTime;
  static char  buzzerFreq;         //delay between buzzer ring
  uint32_t axis,prop1,prop2;

  //PITCH & ROLL only dynamic PID adjustemnt,  depending on throttle value
  if      (sysInfo.rcData[THROTTLE]<1500) prop2 = 100;
  else if (sysInfo.rcData[THROTTLE]<2000) prop2 = 100 - (uint32_t)conf.dynThrPID*(sysInfo.rcData[THROTTLE]-1500)/500;
  else                            prop2 = 100 - conf.dynThrPID;
	
  for(axis=0;axis<3;axis++) {
    uint32_t tmp = min(fabs(sysInfo.rcData[axis]-MIDRC),500);
    if(axis!=2) { //ROLL & PITCH
      uint32_t tmp2 = tmp/100;
      sysInfo.rcCommand[axis] = sysInfo.lookupRX[tmp2] + (tmp-tmp2*100) * (sysInfo.lookupRX[tmp2+1]-sysInfo.lookupRX[tmp2]) / 100;
      prop1 = 100-(uint32_t)conf.rollPitchRate*tmp/500;
      prop1 = (uint32_t)prop1*prop2/100;
    } 
	else 
	{ //YAW
      sysInfo.rcCommand[axis] = tmp;
      prop1 = 100-(uint32_t)conf.yawRate*tmp/500;
    }
    sysInfo.dynP8[axis] = (uint32_t)conf.P8[axis]*prop1/100;
    sysInfo.dynD8[axis] = (uint32_t)conf.D8[axis]*prop1/100;
    if (sysInfo.rcData[axis]<MIDRC) sysInfo.rcCommand[axis] = -sysInfo.rcCommand[axis];
  }
  sysInfo.rcCommand[THROTTLE] = MINTHROTTLE + (MAXTHROTTLE-MINTHROTTLE)* (sysInfo.rcData[THROTTLE]-MINCHECK)/(2000-MINCHECK);
  if ( (sysInfo.calibratingA>0 && ACC )  || (sysInfo.calibratingG>0) ) {  // Calibration phasis
    //LEDPIN_TOGGLE;
  } else {
    if (f.ACC_CALIBRATED) {
	//LEDPIN_OFF;
	}
    //if (armed) {LEDPIN_ON;}
  }

  if ( sysInfo.currentTime > sysInfo.calibratedAccTime ) {
    if (! f.SMALL_ANGLES_25) {
      // the multi uses ACC and is not calibrated or is too much inclinated
      f.ACC_CALIBRATED = 0;
      sysInfo.calibratedAccTime = sysInfo.currentTime + 500000;
    } else {
      f.ACC_CALIBRATED = 1;
    }
  }
}



// ******** Main Loop *********
void loop () 
{
	static uint32_t rcDelayCommand; // this indicates the number of time (multiple of RC measurement at 50Hz) the sticks must be maintained to run or switch off motors
	uint32_t axis,i;
	long error,errorAngle;
	long delta,deltaSum;
	long PTerm,ITerm,DTerm;
	static long lastGyro[3] = {0,0,0};
	static long delta1[3],delta2[3];
	static long errorGyroI[3] = {0,0,0};
	static long errorAngleI[2] = {0,0};
	static uint32_t rcTime  = 0;
	static long ConsoleOutputTime = 0;
	static long initialThrottleHold;
	long AltPID = 0;
	static long lastVelError = 0;

	//annexCode();
 
	sensor.Gyro.UpdateData();
	
	sensor.Acc.UpdateData();
	if(Pru.UpdateInput())
	{
		// #define ROLL       0
		// #define PITCH      1
		// #define YAW        2
		// #define THROTTLE   3
		// #define AUX1       4
		// #define AUX2       5
		// #define AUX3       6
		// #define AUX4       7	
		sysInfo.rcData[0] = Pru.Input2;
		sysInfo.rcData[1] = Pru.Input1;  //4
		sysInfo.rcData[2] = Pru.Input4; // 1
		sysInfo.rcData[3] = Pru.Input3;	//2
		sysInfo.rcData[4] = Pru.Input5;
		sysInfo.rcData[5] = Pru.Input6;
		
		// sysInfo.rcData[0] = Pru.Input3;
		// sysInfo.rcData[1] = Pru.Input4;  //4
		// sysInfo.rcData[2] = Pru.Input1; // 1
		// sysInfo.rcData[3] = Pru.Input2;	//2
		//cout << "C1: " << rcData[0]<< " C2:" << rcData[1] << " C3:" << rcData[2] << " C4:" << rcData[3] << endl;
	}
	else
	{
		cout << "Pru failed update" << endl;
	}

	
	if (sysInfo.currentTime > rcTime ) 
	{ // 50Hz
		rcTime = sysInfo.currentTime + 30;
		// Failsafe routine - added by MIS
		// end of failsave routine - next change is made with RcOptions setting
		if (sysInfo.rcData[THROTTLE] < MINCHECK) 
		{
				//cout<< "rcDelayCommand " << rcDelayCommand << endl;
			  errorGyroI[ROLL] = 0; errorGyroI[PITCH] = 0; errorGyroI[YAW] = 0;
			  errorAngleI[ROLL] = 0; errorAngleI[PITCH] = 0;
			  rcDelayCommand++;
			 if (sysInfo.rcData[YAW] < MINCHECK && sysInfo.rcData[PITCH] < MINCHECK && sysInfo.armed == 0) 
			 {
				if (rcDelayCommand == 20) sysInfo.calibratingG=400;
			 } 
			 else if (sysInfo.rcData[YAW] > MAXCHECK && sysInfo.rcData[PITCH] > MAXCHECK && sysInfo.armed == 0) 
			 {
				if (rcDelayCommand == 20) 
				{
				  //writeServos();
				  sysInfo.previousTime = micros();
				}
			 }
			 else if ( (sysInfo.rcData[YAW] < MINCHECK || sysInfo.rcData[ROLL] < MINCHECK)  && sysInfo.armed == 1) 
			 {
				if (rcDelayCommand == 20) 
				{
					cout << "unarmed" << endl;
					sysInfo.armed = 0; // rcDelayCommand = 20 => 20x20ms = 0.4s = time to wait for a specific RC command to be acknowledged
				}
			 } 
			 else if ( (sysInfo.rcData[YAW] > MAXCHECK || sysInfo.rcData[ROLL] > MAXCHECK) && sysInfo.rcData[PITCH] < MAXCHECK && sysInfo.armed == 0 ) 
			 {
				if (rcDelayCommand == 20) 
				{
				  sysInfo.armed = 1;
				  cout << "armed" << endl;
				  sysInfo.headFreeModeHold = sysInfo.heading;
				}
			} 
			else
			{
				rcDelayCommand = 0;
			}
		} 
		else if (sysInfo.rcData[THROTTLE] > MAXCHECK && !f.ARMED) 
		{
		  if (sysInfo.rcData[YAW] < MINCHECK && sysInfo.rcData[PITCH] < MINCHECK) {  //throttle=max, yaw=left, pitch=min
			if (rcDelayCommand == 20) sysInfo.calibratingA=400;
			rcDelayCommand++;
		  } else if (sysInfo.rcData[YAW] > MAXCHECK && sysInfo.rcData[PITCH] < MINCHECK) { //throttle=max, yaw=right, pitch=min  
			if (rcDelayCommand == 20) f.CALIBRATE_MAG = 1;; // MAG calibration request
			rcDelayCommand++;
		  } else if (sysInfo.rcData[PITCH] > MAXCHECK) {
			 conf.angleTrim[PITCH]+=2;
			 //writeParams();
		  } else if (sysInfo.rcData[PITCH] < MINCHECK) {
			 conf.angleTrim[PITCH]-=2;
			 //writeParams();
		  } else {
			rcDelayCommand = 0;
		  }
		}

		uint16_t auxState = 0;
		for(i=0;i<4;i++)
		  auxState |= (sysInfo.rcData[AUX1+i]<1300)<<(3*i) | (1300<sysInfo.rcData[AUX1+i] && sysInfo.rcData[AUX1+i]<1700)<<(3*i+1) | (sysInfo.rcData[AUX1+i]>1700)<<(3*i+2);
		for(i=0;i<CHECKBOXITEMS;i++)
		  sysInfo.rcOptions[i] = (auxState & conf.activate[i])>0;

		// note: if FAILSAFE is disable, failsafeCnt > 5*FAILSAVE_DELAY is always false
		if (( sysInfo.rcOptions[BOXACC] || (sysInfo.failsafeCnt > 5*FAILSAVE_DELAY) ) && ACC ) { 
		  // bumpless transfer to Level mode
		  if (!f.ACC_MODE) {
			errorAngleI[ROLL] = 0; errorAngleI[PITCH] = 0;
			f.ACC_MODE = 1;
		  }  
		} else {
		  // failsafe support
		  f.ACC_MODE = 0;
		}		

    #if MAG
      if (sysInfo.rcOptions[BOXMAG]) {
        if (!f.MAG_MODE) {
          f.MAG_MODE = 1;
          sysInfo.magHold = sysInfo.heading;
        }
      } else {
        f.MAG_MODE = 0;
      }
      if (sysInfo.rcOptions[BOXHEADFREE]) {
        if (!f.HEADFREE_MODE) {
          f.HEADFREE_MODE = 1;
        }
      } else {
        f.HEADFREE_MODE = 0;
      }
      if (sysInfo.rcOptions[BOXHEADADJ]) {
        sysInfo.headFreeModeHold = sysInfo.heading; // acquire new heading
      }
    #endif		

	
	} 

	//computeIMU();
	    annexCode();
	// // Measure loop rate just afer reading the sensors
	sysInfo.currentTime = micros();
	sysInfo.cycleTime = sysInfo.currentTime - sysInfo.previousTime;
	sysInfo.previousTime = sysInfo.currentTime;


  #if MAG
    if (abs(sysInfo.rcCommand[YAW]) <70 && f.MAG_MODE) {
      int16_t dif = sysInfo.heading - sysInfo.magHold;
      if (dif <= - 180) dif += 360;
      if (dif >= + 180) dif -= 360;
      if ( f.SMALL_ANGLES_25 ) sysInfo.rcCommand[YAW] -= dif*conf.P8[PIDMAG]/30;  // 18 deg
    } else sysInfo.magHold = sysInfo.heading;
  #endif	
	

	//**** PITCH & ROLL & YAW PID ****    
	for(axis=0;axis<3;axis++) 
	{
		if (f.ACC_MODE && axis<2 ) { //LEVEL MODE
		  // 50 degrees max inclination
		  errorAngle = constrain(2*sysInfo.rcCommand[axis] + gpsVars.GPS_angle[axis],-500,+500) - sysInfo.angle[axis] + conf.angleTrim[axis]; //16 bits is ok here
		  #ifdef LEVEL_PDF
			PTerm      = -(int32_t)angle[axis]*conf.P8[PIDLEVEL]/100 ;
		  #else  
			PTerm      = (int32_t)errorAngle*conf.P8[PIDLEVEL]/100 ;                          // 32 bits is needed for calculation: errorAngle*P8[PIDLEVEL] could exceed 32768   16 bits is ok for result
		  #endif
		  PTerm = constrain(PTerm,-conf.D8[PIDLEVEL]*5,+conf.D8[PIDLEVEL]*5);

		  errorAngleI[axis]  = constrain(errorAngleI[axis]+errorAngle,-10000,+10000);    // WindUp     //16 bits is ok here
		  ITerm              = ((int32_t)errorAngleI[axis]*conf.I8[PIDLEVEL])>>12;            // 32 bits is needed for calculation:10000*I8 could exceed 32768   16 bits is ok for result
		}
		else	
		{ //ACRO MODE or YAW axis
		  if (abs(sysInfo.rcCommand[axis])<350) error =          sysInfo.rcCommand[axis]*10*8/conf.P8[axis] ; //16 bits is needed for calculation: 350*10*8 = 28000      16 bits is ok for result if P8>2 (P>0.2)
								   else error = (int32_t)sysInfo.rcCommand[axis]*10*8/conf.P8[axis] ; //32 bits is needed for calculation: 500*5*10*8 = 200000   16 bits is ok for result if P8>2 (P>0.2)
		  error -= sysInfo.gyroData[axis];

		  PTerm = sysInfo.rcCommand[axis];
		  
		  errorGyroI[axis]  = constrain(errorGyroI[axis]+error,-4000,+4000);          //WindUp //16 bits is ok here
		  if (abs(sysInfo.gyroData[axis])>640) errorGyroI[axis] = 0;
		  ITerm = (errorGyroI[axis]/125*conf.I8[axis])>>6;                                   //16 bits is ok here 16000/125 = 128 ; 128*250 = 32000
		}

		if (abs(sysInfo.gyroData[axis])<160) 
		{
			PTerm -=          sysInfo.gyroData[axis]*sysInfo.dynP8[axis]/10/8; //16 bits is needed for calculation   160*200 = 32000         16 bits is ok for result
		}
		else 
		{
			PTerm -= (int32_t)sysInfo.gyroData[axis]*sysInfo.dynP8[axis]/10/8; //32 bits is needed for calculation   
		}
		delta          = sysInfo.gyroData[axis] - lastGyro[axis];                               //16 bits is ok here, the dif between 2 consecutive gyro reads is limited to 800
		lastGyro[axis] = sysInfo.gyroData[axis];
		deltaSum       = delta1[axis]+delta2[axis]+delta;
		delta2[axis]   = delta1[axis];
		delta1[axis]   = delta;

		if (abs(deltaSum)<640) 
		{	
			DTerm = (deltaSum*sysInfo.dynD8[axis])>>5;                       //16 bits is needed for calculation 640*50 = 32000           16 bits is ok for result 
		}
		else 
		{
			DTerm = ((int32_t)deltaSum*sysInfo.dynD8[axis])>>5;              //32 bits is needed for calculation
		}			  

		//axisPID[axis] =  PTerm/3 - DTerm;
		//cout << (axisPID[axis])<< " " << "D " << DTerm << " I " << ITerm << " : ";
		//DTerm = DTerm / 8;
		//DTerm = 0;
		//ITerm = 0;
		sysInfo.axisPID[axis] =  PTerm + ITerm - DTerm;
	}
	//axisPID[2] -= 500;
	//cout << endl;

	mixTable();
	if (sysInfo.currentTime > rcTime ) 
	{
		//cout << "dump data to serial port " << endl;
		//DumpData();
	}

	if(sysInfo.currentTime > ConsoleOutputTime)
	{
		static unsigned long consoleOutputCount = 0;
		cout << consoleOutputCount << " : Cycle Time " << sysInfo.cycleTime <<" " <<  sysInfo.currentTime << " " <<sysInfo.framerate;
			cout << "Gyro Read " << sysInfo.gyroRead<<  " Gyro Fail " << sysInfo.gyroFail << endl ;
			cout << "Acc Read " << sysInfo.AccRead << " Acc Fail " << sysInfo.AccFail << endl;
		ConsoleOutputTime = sysInfo.currentTime + 3000;	// output per second
		consoleOutputCount ++;
		sysInfo.gyroRead = 0;
		sysInfo.gyroFail = 0;
		sysInfo.AccRead = 0;
		sysInfo.AccFail = 0;
		sysInfo.framerate = 0;
		cout << sysInfo.rcData[0]<<endl;
	}
	sysInfo.framerate ++;
	//cout << currentTime << endl;
	// writeServos();
	writeMotors();
	usleep(5000);
	
}



#define ssin(val) (val)
#define scos(val) 1.0f

typedef struct fp_vector {
  float X;
  float Y;
  float Z;
} t_fp_vector_def;

typedef union {
  float   A[3];
  t_fp_vector_def V;
} t_fp_vector;


  #define GYRO_SCALE ((2380 * PI)/((32767.0f / 4.0f ) * 180.0f * 1000000.0f)) //should be 2279.44 but 2380 gives better result

 #define PI 3.1415926
  
int16_t _atan2(float y, float x){
  #define fp_is_neg(val) ((((uint8_t*)&val)[3] & 0x80) != 0)
  float z = y / x;
  int16_t zi = abs(int16_t(z * 100)); 
  int8_t y_neg = fp_is_neg(y);
  if ( zi < 100 ){
    if (zi > 10) 
     z = z / (1.0f + 0.28f * z * z);
   if (fp_is_neg(x)) {
     if (y_neg) z -= PI;
     else z += PI;
   }
  } else {
   z = (PI / 2.0f) - z / (z * z + 0.28f);
   if (y_neg) z -= PI;
  }
  z *= (180.0f / PI * 10); 
  return z;
}

// Rotate Estimated vector(s) with small angle approximation, according to the gyro data
void rotateV(struct fp_vector *v,float* delta) {
  fp_vector v_tmp = *v;
  v->Z -= delta[ROLL]  * v_tmp.X + delta[PITCH] * v_tmp.Y;
  v->X += delta[ROLL]  * v_tmp.Z - delta[YAW]   * v_tmp.Y;
  v->Y += delta[PITCH] * v_tmp.Z + delta[YAW]   * v_tmp.X; 
}

  #define ACC_LPF_FACTOR 100
/* Set the Gyro Weight for Gyro/Acc complementary filter */
/* Increasing this value would reduce and delay Acc influence on the output of the filter*/
#ifndef GYR_CMPF_FACTOR
  #define GYR_CMPF_FACTOR 400.0f
#endif
  
 #define INV_GYR_CMPF_FACTOR   (1.0f / (GYR_CMPF_FACTOR  + 1.0f))
 
void getEstimatedAttitude(){
  uint8_t axis;
  int32_t accMag = 0;
  static t_fp_vector EstG;
  static float accLPF[3];
  static uint16_t previousT;
  uint16_t currentT = micros();
  float scale, deltaGyroAngle[3];

  scale = (currentT - previousT) * GYRO_SCALE;
  previousT = currentT;

  // Initialization
  for (axis = 0; axis < 3; axis++) {
    deltaGyroAngle[axis] = sysInfo.gyroADC[axis]  * scale;
      accLPF[axis] = accLPF[axis] * (1.0f - (1.0f/ACC_LPF_FACTOR)) + sysInfo.accADC[axis] * (1.0f/ACC_LPF_FACTOR);
      sysInfo.accSmooth[axis] = accLPF[axis];
      #define ACC_VALUE 	sysInfo.accSmooth[axis]
    accMag += (int32_t)ACC_VALUE*ACC_VALUE ;
  }
  accMag = accMag*100/((int32_t)sysInfo.acc_1G*sysInfo.acc_1G);

  rotateV(&EstG.V,deltaGyroAngle);

  if ( abs(sysInfo.accSmooth[ROLL])<sysInfo.acc_25deg && abs(sysInfo.accSmooth[PITCH])<sysInfo.acc_25deg && sysInfo.accSmooth[YAW]>0) {
    f.SMALL_ANGLES_25 = 1;
  } else {
    f.SMALL_ANGLES_25 = 0;
  }

  // Apply complimentary filter (Gyro drift correction)
  // If accel magnitude >1.4G or <0.6G and ACC vector outside of the limit range => we neutralize the effect of accelerometers in the angle estimation.
  // To do that, we just skip filter, as EstV already rotated by Gyro
  if ( ( 36 < accMag && accMag < 196 ) || f.SMALL_ANGLES_25 )
    for (axis = 0; axis < 3; axis++) {
      int16_t acc = ACC_VALUE;
      EstG.A[axis] = (EstG.A[axis] * GYR_CMPF_FACTOR + acc) * INV_GYR_CMPF_FACTOR;
    }
  
  // Attitude of the estimated vector
  sysInfo.angle[ROLL]  =  atan2(EstG.V.X , EstG.V.Z) ;
  sysInfo.angle[PITCH] =  _atan2(EstG.V.Y , EstG.V.Z) ;

}

void *SensorThread( void * )
{
	printf("Gyro Thread start\n");
	sensor.Calibrate();
	int gyroADCp[3] = {0,0,0};
	int gyroADCinter[3] = {0,0,0};
	int gyroADCprevious[3] = {0,0,0};
	int i = 0;
	int j = 0;
	while(true)
	{
		int temp[3];
		i ++;
		j ++;
		if(sensor.Gyro.UpdateData())
		{
			temp[0] = sensor.Gyro.x;
			temp[1] = sensor.Gyro.y;
			temp[2] = sensor.Gyro.z;
			int axis = 0;
			for(axis = 0; axis < 3; axis ++)
			{
				temp[axis] = constrain(temp[axis],gyroADCp[axis]-800,gyroADCp[axis]+800);
				gyroADCinter[axis] =  temp[axis]+gyroADCp[axis];
				gyroADCp[axis] = temp[axis];
				sysInfo.gyroData[axis] = (gyroADCinter[axis]+gyroADCprevious[axis])/3;
				gyroADCprevious[axis] = gyroADCinter[axis]/2;
			}
			sysInfo.gyroRead ++;
		}
		else
		{
			sysInfo.gyroFail ++;
		}
		if( i > 10)
		{
			if(sensor.Acc.UpdateData())
			{
				sysInfo.AccRead ++;
				sysInfo.accADC[0] = sensor.Acc.x;
				sysInfo.accADC[1] = sensor.Acc.y;
				sysInfo.accADC[2] = sensor.Acc.z;
				getEstimatedAttitude();
			}
			else
			{
				sysInfo.AccFail ++;
			}
			i = 0;
		}
		if(j > 200)
		{
			printf("read 200 data\n");
			j = 0;
		}
		usleep(2500);
		//DumpData();
	}
	printf("Gyro Thread exit\n");
}

void * DumpDataThread(void *)
{
	printf("Dump Data Thread start\n");
	MemoryStream s(sysInfo, f, conf);
	s.serialCom();
	printf("Dump Data Thread exit\n");
}

// *******************************************************
// Interrupt driven UART transmitter - using a ring buffer
// *******************************************************


int main(int argc, char ** args)
{
	cout<<"start"<<endl;	
	
	cout << "setup " << endl;
	//	Pru.DisablePru(); 	// disable pru and keep programming running for debug
	if(Pru.Init())
	{
		cout << "Pru init success." << endl;
	}
	else
	{
		cout << "Pru init failed." << endl;
	}
    	
	sensor.Init();
	sensor.Calibrate();

	initSysVari(sysInfo);
	InitGpsVars(gpsVars);
	setup();

	//sensor.Calibrate();
    pthread_t thread1, thread2;
//    pthread_create( &thread1, NULL, SensorThread, NULL );
    pthread_create( &thread2, NULL, DumpDataThread, NULL );
	while(1)
	{
		loop();
		usleep(1);
	}
	//loop();
	
	return 0;
}
