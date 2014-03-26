#ifndef __GPIO_HH__
#define __GPIO_HH__
class GPIO
{
public :
	GPIO();
	~GPIO();
	// read input from gpio
	// id the id of the input
	bool	ReadInput(int id);
	// set output of the gpio
	// id, the id the of output
	void 	SetOutput(int id, bool high);
	// set the gpio output to flash status
	void 	SetFlash(int id, int rate);
	
private:
	int  map[10];
};

#endif