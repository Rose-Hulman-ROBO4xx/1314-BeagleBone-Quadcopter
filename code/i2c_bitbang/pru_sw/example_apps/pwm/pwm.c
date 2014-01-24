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

typedef struct PID_t{
	double kP;
	double kI;
	double kD;
	double I;
	double D;
} PID_t;

typedef struct set_point_t{
	double pitch;
	double roll;
	double yaw;
	double z;
} set_point_t;

void calibrate_imu_frame(imu_data_t * imu_frame, imu_data_t * calib_data);
void filter_loop(imu_data_t * imu_frame, comp_filter_t * theta_p, comp_filter_t * theta_r, comp_filter_t * theta_y, double * z_pos, double * z_vel);
void init_filter(comp_filter_t * comp_filter, double alpha, double beta, double g);
void calculate_next_comp_filter(comp_filter_t * prev_data, double acc, double gyro, double dt);
void get_imu_frame(imu_data_t * imu_frame);
pru_pwm_frame_t * get_pwm_pointer();
void initialize_pru();
void start_pru();
void uninitialize_pru();
imu_data_t * get_calibration_data();
void calculate_next_pwm(pwm_frame_t * next_pwm, comp_filter_t * theta_p, comp_filter_t * theta_r, comp_filter_t * theta_y, double * z_pos, double * z_vel, PID_t * PID_pitch, PID_t * PID_roll, PID_t * PID_yaw, PID_t * PID_z, set_point_t * goal);
void init_PID(PID_t * PID_x, double kP, double kI, double kD);
void get_set_point(set_point_t * goal);
double PID_loop(double goal, PID_t * PID_x, double value);
void signal_handler(int sig);

void signal_handler(int sig){
	pruDataMem_int[0] = 0;
}

void get_set_point(set_point_t * goal){
	goal->pitch = 0;
	goal->roll = 0;
	goal->yaw = 0;
	goal->z = 0;
}

void init_PID(PID_t * PID_x, double kP, double kI, double kD){
	PID_x->kP = kP;
	PID_x->kI = kI;
	PID_x->I = 0;
	PID_x->kD = kD;
	PID_x->D = 0; 
}

void init_filter(comp_filter_t * comp_filter, double alpha, double beta, double g){
	comp_filter->alpha = alpha;
	comp_filter->beta = beta;
	comp_filter->g = g;
	comp_filter->th = 0;
}


void calculate_next_comp_filter(comp_filter_t * prev_data, double acc, double gyro, double dt){
	
	gyro = (gyro/GYRO_MAX_RAW)*GYRO_SENSITIVITY;
	double accel_angle = acc/prev_data->g;
	accel_angle = MAX(MIN(accel_angle, 1.0), -1.0);
	accel_angle = asin(accel_angle);
	prev_data->th = prev_data->alpha*(prev_data->th + gyro*dt) + prev_data->beta*accel_angle*RAD_TO_DEG;
}

void get_imu_frame(imu_data_t * imu_frame){
	// Possibly change to an interrupt? (Mike)
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

	pwm_frame_t * next_pwm = malloc(sizeof(comp_filter_t));
	initialize_pru();
	start_pru();
	pru_pwm_frame_t * pwm_out = get_pwm_pointer();
	init_pwm(next_pwm);
	output_pwm(next_pwm, pwm_out);
        signal(SIGINT, signal_handler);

	while(pruDataMem_int[0] != 0){

		scanf("%d %d %d %d", &(next_pwm->zero), &(next_pwm->one), &(next_pwm->two), &(next_pwm->three));
		next_pwm->zero *= 1000;
		next_pwm->one *= 1000;
		next_pwm->two *= 1000;
		next_pwm->three *= 1000;
		output_pwm(next_pwm, pwm_out);


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

void filter_loop(imu_data_t * imu_frame, comp_filter_t * theta_p, comp_filter_t * theta_r, comp_filter_t * theta_y, double * z_pos, double * z_vel){
	double temp_z_acc = imu_frame->z_a*fabs(cos(theta_p->th/RAD_TO_DEG)*cos(theta_r->th/RAD_TO_DEG));
	temp_z_acc = ((temp_z_acc/32768.0f)*8.0f)*DT;
	*z_vel += temp_z_acc;
	
	//forward is +y is towards the ethernet port
	//right is +x is header P9
	//up is +z is the vector pointing from the cape to the beaglebone
	calculate_next_comp_filter(theta_p, -imu_frame->y_a, -imu_frame->x_g, DT);
	calculate_next_comp_filter(theta_r, -imu_frame->x_a, imu_frame->y_g, DT);
	calculate_next_comp_filter(theta_y, imu_frame->z_a, -imu_frame->z_g, DT);
}

void calculate_next_pwm(pwm_frame_t * next_pwm, comp_filter_t * theta_p, comp_filter_t * theta_r, comp_filter_t * theta_y, double * z_pos, double * z_vel, PID_t * PID_pitch, PID_t * PID_roll, PID_t * PID_yaw, PID_t * PID_z, set_point_t * goal){
	double d_pitch = PID_loop(goal->pitch, PID_pitch, theta_p->th);
	double d_roll = PID_loop(goal->roll, PID_roll, theta_r->th);
	double d_yaw = PID_loop(goal->yaw, PID_yaw, theta_y->th);
	double d_z = PID_loop(goal->z, PID_z, *z_vel);
	
	d_z = 0;//FIXME

	next_pwm->zero = d_pitch + d_roll + d_yaw - d_z + PWM_MIN;
	next_pwm->one = -d_pitch + d_roll - d_yaw - d_z + PWM_MIN;
	next_pwm->two = -d_pitch - d_roll + d_yaw - d_z + PWM_MIN;
	next_pwm->three = d_pitch - d_roll - d_yaw - d_z + PWM_MIN;

	next_pwm->zero = MIN(PWM_MAX, MAX(PWM_MIN,next_pwm->zero));
	next_pwm->one = MIN(PWM_MAX, MAX(PWM_MIN,next_pwm->one));
	next_pwm->two = MIN(PWM_MAX, MAX(PWM_MIN,next_pwm->two));
	next_pwm->three = MIN(PWM_MAX, MAX(PWM_MIN,next_pwm->three));

}

double PID_loop(double goal, PID_t * PID_x, double value){
	double P, I, D, delta_error;
	delta_error = goal - value;

	P = PID_x->kP * delta_error;
	I = PID_x->kI*(PID_x->I + delta_error*DT);
	PID_x->I = I;
	D = PID_x->kD*(delta_error - PID_x->D)*DT;
	PID_x->D = delta_error;

	return P + I + D;
}





