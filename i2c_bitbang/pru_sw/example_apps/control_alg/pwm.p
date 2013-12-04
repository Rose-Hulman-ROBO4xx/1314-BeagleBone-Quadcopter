#include "pwm.hp"
#include "conventions.hp"


//------------------------------------------------------------
PWM_DELAY:
	sub SP_reg, SP_reg, 8 // gonna push something onto the stack
	sbco r0, CONST_PRUDRAM, SP_reg, 8 //store r0 and r1
	
	mov r0, 0 // set delay amount to 1 million cycles
	mov r1, PWM_DELAY_TIME

PWM_DELAY_LOOP:
	add r0, r0, 1
	qblt PWM_DELAY_LOOP, r1, r0 //are we done busy waiting yet?
	
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
PWM_ENABLE_GPIO_AND_SET_DIRECTIONS:
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
	
	lbco r0, CONST_PRUDRAM, SP_reg, 8 //pop r0 and r1 off of stack
	add SP_reg, SP_reg, 8
	RET



