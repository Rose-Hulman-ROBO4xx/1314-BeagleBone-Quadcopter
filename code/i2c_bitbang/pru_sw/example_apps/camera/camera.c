#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <prussdrv.h>
#include <pruss_intc_mapping.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#define PRU_NUM 	1
#define PRU_FILE_NAME "camera.bin"

#define DDR_BASEADDR 0x80000000
#define OFFSET_DDR         (224*1024*1024)
#define PRU_DDR_SIZE (16*1024*1024)
#define PRU0_DDR ( DDR_BASEADDR + OFFSET_DDR )
#define PRU1_DDR ( PRU0_DDR +PRU_DDR_SIZE)

volatile static void *pruDataMem;
volatile static signed int *pruDataMem_int;
volatile uint16_t *pru1_ddr;

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
    //
    int mem_fd = open("/dev/mem", O_RDWR);
    if (mem_fd < 0){
	printf("couldn't open /dev/mem\n");
	exit(-1);
    }

    pru1_ddr= (uint16_t*)mmap(0, PRU_DDR_SIZE, PROT_WRITE | PROT_READ, MAP_SHARED, mem_fd, PRU1_DDR);
    if (pru1_ddr == NULL){
        printf("couldn't map pru1 ddr\n");
        exit(-1);
    }
    
    

    pruDataMem_int[0] = 1;
    pruDataMem_int[2] = PRU1_DDR;
    pruDataMem_int[3] = PRU1_DDR + 640*480;
    printf("WE DIDN'T ALL DIE!\n");
    fflush(stdout);
    
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
	uninitialize_pru();
	FILE * image_data = fopen("image.bin", "w");
	if (image_data == NULL){
		fprintf(stderr, "Failed to open image output file");
		exit(-1);
	}
	fwrite((uint16_t *)pru1_ddr, sizeof(uint16_t), 640*480, image_data);
	printf("%d\n", ((volatile uint8_t *)pru1_ddr)[0]);
	fclose(image_data);

	return(0);
}


