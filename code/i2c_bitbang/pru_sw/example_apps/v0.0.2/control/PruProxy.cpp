// PruProxy.cpp
#include <stdio.h>
#include <iostream>
#include <sys/mman.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>
#include <string.h>
#include "prussdrv.h"
#include <pruss_intc_mapping.h>

#include "PruProxy.h"

#define PRUSS_MAX_IRAM_SIZE	0x2000

using namespace std;

PruProxy::PruProxy()
{
	Enabled = true;
}

PruProxy::~PruProxy()
{
}

// Disable the pru
void PruProxy::DisablePru()
{
	Enabled = false;
}

// initialize the pru
bool PruProxy::Init()
{
	Input1 = 1500;
	Input2 = 1500;
	Input3 = 1500;
	Input4 = 1500;
	Input5 = 1500;
	Input6 = 1500;
	
	if(Enabled)
	{
		tpruss_intc_initdata pruss_intc_initdata = PRUSS_INTC_INITDATA;
		prussdrv_init ();
		unsigned int ret = prussdrv_open(PRU_EVTOUT_0);
		if (ret)
		{
		    printf("prussdrv_open open failed\n");
		}
		prussdrv_pruintc_init(&pruss_intc_initdata);
    		prussdrv_map_prumem(PRUSS0_SHARED_DATARAM, &sharedMem);
		((int*)sharedMem)[2048] = 0;
		RunPru0();
		printf("Failed to map the device (%s)\n", strerror(errno));
		InitOutput();
		
	}
   return true;
}

// update input
bool PruProxy::UpdateInput()
{
	if(Enabled)
	{
		/*
		Input1 = (*(unsigned long *)(sharedMem + 0x114))/200;
		Input2 = (*(unsigned long *)(sharedMem + 0x124)/200);
		Input3 = (*(unsigned long *)(sharedMem + 0x134)/200);
		Input4 = (*(unsigned long *)(sharedMem + 0x144)/200);
		Input5 = (*(unsigned long *)(sharedMem + 0x154)/200);
		Input6 = (*(unsigned long *)(sharedMem + 0x164)/200);
		*/
		Input1 = 20000;
		Input2 = 20000;
		Input3 = 20000;
		Input4 = 20000;
		Input5 = 20000;
		Input6 = 20000;
		// if(Input1 > 2000 || Input1 < 1000)
		// {
			// Input1 = 1050;
		// }
		// if(Input2 > 2000 || Input2 < 1000)
		// {
			// Input2 = 1050;
		// }
		// if(Input3 > 2000 || Input3 < 1000)
		// {
			// Input3 = 1050;
		// }
		// if(Input4 > 2000 || Input4 < 1000)
		// {
			// Input4 = 1050;
		// }
	}
	return true;
	
}

// Init Output, set the esc to low throttle value as 1000
// set pulse width as 20 ms
// S1	pin 29		REAR_R		S4
// S2	pin 30		FRONT_R		S3
// S3	pin 27		READ)L		S2
// S4 	pin 28 		FRONT_L		S1
// Pin 20 21 won't work as it is high when reboot, esc wont' work
bool PruProxy::InitOutput()
{
	if(Enabled)
	{

	}
	return true;
}

bool PruProxy::UpdateOutput()
{
	if(Enabled)
	{
		((unsigned int *)sharedMem)[8+2048] = Output1;
		((unsigned int *)sharedMem)[9+2048] = Output2;
		((unsigned int *)sharedMem)[10+2048] = Output3;
		((unsigned int *)sharedMem)[11+2048] = Output4;
		printf("%d %d %d %d\n", Output1, Output2, Output3, Output4);
	
	}
	return true;
}



// Run image on Pru0
bool PruProxy::RunPru0()
{
	prussdrv_exec_program (0, "./control_alg.bin");
	return true;
}


