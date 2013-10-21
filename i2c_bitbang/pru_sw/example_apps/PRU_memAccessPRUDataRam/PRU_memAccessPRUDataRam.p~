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

#define RA_REG R30.w0 //return address register
.setcallreg RA_REG
.origin 0
.entrypoint MEMACCESSPRUDATARAM

#include "PRU_memAccessPRUDataRam.hp"
#define SP_reg r20

#define GPIO1 0x4804c000
#define GPIO0 0x44E07000

#define GPIO_CLEARDATAOUT 0x190
#define GPIO_SETDATAOUT 0x194

#define SCL_BANK GPIO1
#define SCL_BIT_NUMBER 28

#define SDA_BANK GPIO0
#define SDA_BIT_NUMBER 30
#define DELAY_TIME 50000000
#define ARG_0 R21

MEMACCESSPRUDATARAM:
      
#ifdef AM33XX

    // Configure the block index register for PRU0 by setting c24_blk_index[7:0] and
    // c25_blk_index[7:0] field to 0x00 and 0x00, respectively.  This will make C24 point
    // to 0x00000000 (PRU0 DRAM) and C25 point to 0x00002000 (PRU1 DRAM).
    MOV       r0, 0x00000000
    MOV       r1, CTBIR_0
    ST32      r0, r1

#endif
	mov SP_reg, 100 // set up the stack
	call ENABLE_GPIO
	mov r3, 500
	mov ARG_0.b0, 0xD0
	mov ARG_0.b1, 0x6B
	mov ARG_0.b2, 0x00
	call WRITE_BYTE

    //Store result in into memory location c3(PRU0/1 Local Data)+8 using constant table
    SBCO      r3, CONST_PRUDRAM, 8, 4
#ifdef AM33XX	

    // Send notification to Host for program completion
    MOV R31.b0, PRU0_ARM_INTERRUPT+16

#else

    MOV R31.b0, PRU0_ARM_INTERRUPT

#endif

    HALT

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
ENABLE_GPIO:
	sub SP_reg, SP_reg, 4 //push r0 onto stack
	sbco r0, CONST_PRUDRAM, SP_reg, 4
	LBCO r0, C4, 4, 4 //do something for the gpio??? This is black magic to me. but it makes gpio work.
	CLR r0, r0, 4
	SBCO r0, C4, 4, 4
	lbco r0, CONST_PRUDRAM, SP_reg, 8 //pop r0 off of stack
	add SP_reg, SP_reg, 4
	RET
//---------------------------------------------------
SET_SDA:
	sub SP_reg, SP_reg, 8
	sbco r0, CONST_PRUDRAM, SP_reg, 8
	
	mov r0, 1 << SDA_BIT_NUMBER
	mov r1, SDA_BANK | GPIO_CLEARDATAOUT //connected to open collector, so this will allow sda to pull up to 1
	sbbo r0, r1, 0, 4 // write to the GPIO register location
	lbco r0, CONST_PRUDRAM, SP_reg, 8
	add SP_reg, SP_reg, 8 // pop the saved registers off the stack
	ret
//------------------------------------------------------
CLEAR_SDA:
	sub SP_reg, SP_reg, 8
	sbco r0, CONST_PRUDRAM, SP_reg, 8
	
	mov r0, 1 << SDA_BIT_NUMBER
	mov r1, SDA_BANK | GPIO_SETDATAOUT //connected to open collector, so this will allow sda to pull up to 1
	sbbo r0, r1, 0, 4 // write to the GPIO register location
	lbco r0, CONST_PRUDRAM, SP_reg, 8
	add SP_reg, SP_reg, 8 // pop the saved registers off the stack
	ret
//-----------------------------------------------------
SET_SCL:
	sub SP_reg, SP_reg, 8
	sbco r0, CONST_PRUDRAM, SP_reg, 8
	
	mov r0, 1 << SCL_BIT_NUMBER
	mov r1, SCL_BANK | GPIO_CLEARDATAOUT //connected to open collector, so this will allow sda to pull up to 1
	sbbo r0, r1, 0, 4 // write to the GPIO register location
	lbco r0, CONST_PRUDRAM, SP_reg, 8
	add SP_reg, SP_reg, 8 // pop the saved registers off the stack
	ret
//------------------------------------------------------
CLEAR_SCL:
	sub SP_reg, SP_reg, 8
	sbco r0, CONST_PRUDRAM, SP_reg, 8
	
	mov r0, 1 << SCL_BIT_NUMBER
	mov r1, SCL_BANK | GPIO_SETDATAOUT //connected to open collector, so this will allow sda to pull up to 1
	sbbo r0, r1, 0, 4 // write to the GPIO register location
	lbco r0, CONST_PRUDRAM, SP_reg, 8
	add SP_reg, SP_reg, 8 // pop the saved registers off the stack
	ret
//-----------------------------------------------------




