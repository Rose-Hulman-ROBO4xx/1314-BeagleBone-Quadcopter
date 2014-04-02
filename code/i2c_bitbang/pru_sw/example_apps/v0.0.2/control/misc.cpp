#include "misc.h"
#include <pthread.h>
#include <math.h>
pthread_mutex_t mutexTime = PTHREAD_MUTEX_INITIALIZER;

 timespec diff(timespec start, timespec end)
{
	timespec temp;
	if ((end.tv_nsec-start.tv_nsec)<0) {
		temp.tv_sec = end.tv_sec-start.tv_sec-1;
		temp.tv_nsec = 1000000000+end.tv_nsec-start.tv_nsec;
	} else {
		temp.tv_sec = end.tv_sec-start.tv_sec;
		temp.tv_nsec = end.tv_nsec-start.tv_nsec;
	}
	return temp;
}
 
long diffMille(timespec start, timespec end)	// return elapse in milli second
{
	timespec temp;
	if ((end.tv_nsec-start.tv_nsec)<0) {
		temp.tv_sec = end.tv_sec-start.tv_sec-1;
		temp.tv_nsec = 1000000000+end.tv_nsec-start.tv_nsec;
	} else {
		temp.tv_sec = end.tv_sec-start.tv_sec;
		temp.tv_nsec = end.tv_nsec-start.tv_nsec;
	}
	return temp.tv_nsec / 50000 + temp.tv_sec * 20000;
}

static timespec startTime;
static unsigned long mic = 0;
unsigned long  micros()
{
	pthread_mutex_lock( &mutexTime );
	// mic = mic + 1000;
	// return mic;
	timespec now;
	clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &now);
	pthread_mutex_unlock( &mutexTime );
	return diffMille(startTime, now);
}


// init the stop timer
void initStopTimer()
{
	clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &startTime);

}

long constrain(long a, long l, long u)
{
	if(a <= l)
	{
		return l;
	}
	if( a >= u)
	{
		return u;
	}
	return a;
}

unsigned long abs(long v)
{
	return fabs(v);
}