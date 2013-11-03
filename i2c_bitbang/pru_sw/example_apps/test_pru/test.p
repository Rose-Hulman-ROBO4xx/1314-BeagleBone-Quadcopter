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

#include "test.hp"
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
    mov r0, 1
    sbco r0, CONST_PRUDRAM, 8, 4
    
    mov r1, GPIO1 | GPIO_OE
    lbbo r0, r1, 0, 4
    clr r0, r0, 28
    sbbo r0, r1, 0, 4

    mov r0, 2
    sbco r0, CONST_PRUDRAM, 8, 4
    
    mov r0, GPIO1 | GPIO_SETDATAOUT
    mov r1, 1 << 28
    sbbo r1, r0, 0, 4




    mov r0, 5
    sbco r0, CONST_PRUDRAM, 8, 4
#ifdef AM33XX	

    // Send notification to Host for program completion
    MOV R31.b0, PRU0_ARM_INTERRUPT+16

#else

    MOV R31.b0, PRU0_ARM_INTERRUPT

#endif

    HALT
//------------------------------------------------------------

