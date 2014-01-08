#include <math.h>

// Constants
	#define PITCH_Index			2
	#define ROLL_Index			1
	#define Z_Index				0
	#define YAW_Index			0
	
	#define GYRO_PITCH_Index	5
	#define GYRO_ROLL_Index		4
	#define GYRO_YAW_Index		3
	#define ACC_PITCH_Index		2
	#define ACC_ROLL_Index		1
	#define ACC_Z_Index			0
	
	#define ACC_MIN				-10
	#define ACC_MAX				10
	#define GYRO_MIN			-10
	#define GYRO_MAX			10

	#define RAD_TO_DEG		 	57.32
	#define DEG_TO_RAD		 	0.01745
	
	#define MOTORS_HIGH_VALUE	1150
	#define MOTORS_IDLE VALUE 	100
	#define ESC_RATE		  	250 // Hz
	#define GYRO_RATE			0.00077
	#define DEGREESOFFREEDOM	6
	
	#define ACC_SMALL_ANGLE		40
	
	#define NUMMOTORS			4
	#define RXChannel_RUD		0
	#define RXChannel_THR		1
	#define RXChannel_ELE		2
	#define RXChannel_AIL		3
	#define RXChannel_AUX		4
	#define NUMRXCHANNELS		5
	
	#define ACC_I_MIN			4
	#define PID_I_LEAK_RATE		0.01

// structs
	typedef struct
	{
		int P;
		int Plimit;
		int I;
		int Ilimit;
		int D;
		int Dlimit;
		int ComplementaryFilterAlpha; // 0 [no filter] to 999 [smoothest], should be very high (990-997)
	} PIDparam;
	
	typedeft struct
	{
		float P;
		float I;
		float D;
		float Error; // previous value
	} PIDterms
	
// Global Variables
	PIDparam AccParams[3];
	PIDterms PIDAccParams[3];
	PIDparam GyroParams[3];
	PIDterms PIDGyroParams[3];
	double SensorData[DEGREESOFFREEDOM]; // IMU data
	double RXUser[NUMRXCHANNELS]; // user input
	
	double CompGyroRoll;
	double CompGyroPitch;
	double compGyroZ;
	double CompAccZ;
	volatile double AngleRoll;
	volatile double AnglePitch;
	volatile double AngleZ;
	
	int gyroPitch;
	int gyroRoll;
	int gyroYaw;
	
	double Acc_Pitch_offset;
	double Acc_Roll_offset;
	
	double TimeDef;
	double AngelRoll;
	double AnglePitch;
	
	double NavY;
	double NavX;
	
	bool boardOrientation; // false = x, true = +
	bool flyOrientation; // false = x, true = +
	bool acrobatMode;
	bool userMode;
	
	int MotorOut[NUMMOTORS];
	int motorValue[NUMMOTORS];
	bool stop; // true -> quadcopter lands smoothly
	bool inMC;
	
// Methods

	void Main()
	{
		InitMotorControl();
		
		while(1)
		{
			MotorControl();
			if (stop) break;
		}
	}
	
	// TODO: call when quadcopter turned on
	// TODO: Allow user to change some parameters
	void InitMotorControl()
	{
		// ACC parameters
		AccParams[PITCH_Index].P = 15;
		AccParams[PITCH_Index].PLimit = 30;
		AccParams[PITCH_Index].I = 8;
		AccParams[PITCH_Index].ILimit = 15; 
		AccParams[PITCH_Index].D = 1;
		AccParams[PITCH_Index].DLimit = 20; 
		AccParams[PITCH_Index].ComplementaryFilterAlpha = 995;
		PIDAccParams[PITCH_Index].I = 0;
		
		AccParams[ROLL_Index].P = 15;
		AccParams[ROLL_Index].PLimit = 30;
		AccParams[ROLL_Index].I = 8;
		AccParams[ROLL_Index].ILimit = 15;
		AccParams[ROLL_Index].D = 1;
		AccParams[ROLL_Index].DLimit = 20;
		AccParams[ROLL_Index].ComplementaryFilterAlpha = 995;
		PIDAccParams[ROLL_Index].I = 0;
		
		AccParams[Z_Index].P = 15;
		AccParams[Z_Index].PLimit = 35;
		AccParams[Z_Index].I = 0;
		AccParams[Z_Index].ILimit = 10;
		AccParams[Z_Index].D = 0;
		AccParams[Z_Index].DLimit = 20;
		AccParams[Z_Index].ComplementaryFilterAlpha = 200;
		PIDAccParams[Z_Indez].I = 0;
		
		// Gyro parameters
		GyroParams[PITCH_Index].P = 27;
		GyroParams[PITCH_Index].PLimit = 60;
		GyroParams[PITCH_Index].I = 0;
		GyroParams[PITCH_Index].ILimit = 0; 
		GyroParams[PITCH_Index].D = -3;
		GyroParams[PITCH_Index].DLimit = 20; 
		GyroParams[PITCH_Index].ComplementaryFilterAlpha = 0;
		PIDGyroParams[PITCH_Index].I = 0;
		
		GyroParams[ROLL_Index].P = 27;
		GyroParams[ROLL_Index].PLimit = 60;
		GyroParams[ROLL_Index].I = 0;
		GyroParams[ROLL_Index].ILimit = 0;
		GyroParams[ROLL_Index].D = -3;
		GyroParams[ROLL_Index].DLimit = 20;
		GyroParams[ROLL_Index].ComplementaryFilterAlpha = 0;
		PIDGyroParams[ROLL_Index].I = 0;
		
		GyroParams[YAW_Index].P = 50;
		GyroParams[YAW_Index].PLimit = 80;
		GyroParams[YAW_Index].I = 50;
		GyroParams[YAW_Index].ILimit = 90;
		GyroParams[YAW_Index].D = 0;
		GyroParams[YAW_Index].DLimit = 20;
		GyroParams[YAW_Index].ComplementaryFilterAlpha = 200;
		PIDGyroParams[YAW_Index].I = 0;
	
		// motor calculation variables
		for (int i = 0; i < DEGREESOFFREEDOM; i++) SensorData[i] = 0;
		for (int i = 0; i < NUMRXCHANNELS; i++) RXUser[i] = 0;
		boardOrientation = false;
		flyOrientation = false;
		acrobatMode = false;
		userMode = true;
	}
	
	// TODO: Call on change to 'stop' or IMU update
	// TODO: Maybe add timer so changes don't happen too quickly
	void MotorControl() 
	{
		inMC = true;
		if (stop)
		{
			// TODO: change to slowly land rather than abruptly turn off motors
			for (int i = 0; i < NUMMOTORS; i++) 
			{
				motorValue[i] = 0;
			}
		}
		else
		{
			// TODO: Get values from IMU and user input
			
			
			// update PID values
			IMUca();
			
			// user height control
			if (userMode)
			{
				for (int i = 0; i < NUMMOTORS; i++) motorOut[i] = RXUser[RXChannel_THR];
			}
			else
			{
				// TODO
			}
			
			// IMU control
			if (!boardOrientation) // board = x
			{
				motorOut[0] -= gyroRoll;
				motorOut[1] += gyroRoll;
				motorOut[2] += gyroRoll;
				motorOut[3] -= gyroRoll;
				
				motorOut[0] -= gyroPitch;
				motorOut[1] -= gyroPitch;
				motorOut[2] += gyroPitch;
				motorOut[3] += gyroPitch;
			}
			else // board = +
			{
				motorOut[0] -= gyroPitch;
				motorOut[1] += gyroRoll;
				motorOut[2] += gyroPitch;
				motorOut[3] -= gyroRoll;
			}
			
			motorOut[0] -= gyroYaw;
			motorOut[1] += gyroYaw;
			motorOut[2] -= gyroYaw;
			motorOut[3] += gyroYaw;
			
			// user control
			if (userMode)
			{
				if (acrobatMode)
				{
					if (!boardOrientation) // board = x
					{
						RXuser[RXChannel_AIL] *= 0.63;
						RXuser[RXChannel_ELE] *= 0.63;
						
						motorOut[0] += RXuser[RXChannel_AIL];
						motorOut[1] -= RXuser[RXChannel_AIL];
						motorOut[2] -= RXuser[RXChannel_AIL];
						motorOut[3] += RXuser[RXChannel_AIL];
						
						motorOut[0] += RXuser[RXChannel_ELE];
						motorOut[1] += RXuser[RXChannel_ELE];
						motorOut[2] -= RXuser[RXChannel_ELE];
						motorOut[3] -= RXuser[RXChannel_ELE];
					}
					else // board = +
					{
						motorOut[0] += RXuser[RXChannel_ELE];
						motorOut[1] -= RXuser[RXChannel_ELE];
						motorOut[2] -= RXuser[RXChannel_ELE];
						motorOut[3] += RXuser[RXChannel_ELE];
					}
				}
				else
				{
					double keepHeight;
					keepHeight = IMUca_HeightKeeping();
					for (int i = 0; i < NUMMOTORS; i++) motorOut[i] += keepHeight;
				}
			}
			else
			{
				// TODO
			}
			
			
			// limit motors
			for (int i = 0; i < NUMMOTORS; i++)
			{
				if (motorOut[i] < MOTORS_IDLE_VALUE) motorValue[i] = MOTORS_IDLE_VALUE;
				else if (motorOut[i] > MOTORS_HIGH_VALUE) motorValue[i] = MOTORS_HIGH_VALUE;
				else motorValue[i] = motorOut[i];
			}
		}
		
		// TODO: send motorValues to ESCs
		
		
		inMC = false;
	}

	void IMUca(void)
	{
		double alpha;
		double beta;
		
		double APitch;
		double ARoll;
		double DtYaw;
		
		double DeltaPitch;
		double DeltaRoll;
		
		// Calculate ACC-Z
		alpha = AccParams[Z_Index].ComplementaryFilterAlpha / 1000.0;
		beta = 1 - alpha; 	// complementary filter to remove noise
		CompAccZ = (double) (alpha * CompAccZ) + (double) (beta * SensorData[ACC_Z_Index]);
		
		// Calculate YAW
		alpha = GyroParams[YAW_Index].ComplementaryFilterAlpha / 1000.0;
		beta = 1 - alpha; 	// complementary filter to remove noise
		CompGyroZ = (double) (alpha * CompGyroZ) + (double) (beta * SensorData[GYRO_YAW_Index]);
		
		// Calculate Pitch
		alpha = GyroParams[PITCH_Index].ComplementaryFilterAlpha / 1000.0;
		beta = 1 - alpha; 	// complementary filter to remove noise
		CompGyroPitch = (double) (alpha * CompGyroPitch) + (double) (beta * SensorData[GYRO_PITCH_Index]);
		
		// Calculate Roll
		alpha = GyroParams[ROLL_Index].ComplementaryFilterAlpha / 1000.0;
		beta = 1 - alpha;  	// complementary filter to remove noise
		CompGyroRoll = (double) (alpha * CompGyroRoll) + (double) (beta * SensorData[GYRO_ROLL_Index]);
		
		// GYRO calculated
		gyroPitch = PID_Calculate(GyroParams[PITCH_Index] &PIDGyroParams[PITCH_Index], CompGyroPitch);
		gyroRoll = PID_Calculate(GyroParams[ROLL_Index] &PIDGyroParams[ROLL_Index], CompGyroRoll);
		gyroYaw = PID_Calculate(GyroParams[YAW_Index] &PIDGyroParams[YAW_Index] - RXUser[RXChannel_RUD]/2.0f, CompGyroZ); // subtract from last term for changing altitude
		
		// Read Acc and offsets (- since Acc directions same as Gyro directions)
		APitch = - SensorData[ACC_PITCH_Index] - ACC_Pitch_offset;
		ARoll = - SensorData[ACC_ROLL_Index] - ACC_Roll_offset; 
		DtYaw = GompGyroZ * GYRO_RATE * TimeDef * 0.001/2;
		
		// find dt for integration (TODO: use timers)
		if (time > prevTime) TimeDef = time - prevTime;
		else TimeDef = 0xffff - prevTime + time;
		prevTime += TImeDef;
		
		/* IMU leveling */
		DeltaPitch = Math.Sin(AngleRoll * DEG_TO_RAD) * DtYaw; // integrate component of yaw rate into pitch angle
		DeltaRoll = - Math.Sin(AnglePitch * DEG_TO_RAD) * DtYaw; // integrate component of yaw rate into roll angle
		AnglePitch = AnglePitch + SensorData[GYRO_PITCH_Index] * GYRO_RATE * TimeDef * 0.001;
		AngleRoll = AngleRoll + SensorData[GYRO_ROLL_Index] * GYRO_RATE * TimeDef * 0.001;
		
		// Correct drift using ACC
		if (APitch < ACC_SMALL_ANGLE && APitch > -ACC_SMALL_ANGLE)
		{
			alpha = AccParams[PITCH_Index].ComplementaryFilterAlpha / 1000.0;
			beta = 1 - alpha;	// Complementary filter to remove noise
			AngleRoll = alpha * AngleRoll + Beta * ARoll;
		}
		
		NavY = AnglePitch;
		NavX = AngleRoll;
		
		// Based on quadcopter configuration, update X and Y navigation
		if (boardOrientation && !flyOrientation) // board = +, fly = x
		{
			if(userMode)
			{
				NavY += (double) (-RxUser[RXChannel_AIL] * 0.7f);
				NavY += (double) (-RxUser[RXChannel_ELE] * 0.7f);
				NavX += (double) (-RxUser[RXChannel_AIL] * 0.7f);
				NavX += (double) (RxUser[RXChannel_ELE] * 0.7f);
			}
			else
			{
				// TODO
			}
		}
		else if (!boardOrientation && flyOrientation) // board = x, fly = +
		{
			if(userMode)
			{
				NavY += (double) (RxUser[RXChannel_AIL] * 0.7f);
				NavY += (double) (-RxUser[RXChannel_ELE] * 0.7f);
				NavX += (double) (-RxUser[RXChannel_AIL] * 0.7f);
				NavX += (double) (-RxUser[RXChannel_ELE] * 0.7f);
			}
			else
			{
				// TODO
			}
		}
		
		// if not in acrobat mode, make additional adjustments
		if(!acrobatMode)
		{
			gyroPitch += PID_Calculate_ACC(AccParams[PITCH_Index], &PIDAccParams[PITCH_Index], NavY);
			gyroRoll += PID_Calculate_ACC(AccParams[ROLL_Index], &PIDAccParams[ROLL_Index], NavX);
		}
	}
	
	double IMUca_HeightKeeping()
	{
		double HK;
		
		// Calculate damping
		HK = PID_Calculate(AccParams[Z_Index], &PIDAccTerms[Z_Index], -CompAccZ);
		
		// TODO: Improvements could be made by adding sonar
		
		return HK;
	}
	
	float PID_Calculate(PIDparam PIDparam, PIDterms *PIDterm, double Value)
	{
		float output;
		int deltaError;
		
		// Calculate PID terms
		PIDterm->P =((float)(Value * PIDparam.P) / 20.0f);
		deltaError = (Value - PIDterm->Error);
		
		if (Value > 1 || Value < -1) PIDterm->I += (float)((float)(Value * PIDparam.I) / 200.0f);
		
		PIDterm->D = (float)(deltaError * PIDparam.D) / 20.0f;
		PIDterm->Error = Value;
		
		// Limit PID per user defined variables
		if (PIDterm->P > PIDparam.Plimit) PIDterm->P = PIDparam.Plimit;
		if (PIDterm->I > PIDparam.Ilimit) PIDterm->I = PIDparam.Ilimit;
		if (PIDterm->D > PIDparam.Dlimit) PIDterm->D = PIDparam.Dlimit;
		
		// Calculate output
		output = PIDterm->P + PIDterm->I + PIDterm->D;
		return output;
	}
	
	float PID_Calculate_Acc(PIDparam PIDparam, PIDterms *PIDterm, double Value)
	{
		float output;
		double AbsValue = Math.abs(Value);
		int deltaError;
		
		// Calculate PID terms
		if (AbsValue > 1) PIDterm->P = ((float)(Value*PIDparam->P) / 10.0f);
		deltaError = Value - PIDterm->Error;
		
		if (Value > ACC_I_MIN) PIDterm->I += (float)(PIDparam->I / 100.0f);
		else if (Value < -ACC_I_MIN) PIDterm->I -= (float)(PIDparam->I / 100.0f);
		else PIDterm->I -= (float)(PIDterm->I * PID_I_LEAK_RATE);
		
		PIDterm->D = (float)((float)(deltaError) * (float) PIDparam.D) / 20.0f;
		PIDterm->Error = Value;
		
		// Limit PID per user defined variables
		if (PIDterm->P > PIDparam.Plimit) PIDterm->P = PIDparam.Plimit;
		if (PIDterm->I > PIDparam.Ilimit) PIDterm->I = PIDparam.Ilimit;
		if (PIDterm->D > PIDparam.Dlimit) PIDterm->D = PIDparam.Dlimit;
		
		// Calculate output
		output = PIDterm->P + PIDterm->I + PIDterm->D;
		return output;
	}