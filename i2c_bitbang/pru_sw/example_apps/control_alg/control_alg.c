#include <stdio.h>
#include <stdlib.h>
#include <prussdrv.h>
#include <pruss_intc_mapping.h>
#include <math.h>

#define PRU_NUM 	0
#define PWM_N_ADDRESS 8
#define PWM_E_ADDRESS 9
#define PWM_S_ADDRESS 10
#define PWM_W_ADDRESS 11
#define ALPHA		.999
#define BETA		1-ALPHA
#define G		8192
#define AM33XX
#define CALIBRATION_SAMPLES 200
volatile static void *pruDataMem;
volatile static signed int *pruDataMem_int;


typedef struct imu_data_t{
	double x_a;
	double y_a;
	double z_a;
	double x_g;
	double y_g;
	double z_g;
	int sample_num;
	
}imu_data_t;

typedef struct comp_filter_t {
	double alpha;
	double th;
	double beta;
	double g;
} comp_filter_t;


typedef struct pwm_frame_t{
	volatile int* N;
	volatile int* E;
	volatile int* S;
	volatile int* W;
} pwm_frame_t;


void calibrate_imu_frame(imu_data_t * imu_frame, imu_data_t * calib_data);
void filter_loop(imu_data_t * imu_frame, comp_filter_t * theta_p, comp_filter_t * theta_r, comp_filter_t * theta_y);
void init_filter(comp_filter_t * comp_filter, double alpha, double beta, double g);
void calculate_next_comp_filter(comp_filter_t * prev_data, double acc, double gyro, double dt);
void get_imu_frame(imu_data_t * imu_frame);
pwm_frame_t * get_pwm_pointer();
void initialize_pru();
void start_pru();
void uninitialize_pru();
imu_data_t * get_calibration_data();


void init_filter(comp_filter_t * comp_filter, double alpha, double beta, double g){
	comp_filter->alpha = alpha;
	comp_filter->beta = beta;
	comp_filter->g = g;
	comp_filter->th = 0;
}


void calculate_next_comp_filter(comp_filter_t * prev_data, double acc, double gyro, double dt){
	gyro = (gyro/32768.0f)*1000.0f;
	printf("%f\n", gyro);
	prev_data->th = prev_data->alpha*(prev_data->th + gyro*dt) + prev_data->beta*asin(acc/prev_data->g)*57.2957795f;
}

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

imu_data_t * get_calibration_data(){
	imu_data_t * calib_data = malloc(sizeof(imu_data_t));
	imu_data_t imu_data;
	calib_data->x_a = 0;
	calib_data->y_a = 0;
	calib_data->z_a = 0;
	calib_data->x_g = 0;
	calib_data->y_g = 0;
	calib_data->z_g = 0;
	
	int i;
	printf("clearing DLPF on imu..\n");
	for (i = 0; i < 200; i++){
		get_imu_frame(&imu_data);
	}

	for (i = 0; i < CALIBRATION_SAMPLES; i++){
		get_imu_frame(&imu_data);
		
//		printf("%f, %f, %f, %f, %f, %f\n", calib_data->x_a, calib_data->y_a, calib_data->z_a, calib_data->x_g, calib_data->y_g, calib_data->z_g);
		calib_data->x_a += imu_data.x_a;
		calib_data->y_a += imu_data.y_a;
		calib_data->z_a += imu_data.z_a;

		calib_data->x_g += imu_data.x_g;
		calib_data->y_g += imu_data.y_g;
		calib_data->z_g += imu_data.z_g;
	}
	calib_data->x_a /= CALIBRATION_SAMPLES;
	calib_data->y_a /= CALIBRATION_SAMPLES;
	calib_data->z_a /= CALIBRATION_SAMPLES;
	calib_data->x_g /= CALIBRATION_SAMPLES;
	calib_data->y_g /= CALIBRATION_SAMPLES;
	calib_data->z_g /= CALIBRATION_SAMPLES;
	return calib_data;
}

int main (void)
{
	initialize_pru();
	start_pru();
	pwm_frame_t * pwm_frame = get_pwm_pointer();
	*(pwm_frame->N) = 100000;
	*(pwm_frame->E) = 100000;
	*(pwm_frame->S) = 100000;
	*(pwm_frame->W) = 100000;

	imu_data_t * calib_data;
	calib_data = get_calibration_data();
	printf("check calibration data for sanity: %f, %f, %f, %f, %f, %f\n", calib_data->x_a, calib_data->y_a, calib_data->z_a, calib_data->x_g, calib_data->y_g, calib_data->z_g);
	
	comp_filter_t * theta_p = malloc(sizeof(comp_filter_t));
	comp_filter_t * theta_r = malloc(sizeof(comp_filter_t));
	comp_filter_t * theta_y = malloc(sizeof(comp_filter_t));
	imu_data_t * imu_frame = malloc(sizeof(comp_filter_t));
	
	init_filter(theta_p, ALPHA, BETA, G);
	init_filter(theta_r, ALPHA, BETA, G);

	
	while(1){
		
		get_imu_frame(imu_frame);
		calibrate_imu_frame(imu_frame, calib_data);
		filter_loop(imu_frame, theta_p, theta_r, theta_y);
		printf("pitch: %f, roll: %f\n", theta_p->th, theta_r->th);
	}

	uninitialize_pru();
	return(0);
}


void calibrate_imu_frame(imu_data_t * imu_frame, imu_data_t * calib_data){
	imu_frame->x_a -= calib_data->x_a;
	imu_frame->y_a -= calib_data->y_a;
	imu_frame->z_a -= calib_data->z_a;

	imu_frame->x_g -= calib_data->x_g;
	imu_frame->y_g -= calib_data->y_g;
	imu_frame->z_g -= calib_data->z_g;
}
void filter_loop(imu_data_t * imu_frame, comp_filter_t * theta_p, comp_filter_t * theta_r, comp_filter_t * theta_y){
	calculate_next_comp_filter(theta_p, imu_frame->y_a, -imu_frame->x_g, 1/250.0f);
	calculate_next_comp_filter(theta_r, imu_frame->x_a, -imu_frame->y_g, 1/250.0f);
}

