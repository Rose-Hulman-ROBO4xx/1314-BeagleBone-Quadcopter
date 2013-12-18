#include "pwm.hp"
#define RA_REG 			R30.w0 //return address register
.setcallreg 			RA_REG
.origin 0
.entrypoint PWM_MAIN

#define SP_reg 			r18

#define GPIO1 0x4804c000
#define GPIO0 0x44E07000
#define GPIO_OE			0x134
#define GPIO_DATAIN		0x138
#define GPIO_CLEARDATAOUT 	0x190
#define GPIO_SETDATAOUT 	0x194

#define PWM_0_BANK		GPIO0
#define PWM_0_BIT		3
#define PWM_1_BANK		GPIO0
#define PWM_1_BIT		2
#define PWM_2_BANK		GPIO0
#define PWM_2_BIT		15
#define PWM_3_BANK		GPIO0
#define PWM_3_BIT		14

#define DELAY_TIME		500000
#define PRU_CONFIG		C4

#define ARG_0 			R17
#define ARG_1			R18
#define ARG_2			R19
#define ARG_3			R20
#define RET_VAL_0		R21
#define RET_VAL_1		R22
#define RET_VAL_2		R23
#define RET_VAL_3		R24



PWM_MAIN:
	// Configure the block index register for PRU0 by setting c24_blk_index[7:0] and
	// c25_blk_index[7:0] field to 0x00 and 0x00, respectively.  This will make C24 point
	// to 0x00000000 (PRU0 DRAM) and C25 point to 0x00002000 (PRU1 DRAM).
	MOV       r0, 0x00000000
	MOV       r1, CTBIR_0
	ST32      r0, r1

	mov SP_reg, 100 // set up the stack
	call ENABLE_GPIO_AND_SET_DIRECTIONS


MAIN_PWM_LOOP:
	lbco ARG_0,CONST_PRUDRAM, 8, 4
	lbco ARG_1,CONST_PRUDRAM, 12, 4
	lbco ARG_2,CONST_PRUDRAM, 16, 4
	lbco ARG_3,CONST_PRUDRAM, 20, 4

	call SEND_PWM_PULSE
	call DELAY
	
	lbCo r0, CONST_PRUDRAM, 4, 4
	qbne MAIN_PWM_LOOP, r0, 0
	

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
SEND_PWM_PULSE:
	//ARG_0: channel 0 high period
	sub SP_reg, SP_reg, 4
	sbco RA_REG, CONST_PRUDRAM, SP_reg, 4
	sub SP_reg, SP_reg, 16
	sbco r0, CONST_PRUDRAM, SP_reg, 16
	//set the channel 0 and channel 1 high
	mov r1, 1 << PWM_0_BIT
	mov r2, PWM_0_BANK | GPIO_SETDATAOUT
	sbbo r1, r2, 0, 4

	mov r1,  1 << PWM_1_BIT
	mov r2, PWM_1_BANK | GPIO_SETDATAOUT
	sbbo r1, r2, 0, 4

	mov r1,  1 << PWM_2_BIT
	mov r2, PWM_2_BANK | GPIO_SETDATAOUT
	sbbo r1, r2, 0, 4

	mov r1,  1 << PWM_3_BIT
	mov r2, PWM_3_BANK | GPIO_SETDATAOUT
	sbbo r1, r2, 0, 4
	
	mov r0, 0	//counting variable
	mov r3, 0b00001111 //need to clear all these channels
PWM_PULSE_LOOP:
	add r0, r0, 1
	qblt SKIP_0, ARG_0, r0

	mov r1,  1 << PWM_0_BIT
	mov r2, PWM_0_BANK | GPIO_CLEARDATAOUT
	sbbo r1, r2, 0, 4
	clr r3, r3, 0
	
	add r0, r0, 4
SKIP_0:
	add r0, r0, 1
	qblt SKIP_1, ARG_1, r0

	mov r1, 1 << PWM_1_BIT
	mov r2, PWM_1_BANK | GPIO_CLEARDATAOUT
	sbbo r1, r2, 0, 4
	clr r3, r3, 1

	add r0, r0, 4

SKIP_1:
	add r0, r0, 1
	qblt SKIP_2, ARG_2, r0

	mov r1,  1 << PWM_2_BIT
	mov r2, PWM_2_BANK | GPIO_CLEARDATAOUT
	sbbo r1, r2, 0, 4
	clr r3, r3, 2
	
	add r0, r0, 4
SKIP_2:
	add r0, r0, 1
	qblt SKIP_3, ARG_3, r0

	mov r1,  1 << PWM_3_BIT
	mov r2, PWM_3_BANK | GPIO_CLEARDATAOUT
	sbbo r1, r2, 0, 4
	clr r3, r3, 3
	
	add r0, r0, 4
SKIP_3:

	add r0, r0, 1
	qbne PWM_PULSE_LOOP, r3, 0

	
	lbco r0, CONST_PRUDRAM, SP_reg, 16
	add SP_reg, SP_reg, 16
	lbco RA_REG, CONST_PRUDRAM, SP_reg, 4
	add SP_reg, SP_reg, 4
	
	ret


//--------------------------------------------------
ENABLE_GPIO_AND_SET_DIRECTIONS:
	sub SP_reg, SP_reg, 8 //push r0 and r1 onto stack
	sbco r0, CONST_PRUDRAM, SP_reg, 8

	LBCO r0, C4, 4, 4 //do something for the gpio??? This is black magic to me. but it makes gpio work.
	CLR r0, r0, 4
	SBCO r0, C4, 4, 4
	
	// set sda_write and scl_write pins to outputs
	mov r1, PWM_0_BANK | GPIO_OE
	lbbo r0, r1, 0, 4
	clr r0, r0, PWM_0_BIT
        sbbo r0, r1, 0, 4
	
	mov r1, PWM_1_BANK | GPIO_OE
	lbbo r0, r1, 0, 4
	clr r0, r0, PWM_1_BIT
        sbbo r0, r1, 0, 4

	mov r1, PWM_2_BANK | GPIO_OE
	lbbo r0, r1, 0, 4
	clr r0, r0, PWM_2_BIT
        sbbo r0, r1, 0, 4

	mov r1, PWM_3_BANK | GPIO_OE
	lbbo r0, r1, 0, 4
	clr r0, r0, PWM_3_BIT
        sbbo r0, r1, 0, 4
	
	lbco r0, CONST_PRUDRAM, SP_reg, 8 //pop r0 and r1 off of stack
	add SP_reg, SP_reg, 8
	RET



