// itg3200.cpp
// demo of read itg3200 gyro

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <linux/i2c-dev.h>
#include <sys/ioctl.h>
#include <iostream>;
#include <termios.h>
#include "itg3200.h"

using namespace std;

char filename[20];

// itg3200 init codes
char write_buf_1[2] = { 0x3E, 0x80 };
char write_buf_2[2] = { 0x15, 0x00 };
char write_buf_3[2] = { 0x16, 0x18 };

char write_buf_4[2] = { 0x17, 0x21 };		// enable interrupt
char write_buf_5[5] = { 0x3E, 0x03 };

char GYRO_XOUT_H[1] = { 0x1D };
char GYRO_XOUT_L[1] = { 0x1E };
char GYRO_YOUT_H[1] = { 0x1F };
char GYRO_YOUT_L[1] = { 0x20 };
char GYRO_ZOUT_H[1] = { 0x21 };
char GYRO_ZOUT_L[1] = { 0x22 };

char INT_STATUS[1] = {0x1a};

char buf[8];

int file;
int adapter_nr = 3; 
int i2c_addr = 0x69; // itg3200 I2C address 
int i;
int read_bytes = 1;



/* I2C busses, u-boot are 0 - 2, userspace is 1 - 3 */

/* i2c_bus_init() open i2c bus 3 if available */
bool ITG3200::i2c_bus_init() {
	bool r = false;
	sprintf(filename, "/dev/i2c-3");
	if ((file = open(filename, O_RDWR)) < 0) {
		printf("Failed to open the bus.\n");
		/* ERROR HANDLING; you can check errno to see what went wrong */
		if (file >= 0)
			close(file);
		r = false;
	} else {
		printf("Opened /dev/i2c-%d\n", adapter_nr);
		r = true;
	}
	return r;
}

// init itg3200
bool ITG3200::i2c_3200_init() {
	bool r = false;
	i2c_addr = 0x69;

	if (ioctl(file, I2C_SLAVE, i2c_addr) < 0) {
		printf("Failed to acquire bus access and/or talk to slave.\n");
		/* ERROR HANDLING; you can check errno to see what went wrong */
		if (file >= 0)
			close(file);
		return false;
	} else {
		printf("Comm to itg3200 is open.\n");
	}
	usleep(100000);
	if (write(file, write_buf_1, 2) != 2) /* first init string */
	{
		printf("i2c send %x failed\n", i2c_addr);
		close(file);
		return false;

	} else {
		printf("i2c send buf[1] success.\n");
	}
	usleep(100000);

	if (write(file, write_buf_2, 2) != 2) /* second init string */
	{
		printf("i2c send %x failed\n", i2c_addr);
		close(file);
		return false;
	} else {
		printf("i2c send buf[2] success.\n");
	}
	usleep(100000);

	if (write(file, write_buf_3, 2) != 2) /* second init string */
	{
		printf("i2c send %x failed\n", i2c_addr);
		close(file);
		return false;
	} else {
		printf("i2c send buf[3] success.\n");
	}
	usleep(100000);

	if (write(file, write_buf_4, 2) != 2) /* second init string */
	{
		printf("i2c send %x failed\n", i2c_addr);
		close(file);
		return false;
	} else {
		printf("i2c send buf[4] success.\n");
	}
	usleep(100000);
	if (write(file, write_buf_5, 2) != 2) /* second init string */
	{
		printf("i2c send %x failed\n", i2c_addr);
		close(file);
		return false;
	} else {
		printf("i2c send buf[5] success.\n");
	}
	usleep(100000);
	return true;
}


unsigned char ITG3200::i2c_itg3200_read(char * command)
{
	unsigned char buf2[1] = {0};
	int rb = 1;
	if (write(file, command, 1) != 1) /* send to format data */
	{
		printf("I2C Send %x Failed\n", i2c_addr);
		close(file);
		exit(1);
	}
	// delay for adjusting reporting speed. Bad data without.
	usleep(100); // 100k uS = 0.1S, 1k uSec = 0.001S
	if (read(file, buf2, rb) != rb) {
		printf("I2C Send %x Failed\n", i2c_addr);
	}
	return buf2[0];
}

long ITG3200::itg3200_readX()
{
	unsigned char a1;
	unsigned char b1;
	a1 = i2c_itg3200_read(GYRO_XOUT_H);
	b1 = i2c_itg3200_read(GYRO_XOUT_L);
	short m = 0;
	unsigned char a2[2] = {b1, a1};
	memcpy(&m, a2, 2);
	return m;
}

long ITG3200::itg3200_readY()
{
	unsigned char a = i2c_itg3200_read(GYRO_YOUT_H);
	unsigned char b = i2c_itg3200_read(GYRO_YOUT_L);
	short m = 0;
	unsigned char a2[2] = {b, a};
	memcpy(&m, a2, 2);
	return m;
}

long ITG3200::itg3200_readZ()
{
	unsigned char a = i2c_itg3200_read(GYRO_ZOUT_H);
	unsigned char b = i2c_itg3200_read(GYRO_ZOUT_L);
	short m = 0;
	unsigned char a2[2] = {b, a};
	memcpy(&m, a2, 2);
	return m;
}


// wait for data ready, ture for ready, false for timeout
bool ITG3200::WaitForData()
{
	// try 5 times
	for(int i = 0; i < 5; i ++)
	{
		unsigned char aa = i2c_itg3200_read(INT_STATUS);
		if((aa & 0x1) == 0x1)
		{
			return true;
		}
		usleep(1);
		printf("not ready \r\n");
	}
	return false;
}


ITG3200::ITG3200()
{
	x = 0;
	y = 0;
	z = 0;
}
ITG3200::~ITG3200()
{

}
// return true if init OK
bool ITG3200::Init()
{
	bool r = i2c_bus_init();
	if(!r)
	{
		return r;
	}
	r = i2c_3200_init();
	
	if(!r)
	{
		return r;
	}
	
	m_xOffset = 0;
	m_yOffset = 0;
	m_zOffset = 0;
	int count = 50;
	for(int i = 0; i < count; i ++)
	{
		if(WaitForData())
		{
			m_xOffset += itg3200_readX();
			m_yOffset += itg3200_readY();
			m_zOffset += itg3200_readZ();
		}
		else
		{
			return false;
		}
	}
		
	m_xOffset = m_xOffset/count;
	m_yOffset = m_yOffset/count;
	m_zOffset = m_zOffset/count;
	cout << "X offset: " << m_xOffset << " Y offset:" << m_yOffset << " Z offset:" << m_zOffset << endl;
	return true;

}
// return true if data been updated correctly.
bool ITG3200::UpdateData()
{
	if(WaitForData())
	{
		x = (itg3200_readX() - m_xOffset) ;
		y = (itg3200_readY() - m_yOffset) ;
		z = (itg3200_readZ() - m_zOffset) ;
		return true;
	}
	return false;
}

// int main(int argc, void ** argv)
// {
	// i2c_bus_init();
	// i2c_3200_init();
	// int offsetX = 0;
	// int offsetY = 0;
	// int offsetZ = 0;
	// int offsetX2= 0;
	
	// // get offset, keep gyro stable
	// for(int i = 0; i < 500; i ++)
	// {
		// WaitForData();
		// short x = itg3200_readX();
		// short y = itg3200_readY();
		// short z = itg3200_readZ();
		// short x2 = itg3200_readX2();
		// offsetX = offsetX + x;
		// offsetY = offsetY + y;
		// offsetZ = offsetZ + z;
		// offsetX2 = offsetX2 + x2;
	// }
	// offsetX = offsetX / 500;
	// offsetY = offsetY / 500;
	// offsetZ = offsetZ / 500;
	// offsetX2 = offsetX2 /500;
	// printf("offset  X: %8d  Y: %8d Z: %8d  X2: %8d \r\n", offsetX, offsetY, offsetZ, offsetX2);

	// int i  = 0;
	// while(1)
	// {
		// i++;
		// WaitForData();
		// // int x = itg3200_readX() - offsetX;
		// // int x2 = itg3200_readX2() - offsetX2;
		// // int y = itg3200_readY() - offsetY;
		// // int z = itg3200_readZ() - offsetZ;
		// if(i / 20 * 20 == i)
		// {
			// //printf("%8d %8d %8d %8d %8d\r\n", i, x, y, z, x2);
		// }
		// //usleep(10000);
	// }
	
	// return 0;
// }