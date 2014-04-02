#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <iostream>
#include "config.h"
#include "def.h"
#include "typedef.h"
using namespace std;
// load from file
bool Config::LoadFromFile(char * filename)
{
	int fp = open(filename, O_RDONLY);
	if(fp == -1)
	{
		cout << "file " << filename << " is not found" << endl;
		cout << "load file failed" << endl;
		return false;
	}
	Config f;
	int l = sizeof(f);
	int n = read(fp, this, sizeof(Config));
	if (n > 0)
	{
		cout << "load data from "<< filename << endl;
	}
	close(fp);
}

// save config to file
bool Config::SaveToFile(char * filename)
{
	int fp = open(filename, O_WRONLY | O_CREAT);
	if(fp == -1)
	{
		cout << "file "<< filename <<" is not found" << endl;
		cout << "save file failed" << endl;
		return false;
	}
	int n = write(fp, this, sizeof(Config));
	if (n > 0)
	{
		cout << "save data to " << filename << endl;
	}
	close(fp);
}
// load default setting
void Config::LoadDefault()
{
	P8[ROLL]  = 30;  I8[ROLL] = 30; D8[ROLL]  = 23;
	P8[PITCH] = 30; I8[PITCH] = 30; D8[PITCH] = 23;
	P8[YAW]   = 65;  I8[YAW]  = 45;  D8[YAW]  = 0;
	P8[PIDALT]   = 16; I8[PIDALT]   = 15; D8[PIDALT]   = 7;

	P8[PIDPOS]  = POSHOLD_P * 100;     I8[PIDPOS]    = POSHOLD_I * 100;       D8[PIDPOS]    = 0;
	P8[PIDPOSR] = POSHOLD_RATE_P * 10; I8[PIDPOSR]   = POSHOLD_RATE_I * 100;  D8[PIDPOSR]   = POSHOLD_RATE_D * 1000;
	P8[PIDNAVR] = NAV_P * 10;          I8[PIDNAVR]   = NAV_I * 100;           D8[PIDNAVR]   = NAV_D * 1000;

	P8[PIDLEVEL] = 70; I8[PIDLEVEL] = 10; D8[PIDLEVEL] = 100;
	P8[PIDMAG] = 40;

	P8[PIDVEL] = 0;  I8[PIDVEL] = 0;  D8[PIDVEL] = 0;

	rcRate8 = 90; rcExpo8 = 65;
	rollPitchRate = 0;
	yawRate = 0;
	dynThrPID = 0;
	thrMid8 = 50; thrExpo8 = 0;
	for(uint8_t i=0;i<CHECKBOXITEMS;i++) 
	{
		activate[i] = 0;
	}
	angleTrim[0] = 0; angleTrim[1] = 0;
	powerTrigger1 = 0;
}
// construct
Config::Config()
{
	LoadDefault();
}

// dump Config
void Config::Dump()
{
	cout << "Config Dump" << endl;
	cout << "P8[ROLL]  =" << (int)P8[ROLL] << " I8[ROLL] = " << (int)I8[ROLL] << " D8[ROLL] = " << (int)D8[ROLL] << endl;
	cout << "P8[PITCH] =" << (int)P8[PITCH] << " I8[PITCH] =" << (int)I8[PITCH] << " D8[PITCH] =" << (int)D8[PITCH] << endl;
	cout << "P8[YAW]   =" << (int)P8[YAW] << " I8[YAW]  =" << (int)I8[YAW] << " D8[YAW]  =" <<  (int)D8[YAW] << endl;
	
     cout << "rcRate8 = " << (int)rcRate8 << endl;
     cout << "rcExpo8  = " << (int)rcExpo8  << endl;
     cout << "rollPitchRate  = " << (int)rollPitchRate  << endl;
     cout << "yawRate  = " << (int)yawRate  << endl;
     cout << "dynThrPID  = " << (int)dynThrPID  << endl;
     cout << "thrMid8  = " << (int)thrMid8  << endl;
     cout << "thrExpo8  = " << (int)thrExpo8  << endl;
}
