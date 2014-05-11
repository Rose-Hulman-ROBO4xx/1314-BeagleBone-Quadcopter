
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
#define exit r6
#define num_bytes 393216
#define NUM_BYTES r17
#define SET_RCK set r30, r30, CAM_RCK
#define CLR_RCK clr r30, r30, CAM_RCK
#define buffNo r8
#define buff2 r9
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

	MOV       buff2, CAM_BUFF_2*4
	LBBO      buff2, buff2, 0, 4
	
	MOV       currBuff, buff0
	mov	  buffNo, 0
	mov		r15, 0
	mov r16,   100*4
	mov NUM_BYTES, num_bytes

		Loop:
	add r15, r15, 1 //increment frame count
	sbbo r15, r16, 0, 4


		WBS CAM_BITS, CAM_VSYNC //Wait for a frame to start
		CLR   R30, R30, CAM_WE //enable writing to fifo
		WBC   CAM_BITS, CAM_VSYNC //wait for frame to end
		SET   R30, R30, CAM_WE //disable writing to fifo
		SET   R30, R30, CAM_RCK //need to pulse the clock for it to take
		CLR   R30, R30, CAM_RRST //reset the read pointer
		DELAY
		DELAY
		DELAY
		DELAY
		DELAY
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
		DELAY
		DELAY
		DELAY
		CLR   R30, R30, CAM_RCK
		
		SET   R30, R30, CAM_RRST //reset the read pointer
		DELAY
		DELAY
		DELAY
		DELAY
		DELAY
		DELAY
		DELAY
		DELAY
		MOV   pixelByteCount, 0

		Pixels:
			MOV   r0.b0, CAM_BITS.b0
			SET R30, R30, CAM_RCK
			DELAY
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
			MOV   r0.b1, CAM_BITS.b0
			SET R30, R30, CAM_RCK
			DELAY
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
			SBBO  r0.b0, currBuff, pixelByteCount, 2
			ADD   pixelByteCount, pixelByteCount, 2
			QBGE  Pixels, pixelByteCount, NUM_BYTES
	
	
	QBEQ handleBuff0, buffNo, 0
	QBEQ handleBuff1, buffNo, 1
	QBEQ handleBuff2, buffNo, 2
	QBEQ handleBuff3, buffNo, 3
	QBEQ handleBuff4, buffNo, 4
handleBuff0:
	mov currBuff, CAM_BUFF_1*4
	LBBO currBuff, currBuff, 0, 4
	sbbo buffNo, swapBuff, 0, 4
	mov buffNo, 1
	QBA BUFF_DONE
handleBuff1:
	mov currBuff, CAM_BUFF_2*4
	LBBO currBuff, currBuff, 0, 4
	sbbo buffNo, swapBuff, 0, 4
	mov buffNo, 2
	QBA BUFF_DONE
handleBuff2:
	mov currBuff, CAM_BUFF_3*4
	LBBO currBuff, currBuff, 0, 4
	sbbo buffNo, swapBuff, 0, 4
	mov buffNo, 3
	QBA BUFF_DONE


handleBuff3:
	mov currBuff, CAM_BUFF_4*4
	LBBO currBuff, currBuff, 0, 4
	sbbo buffNo, swapBuff, 0, 4
	mov buffNo, 4
	QBA BUFF_DONE


handleBuff4:
	mov currBuff, CAM_BUFF_0*4
	LBBO currBuff, currBuff, 0, 4
	sbbo buffNo, swapBuff, 0, 4
	mov buffNo, 0
	QBA BUFF_DONE

BUFF_DONE:

	LBBO r0, exit, 0, 4
	QBEQ Quit, r0, 0

	QBA Loop






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
