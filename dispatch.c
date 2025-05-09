#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "dispatch.h"

job_t jobs[] = {
    REGISTER_JOB(dummy),
    REGISTER_JOB(exit),
    REGISTER_JOB(quit),
};

worker_fn_t find_job_worker(const char *words) {
	for (int i = 0; i < sizeof(jobs) / sizeof(job_t); i++) {
		if (strcmp(words, jobs[i].job_name) == 0) {
			return jobs[i].fn;
		}
	}
	Lerror("Cannot find a job with name: `%s`", words);
	return NULL;
}
