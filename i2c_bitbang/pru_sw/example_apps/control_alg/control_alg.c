#include <stdio.h>
#include <stdlib.h>
#include <prussdrv.h>
#include <pruss_intc_mapping.h>

#define PRU_NUM 	0
#define PWM_N_ADDRESS 8
#define PWM_E_ADDRESS 9
#define PWM_S_ADDRESS 10
#define PWM_W_ADDRESS 11

#define AM33XX

volatile static void *pruDataMem;
volatile static signed int *pruDataMem_int;


typedef struct imu_data_t{
	signed short x_a;
	signed short y_a;
	signed short z_a;
	signed short x_g;
	signed short y_g;
	signed short z_g;
	int sample_num;
	
}imu_data_t;

typedef struct pwm_frame_t{
	volatile int* N;
	volatile int* E;
	volatile int* S;
	volatile int* W;
} pwm_frame_t;

void get_imu_frame(imu_data_t * imu_frame){
	while(!pruDataMem_int[1]){ // wait for pru to signal that new data is available
	}
	pruDataMem_int[1] = 0;
	imu_frame->x_a = (signed short)pruDataMem_int[2];
        imu_frame->y_a = (signed short)pruDataMem_int[3];
        imu_frame->z_a = (signed short)pruDataMem_int[4];
        imu_frame->x_g = (signed short)pruDataMem_int[5];
        imu_frame->y_g = (signed short)pruDataMem_int[6];
        imu_frame->z_g = (signed short)pruDataMem_int[7];
	imu_frame->sample_num = pruDataMem_int[12];
}

pwm_frame_t * get_pwm_pointer(){
	pwm_frame_t * pwm_frame = malloc(sizeof(pwm_frame_t));
	pwm_frame->N = &(pruDataMem_int[PWM_N_ADDRESS]);
	pwm_frame->E = &(pruDataMem_int[PWM_E_ADDRESS]);
	pwm_frame->S = &(pruDataMem_int[PWM_S_ADDRESS]);
	pwm_frame->W = &(pruDataMem_int[PWM_W_ADDRESS]);
	return pwm_frame;
}

void initialize_pru(){
    unsigned int ret;
    tpruss_intc_initdata pruss_intc_initdata = PRUSS_INTC_INITDATA;

    prussdrv_init ();
    /* Open PRU Interrupt */
    ret = prussdrv_open(PRU_EVTOUT_0);
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
    prussdrv_pru_wait_event (PRU_EVTOUT_0);

    printf("\tINFO: PRU completed transfer.\r\n");
    prussdrv_pru_clear_event (PRU0_ARM_INTERRUPT);

    /* Disable PRU and close memory mapping*/
    prussdrv_pru_disable (PRU_NUM);
    prussdrv_exit ();


}

int main (void)
{
    
    initialize_pru();
    int count = 0; 
    pwm_frame_t * pwm_frame = get_pwm_pointer();

    *(pwm_frame->N) = 100000;
    *(pwm_frame->E) = 100000;
    *(pwm_frame->S) = 100000;
    *(pwm_frame->W) = 100000;
    imu_data_t imu_data;
    start_pru();
    int i;
    printf("clearing DLPF on imu..\n");
    for (i = 0; i < 200; i++){
	    get_imu_frame(&imu_data);
    }
    double x_g_avg = 0;
    for (i = 0; i < 2000; i++){
        get_imu_frame(&imu_data);
	x_g_avg += imu_data.x_g;
        printf("%d: %d\n", imu_data.sample_num, imu_data.x_g);
        count += 1;
    }
    x_g_avg/=2000;
    double current_rot = 0;

    for (i = 0; i < 5000000; i++){
        get_imu_frame(&imu_data);
	current_rot += imu_data.x_g-x_g_avg;
        *(pwm_frame->N) = 130000-(current_rot/30);
	printf("current_rot: %d\n", *(pwm_frame->N));
    }
    printf("%d\n", count);
    uninitialize_pru();
    return(0);
}

