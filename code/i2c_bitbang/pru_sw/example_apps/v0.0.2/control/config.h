/*******************************/
/****CONFIGURABLE PARAMETERS****/
/*******************************/

/* Set the minimum throttle command sent to the ESC (Electronic Speed Controller)
   This is the minimum value that allow motors to run at a idle speed  */
//#define MINTHROTTLE 1300 // for Turnigy Plush ESCs 10A
//#define MINTHROTTLE 1120 // for Super Simple ESCs 10A
//#define MINTHROTTLE 1220
#define MINTHROTTLE 1080 

/* The type of multicopter */
//#define GIMBAL
//#define BI
//#define TRI
//#define QUADP
#define QUADX
//#define Y4
//#define Y6
//#define HEX6
//#define HEX6X
//#define OCTOX8
//#define OCTOFLATP
//#define OCTOFLATX
//#define FLYING_WING
//#define VTAIL4

#define YAW_DIRECTION 1 // if you want to reverse the yaw correction direction
//#define YAW_DIRECTION -1

#define I2C_SPEED 100000L     //100kHz normal mode, this value must be used for a genuine WMP
//#define I2C_SPEED 400000L   //400kHz fast mode, it works only with some WMP clones


/* This option should be uncommented if ACC Z is accurate enough when motors are running*/
/* should now be ok with BMA020 and BMA180 ACC */
#define TRUSTED_ACCZ

/* This will activate the ACC-Inflight calibration if unchecked */  
//#define InflightAccCalibration


/* Pseudo-derivative conrtroller for level mode (experimental)
   Additional information: http://www.multiwii.com/forum/viewtopic.php?f=8&t=503 */
//#define LEVEL_PDF

/* introduce a deadband around the stick center
   Must be greater than zero, comment if you dont want a deadband on roll, pitch and yaw */
//#define DEADBAND 6


/* Failsave settings - added by MIS
   Failsafe check pulse on THROTTLE channel. If the pulse is OFF (on only THROTTLE or on all channels) the failsafe procedure is initiated.
   After FAILSAVE_DELAY time of pulse absence, the level mode is on (if ACC or nunchuk is avaliable), PITCH, ROLL and YAW is centered
   and THROTTLE is set to FAILSAVE_THR0TTLE value. You must set this value to descending about 1m/s or so for best results. 
   This value is depended from your configuration, AUW and some other params. 
   Next, afrer FAILSAVE_OFF_DELAY the copter is disarmed, and motors is stopped.
   If RC pulse coming back before reached FAILSAVE_OFF_DELAY time, after the small quard time the RC control is returned to normal.
   If you use serial sum PPM, the sum converter must completly turn off the PPM SUM pusles for this FailSafe functionality.*/
#define FAILSAFE                                  // Alex: comment this line if you want to deactivate the failsafe function
#define FAILSAVE_DELAY     10                     // Guard time for failsafe activation after signal lost. 1 step = 0.1sec - 1sec in example
#define FAILSAVE_OFF_DELAY 200                    // Time for Landing before motors stop in 0.1sec. 1 step = 0.1sec - 20sec in example
#define FAILSAVE_THR0TTLE  (MINTHROTTLE + 200)    // Throttle level used for landing - may be relative to MINTHROTTLE - as in this case


/* interleaving delay in micro seconds between 2 readings WMP/NK in a WMP+NK config
   if the ACC calibration time is very long (20 or 30s), try to increase this delay up to 4000
   it is relevent only for a conf with NK */
#define INTERLEAVING_DELAY 3000

/* this is the value for the ESCs when they are not armed
   in some cases, this value must be lowered down to 900 for some specific ESCs */
#define MINCOMMAND 1000

/* this is the maximum value for the ESCs at full power
   this value can be increased up to 2000 */
#define MAXTHROTTLE 1900

/* motors will not spin when the throttle command is in low position
   this is an alternative method to stop immediately the motors */
//#define MOTOR_STOP

/* some radios have not a neutral point centered on 1500. can be changed here */
#define MIDRC 1500

#define SETTINGFILE	"QuadSetting"
