#include <stdlib.h>

#include "../utils/utils.h"
#include "job.h"

DEFINE_JOB(exit) {
	int return_code = 0;
	if (argc > 1) {
		return_code = atoi(argv[1]);
	}
	exit_program(return_code, 1);
}
