#ifndef __PRU_PROXY_HH
#define __PRU_PROXY_HH
//PruProxy.h
class PruProxy
{
public :
	unsigned int Input1, Input2, Input3, Input4, Input5, Input6;
	unsigned int Output1, Output2, Output3, Output4;
	PruProxy();
	~PruProxy();
	// disable Pru, function for debug program without getting bus error
	void DisablePru();
	// init the Pru
	bool Init();
	// update input, call this method before using Input1 to Input4
	bool UpdateInput();
	// Init the output pulse width and set low value
	bool InitOutput();	
	// update output, call this method after updating Output1 to Output4
	bool UpdateOutput();
	// Run image on Pru0
	bool RunPru0();
	// Run Image to Pru1
	bool RunPru1();
private:
	void *sharedMem;

	bool Enabled;	// flag for if the pru is enabled 
	
};
#endif
