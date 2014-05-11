#include <stdio.h>
#include <stdlib.h>
#include <prussdrv.h>
#include <pruss_intc_mapping.h>
#define PRU_NUM 	1
#define PRU_FILE_NAME "changeme.bin"
volatile static void *pruDataMem;
volatile static signed int *pruDataMem_int;



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
}
void start_pru(){
    prussdrv_exec_program (PRU_NUM, "./control_alg.bin");
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
		
	while(pruDataMem_int[0] != 0){
	}

	uninitialize_pru();
	return(0);
}


