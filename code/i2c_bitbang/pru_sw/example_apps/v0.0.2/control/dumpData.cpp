// dumpData.cpp
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
#include "typedef.h"
#define BAUDRATE B115200
#define SERIALPORT "/dev/ttyO2"
using namespace std;
#include "dumpData.h"

#define INBUF_SIZE 64

#define uint8_t unsigned char
static unsigned char inBuf[INBUF_SIZE];

// Multiwii Serial Protocol 0 
#define MSP_VERSION				 0

#define MSP_IDENT                100   //out message         multitype + multiwii version + protocol version + capability variable
#define MSP_STATUS               101   //out message         cycletime & errors_count & sensor present & box activation
#define MSP_RAW_IMU              102   //out message         9 DOF
#define MSP_SERVO                103   //out message         8 servos
#define MSP_MOTOR                104   //out message         8 motors
#define MSP_RC                   105   //out message         8 rc chan
#define MSP_RAW_GPS              106   //out message         fix, numsat, lat, lon, alt, speed
#define MSP_COMP_GPS             107   //out message         distance home, direction home
#define MSP_ATTITUDE             108   //out message         2 angles 1 heading
#define MSP_ALTITUDE             109   //out message         1 altitude
#define MSP_BAT                  110   //out message         vbat, powermetersum
#define MSP_RC_TUNING            111   //out message         rc rate, rc expo, rollpitch rate, yaw rate, dyn throttle PID
#define MSP_PID                  112   //out message         up to 16 P I D (8 are used)
#define MSP_BOX                  113   //out message         up to 16 checkbox (11 are used)
#define MSP_MISC                 114   //out message         powermeter trig + 8 free for future use
#define MSP_MOTOR_PINS           115   //out message         which pins are in use for motors & servos, for GUI 
#define MSP_BOXNAMES             116   //out message         the aux switch names
#define MSP_PIDNAMES             117   //out message         the PID names
#define MSP_WP                   118   //out message         get a WP, WP# is in the payload, returns (WP#, lat, lon, alt, flags) WP#0-home, WP#16-poshold

#define MSP_SET_RAW_RC           200   //in message          8 rc chan
#define MSP_SET_RAW_GPS          201   //in message          fix, numsat, lat, lon, alt, speed
#define MSP_SET_PID              202   //in message          up to 16 P I D (8 are used)
#define MSP_SET_BOX              203   //in message          up to 16 checkbox (11 are used)
#define MSP_SET_RC_TUNING        204   //in message          rc rate, rc expo, rollpitch rate, yaw rate, dyn throttle PID
#define MSP_ACC_CALIBRATION      205   //in message          no param
#define MSP_MAG_CALIBRATION      206   //in message          no param
#define MSP_SET_MISC             207   //in message          powermeter trig + 8 free for future use
#define MSP_RESET_CONF           208   //in message          no param
#define MSP_WP_SET               209   //in message          sets a given WP (WP#,lat, lon, alt, flags)

#define MSP_EEPROM_WRITE         250   //in message          no param

#define MSP_DEBUG                254   //out message         debug1,debug2,debug3,debug4


static uint8_t checksum;
static uint8_t indRX;
static uint8_t cmdMSP;

static MemoryStream * sp;

const char boxnames[]  = // names for dynamic generation of config GUI
  "ACC;"
  "BARO;"
  "MAG;"
  "CAMSTAB;"
  "CAMTRIG;"
  "ARM;"
  "GPS HOME;"
  "GPS HOLD;"
  "PASSTHRU;"
  "HEADFREE;"
  "BEEPER;"
  "LEDMAX;"
  "LLIGHTS;"
  "HEADADJ;"
;

const char pidnames[]  =
  "ROLL;"
  "PITCH;"
  "YAW;"
  "ALT;"
  "Pos;"
  "PosR;"
  "NavR;"
  "LEVEL;"
  "MAG;"
  "VEL;"
;


// read one byte
uint8_t SerialRead() 
{
	return sp->ReadByte();
}

uint8_t read8()  {
  return inBuf[indRX++]&0xff;
}

uint16_t read16() {
  uint16_t t = read8();
  t+= (uint16_t)read8()<<8;
  return t;
}

uint32_t read32() {
  uint32_t t = read16();
  t+= (uint32_t)read16()<<16;
  return t;
}


void serialize8(uint8_t a) {
	sp->WriteChar(a);
  checksum ^= a;
  //sp->DumpData();
}

void serialize16(int16_t a) {
  serialize8((a   ) & 0xFF);
  serialize8((a>>8) & 0xFF);
}


void serialize32(uint32_t a) {
  serialize8((a    ) & 0xFF);
  serialize8((a>> 8) & 0xFF);
  serialize8((a>>16) & 0xFF);
  serialize8((a>>24) & 0xFF);
}

void headSerialResponse(uint8_t err, uint8_t s) {
  serialize8('$');
  serialize8('M');
  serialize8(err ? '!' : '>');
  checksum = 0; // start calculating a new checksum
  serialize8(s);
  serialize8(cmdMSP);
}

void headSerialReply(uint8_t s) {
  headSerialResponse(0, s);
}

void inline headSerialError(uint8_t s) {
  headSerialResponse(1, s);
}

void tailSerialReply() {
  serialize8(checksum);
  sp->DumpData();
}


void serializeNames(const char * ss ) 
{
	printf("serializeNames");
   for (int i = 0; i < strlen(ss);  i++) 
   {
     serialize8(ss[i]);
   }
}



void SerialWrite(uint8_t c)
{
	sp->WriteChar(c);
	sp->DumpData();
}

void checkFirstTime()
{

}


uint8_t PWM_PIN[8] = {9,10,11,3,6,5,2, 12};   //for a quad+: rear,right,left,front   





void MemoryStream::evaluateCommand() 
{
  switch(cmdMSP) {
   case MSP_SET_RAW_RC:
     for(uint8_t i=0;i<8;i++) {
       sysInfo.rcData[i] = read16();
     }
     headSerialReply(0);
     break;
   case MSP_SET_PID:
	cout << "PIDItems" << PIDITEMS << endl;
     for(uint8_t i=0;i<PIDITEMS;i++) {
       conf.P8[i]=read8();
       conf.I8[i]=read8();
       conf.D8[i]=read8();
     }
	 conf.Dump();
     headSerialReply(0);
     break;
   case MSP_SET_BOX:
     for(uint8_t i=0;i<CHECKBOXITEMS;i++) {
       conf.activate[i]=read16();
     }
     headSerialReply(0);
     break;
   case MSP_SET_RC_TUNING:
	cout << "set rc turning" << endl;
     conf.rcRate8 = read8();
     conf.rcExpo8 = read8();
     conf.rollPitchRate = read8();
     conf.yawRate = read8();
     conf.dynThrPID = read8();
     conf.thrMid8 = read8();
     conf.thrExpo8 = read8();
     headSerialReply(0);
	 conf.Dump();
     break;
   case MSP_SET_MISC:
	read16();
	cout<< "set misc" << endl;
     headSerialReply(0);
     break;
   case MSP_IDENT:
     headSerialReply(7);
     serialize8(VERSION);   // multiwii version
     serialize8(MULTITYPE); // type of multicopter
     serialize8(MSP_VERSION);         // MultiWii Serial Protocol Version
     serialize32(0);        // "capability"
     break;
   case MSP_STATUS:
     headSerialReply(10);
	//cout<< "status " << endl;
     serialize16(sysInfo.cycleTime);
     serialize16(sysInfo.i2c_errors_count);
     serialize16(ACC|BARO<<1|MAG<<2|GPS<<3|SONAR<<4);
     serialize32(f.ACC_MODE<<BOXACC|f.BARO_MODE<<BOXBARO|f.MAG_MODE<<BOXMAG|f.ARMED<<BOXARM|
                 sysInfo.rcOptions[BOXCAMSTAB]<<BOXCAMSTAB | sysInfo.rcOptions[BOXCAMTRIG]<<BOXCAMTRIG |
                 f.GPS_HOME_MODE<<BOXGPSHOME|f.GPS_HOLD_MODE<<BOXGPSHOLD|f.HEADFREE_MODE<<BOXHEADFREE|
                 f.PASSTHRU_MODE<<BOXPASSTHRU|sysInfo.rcOptions[BOXBEEPERON]<<BOXBEEPERON|
				 sysInfo.rcOptions[BOXLEDMAX]<<BOXLEDMAX|sysInfo.rcOptions[BOXLLIGHTS]<<BOXLLIGHTS|
				 sysInfo.rcOptions[BOXHEADADJ]<<BOXHEADADJ);
     break;
   case MSP_RAW_IMU:
     headSerialReply(18);
     for(uint8_t i=0;i<3;i++) serialize16(sysInfo.accSmooth[i]);
     for(uint8_t i=0;i<3;i++) serialize16(sysInfo.gyroData[i]);
     for(uint8_t i=0;i<3;i++) serialize16(sysInfo.magADC[i]);
     break;
   case MSP_SERVO:
     headSerialReply(16);
     for(uint8_t i=0;i<8;i++)
       serialize16(0);
     break;
   case MSP_MOTOR:
     headSerialReply(16);
     for(uint8_t i=0;i<8;i++) {
       serialize16( (i < NUMBER_MOTOR) ? sysInfo.motor[i] : 0 );
     }
     break;
   case MSP_RC:
     headSerialReply(16);
     for(uint8_t i=0;i<8;i++) serialize16(sysInfo.rcData[i]);
     break;
   case MSP_ATTITUDE:
     headSerialReply(8);
     for(uint8_t i=0;i<2;i++) serialize16(sysInfo.angle[i]);
     serialize16(sysInfo.heading);
     serialize16(sysInfo.headFreeModeHold);
     break;
   case MSP_ALTITUDE:
     headSerialReply(4);
     serialize32(sysInfo.EstAlt);
     break;
   case MSP_BAT:
     headSerialReply(3);
     serialize8(sysInfo.vbat);
     serialize16(sysInfo.intPowerMeterSum);
     break;
   case MSP_RC_TUNING:
	cout << "get rc turnning" << endl;
	
     headSerialReply(7);
     serialize8(conf.rcRate8);
     serialize8(conf.rcExpo8);
     serialize8(conf.rollPitchRate);
     serialize8(conf.yawRate);
     serialize8(conf.dynThrPID);
     serialize8(conf.thrMid8);
     serialize8(conf.thrExpo8);
     break;
   case MSP_PID:
	cout << "get pid turnning" << endl;
     headSerialReply(3*PIDITEMS);
     for(uint8_t i=0;i<PIDITEMS;i++) {
       serialize8(conf.P8[i]);
       serialize8(conf.I8[i]);
       serialize8(conf.D8[i]);
     }
     break;
   case MSP_BOX:
	cout << "get box" << endl;
   
     headSerialReply(2*CHECKBOXITEMS);
     for(uint8_t i=0;i<CHECKBOXITEMS;i++) {
       serialize16(conf.activate[i]);
     }
     break;
   case MSP_BOXNAMES:
	cout << "get box name" << endl;
     headSerialReply(strlen(boxnames));
     serializeNames(boxnames);
     break;
   case MSP_PIDNAMES:
	cout << "get pid name" << endl;
     headSerialReply(strlen(pidnames));
     serializeNames(pidnames);
     break;
   case MSP_MISC:
	cout << "get misc" << endl;
   
     headSerialReply(2);
     serialize16(sysInfo.intPowerTrigger1);
     break;
   case MSP_MOTOR_PINS:
     headSerialReply(8);
     for(uint8_t i=0;i<8;i++) {
       serialize8(PWM_PIN[i]);
     }
     break;
 
	 
   case MSP_RESET_CONF:
     conf.checkNewConf++;
     checkFirstTime();
     headSerialReply(0);
     break;
   case MSP_ACC_CALIBRATION:
     sysInfo.calibratingA=400;
     headSerialReply(0);
     break;
   case MSP_MAG_CALIBRATION:
     f.CALIBRATE_MAG = 1;
     headSerialReply(0);
     break;
   case MSP_EEPROM_WRITE:
	cout<< "Write" << endl;
     conf.SaveToFile(SETTINGFILE);
     headSerialReply(0);
     break;
   case MSP_DEBUG:
     headSerialReply(8);
     for(uint8_t i=0;i<4;i++) {
       //serialize16(debug[i]); // 4 variables are here for general monitoring purpose
	   serialize16(0); // 4 variables are here for general monitoring purpose
     }
     break;
   default:  // we do not know how to handle the (valid) message, indicate error MSP $M!
     headSerialError(0);
     break;
  }
  tailSerialReply();
}

// evaluate all other incoming serial data
void MemoryStream::evaluateOtherData(uint8_t sr) {
  switch (sr) {

  // Note: we may receive weird characters here which could trigger unwanted features during flight.
  //       this could lead to a crash easily.
  //       Please use if (!f.ARMED) where neccessary


  }
}

  static enum _serial_state {
    IDLE,
    HEADER_START,
    HEADER_M,
    HEADER_ARROW,
    HEADER_SIZE,
    HEADER_CMD,
  } c_state = IDLE;



void MemoryStream::serialCom() {
  uint8_t c;  
  static uint8_t offset;
  static uint8_t dataSize;
	sp = this;
	while (1) 
	{
		c = SerialRead();
		//continue;
		if (c_state == IDLE) 
		{
			c_state = (c=='$') ? HEADER_START : IDLE;
			if (c_state == IDLE) 
			{	
				evaluateOtherData(c); // evaluate all other incoming serial data
			}
		} 
		else if (c_state == HEADER_START) 
		{
		  c_state = (c=='M') ? HEADER_M : IDLE;
		} 
		else if (c_state == HEADER_M) 
		{
		  c_state = (c=='<') ? HEADER_ARROW : IDLE;
		}
		else if (c_state == HEADER_ARROW) 
		{
			// if (c > INBUF_SIZE) 
			// {  // now we are expecting the payload size
				// c_state = IDLE;
				// continue;
			// }
			dataSize = c;
			offset = 0;
			checksum = 0;
			indRX = 0;
			checksum ^= c;
			c_state = HEADER_SIZE;  // the command is to follow
		} 
		else if (c_state == HEADER_SIZE) 
		{
		  cmdMSP = c;
		  checksum ^= c;
		  c_state = HEADER_CMD;
		}
		else if (c_state == HEADER_CMD && offset < dataSize) 
		{
		  checksum ^= c;
		  inBuf[offset++] = c;
		} 
		else if (c_state == HEADER_CMD && offset >= dataSize) 
		{
			if (checksum == c) 
			{  // compare calculated and transferred checksum
				evaluateCommand();  // we got a valid packet, evaluate it
			}
			c_state = IDLE;
		}
		usleep(100);
	}
}


// DumpData.h
int MemoryStream::OpenPort()
{
	fd = 0;
	struct termios newtio;
	if((fd = open(SERIALPORT, O_RDWR | O_NOCTTY ))==-1)
	{
			//printf("Fail to open \n");
			return -1;
	}

	if (fd < 0)
	{
			printf("open error %d %s\n", errno, strerror(errno));
	}
	else
	{
		memset (&newtio, 0, sizeof(newtio));

		newtio.c_cflag = (CLOCAL | CREAD);
		newtio.c_iflag = (IGNPAR);
		newtio.c_oflag = 0;

		newtio.c_cflag |= BAUDRATE;				

		newtio.c_cflag &= ~(PARENB | PARODD);	
		newtio.c_cflag &= ~CSTOPB;

		newtio.c_cflag |= CS8;

		newtio.c_iflag &= ~(IXON | IXOFF | IXANY);
		newtio.c_cflag &= ~CRTSCTS;
		newtio.c_lflag =0;
		
		newtio.c_cc[VMIN] = 0;
		newtio.c_cc[VTIME] = 0.5;		// time out in 1 seoncds

		newtio.c_cc[VSTOP] = 0;
		newtio.c_cc[VSTART] = 0;

		tcflush(fd, TCIFLUSH);
		tcsetattr(fd,TCSANOW, &newtio);
	} 
	return fd;	
}

void MemoryStream::ClosePort()
{
	close(fd);
}

int MemoryStream::WritePort(char * psOutput, int len)
{
	if(fd < 1)
	{
		return -1;
	}
	//cout << "write port " << endl;
	int iOut = write(fd, psOutput, len);
	return iOut;
}


MemoryStream::MemoryStream(SystemVaribles & _sysInfo, flags_struct & _f, Config & _conf):sysInfo(_sysInfo),f(_f), conf(_conf)
{
	cout << "init M" << endl;
	OpenPort();
	if(fd > 0)
	{
		cout << "port openedd" << endl;
	}
}
MemoryStream::~MemoryStream()
{
	ClosePort();
}

void MemoryStream::WriteChar(unsigned char ch)
{
	//cout << "ch = " << ch << endl;
	buffer.push_back(ch);
}

void MemoryStream::WriteUint16(unsigned short s)
{
	//cout << "Write Unsigned Uint16 " << s << endl;
	unsigned char buf[2];
	memcpy(buf, &s, 2);
	buffer.push_back(buf[0]);
	buffer.push_back(buf[1]);
}

void MemoryStream::WriteInt16(short s)
{
	//cout<<"Write Int 16 " << s << endl;
	unsigned char buf[2];
	memcpy(buf, &s, 2);
	buffer.push_back(buf[0]);
	buffer.push_back(buf[1]);
}

void MemoryStream::WriteLong(long l)
{
	//cout << "Write long " << l << endl;
	unsigned char buf[4];
	memcpy(buf, &l, 4);
	buffer.push_back(buf[0]);
	buffer.push_back(buf[1]);
	buffer.push_back(buf[2]);
	buffer.push_back(buf[3]);

}
void MemoryStream::WriteUnsignedLong(unsigned long l)
{
	//cout << "Write Unsigned Long " << l << endl;
	unsigned char buf[4];
	memcpy(buf, &l, 4);
	buffer.push_back(buf[0]);
	buffer.push_back(buf[1]);
	buffer.push_back(buf[2]);
	buffer.push_back(buf[3]);
}

void	MemoryStream::DumpData()
{
	for(int i =0; i < buffer.size(); i ++)
	{
		//printf("%d %c ", buffer[i], buffer[i]);
	}
	//printf("Dump Data");
	char * p = new char[buffer.size()];
	for(int i = 0; i < buffer.size(); i ++)
	{
		p[i] = buffer[i];
	}
	WritePort(p, buffer.size());
	buffer.clear();
	delete []p;	
}

int MemoryStream::ReadData(char * buffer, int size)
{
	if(fd < 1)
	{
		return -1;
	}
	//cout << "write port " << endl;
	int iOut = read(fd, buffer, size);
	return iOut;
}

unsigned char MemoryStream::ReadByte()
{
	unsigned char buffer[1];
	
	int iOut = read(fd, buffer, 1);
	while(iOut < 1)
	{
		usleep(1000);
		iOut = read(fd, buffer, 1);
	}
	return buffer[0];
}