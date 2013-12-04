#include "control_alg.hp"
#include "conventions.hp"
#include "pwm.p"
#include "imu.p"



CONTROL_MAIN:
	// Configure the block index register for PRU0 by setting c24_blk_index[7:0] and
	// c25_blk_index[7:0] field to 0x00 and 0x00, respectively.  This will make C24 point
	// to 0x00000000 (PRU0 DRAM) and C25 point to 0x00002000 (PRU1 DRAM).
	MOV       r0, 0x00000000
	MOV       r1, CTBIR_0
	ST32      r0, r1

	mov SP_reg, 100 // set up the stack
	call PWM_ENABLE_GPIO_AND_SET_DIRECTIONS
	call IMU_ENABLE_GPIO_AND_SET_DIRECTIONS


    // Send notification to Host for program completion
    MOV R31.b0, PRU0_ARM_INTERRUPT+16
    
    HALT
//------------------------------------------------------------

