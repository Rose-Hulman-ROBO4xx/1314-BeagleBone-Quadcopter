#define PRU_NUM 	0
#define PWM_0_ADDRESS 9
#define PWM_1_ADDRESS 10
#define PWM_2_ADDRESS 11
#define PWM_3_ADDRESS 8
#define ALPHA		.95
#define BETA		(1-ALPHA)
#define G		2048
#define AM33XX
#define CALIBRATION_SAMPLES 256
#define PI 3.141592653589793238462643383279502884197169399375105
#define RAD_TO_DEG	57.2957795f
#define DT		.005f
#define PWM_OFF		110000
#define PWM_MIN		120000//110000
#define PWM_MAX		170000//160000
#define MIN(a,b)	(a<b ? a : b)
#define MAX(a,b)	(a>b ? a : b)
#define BIAS_INCREASE_RATE 1
#define GYRO_SENSITIVITY 2000 //gyro sensitivity in degrees/second
#define GYRO_MAX_RAW	32768 //maximum raw output of gyro

#define P_DEF		18 // 10
#define I_DEF		0  // 5	
#define D_DEF		0  // 7

#define BIAS0 0.0f
#define BIAS1 0.0f
#define BIAS2 0.0f
#define BIAS3 0.0f

#define MULT0 1.00f
#define MULT1 1.00f
#define MULT2 1.00f
#define MULT3 1.00f

#define BIAS_MAX 25000 //50000

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
void calculate_next_pwm(pwm_frame_t * next_pwm, comp_filter_t * theta_p, comp_filter_t * theta_r, comp_filter_t * theta_y, double * z_pos, double * z_vel, PID_t * PID_pitch, PID_t * PID_roll, PID_t * PID_yaw, PID_t * PID_z, set_point_t * goal, int bias, set_point_t * cf);
void init_PID(PID_t * PID_x, double kP, double kI, double kD);
void get_set_point(set_point_t * goal);
double PID_loop(double goal, PID_t * PID_x, double value);
void signal_handler(int sig);
