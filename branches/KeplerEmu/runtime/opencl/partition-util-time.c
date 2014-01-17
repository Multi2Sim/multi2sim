#include <time.h>
#include "partition-util-time.h"

long long get_time()
{
	struct timespec t;
	clock_gettime(CLOCK_MONOTONIC, &t);

	return 1000000000LL * t.tv_sec + t.tv_nsec;	
}

