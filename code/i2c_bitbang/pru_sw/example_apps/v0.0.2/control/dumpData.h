#ifndef DUMP_DATA_HH
#define DUMP_DATA_HH
#include <vector>

using namespace std;

struct SystemVaribles;
struct flags_struct;
struct Config;
// DumpData.h
class MemoryStream
{
	vector<char> buffer;
	int fd;
	int OpenPort();
	void ClosePort();
	int WritePort(char * psOutput, int len);
	SystemVaribles & sysInfo;
	flags_struct & f;
	Config & conf;
void evaluateOtherData(unsigned char sr);
void evaluateCommand() ;
	
	
public :

void serialCom() ;
MemoryStream(SystemVaribles & _sysInfo, flags_struct & _f, Config & _conf);
~MemoryStream();
void WriteChar(unsigned char ch);

void WriteUint16(unsigned short s);

void WriteInt16(short s);

void WriteLong(long l);
void WriteUnsignedLong(unsigned long l);

void	DumpData();
// read data from serila port,
// buffer,
// read size
int ReadData(char * buffer, int size);

unsigned char ReadByte();


};
#endif