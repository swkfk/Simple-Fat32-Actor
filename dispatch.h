#ifndef _DISPATCH_H_
#define _DISPATCH_H_

#include "job/job.h"

typedef int (*worker_fn_t)(int, char **);

typedef struct {
	const char *job_name;
	worker_fn_t fn;
	const char *help_string;
} job_t;

#define REGISTER_JOB(_name, _help)                                                                 \
	(job_t) {                                                                                  \
		.job_name = #_name, .fn = JOB_FN_NAME(_name), .help_string = _help                 \
	}
#define REGISTER_JOB_ALIAS(_alias, _name, _help)                                                   \
	(job_t) {                                                                                  \
		.job_name = #_alias, .fn = JOB_FN_NAME(_name), .help_string = _help                \
	}

worker_fn_t find_job_worker(const char *);

#endif
