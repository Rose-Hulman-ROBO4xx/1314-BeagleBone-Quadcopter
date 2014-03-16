
.origin 0
.entrypoint MEMACCESSPRUDATARAM

#include "pru_common.hp"
#include "mem_map.h"

#define CAM_BITS r31

#define CAM_DATA_MASK 0xFF
#define CAM_VSYNC_MASK     0x100
#define CAM_HSYNC_MASK     0x200
#define CAM_CLK_MASK    0x10000

#define CAM_VSYNC 16 //P9_26
#define CAM_WE	  9  //P8_29
#define CAM_RCK   8  //P8_27
#define CAM_OE    10 //P8_28
#define CAM_RRST  11 //P8_30
#define DELAY     mov r0, r0

    #define GPIO1 0x4804c000
#define GPIO2 0x481ac000
#define GPIO_CLEARDATAOUT 0x190
#define GPIO_SETDATAOUT 0x194
#define GPIO_DATAOUT 0x13C

#define currBuff r7
#define swapBuff r10
#define buff0 r11
#define buff1 r12
#define pixelByteCount   r13
#define lineCount    r14
#define numCols r8
#define numRows r9
#define exit r6
#define num_bytes 640*480*2
#define NUM_BYTES r17
MEMACCESSPRUDATARAM:

	MOV exit,EXIT_CTRL*4
	// Enable OCP master port
	LBCO      r0, CONST_PRUCFG, 4, 4
	CLR     r0, r0, 4         // Clear SYSCFG[STANDBY_INIT] to enable OCP master port
	SBCO      r0, CONST_PRUCFG, 4, 4

	// set parallel input capture
	LBCO      r0, CONST_PRUCFG, 0xC, 4
	clr     r0, r0, 0
	clr     r0, r0, 1
	SBCO      r0, CONST_PRUCFG, 0xC, 4

	clr       R30, R30, CAM_OE
	MOV       swapBuff, CAM_SWAP_BUFF*4
	MOV       buff0, CAM_BUFF_0*4
	LBBO      buff0, buff0, 0, 4
	MOV       buff1, CAM_BUFF_1*4
	LBBO      buff1, buff1, 0, 4

	MOV       currBuff, buff0
	mov		r15, 0
	mov r16,   100*4
	mov NUM_BYTES, num_bytes

		Loop:

		add r15, r15, 1
		sbbo r15, r16, 0, 4
		MOV   lineCount, 0
		
		WBS   CAM_BITS, CAM_VSYNC //Wait for a frame to start
		CLR   R30, R30, CAM_WE //enable writing to fifo
		WBC   CAM_BITS, CAM_VSYNC //wait for frame to end
		SET   R30, R30, CAM_WE //disable writing to fifo
		WBS   CAM_BITS, CAM_VSYNC //Wait for a frame to start
		CLR   R30, R30, CAM_RCK //need to pulse the clock for it to take
		DELAY
		DELAY
		DELAY
		DELAY
		DELAY

		CLR   R30, R30, CAM_RRST //reset the read pointer
		DELAY
		DELAY
		DELAY
		DELAY
		DELAY
		SET   R30, R30, CAM_RCK
		DELAY
		DELAY
		DELAY
		DELAY
		DELAY
		SET   R30, R30, CAM_RRST //reset the read pointer
		DELAY
		DELAY
		DELAY
		DELAY
		DELAY
		CLR   R30, R30, CAM_RCK

		DELAY
		DELAY
		DELAY
		DELAY
		DELAY
		MOV   pixelByteCount, 0

			Pixels:
			SET R30, R30, CAM_RCK
		DELAY
		DELAY
		DELAY
		DELAY
		DELAY
		DELAY

			MOV   r0.b0, CAM_BITS.b0
		DELAY
		DELAY
		DELAY
		DELAY
		DELAY
			CLR R30, R30, CAM_RCK
		DELAY
		DELAY
		DELAY
		DELAY
		DELAY
		DELAY
		DELAY
		DELAY
			SBBO  r0.b0, currBuff, pixelByteCount, 1
			ADD   pixelByteCount, pixelByteCount, 1
			QBGE  Pixels, pixelByteCount, NUM_BYTES

		QBNE Loop, r15, 5

	MOV   r0, 0
	SBBO  r0, swapBuff, 0, 4
	MOV   r0, buff0
	MOV   buff0, buff1
	MOV   buff1, r0
	MOV   currBuff, buff0

	LBBO r0, exit, 0, 4
	QBNE Quit, r15, 5

	//QBA Loop






Quit:

    // Send notification to Host for program completion
    MOV R31.b0, PRU1_ARM_INTERRUPT+16


    HALT

    
    
    
    
    
    
    
    
    
    
    
#ifdef junk
start:
    MOV r1, 10
BLINK:
    MOV r2, 7<<22
    MOV r3, GPIO1 | GPIO_SETDATAOUT
    SBBO r2, r3, 0, 4
    MOV r0, 0x00a00000
DELAY:
    SUB r0, r0, 1
    QBNE DELAY, r0, 0
    MOV r2, 7<<22
    MOV r3, GPIO1 | GPIO_CLEARDATAOUT
    SBBO r2, r3, 0, 4
    MOV r0, 0x00a00000
DELAY2:
    SUB r0, r0, 1
    QBNE DELAY2, r0, 0
    SUB r1, r1, 1
    QBNE BLINK, r1, 0
    
qba start
#endif
