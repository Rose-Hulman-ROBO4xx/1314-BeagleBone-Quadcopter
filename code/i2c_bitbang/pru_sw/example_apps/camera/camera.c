#include <stdio.h>
#include <stdlib.h>
#include <prussdrv.h>
#include <pruss_intc_mapping.h>
#define PRU_NUM 	1
#define PRU_FILE_NAME "camera.bin"
volatile static void *pruDataMem;
volatile static void *pruExtMem;
volatile static signed int *pruDataMem_int;
volatile static unsigned char * pruExtMem_byte;



void initialize_pru();
void start_pru();
void uninitialize_pru();
void signal_handler(int sig);

void signal_handler(int sig){
	pruDataMem_int[0] = 0;
}

void initialize_pru(){
    unsigned int ret;
    tpruss_intc_initdata pruss_intc_initdata = PRUSS_INTC_INITDATA;

    prussdrv_init ();
    /* Open PRU Interrupt */
    if (PRU_NUM == 0){
        ret = prussdrv_open(PRU_EVTOUT_0);
    }
    if (PRU_NUM == 1){
        ret = prussdrv_open(PRU_EVTOUT_1);
    }
    if (ret)
    {
        printf("prussdrv_open open failed\n");
        exit(ret);
    }
    
    /* Get the interrupt initialized */
    prussdrv_pruintc_init(&pruss_intc_initdata);

    //Initialize pointer to PRU data memory
    if (PRU_NUM == 0)
    {
      prussdrv_map_prumem (PRUSS0_PRU0_DATARAM, &pruDataMem);
    }
    else if (PRU_NUM == 1)
    {
      prussdrv_map_prumem (PRUSS0_PRU1_DATARAM, &pruDataMem);
    }
    pruDataMem_int = (volatile signed int*) pruDataMem;

    //initialize external ram:
    prussdrv_map_extmem(&pruExtMem);
    pruExtMem_byte = (volatile unsigned char*) pruExtMem;
    int i;
    for (i = 0; i < 0x80001; i++){
        pruExtMem_byte[i] = 255;
    }
    pruDataMem_int[0] = 1;
    pruDataMem_int[2] = pruExtMem;
    pruDataMem_int[3] = pruExtMem + 640*480;//((int)prussdrv_get_phys_addr(pruExtMem)) + 640*480;
    printf("%p, %p\n", (void *) pruExtMem_byte, (void*) pruExtMem);
    printf("%p\n", prussdrv_get_phys_addr(pruExtMem));
    
}
void start_pru(){
    prussdrv_exec_program (PRU_NUM, PRU_FILE_NAME);
    pruDataMem_int[0] = 1;
}


void uninitialize_pru(){
    pruDataMem_int[0] = 0;

    /* Wait until PRU0 has finished execution */
    printf("\tINFO: Waiting for HALT command.\r\n");
    if (PRU_NUM == 0){
        prussdrv_pru_wait_event (PRU_EVTOUT_0);
    }
    if (PRU_NUM == 1){
        prussdrv_pru_wait_event (PRU_EVTOUT_1);
    }

    printf("\tINFO: PRU completed transfer.\r\n");
    if (PRU_NUM == 0){
        prussdrv_pru_clear_event (PRU0_ARM_INTERRUPT);
    }
    if (PRU_NUM == 1){
        prussdrv_pru_clear_event (PRU1_ARM_INTERRUPT);
    }



    /* Disable PRU and close memory mapping*/
    prussdrv_pru_disable (PRU_NUM);
    prussdrv_exit ();


}
int main (void)
{

	initialize_pru();
	start_pru();
/*
	system( "echo 87 > /sys/class/gpio/unexport" );
	system( "echo 86 > /sys/class/gpio/unexport" );
	system( "echo 14 > /sys/class/gpio/unexport" );

	if( 0 != system( "echo 07 > /sys/kernel/debug/omap_mux/gpmc_ad8" ) )
		exit(-1)

	if( 0 != system( "echo 07 > /sys/kernel/debug/omap_mux/gpmc_ad8" ) )
		exit(-1)

	if( 0 != system( "echo 07 > /sys/kernel/debug/omap_mux/gpmc_ad8" ) )
		exit(-1)
*/
	/*while(1){
		printf("%d\n", pruDataMem_int[100]);
	}	
*/
	uninitialize_pru();
	return(0);
}


