#include <stdio.h>
#include <stdlib.h>
#include <prussdrv.h>
#include <pruss_intc_mapping.h>
#include <math.h>
#include <signal.h>
#define PRU_NUM 	0
#define PWM_0_ADDRESS 9
#define PWM_1_ADDRESS 10
#define PWM_2_ADDRESS 11
#define PWM_3_ADDRESS 8
#define ALPHA		.999
#define BETA		(1-ALPHA)
#define G		2048
#define AM33XX
#define CALIBRATION_SAMPLES 256
#define PI 3.141592653589793238462643383279502884197169399375105
#define RAD_TO_DEG	57.2957795f
#define DT		0.005f
#define PWM_MIN		105000
#define PWM_MAX		170000
#define MIN(a,b)	(a<b ? a : b)
#define MAX(a,b)	(a>b ? a : b)

#define GYRO_SENSITIVITY 2000 //gyro sensitivity in degrees/second
#define GYRO_MAX_RAW	32768 //maximum raw output of gyro

#define P_DEF		1000
#define I_DEF		.1
#define D_DEF		.1
#define TIME_STEP	0.002f


volatile static void *pruDataMem;
volatile static signed int *pruDataMem_int;



typedef struct pru_pwm_frame_t{
	volatile int* zero;
	volatile int* one;
	volatile int* two;
	volatile int* three;
} pru_pwm_frame_t;

typedef struct pwm_frame_t{
	int zero;
	int one;
	int two;
	int three;
} pwm_frame_t;
pru_pwm_frame_t * get_pwm_pointer();
void initialize_pru();
void start_pru();
void uninitialize_pru();
void signal_handler(int sig);

void signal_handler(int sig){
	pruDataMem_int[0] = 0;
}

pru_pwm_frame_t * get_pwm_pointer(){
	pru_pwm_frame_t * pwm_frame = malloc(sizeof(pru_pwm_frame_t));
	pwm_frame->zero = &(pruDataMem_int[PWM_0_ADDRESS]);
	pwm_frame->one = &(pruDataMem_int[PWM_1_ADDRESS]);
	pwm_frame->two = &(pruDataMem_int[PWM_2_ADDRESS]);
	pwm_frame->three = &(pruDataMem_int[PWM_3_ADDRESS]);
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
    prussdrv_exec_program (PRU_NUM, "./pwm_control_alg.bin");
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
void init_pwm(pwm_frame_t * pwm_frame){
	pwm_frame->zero = PWM_MIN;
	pwm_frame->one = PWM_MIN;
	pwm_frame->two = PWM_MIN;
	pwm_frame->three = PWM_MIN;
}

void output_pwm(pwm_frame_t * pwm_frame_next, pru_pwm_frame_t * pwm_out){
	*(pwm_out->zero) = pwm_frame_next->zero;
	*(pwm_out->one) = pwm_frame_next->one;
	*(pwm_out->two) = pwm_frame_next->two;
	*(pwm_out->three) = pwm_frame_next->three;
}

int main (void)
{

	pwm_frame_t * next_pwm = malloc(sizeof(pwm_frame_t));
	initialize_pru();
	start_pru();
	pru_pwm_frame_t * pwm_out = get_pwm_pointer();
	init_pwm(next_pwm);
	output_pwm(next_pwm, pwm_out);
        signal(SIGINT, signal_handler);

	while(pruDataMem_int[0] != 0){

		//scanf("%d %d %d %d", &(next_pwm->zero), &(next_pwm->one), &(next_pwm->two), &(next_pwm->three));
		if (pruDataMem_int[2]){
			printf("ping: %d                \r", pruDataMem_int[1]/5800);
			pruDataMem_int[2] = 0;
		}
		next_pwm->zero *= 1000;
		next_pwm->one *= 1000;
		next_pwm->two *= 1000;
		next_pwm->three *= 1000;
		output_pwm(next_pwm, pwm_out);


	}

	uninitialize_pru();
	
	return(0);
}


