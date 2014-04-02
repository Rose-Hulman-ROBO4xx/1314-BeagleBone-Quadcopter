#ifndef __ITG_3200_HH__
#define __ITG_3200_HH__



class ITG3200
{
public:
long x, y, z;	// value of x, y, z, valid after update
ITG3200();
~ITG3200();
// return true if init OK
bool Init();
// return true if data been updated correctly.
bool UpdateData();

private:
long m_xOffset, m_yOffset, m_zOffset;

bool i2c_bus_init() ;
// init itg3200
bool i2c_3200_init();

unsigned char i2c_itg3200_read(char * command);
long itg3200_readX();
long itg3200_readY();
long itg3200_readZ();
bool WaitForData();

};

#endif