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
.struct imu_avg_data_frame
	.u32 x_a
	.u32 y_a
	.u32 z_a
	.u32 x_g
	.u32 y_g
	.u32 z_g
.ends




CONTROL_MAIN:
	// Configure the block index register for PRU0 by setting c24_blk_index[7:0] and
	// c25_blk_index[7:0] field to 0x00 and 0x00, respectively.  This will make C24 point
	// to 0x00000000 (PRU0 DRAM) and C25 point to 0x00002000 (PRU1 DRAM).
	MOV       r0, 0x00000000
	MOV       r1, CTBIR_0
	ST32      r0, r1
	
	mov SP_reg, 100 // set up the stack
	
	.assign imu_avg_data_frame, R8, R13, imu_temporary_average
	.assign imu_data_frame, RET_VAL_0, RET_VAL_2, imu_return_data
	.assign imu_data_frame, R0, r2, imu_avg

	call PWM_ENABLE_GPIO_AND_SET_DIRECTIONS
	call IMU_ENABLE_GPIO_AND_SET_DIRECTIONS
	mov imu_temporary_average.x_a, 0
	mov imu_temporary_average.y_a, 0
	mov imu_temporary_average.z_a, 0

	mov imu_temporary_average.x_g, 0
	mov imu_temporary_average.y_g, 0
	mov imu_temporary_average.z_g, 0

	mov R6, 0
	mov R7, 4096

GET_AVERAGE_IMU_LOOP:
	call GET_IMU_DATA
	add imu_temporary_average.x_a, imu_temporary_average.x_a,  imu_return_data.x_a
	add imu_temporary_average.y_a, imu_temporary_average.y_a,  imu_return_data.y_a
	add imu_temporary_average.z_a, imu_temporary_average.z_a,  imu_return_data.z_a
	add imu_temporary_average.x_g, imu_temporary_average.x_g,  imu_return_data.x_g
	add imu_temporary_average.y_g, imu_temporary_average.y_g,  imu_return_data.y_g
	add imu_temporary_average.z_g, imu_temporary_average.z_g,  imu_return_data.z_g
	add R6, R6, 1
	QBNE GET_AVERAGE_IMU_LOOP, R6, R7

	lsr imu_avg.x_a, imu_temporary_average.x_a, 12
	lsr imu_avg.y_a, imu_temporary_average.y_a, 12
	lsr imu_avg.z_a, imu_temporary_average.z_a, 12
	lsr imu_avg.x_g, imu_temporary_average.x_g, 12
	lsr imu_avg.y_g, imu_temporary_average.y_g, 12
	lsr imu_avg.z_g, imu_temporary_average.z_g, 12

	lsr imu_temporary_average.x_a, imu_temporary_average.x_a, 12
	lsr imu_temporary_average.y_a, imu_temporary_average.y_a, 12
	lsr imu_temporary_average.z_a, imu_temporary_average.z_a, 12
	lsr imu_temporary_average.x_g, imu_temporary_average.x_g, 12
	lsr imu_temporary_average.y_g, imu_temporary_average.y_g, 12
	lsr imu_temporary_average.z_g, imu_temporary_average.z_g, 12
	

	SBCO      imu_temporary_average, CONST_PRUDRAM, 8, 24


    // Send notification to Host for program completion
    MOV R31.b0, PRU0_ARM_INTERRUPT+16
    
    HALT
//------------------------------------------------------------

#include "pwm.p"
#include "imu.p"
