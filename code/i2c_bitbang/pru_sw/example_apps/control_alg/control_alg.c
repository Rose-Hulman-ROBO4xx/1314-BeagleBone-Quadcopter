#include <stdio.h>
#include <stdlib.h>
#include <prussdrv.h>
#include <pruss_intc_mapping.h>
#include <math.h>
#include <signal.h>
#include "control_alg.h"
volatile static void *pruDataMem;
volatile static signed int *pruDataMem_int;

void signal_handler(int sig){
	pruDataMem_int[0] = 0;
}

void get_set_point(set_point_t * goal){
	goal->pitch = 30;
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
    pruDataMem_int[0] = 1;
    prussdrv_exec_program (PRU_NUM, "./control_alg.bin");
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
	for (i = 0; (i < 200); i++){
		get_imu_frame(&imu_data);
	}

	for (i = 0; (i < CALIBRATION_SAMPLES); i++){
		get_imu_frame(&imu_data);
		
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
	pwm_frame->zero = PWM_OFF;
	pwm_frame->one = PWM_OFF;
	pwm_frame->two = PWM_OFF;
	pwm_frame->three = PWM_OFF;
}

void output_pwm(pwm_frame_t * pwm_frame_next, pru_pwm_frame_t * pwm_out){
	*(pwm_out->zero) = pwm_frame_next->zero;
	*(pwm_out->one) = pwm_frame_next->one;
	*(pwm_out->two) = pwm_frame_next->two;
	*(pwm_out->three) = pwm_frame_next->three;
}

int main (void)
{

	comp_filter_t * theta_p = malloc(sizeof(comp_filter_t));
	comp_filter_t * theta_r = malloc(sizeof(comp_filter_t));
	comp_filter_t * theta_y = malloc(sizeof(comp_filter_t));
	pwm_frame_t * next_pwm = malloc(sizeof(comp_filter_t));
	double * z_pos = malloc(sizeof(double)); 
	double * z_vel = malloc(sizeof(double));
	PID_t * PID_pitch = malloc(sizeof(PID_t));
	PID_t * PID_roll = malloc(sizeof(PID_t));
	PID_t * PID_yaw = malloc(sizeof(PID_t));
	PID_t * PID_z = malloc(sizeof(PID_t));
	set_point_t * goal = malloc(sizeof(set_point_t));
	set_point_t * cf = malloc(sizeof(set_point_t));

	FILE * response_log = fopen("system_response.csv", "w");
	if (response_log == NULL){
		fprintf(stderr, "Couldn't open response.csv");
		exit(-1);
	}
	fprintf(response_log, "bias,pitch,cf_pitch,roll,cf_roll,yaw,cf_yaw,z,m0,m1,m2,m3\n");

	initialize_pru();
	pru_pwm_frame_t * pwm_out = get_pwm_pointer();
	init_pwm(next_pwm);
	output_pwm(next_pwm, pwm_out);
	start_pru();

	imu_data_t * calib_data;
	FILE * calib_file = fopen("./cal.txt", "r");
	if (calib_file == NULL){
		printf("generating new calibration data\n");
		calib_data = get_calibration_data();
		calib_file = fopen("./cal.txt", "w");
		if (calib_file == NULL){
			fprintf(stderr, "couldn't open cal.txt\n");
		} else{
			fprintf(calib_file, "%f,%f,%f,%f,%f,%f\n", calib_data->x_a, calib_data->y_a, calib_data->z_a, calib_data->x_g, calib_data->y_g, calib_data->z_g);
			fclose(calib_file);
		}

		uninitialize_pru();
		exit(-1);
	} else{
		calib_data = malloc(sizeof(imu_data_t));
		fscanf(calib_file, "%lf,%lf,%lf,%lf,%lf,%lf\n", &(calib_data->x_a), &(calib_data->y_a), &(calib_data->z_a), &(calib_data->x_g), &(calib_data->y_g), &(calib_data->z_g));
//		printf("cal data: %f, %f, %f, %f, %f, %f\n", calib_data->x_a, calib_data->y_a, calib_data->z_a, calib_data->x_g, calib_data->y_g, calib_data->z_g);
		fclose(calib_file);
	}
	// P,I,D values should probably be different between the different loops (Mike)
	init_PID(PID_pitch, P_DEF+1, I_DEF, D_DEF);
	init_PID(PID_roll, P_DEF, I_DEF, D_DEF);
	init_PID(PID_yaw, P_DEF, I_DEF, D_DEF);
	init_PID(PID_z, P_DEF, I_DEF, D_DEF);
	get_set_point(goal);

        signal(SIGINT, signal_handler);
	printf("check calibration data for sanity: %f, %f, %f, %f, %f, %f\n", calib_data->x_a, calib_data->y_a, calib_data->z_a, calib_data->x_g, calib_data->y_g, calib_data->z_g);
	
	*z_pos = 0;
	*z_vel = 0;

	imu_data_t * imu_frame = malloc(sizeof(comp_filter_t));
	
	init_filter(theta_p, ALPHA, BETA, G);
	init_filter(theta_r, ALPHA, BETA, G);
	init_filter(theta_y, 1, 0, G);

	
	int bias = 0;
	int count = 0;
	while(pruDataMem_int[0] != 0){
		if (bias < BIAS_MAX){
			bias += 5;
		}
		
		get_imu_frame(imu_frame);
		calibrate_imu_frame(imu_frame, calib_data);
		filter_loop(imu_frame, theta_p, theta_r, theta_y, z_pos, z_vel);
		calculate_next_pwm(next_pwm, theta_p, theta_r, theta_y, z_pos, z_vel, PID_pitch, PID_roll, PID_yaw, PID_z, goal, bias, cf);
		output_pwm(next_pwm, pwm_out);

		if (count == 20){
			printf("bias: % 03d, pitch: % 03.5f, cf_pitch: % 03.5f, roll: % 03.5f, cf_roll: % 03.5f, yaw: % 03.5f, cf->yaw: % 03.5f, z: % 03.5f m0: %d, m1: %d, m2: %d, m3: %d\n", bias, theta_p->th, cf->pitch, theta_r->th,cf->roll,theta_y->th, cf->yaw, *z_vel, next_pwm->zero, next_pwm->one, next_pwm->two, next_pwm->three);
			count = 0;
		}
		count++;

		fprintf(response_log, "%d,%3.5f,%3.5f,%3.5f,%3.5f,%3.5f,%3.5f,%3.5f,\t%d,%d,%d,%d\n", bias, theta_p->th,cf->pitch, theta_r->th,cf->roll, theta_y->th, cf->yaw, *z_vel, next_pwm->zero, next_pwm->one, next_pwm->two, next_pwm->three);
//		printf("x_g: % 05.5f, x_a: % 05.5f, y_g: % 05.5f, y_a: % 05.5f, z_g: % 05.5f, z_a: % 05.5f\n", imu_frame->x_g, imu_frame->x_a, imu_frame->y_g, imu_frame->y_a, imu_frame->z_g, imu_frame->z_a);
	}
	printf("exiting...\n");

	uninitialize_pru();
	fclose(response_log);
	free(theta_p);
	free(theta_r);
	free(theta_y);
	free(z_pos);
	free(z_vel);
	free(calib_data);
	free(PID_pitch);
	free(PID_roll);
	free(PID_yaw);
	free(PID_z);
	free(goal);
	return(0);
}


void calibrate_imu_frame(imu_data_t * imu_frame, imu_data_t * calib_data){
	imu_frame->x_a -= calib_data->x_a;
	imu_frame->y_a -= calib_data->y_a;
	imu_frame->z_a -= calib_data->z_a + 1;

	imu_frame->x_g -= calib_data->x_g;
	imu_frame->y_g -= calib_data->y_g;
	imu_frame->z_g -= calib_data->z_g;
}

void filter_loop(imu_data_t * imu_frame, comp_filter_t * theta_p, comp_filter_t * theta_r, comp_filter_t * theta_y, double * z_pos, double * z_vel){
	double temp_z_acc = imu_frame->z_a*fabs(cos(theta_p->th/RAD_TO_DEG)*cos(theta_r->th/RAD_TO_DEG));
	temp_z_acc = ((temp_z_acc/32768.0f)*8.0f);
	*z_vel += temp_z_acc;
	
	//forward is +y is towards the ethernet port
	//right is +x is header P9
	//up is +z is the vector pointing from the cape to the beaglebone
	calculate_next_comp_filter(theta_p, imu_frame->y_a, -imu_frame->x_g, DT);
	calculate_next_comp_filter(theta_r, imu_frame->x_a, imu_frame->y_g, DT);
	calculate_next_comp_filter(theta_y, imu_frame->z_a, imu_frame->z_g, DT);
}

void calculate_next_pwm(pwm_frame_t * next_pwm, comp_filter_t * theta_p, comp_filter_t * theta_r, comp_filter_t * theta_y, double * z_pos, double * z_vel, PID_t * PID_pitch, PID_t * PID_roll, PID_t * PID_yaw, PID_t * PID_z, set_point_t * goal, int bias, set_point_t * cf){
	double d_pitch = PID_loop(goal->pitch, PID_pitch, theta_p->th);
	double d_roll = PID_loop(goal->roll, PID_roll, theta_r->th);
	double d_yaw = PID_loop(goal->yaw, PID_yaw, theta_y->th);
	double d_z = PID_loop(goal->z, PID_z, *z_vel);
	cf->roll = d_roll;
	cf->pitch = d_pitch;
	cf->yaw = d_yaw;
	
	d_z = 0;//FIXME
	d_yaw = 0;
	// d_roll = 0;
	d_pitch = 0;

	next_pwm->zero = d_pitch + d_roll + d_yaw - d_z + PWM_MIN;
	next_pwm->one = -d_pitch + d_roll - d_yaw - d_z + PWM_MIN;
	next_pwm->two = -d_pitch - d_roll + d_yaw - d_z + PWM_MIN;
	next_pwm->three = d_pitch - d_roll - d_yaw - d_z + PWM_MIN;
	
	next_pwm->zero = next_pwm->zero * MULT0 + bias + BIAS0;
	next_pwm->one = next_pwm->one * MULT1 + bias + BIAS1;
	next_pwm->two = next_pwm->two * MULT2 + bias + BIAS2;
	next_pwm->three = next_pwm->three * MULT3 + bias + BIAS3;

	next_pwm->zero = MIN(PWM_MAX, MAX(PWM_MIN,next_pwm->zero));
	next_pwm->one = MIN(PWM_MAX, MAX(PWM_MIN,next_pwm->one));
	next_pwm->two = MIN(PWM_MAX, MAX(PWM_MIN,next_pwm->two));
	next_pwm->three = MIN(PWM_MAX, MAX(PWM_MIN,next_pwm->three));
}

double PID_loop(double goal, PID_t * PID_x, double value){
	double P, I, D, delta_error;
	delta_error = goal - value;

	P = PID_x->kP * delta_error;
	I = (PID_x->kI * (PID_x->I + delta_error)) * DT;
	PID_x->I = I;
	D = PID_x->kD*((delta_error - PID_x->D) / DT);
	PID_x->D = delta_error;

	return P + I + D;
}





