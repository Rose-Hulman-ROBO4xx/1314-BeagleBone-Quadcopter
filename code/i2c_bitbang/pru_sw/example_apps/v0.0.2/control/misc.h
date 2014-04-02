#ifndef __MISC_H__
#define __MISC_H__

#include <time.h>

timespec diff(timespec start, timespec end);
 
long diffMille(timespec start, timespec end);	// return elapse in milli second

unsigned long  micros();

long constrain(long a, long l, long u);

// init the stop timer
void initStopTimer();

unsigned long abs(long v);
#endif