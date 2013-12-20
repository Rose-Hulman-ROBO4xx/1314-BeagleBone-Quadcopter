#include "control_alg.hp"
#include "conventions.hp"

.struct imu_data_frame
	.u16 x_a
	.u16 y_a
	.u16 z_a
	.u16 x_g
	.u16 y_g
	.u16 z_g
.ends
.struct pwm_state_frame
	.u32 N
	.u32 E
	.u32 S
	.u32 W
.ends



CONTROL_MAIN:
	// Configure the block index register for PRU0 by setting c24_blk_index[7:0] and
	// c25_blk_index[7:0] field to 0x00 and 0x00, respectively.  This will make C24 point
	// to 0x00000000 (PRU0 DRAM) and C25 point to 0x00002000 (PRU1 DRAM).
	MOV       r0, 0x00000000
	MOV       r1, CTBIR_0
	ST32      r0, r1
	
	mov SP_reg, 1000 // set up the stack
	
	.assign imu_data_frame, RET_VAL_0, RET_VAL_2, imu_return_data
	.assign pwm_state_frame, R9, R12, pwm_state

	call PWM_ENABLE_GPIO_AND_SET_DIRECTIONS
	
	mov pwm_state.N, PWM_MIN
	mov pwm_state.E, PWM_MIN
	mov pwm_state.S, PWM_MIN
	mov pwm_state.W, PWM_MIN
	mov r14, 0
DATA_LOOP:
	lbco ARG_0,CONST_PRUDRAM, 32, 4
	lbco ARG_1,CONST_PRUDRAM, 36, 4
	lbco ARG_2,CONST_PRUDRAM, 40, 4
	lbco ARG_3,CONST_PRUDRAM, 44, 4
	call SEND_PWM_PULSE
	call PWM_DELAY
	

	lbco r13, CONST_PRUDRAM, 0, 4
	QBNE DATA_LOOP, R13, 0
EXIT:
	mov r3, 0
	sbco r3, CONST_PRUDRAM, 0, 4
	mov ARG_0, PWM_MIN
	mov ARG_1, PWM_MIN
	mov ARG_2, PWM_MIN
	mov ARG_3, PWM_MIN
	
	//clear esc lpf
	call SEND_PWM_PULSE
	call PWM_DELAY
	call SEND_PWM_PULSE
	call PWM_DELAY
	call SEND_PWM_PULSE
	call PWM_DELAY
	call SEND_PWM_PULSE
	call PWM_DELAY
	call SEND_PWM_PULSE
	call PWM_DELAY
	call SEND_PWM_PULSE
	call PWM_DELAY
	call SEND_PWM_PULSE
	call PWM_DELAY
	call SEND_PWM_PULSE
	call PWM_DELAY

    // Send notification to Host for program completion
    MOV R31.b0, PRU0_ARM_INTERRUPT+16
    
    HALT
//------------------------------------------------------------

#include "pwm.p"
