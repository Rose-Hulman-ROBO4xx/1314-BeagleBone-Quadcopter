
.origin 0
.entrypoint MEMACCESSPRUDATARAM

#include "pru_common.hp"
#include "mem_map.h"

#define CAM_BITS r31

#define CAM_DATA_MASK 0xFF
#define CAM_VSYNC_MASK     0x100
#define CAM_HSYNC_MASK     0x200
#define CAM_CLK_MASK    0x10000

#define CAM_VSYNC 8
#define CAM_HSYNC 9
#define CAM_CLK   16

    #define GPIO1 0x4804c000
#define GPIO2 0x481ac000
#define GPIO_CLEARDATAOUT 0x190
#define GPIO_SETDATAOUT 0x194
#define GPIO_DATAOUT 0x13C

#define currBuff r7
#define swapBuff r10
#define buff0 r11
#define buff1 r12
#define pixelByteCount   r13
#define lineCount    r14
#define numCols r8
#define numRows r9
#define exit r6

MEMACCESSPRUDATARAM:

    MOV exit,EXIT_CTRL*4
// Enable OCP master port
    LBCO      r0, CONST_PRUCFG, 4, 4
    CLR     r0, r0, 4         // Clear SYSCFG[STANDBY_INIT] to enable OCP master port
    SBCO      r0, CONST_PRUCFG, 4, 4
    
    // set parallel input capture
    LBCO      r0, CONST_PRUCFG, 0xC, 4
    SET     r0, r0, 0
    SBCO      r0, CONST_PRUCFG, 0xC, 4
    

    MOV       swapBuff, CAM_SWAP_BUFF*4
    MOV       buff0, CAM_BUFF_0*4
    LBBO      buff0, buff0, 0, 4
    MOV       buff1, CAM_BUFF_1*4
    LBBO      buff1, buff1, 0, 4
    
    MOV       currBuff, buff0
    MOV       numCols, 639*2
    MOV       numRows, 480
    
  Loop:
  
    MOV   lineCount, 0
    
    WBC   CAM_BITS, CAM_VSYNC
    WBS   CAM_BITS, CAM_VSYNC
    
    Lines:
      MOV   pixelByteCount, 0
      
      WBC   CAM_BITS, CAM_HSYNC
      WBS   CAM_BITS, CAM_HSYNC

      MOV   r0.b0, CAM_BITS.b0
      WBC   CAM_BITS, CAM_CLK
      WBS   CAM_BITS, CAM_CLK
      MOV   r0.b1, CAM_BITS.b0
 
      SBBO  r0.w0, currBuff, pixelByteCount, 2
      ADD   pixelByteCount, pixelByteCount, 2

      Pixels:
        
        WBC   CAM_BITS, CAM_CLK
        WBS   CAM_BITS, CAM_CLK
        MOV   r0.b0, CAM_BITS.b0
      
        WBC   CAM_BITS, CAM_CLK
        WBS   CAM_BITS, CAM_CLK
        MOV   r0.b1, CAM_BITS.b0
   
        SBBO  r0.w0, currBuff, pixelByteCount, 2
        ADD   pixelByteCount, pixelByteCount, 2
        QBGE  Pixels, pixelByteCount, numCols
    
      ADD   currBuff, currBuff, pixelByteCount
      ADD   lineCount, lineCount, 1
      QBGE  Lines, lineCount, numRows
    
    MOV r2, 6<<22
    MOV r3, GPIO1 | GPIO_DATAOUT
    LBBO r1, r3, 0, 4
    XOR  r1, r1, r2
    SBBO r1, r3, 0, 4
    
    LBBO r0, swapBuff, 0, 4
    QBEQ Loop, r0, 0
    
    MOV   r0, 0
    SBBO  r0, swapBuff, 0, 4
    MOV   r0, buff0
    MOV   buff0, buff1
    MOV   buff1, r0
    MOV   currBuff, buff0

    LBBO r0, exit, 0, 4
    QBEQ Quit, r0, 1
    
    QBA Loop
    





Quit:

    // Send notification to Host for program completion
    MOV R31.b0, PRU1_ARM_INTERRUPT+16


    HALT

    
    
    
    
    
    
    
    
    
    
    
#ifdef junk
start:
    MOV r1, 10
BLINK:
    MOV r2, 7<<22
    MOV r3, GPIO1 | GPIO_SETDATAOUT
    SBBO r2, r3, 0, 4
    MOV r0, 0x00a00000
DELAY:
    SUB r0, r0, 1
    QBNE DELAY, r0, 0
    MOV r2, 7<<22
    MOV r3, GPIO1 | GPIO_CLEARDATAOUT
    SBBO r2, r3, 0, 4
    MOV r0, 0x00a00000
DELAY2:
    SUB r0, r0, 1
    QBNE DELAY2, r0, 0
    SUB r1, r1, 1
    QBNE BLINK, r1, 0
    
qba start
#endif
