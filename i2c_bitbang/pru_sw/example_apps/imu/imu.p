// *
// * PRU_memAccessPRUDataRam.p
// *
// * Copyright (C) 2012 Texas Instruments Incorporated - http://www.ti.com/
// *
// *
// *  Redistribution and use in source and binary forms, with or without
// *  modification, are permitted provided that the following conditions
// *  are met:
// *
// *    Redistributions of source code must retain the above copyright
// *    notice, this list of conditions and the following disclaimer.
// *
// *    Redistributions in binary form must reproduce the above copyright
// *    notice, this list of conditions and the following disclaimer in the
// *    documentation and/or other materials provided with the
// *    distribution.
// *
// *    Neither the name of Texas Instruments Incorporated nor the names of
// *    its contributors may be used to endorse or promote products derived
// *    from this software without specific prior written permission.
// *
// *  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
// *  "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
// *  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
// *  A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
// *  OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
// *  SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
// *  LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
// *  DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
// *  THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
// *  (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// *  OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
// *
// *

// *
// * ============================================================================
// * Copyright (c) Texas Instruments Inc 2010-12
// *
// * Use of this software is controlled by the terms and conditions found in the
// * license agreement under which this software has been supplied or provided.
// * ============================================================================
// *

// *****************************************************************************/
// file:   PRU_memAccessPRUDataRam.p 
//
// brief:  PRU access of internal Data Ram.
//
//
//  (C) Copyright 2012, Texas Instruments, Inc
//
//  author     M. Watkins
//
//  version    0.1     Created
// *****************************************************************************/

#include "imu.hp"
#define RA_REG 			R30.w0 //return address register
.setcallreg 			RA_REG
.origin 0
.entrypoint IMU_MAIN

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

#define SCL_WRITE_BANK		GPIO1
#define SCL_WRITE_BIT_NUMBER	28
#define SCL_READ_BANK		GPIO1
#define SCL_READ_BIT_NUMBER	18

#define SDA_WRITE_BANK 		GPIO0
#define SDA_WRITE_BIT_NUMBER	30
#define SDA_READ_BANK		GPIO0
#define SDA_READ_BIT_NUMBER	31

#define IMU_INT_BANK		GPIO1
#define IMU_INT_BIT_NUMBER	16

#define DELAY_TIME 		50
#define ARG_0 			R19
#define ARG_1			R20
#define ARG_2			R21
#define RET_VAL_0		R22
#define RET_VAL_1		R23
#define RET_VAL_2		R24


IMU_MAIN:
      
#ifdef AM33XX

    // Configure the block index register for PRU0 by setting c24_blk_index[7:0] and
    // c25_blk_index[7:0] field to 0x00 and 0x00, respectively.  This will make C24 point
    // to 0x00000000 (PRU0 DRAM) and C25 point to 0x00002000 (PRU1 DRAM).
    MOV       r0, 0x00000000
    MOV       r1, CTBIR_0
    ST32      r0, r1

#endif
	mov SP_reg, 100 // set up the stack
	call ENABLE_GPIO_AND_SET_DIRECTIONS

	mov ARG_0.b0, 0xD0 //wake up the device
	mov ARG_0.b1, 0x6B
	mov ARG_0.b2, 0x00
	call WRITE_BYTE

	mov ARG_0.b0, 0xD0	//configure the interrupt 
	mov ARG_0.b1, 0x37
	mov ARG_0.b2, 0b00110000
	call WRITE_BYTE
	
	mov ARG_0.b0, 0xD0 //enable the interrupt pin on data_rdy
	mov ARG_0.b1, 0x38
	mov ARG_0.b2, 0x01
	call WRITE_BYTE	
	
	mov ARG_0.b0, 0xD0 //set the DLPF
	mov ARG_0.b1, 0x1A
	mov ARG_0.b2, 0x06
	call WRITE_BYTE
	
	mov ARG_0.b0, 0xD0 //set the sample rate
	mov ARG_0.b1, 0x19
	mov ARG_0.b2, 0x01
	call WRITE_BYTE

	//configure tmp101

	mov r5, 0
	sbco r5, CONST_PRUDRAM, 32, 4
	mov r4, 2000
REPEAT_MEASURE:
	
WAIT_FOR_DATA_TO_BE_READY:
	call READ_IMU_INT
	qbeq WAIT_FOR_DATA_TO_BE_READY, RET_VAL_0, 0
	

	//read the tmp101
	mov ARG_0.b0, 0x92
	mov ARG_0.b1, 0x00
	call READ_BYTE
	mov r3, RET_VAL_0
	sbco r3, CONST_PRUDRAM, 36, 4


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

	add r5, r5, 1
	
	qbgt REPEAT_MEASURE, r5, r4
	
	mov r3, 0
	sbco r3, CONST_PRUDRAM, 0, 4
#ifdef AM33XX	

    // Send notification to Host for program completion
    MOV R31.b0, PRU0_ARM_INTERRUPT+16

#else

    MOV R31.b0, PRU0_ARM_INTERRUPT

#endif

    HALT
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
	call SET_SDA //gotta let the slave ack, so release sda
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
	
	

	call CLEAR_SCL //master needs to acknowledge the slave
	call DELAY
	call CLEAR_SDA
	call DELAY
	call SET_SCL
	call DELAY
	call CLEAR_SCL
	call DELAY
	call SET_SDA
	call DELAY
	call SET_SCL
	call DELAY
	
	call SEND_START
	

	or r1.b0, r1.b3, 0x01 //clear the r/w bit (we are writing now)
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
	
	
	
	call CLEAR_SCL
	call SET_SDA
	call DELAY
	call SET_SCL
	call DELAY //TODO: supposed to be a wait for ack
	call CLEAR_SCL
	call DELAY

	//now read the actuall data! yay!
	
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

	
	
	call CLEAR_SCL
	call DELAY
	call SET_SDA
	call DELAY
	call SET_SCL
	call DELAY
	call CLEAR_SCL
	call DELAY
	call CLEAR_SDA
	call SET_SCL
	call SET_SDA
	
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
	call SET_SDA //gotta let the slave ack, so release sda
	call DELAY
	call SET_SCL //clock ack bit

//WRITE_BYTE_WAIT_FOR_ACK_1:
//	call READ_SDA
//	qbne WRITE_BYTE_WAIT_FOR_ACK_1, RET_VAL_0, 0
	
	

	call DELAY
	call CLEAR_SCL
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
	
	
	call CLEAR_SCL //master needs to acknowledge the slave
	call DELAY
	call CLEAR_SDA
	call DELAY
	call SET_SCL
	call DELAY
	call CLEAR_SCL
	call DELAY
	call SET_SDA
	call DELAY

	
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
	
	call CLEAR_SCL
	call DELAY
	call CLEAR_SDA
	call DELAY
	call SET_SCL
	call DELAY
	call CLEAR_SCL
	call DELAY
	call SET_SCL
	call DELAY
	call SET_SDA
	call DELAY
	
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
	mov r1, SDA_WRITE_BANK | GPIO_OE
	lbbo r0, r1, 0, 4
	clr r0, r0, SDA_WRITE_BIT_NUMBER
        sbbo r0, r1, 0, 4

	mov r1, SCL_WRITE_BANK | GPIO_OE
	clr r0, r0, SCL_WRITE_BIT_NUMBER
	sbbo r0, r1, 0, 4
	
	// set sda_read and scl_read and INT_IMU pins to inputs
	mov r1, SDA_READ_BANK | GPIO_OE
	lbbo r0, r1, 0, 4
	set r0, r0, SDA_READ_BIT_NUMBER
	sbbo r0, r1, 0, 4
	mov r1, SCL_READ_BANK | GPIO_OE
	lbbo r0, r1, 0, 4
	set r0, r0, SCL_READ_BIT_NUMBER
	sbbo r0, r1, 0, 4

	mov r1, IMU_INT_BANK | GPIO_OE
	lbbo r0, r1, 0, 4
	set r0, r0, IMU_INT_BIT_NUMBER
	sbbo r0, r1, 0, 4

	
	lbco r0, CONST_PRUDRAM, SP_reg, 8 //pop r0 and r1 off of stack
	add SP_reg, SP_reg, 8
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
	mov RET_VAL_0, SDA_READ_BANK | GPIO_DATAIN
	lbbo RET_VAL_0, RET_VAL_0, 0, 4 //read the SDA line
	qbbs READ_SDA_HIGH, RET_VAL_0, SDA_READ_BIT_NUMBER
	mov RET_VAL_0, 0
	jmp READ_SDA_DONE
READ_SDA_HIGH:
	mov RET_VAL_0, 1
READ_SDA_DONE:
	ret
	
//------------------------------------------------------------
READ_SCL:
	//using one of the retval registers because I don't want to have to touch the stack to save one of our gp registers
	mov RET_VAL_0, SCL_READ_BANK | GPIO_DATAIN
	lbbo RET_VAL_0, RET_VAL_0, 0, 4 //read the SDA line
	qbbs READ_SCL_HIGH, RET_VAL_0, SCL_READ_BIT_NUMBER
	mov RET_VAL_0, 0
	jmp READ_SCL_DONE
READ_SCL_HIGH:
	mov RET_VAL_0, 1
READ_SCL_DONE:
	ret
	
	
	
//---------------------------------------------------
SET_SDA:
	sub SP_reg, SP_reg, 8
	sbco r0, CONST_PRUDRAM, SP_reg, 8
	
	mov r0, 1 << SDA_WRITE_BIT_NUMBER
	mov r1, SDA_WRITE_BANK | GPIO_CLEARDATAOUT //connected to open collector, so this will allow sda to pull up to 1
	sbbo r0, r1, 0, 4 // write to the GPIO register location
	lbco r0, CONST_PRUDRAM, SP_reg, 8
	add SP_reg, SP_reg, 8 // pop the saved registers off the stack
	ret
//------------------------------------------------------
CLEAR_SDA:
	sub SP_reg, SP_reg, 8
	sbco r0, CONST_PRUDRAM, SP_reg, 8
	
	mov r0, 1 << SDA_WRITE_BIT_NUMBER
	mov r1, SDA_WRITE_BANK | GPIO_SETDATAOUT //connected to open collector, so this will allow sda to pull up to 1
	sbbo r0, r1, 0, 4 // write to the GPIO register location
	lbco r0, CONST_PRUDRAM, SP_reg, 8
	add SP_reg, SP_reg, 8 // pop the saved registers off the stack
	ret
//-----------------------------------------------------
SET_SCL:
	sub SP_reg, SP_reg, 8
	sbco r0, CONST_PRUDRAM, SP_reg, 8
	
	mov r0, 1 << SCL_WRITE_BIT_NUMBER
	mov r1, SCL_WRITE_BANK | GPIO_CLEARDATAOUT //connected to open collector, so this will allow sda to pull up to 1
	sbbo r0, r1, 0, 4 // write to the GPIO register location
	lbco r0, CONST_PRUDRAM, SP_reg, 8
	add SP_reg, SP_reg, 8 // pop the saved registers off the stack
	ret
//------------------------------------------------------
CLEAR_SCL:
	sub SP_reg, SP_reg, 8
	sbco r0, CONST_PRUDRAM, SP_reg, 8
	
	mov r0, 1 << SCL_WRITE_BIT_NUMBER
	mov r1, SCL_WRITE_BANK | GPIO_SETDATAOUT //connected to open collector, so this will allow sda to pull up to 1
	sbbo r0, r1, 0, 4 // write to the GPIO register location
	lbco r0, CONST_PRUDRAM, SP_reg, 8
	add SP_reg, SP_reg, 8 // pop the saved registers off the stack
	ret
//-----------------------------------------------------




