#include "control_alg.hp"
#define RA_REG 			R30.w0 //return address register
.setcallreg 			RA_REG
.origin 0
.entrypoint CONTROL_MAIN

#define SP_reg 			r18

#define GPIO1 0x4804c000
#define GPIO0 0x44E07000
#define GPIO_OE			0x134
#define GPIO_DATAIN		0x138
#define GPIO_CLEARDATAOUT 	0x190
#define GPIO_SETDATAOUT 	0x194

#define PWM_0_BANK		GPIO1
#define PWM_0_BIT		28
#define PWM_1_BANK		GPIO0
#define PWM_1_BIT		30
#define PWM_2_BANK		GPIO1
#define PWM_2_BIT		19
#define PWM_3_BANK		GPIO0
#define PWM_3_BIT		5

#define PWM_DELAY_TIME		500000
#define IMU_DELAY_TIME
#define PRU_CONFIG		C4

#define SCL_BANK		GPIO0
#define SCL_BIT			31

#define SDA_BANK 		GPIO1
#define SDA_BIT			18

#define IMU_INT_BANK		GPIO1
#define IMU_INT_BIT_NUMBER	16

#define ARG_0 			R17
#define ARG_1			R18
#define ARG_2			R19
#define ARG_3			R20
#define RET_VAL_0		R21
#define RET_VAL_1		R22
#define RET_VAL_2		R23
#define RET_VAL_3		R24




CONTROL_MAIN:
	// Configure the block index register for PRU0 by setting c24_blk_index[7:0] and
	// c25_blk_index[7:0] field to 0x00 and 0x00, respectively.  This will make C24 point
	// to 0x00000000 (PRU0 DRAM) and C25 point to 0x00002000 (PRU1 DRAM).
	MOV       r0, 0x00000000
	MOV       r1, CTBIR_0
	ST32      r0, r1

	mov SP_reg, 100 // set up the stack
	call ENABLE_GPIO_AND_SET_DIRECTIONS

	mov ARG_0.b0, 0x68 //wake up the device
	mov ARG_0.b1, 0x6B
	mov ARG_0.b2, 0x00
	call WRITE_BYTE

	mov ARG_0.b0, 0x68	//configure the interrupt 
	mov ARG_0.b1, 0x37
	mov ARG_0.b2, 0b00110000
	call WRITE_BYTE
	
	mov ARG_0.b0, 0x68 //enable the interrupt pin on data_rdy
	mov ARG_0.b1, 0x38
	mov ARG_0.b2, 0x01
	call WRITE_BYTE
	
	mov ARG_0.b0, 0x68 //set the DLPF
	mov ARG_0.b1, 0x1A
	mov ARG_0.b2, 0x06
	call WRITE_BYTE

	mov ARG_0.b0, 0x68 //set the sample rate
	mov ARG_0.b1, 0x19
	mov ARG_0.b2, 0x03
	call WRITE_BYTE



MAIN_CONTROL_LOOP:
	lbco ARG_0,CONST_PRUDRAM, 8, 4
	lbco ARG_1,CONST_PRUDRAM, 12, 4
	lbco ARG_2,CONST_PRUDRAM, 16, 4
	lbco ARG_3,CONST_PRUDRAM, 20, 4

	call SEND_PWM_PULSE
	call PWM_DELAY
	
	lbCo r0, CONST_PRUDRAM, 4, 4
	qbne MAIN_CONTROL_LOOP, r0, 0
	

    // Send notification to Host for program completion
    MOV R31.b0, PRU0_ARM_INTERRUPT+16
    
    HALT
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

	// set sda_write and scl_write pins to outputs
	mov r1, SDA_BANK | GPIO_OE
	lbbo r0, r1, 0, 4
	clr r0, r0, SDA_BIT
        sbbo r0, r1, 0, 4

	mov r1, SCL_BANK | GPIO_OE
	lbbo r0, r1, 0, 4
	clr r0, r0, SCL_BIT
	sbbo r0, r1, 0, 4
	
	mov r1, IMU_INT_BANK | GPIO_OE
	lbbo r0, r1, 0, 4
	set r0, r0, IMU_INT_BIT_NUMBER
	sbbo r0, r1, 0, 4


	lbco r0, CONST_PRUDRAM, SP_reg, 8 //pop r0 and r1 off of stack
	add SP_reg, SP_reg, 8
	RET


GET_IMU_DATA:

WAIT_FOR_DATA_TO_BE_READY:
	call READ_IMU_INT
	qbeq WAIT_FOR_DATA_TO_BE_READY, RET_VAL_0, 0


	mov ARG_0.b0, 0x68
	mov ARG_0.b1, 0x3B
	call READ_BYTE
	mov r3, RET_VAL_0
	lsl r3, r3, 8
	mov ARG_0.b1, 0x3C
	call READ_BYTE
	or r3, r3, RET_VAL_0
    //Store result in into memory location c3(PRU0/1 Local Data)+8 using constant table
	SBCO      r3, CONST_PRUDRAM, 8, 4

	mov ARG_0.b0, 0x68
	mov ARG_0.b1, 0x3D
	call READ_BYTE
	mov r3, RET_VAL_0
	lsl r3, r3, 8
	mov ARG_0.b1, 0x3E
	call READ_BYTE
	or r3, r3, RET_VAL_0
    //Store result in into memory location c3(PRU0/1 Local Data)+8 using constant table
	SBCO      r3, CONST_PRUDRAM, 12, 4

	mov ARG_0.b0, 0x68
	mov ARG_0.b1, 0x3F
	call READ_BYTE
	mov r3, RET_VAL_0
	lsl r3, r3, 8
	mov ARG_0.b1, 0x40
	call READ_BYTE
	or r3, r3, RET_VAL_0
    //Store result in into memory location c3(PRU0/1 Local Data)+8 using constant table
	SBCO      r3, CONST_PRUDRAM, 16, 4

	mov ARG_0.b0, 0x68
	mov ARG_0.b1, 0x43
	call READ_BYTE
	mov r3, RET_VAL_0
	lsl r3, r3, 8
	mov ARG_0.b1, 0x44
	call READ_BYTE
	or r3, r3, RET_VAL_0
    //Store result in into memory location c3(PRU0/1 Local Data)+8 using constant table
	SBCO      r3, CONST_PRUDRAM, 20, 4

	mov ARG_0.b0, 0x68
	mov ARG_0.b1, 0x45
	call READ_BYTE
	mov r3, RET_VAL_0
	lsl r3, r3, 8
	mov ARG_0.b1, 0x46
	call READ_BYTE
	or r3, r3, RET_VAL_0
    //Store result in into memory location c3(PRU0/1 Local Data)+8 using constant table
	SBCO      r3, CONST_PRUDRAM, 24, 4

	mov ARG_0.b0, 0x68
	mov ARG_0.b1, 0x47
	call READ_BYTE
	mov r3, RET_VAL_0
	lsl r3, r3, 8
	mov ARG_0.b1, 0x48
	call READ_BYTE
	or r3, r3, RET_VAL_0
    //Store result in into memory location c3(PRU0/1 Local Data)+8 using constant table
	SBCO      r3, CONST_PRUDRAM, 28, 4

	mov r3, 1
	sbco r3, CONST_PRUDRAM, 4, 4

	RET
//------------------------------------------------------------

//this function will read a byte from an i2c device
//ARG_0.b0 is the address
//ARG_0.b1 is the reg number to read

READ_BYTE:
	sub SP_reg, SP_reg, 4
	sbco RA_REG, CONST_PRUDRAM, SP_reg, 4
	
	sub SP_reg, SP_reg, 4
	sbco ARG_0, CONST_PRUDRAM, SP_reg, 4
		
	sub SP_reg, SP_reg, 8
	sbco R0, CONST_PRUDRAM, SP_reg, 8

	lsl ARG_0.b0, ARG_0.b0, 1 //make room for the r/w bit


	call SEND_START //send the start code
	and r1.b0, ARG_0.b0, 0xFE //clear the r/w bit (we are writing now)
	mov r1.b3, r1.b0 //save this address
	mov r0.b0, 0
READ_BYTE_LOOP1:
	call CLEAR_SCL
	call DELAY
	
	and ARG_0.b0, r1.b0, 0x80 //write_sda(address & 0x80)
	call WRITE_SDA
	LSL r1.b0, r1.b0, 1 //address = address << 1;
	call DELAY
	call SET_SCL
	call DELAY
	add r0.b0, r0.b0, 1 //i += 1
	qbgt READ_BYTE_LOOP1, r0.b0, 8
	
	
	

	call CLEAR_SCL //clocking ack bit
	call RELEASE_SDA //gotta let the slave ack, so release sda
	call DELAY
	call SET_SCL //clock ack bit
//READ_BYTE_WAIT_FOR_ACK_1:
//	call READ_SDA
//	qbne READ_BYTE_WAIT_FOR_ACK_1, RET_VAL_0, 0
	
	call DELAY
	call CLEAR_SCL
	call DELAY
	
	
	
	//now write the register address
	mov r1.b1, ARG_0.b1
	mov r0.b0, 0
READ_BYTE_LOOP2:
	call CLEAR_SCL
	call DELAY
	
	and ARG_0.b0, r1.b1, 0x80 //write_sda(register & 0x80)
	call WRITE_SDA
	LSL r1.b1, r1.b1, 1 //register = register << 1;
	call DELAY
	call SET_SCL
	call DELAY
	add r0.b0, r0.b0, 1 //i += 1
	qbgt READ_BYTE_LOOP2, r0.b0, 8
	
	call RELEASE_SDA
	call DELAY
	call CLEAR_SCL //master needs to acknowledge the slavea
	call DELAY
	call SET_SCL
	call DELAY
	call CLEAR_SCL

	call SEND_START
	

	or r1.b0, r1.b3, 0x01 //set the r/w bit (we are reading now)
	mov r0.b0, 0
READ_BYTE_LOOP3:
	call CLEAR_SCL
	call DELAY
	
	and ARG_0.b0, r1.b0, 0x80 //write_sda(address & 0x80)
	call WRITE_SDA
	LSL r1.b0, r1.b0, 1 //address = address << 1;
	call DELAY
	call SET_SCL
	call DELAY
	add r0.b0, r0.b0, 1 //i += 1
	qbgt READ_BYTE_LOOP3, r0.b0, 8
	
	
	
	call RELEASE_SDA
	call CLEAR_SCL
	call DELAY
	call SET_SCL
	call DELAY //TODO: supposed to be a wait for ack
	call CLEAR_SCL
	call DELAY

	//now read the actuall data! yay!
	call RELEASE_SDA
	call DELAY
	
	mov r0.b0, 0
	mov r1, 0
READ_BYTE_LOOP4:
	call CLEAR_SCL
	call DELAY
	lsl r1, r1, 1
	call READ_SDA
	or r1, r1, RET_VAL_0
	call DELAY
	call SET_SCL
	call DELAY
	add r0.b0, r0.b0, 1
	qbgt READ_BYTE_LOOP4, r0.b0, 8

	call SET_SDA
	call SET_SCL
	call DELAY
	call CLEAR_SCL
	call DELAY
	call CLEAR_SDA
	call DELAY
	call SET_SCL
	call DELAY
	call SET_SDA
	call DELAY
	
	mov RET_VAL_0, r1
	
	lbco R0, CONST_PRUDRAM, SP_reg, 8
	add SP_reg, SP_reg, 8

	lbco ARG_0, CONST_PRUDRAM, SP_reg, 4
	add SP_reg, SP_reg, 4
	
	lbco RA_REG, CONST_PRUDRAM, SP_reg, 4
	add SP_reg, SP_reg, 4
	ret

//------------------------------------------------------------
WRITE_BYTE:
	//This function will write a byte to an i2c device
	//ARG_0.b0 is the address
	//ARG_0.b1 is the reg number on the target
	//ARG_0.b2 is the data to be sent to the target

	//gonna push the return address onto the stack
	sub SP_reg, SP_reg, 4
	sbco RA_REG, CONST_PRUDRAM, SP_reg, 4

	// push the argument onto the stack
	sub SP_reg, SP_reg, 4
	sbco ARG_0, CONST_PRUDRAM, SP_reg, 4
	
	sub SP_reg, SP_reg, 8
	sbco R0, CONST_PRUDRAM, SP_reg, 8

	lsl ARG_0.b0, ARG_0.b0, 1 //make room for the r/w bit

	call SEND_START //send the start code
	and r1.b0, ARG_0.b0, 0xFE //clear the r/w bit (we are writing now)
	mov r1.b3, r1.b0 //save this address
	mov r0.b0, 0
WRITE_BYTE_LOOP1:
	call CLEAR_SCL
	call DELAY
	
	and ARG_0.b0, r1.b0, 0x80 //write_sda(address & 0x80)
	call WRITE_SDA
	LSL r1.b0, r1.b0, 1 //address = address << 1;
	call DELAY
	call SET_SCL
	call DELAY
	add r0.b0, r0.b0, 1 //i += 1
	qbgt WRITE_BYTE_LOOP1, r0.b0, 8
	
	call CLEAR_SCL //clocking ack bit
	call DELAY
	call SET_SCL //clock ack bit

//WRITE_BYTE_WAIT_FOR_ACK_1:
//	call READ_SDA
//	qbne WRITE_BYTE_WAIT_FOR_ACK_1, RET_VAL_0, 0
	
	

	call DELAY
	call CLEAR_SCL
	call RELEASE_SDA //gotta let the slave ack, so release 
	call DELAY
	
	//now write the register address
	mov r1.b1, ARG_0.b1
	mov r0.b0, 0
WRITE_BYTE_LOOP2:
	call CLEAR_SCL
	call DELAY
	
	and ARG_0.b0, r1.b1, 0x80 //write_sda(register & 0x80)
	call WRITE_SDA
	LSL r1.b1, r1.b1, 1 //register = register << 1;
	call DELAY
	call SET_SCL
	call DELAY
	add r0.b0, r0.b0, 1 //i += 1
	qbgt WRITE_BYTE_LOOP2, r0.b0, 8
	
	CALL CLEAR_SCL
	CALL CLEAR_SDA
	CALL DELAY
	CALL SET_SCL
	CALL DELAY
	CALL CLEAR_SCL
	CALL RELEASE_SDA

	
	//now we can write the actual data! yay~! ^_^
	mov r1.b2, ARG_0.b2
	mov r0.b0, 0
WRITE_BYTE_LOOP3:
	call CLEAR_SCL
	call DELAY
	
	and ARG_0.b0, r1.b2, 0x80 //write_sda(register & 0x80)
	call WRITE_SDA
	LSL r1.b2, r1.b2, 1 //register = register << 1;
	call DELAY
	call SET_SCL
	call DELAY
	add r0.b0, r0.b0, 1 //i += 1
	qbgt WRITE_BYTE_LOOP3, r0.b0, 8

	CALL CLEAR_SCL
	CALL CLEAR_SDA
	CALL DELAY
	CALL SET_SCL
	CALL DELAY
	CALL CLEAR_SCL
	CALL RELEASE_SDA
	CALL DELAY
	CALL SET_SCL
	CALL DELAY
	CALL SET_SDA

	lbco R0, CONST_PRUDRAM, SP_reg, 8
	add SP_reg, SP_reg, 8

	lbco ARG_0, CONST_PRUDRAM, SP_reg, 4
	add SP_reg, SP_reg, 4
	
	lbco RA_REG, CONST_PRUDRAM, SP_reg, 4
	add SP_reg, SP_reg, 4
	ret
//------------------------------------------------------------
WRITE_SDA:
	//ARG_0.b0 logical value (not bit value) is what SDA will be set to
	sub SP_reg, SP_reg, 4
	sbco RA_REG, CONST_PRUDRAM, SP_reg, 4
	
	qbeq WRITE_SDA_0, ARG_0.b0, 0
	call SET_SDA
	jmp WRITE_SDA_DONE
WRITE_SDA_0:
	call CLEAR_SDA
WRITE_SDA_DONE:

	lbco RA_REG, CONST_PRUDRAM, SP_reg, 4
	add SP_reg, SP_reg, 4
	ret
	
//------------------------------------------------------------
SEND_START:
	sub SP_reg, SP_reg, 4
	sbco RA_REG, CONST_PRUDRAM, SP_reg, 4
	
	call SET_SCL
	call SET_SDA
	call DELAY
	call CLEAR_SDA
	call DELAY	

	lbco RA_REG, CONST_PRUDRAM, SP_reg, 4
	add SP_reg, SP_reg, 4
	ret
//------------------------------------------------------------
DELAY:
	sub SP_reg, SP_reg, 8 // gonna push something onto the stack
	sbco r0, CONST_PRUDRAM, SP_reg, 8 //store r0 and r1
	
	mov r0, 0 // set delay amount to 1 million cycles
	mov r1, IMU_DELAY_TIME

DELAY_LOOP:
	add r0, r0, 1
	qblt DELAY_LOOP, r1, r0 //are we done busy waiting yet?
	
	lbco r0, CONST_PRUDRAM, SP_reg, 8
	add SP_reg, SP_reg, 8
	ret
//--------------------------------------------------
READ_IMU_INT:
	//using one of the retval registers because I don't want to have to touch the stack to save one of our gp registers
	mov RET_VAL_0, IMU_INT_BANK | GPIO_DATAIN
	lbbo RET_VAL_0, RET_VAL_0, 0, 4 //read the SDA line
	qbbs READ_IMU_INT_HIGH, RET_VAL_0, IMU_INT_BIT_NUMBER
	mov RET_VAL_0, 0
	jmp READ_IMU_INT_DONE
READ_IMU_INT_HIGH:
	mov RET_VAL_0, 1
READ_IMU_INT_DONE:
	ret
	
//------------------------------------------------------------
READ_SDA:
	//using one of the retval registers because I don't want to have to touch the stack to save one of our gp registers
	sub SP_reg, SP_reg, 8
	sbco r0, CONST_PRUDRAM, SP_reg, 8
	
	
	//mov r1, SDA_BANK | GPIO_OE
	//lbbo r0, r1, 0, 4
	//set r0, r0, SDA_BIT
	//sbbo r0, r1, 0, 4


	mov RET_VAL_0, SDA_BANK | GPIO_DATAIN
	lbbo RET_VAL_0, RET_VAL_0, 0, 4 //read the SDA line
	qbbs READ_SDA_HIGH, RET_VAL_0, SDA_BIT
	mov RET_VAL_0, 0
	jmp READ_SDA_DONE
READ_SDA_HIGH:
	mov RET_VAL_0, 1
READ_SDA_DONE:
	lbco r0, CONST_PRUDRAM, SP_reg, 8
	add SP_reg, SP_reg, 8 // pop the saved registers off the stack

	ret
	
	
//---------------------------------------------------
CLEAR_SDA:
	sub SP_reg, SP_reg, 8
	sbco r0, CONST_PRUDRAM, SP_reg, 8
	
	mov r1, SDA_BANK | GPIO_OE
	lbbo r0, r1, 0, 4
	clr r0, r0, SDA_BIT
	sbbo r0, r1, 0, 4


	mov r0, 1 << SDA_BIT
	mov r1, SDA_BANK | GPIO_CLEARDATAOUT //connected to open collector, so this will allow sda to pull up to 1
	sbbo r0, r1, 0, 4 // write to the GPIO register location
	lbco r0, CONST_PRUDRAM, SP_reg, 8
	add SP_reg, SP_reg, 8 // pop the saved registers off the stack
	ret
//------------------------------------------------------
SET_SDA:
	sub SP_reg, SP_reg, 8
	sbco r0, CONST_PRUDRAM, SP_reg, 8
	
	mov r1, SDA_BANK | GPIO_OE
	lbbo r0, r1, 0, 4
	clr r0, r0, SDA_BIT
	sbbo r0, r1, 0, 4

	mov r0, 1 << SDA_BIT
	mov r1, SDA_BANK | GPIO_SETDATAOUT //connected to open collector, so this will allow sda to pull up to 1
	sbbo r0, r1, 0, 4 // write to the GPIO register location
	lbco r0, CONST_PRUDRAM, SP_reg, 8
	add SP_reg, SP_reg, 8 // pop the saved registers off the stack
	ret
//-----------------------------------------------------
CLEAR_SCL:
	sub SP_reg, SP_reg, 8
	sbco r0, CONST_PRUDRAM, SP_reg, 8
	
	mov r0, 1 << SCL_BIT
	mov r1, SCL_BANK | GPIO_CLEARDATAOUT //connected to open collector, so this will allow sda to pull up to 1
	sbbo r0, r1, 0, 4 // write to the GPIO register location
	lbco r0, CONST_PRUDRAM, SP_reg, 8
	add SP_reg, SP_reg, 8 // pop the saved registers off the stack
	ret
//------------------------------------------------------
SET_SCL:
	sub SP_reg, SP_reg, 8
	sbco r0, CONST_PRUDRAM, SP_reg, 8
	
	mov r0, 1 << SCL_BIT
	mov r1, SCL_BANK | GPIO_SETDATAOUT //connected to open collector, so this will allow sda to pull up to 1
	sbbo r0, r1, 0, 4 // write to the GPIO register location
	lbco r0, CONST_PRUDRAM, SP_reg, 8
	add SP_reg, SP_reg, 8 // pop the saved registers off the stack
	ret
//-----------------------------------------------------
RELEASE_SDA:
	sub SP_reg, SP_reg, 8
	sbco r0, CONST_PRUDRAM, SP_reg, 8
	
	mov r1, SDA_BANK | GPIO_OE
	lbbo r0, r1, 0, 4
	set r0, r0, SDA_BIT
	sbbo r0, r1, 0, 4

	lbco r0, CONST_PRUDRAM, SP_reg, 8
	add SP_reg, SP_reg, 8
	
	ret




