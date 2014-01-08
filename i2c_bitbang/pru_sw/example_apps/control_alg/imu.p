#include "imu.hp"
#include "conventions.hp"


GET_IMU_DATA:
	sub SP_reg, SP_reg, 4
	sbco RA_REG, CONST_PRUDRAM, SP_reg, 4

	//make room for the data
	sub SP_reg, SP_reg, 12
	sbco r0, CONST_PRUDRAM, SP_reg, 12
	
WAIT_FOR_DATA_TO_BE_READY:
	call READ_IMU_INT
	qbeq WAIT_FOR_DATA_TO_BE_READY, RET_VAL_0, 0


	mov ARG_0.b0, 0x68
	mov ARG_0.b1, 0x3B
	call READ_BYTE
	mov r0.b1, RET_VAL_0
	mov ARG_0.b1, 0x3C
	call READ_BYTE
	mov r0.b0, RET_VAL_0

	mov ARG_0.b0, 0x68
	mov ARG_0.b1, 0x3D
	call READ_BYTE
	mov r0.b3, RET_VAL_0
	mov ARG_0.b1, 0x3E
	call READ_BYTE
	mov r0.b2, RET_VAL_0

	mov ARG_0.b0, 0x68
	mov ARG_0.b1, 0x3F
	call READ_BYTE
	mov r1.b1, RET_VAL_0
	mov ARG_0.b1, 0x40
	call READ_BYTE
	mov r1.b0, RET_VAL_0

	mov ARG_0.b0, 0x68
	mov ARG_0.b1, 0x43
	call READ_BYTE
	mov r1.b3, RET_VAL_0
	mov ARG_0.b1, 0x44
	call READ_BYTE
	mov r1.b2, RET_VAL_0

	mov ARG_0.b0, 0x68
	mov ARG_0.b1, 0x45
	call READ_BYTE
	mov r2.b1, RET_VAL_0
	mov ARG_0.b1, 0x46
	call READ_BYTE
	mov r2.b0, RET_VAL_0

	mov ARG_0.b0, 0x68
	mov ARG_0.b1, 0x47
	call READ_BYTE
	mov r2.b3, RET_VAL_0
	mov ARG_0.b1, 0x48
	call READ_BYTE
	mov r2.b2, RET_VAL_0

	mov RET_VAL_0, r0
	mov RET_VAL_1, r1
	mov RET_VAL_2, r2


	//make room for the data
	lbco r0, CONST_PRUDRAM, SP_reg, 12
	add SP_reg, SP_reg, 12
	
	lbco RA_REG, CONST_PRUDRAM, SP_reg, 4
	add SP_reg, SP_reg, 4

	ret

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
IMU_ENABLE_GPIO_AND_SET_DIRECTIONS:
	sub SP_reg, SP_reg, 4
	sbco RA_REG, CONST_PRUDRAM, SP_reg, 4

	sub SP_reg, SP_reg, 8 //push r0 and r1 onto stack
	sbco r0, CONST_PRUDRAM, SP_reg, 8

	LBCO r0, C4, 4, 4 //do something for the gpio??? This is black magic to me. but it makes gpio work.
	CLR r0, r0, 4
	SBCO r0, C4, 4, 4
	
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

	//configure IMU
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
	mov ARG_0.b2, 0x04
	call WRITE_BYTE

	mov ARG_0.b0, 0x68 //set the accelerometer sensitivity
	mov ARG_0.b1, 0x1C
	mov ARG_0.b2, 0b00011000
	call WRITE_BYTE

	mov ARG_0.b0, 0x68 //set the gyro sensitivity
	mov ARG_0.b1, 0x1B
	mov ARG_0.b2, 0b00011000
	call WRITE_BYTE





	lbco r0, CONST_PRUDRAM, SP_reg, 8 //pop r0 and r1 off of stack
	add SP_reg, SP_reg, 8

	lbco RA_REG, CONST_PRUDRAM, SP_reg, 4
	add SP_reg, SP_reg, 4


	RET
//------------------------------------------------------------
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




