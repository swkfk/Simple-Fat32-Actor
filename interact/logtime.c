#include <sys/time.h>
#include <unistd.h>

static time_t start_time_us;

static time_t get_time_us() {
	struct timeval tv;
	gettimeofday(&tv, NULL);

	return tv.tv_sec * 1000000 + tv.tv_usec;
}

time_t init_start_time() {
	return start_time_us = get_time_us();
}

time_t get_time_delta_us() {
	return get_time_us() - start_time_us;
}
