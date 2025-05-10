#include "../dispatch.h"
#include "job.h"

extern job_t jobs[];
extern size_t job_count;

DEFINE_JOB(help) {
	display("    Job     | Description\n");
	display("----------- | -----------\n");
	for (int i = 0; i < job_count; i++) {
		display("%11s | %s\n", jobs[i].job_name, jobs[i].help_string);
	}
	return 0;
}
