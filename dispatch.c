#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "dispatch.h"

job_t jobs[] = {
    REGISTER_JOB(help, "Show this help message"),
    REGISTER_JOB(dummy, "Just a test job, print the <Arg...> arguments out"),
    REGISTER_JOB(load, "Load the fat32 image from <Arg1> file"),
    REGISTER_JOB(unload, "Unload the loaded image file"),
    REGISTER_JOB(info, "Show the information of the loaded file"),
    REGISTER_JOB(exit, "Exit the program"),
    REGISTER_JOB(quit, "Exit the program"),
    REGISTER_JOB_ALIAS(.fat.read, read_data,
		       "Read data from the <Arg1>-th cluster. The bytes read will be limited to "
		       "<Arg2> if it is set"),
    REGISTER_JOB_ALIAS(.fat.ls, read_directory, "Read the directory from the <Arg1>-th cluster"),
};

size_t job_count = sizeof(jobs) / sizeof(job_t);

worker_fn_t find_job_worker(const char *words) {
	for (int i = 0; i < job_count; i++) {
		if (strcmp(words, jobs[i].job_name) == 0) {
			return jobs[i].fn;
		}
	}
	Lerror("Cannot find a job with name: `%s`", words);
	return NULL;
}
