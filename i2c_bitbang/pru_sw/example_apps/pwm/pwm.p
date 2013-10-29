#include "pwm.hp"
#define RA_REG 			R30.w0 //return address register
.setcallreg 			RA_REG
.origin 0
.entrypoint PWM_MAIN

#define SP_reg 			r18

#define GPIO1 0x4804c000
#define GPIO0 0x44E07000
//SDA_WRITE P9_11
//SDA_READ  P9_13
//SCL_WRITE P9_12
//SCL_READ  P9_14
//IMU_INT   P9_15

#define GPIO_OE			0x134
#define GPIO_DATAIN		0x138
#define GPIO_CLEARDATAOUT 	0x190
#define GPIO_SETDATAOUT 	0x194

#define PWM_0_WRITE_BANK	GPIO1
#define PWM_0_WRITE_BIT_NUMBER	28
#define DELAY_TIME		1000


PWM_MAIN:
	// Configure the block index register for PRU0 by setting c24_blk_index[7:0] and
	// c25_blk_index[7:0] field to 0x00 and 0x00, respectively.  This will make C24 point
	// to 0x00000000 (PRU0 DRAM) and C25 point to 0x00002000 (PRU1 DRAM).
	MOV       r0, 0x00000000
	MOV       r1, CTBIR_0
	ST32      r0, r1

	mov SP_reg, 100 // set up the stack
	call ENABLE_GPIO_AND_SET_DIRECTIONS

    //Store result in into memory location c3(PRU0/1 Local Data)+8 using constant table
	SBCO      r3, CONST_PRUDRAM, 28, 4


    // Send notification to Host for program completion
    MOV R31.b0, PRU0_ARM_INTERRUPT+16
    
    HALT
//------------------------------------------------------------
DELAY:
	sub SP_reg, SP_reg, 8 // gonna push something onto the stack
	sbco r0, CONST_PRUDRAM, SP_reg, 8 //store r0 and r1
	
	mov r0, 0 // set delay amount to 1 million cycles
	mov r1, DELAY_TIME

DELAY_LOOP:
	add r0, r0, 1
	qblt DELAY_LOOP, r1, r0 //are we done busy waiting yet?
	
	lbco r0, CONST_PRUDRAM, SP_reg, 8
	add SP_reg, SP_reg, 8
	ret
//--------------------------------------------------
ENABLE_GPIO_AND_SET_DIRECTIONS:
	sub SP_reg, SP_reg, 8 //push r0 and r1 onto stack
	sbco r0, CONST_PRUDRAM, SP_reg, 8

	LBCO r0, C4, 4, 4 //do something for the gpio??? This is black magic to me. but it makes gpio work.
	CLR r0, r0, 4
	SBCO r0, C4, 4, 4
	
	// set sda_write and scl_write pins to outputs
	mov r1, PWM_0_WRITE_BANK | GPIO_OE
	lbbo r0, r1, 0, 4
	clr r0, r0, PWM_0_WRITE_BIT_NUMBER
        sbbo r0, r1, 0, 4
	
	lbco r0, CONST_PRUDRAM, SP_reg, 8 //pop r0 and r1 off of stack
	add SP_reg, SP_reg, 8
	RET

//---------------------------------------------------
SET_PWM_0:
	sub SP_reg, SP_reg, 8
	sbco r0, CONST_PRUDRAM, SP_reg, 8
	
	mov r0, 1 << PWM_0_WRITE_BIT_NUMBER
	mov r1, PWM_0_WRITE_BANK | GPIO_CLEARDATAOUT //connected to open collector, so this will allow sda to pull up to 1
	sbbo r0, r1, 0, 4 // write to the GPIO register location
	lbco r0, CONST_PRUDRAM, SP_reg, 8
	add SP_reg, SP_reg, 8 // pop the saved registers off the stack
	ret
//------------------------------------------------------
CLEAR_PWM_0:
	sub SP_reg, SP_reg, 8
	sbco r0, CONST_PRUDRAM, SP_reg, 8
	
	mov r0, 1 << PWM_0_WRITE_BIT_NUMBER
	mov r1, PWM_0_WRITE_BANK | GPIO_SETDATAOUT //connected to open collector, so this will allow sda to pull up to 1
	sbbo r0, r1, 0, 4 // write to the GPIO register location
	lbco r0, CONST_PRUDRAM, SP_reg, 8
	add SP_reg, SP_reg, 8 // pop the saved registers off the stack
	ret
//---------------------------------------------------------

