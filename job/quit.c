#include <stdlib.h>

#include "../utils/utils.h"
#include "job.h"

__attribute__((noreturn)) void _common_exit(int argc, char **argv) {
	int return_code = 0;
	if (argc > 1) {
		return_code = atoi(argv[1]);
	}
	exit_program(return_code, 1);
}

DEFINE_JOB(exit) {
	_common_exit(argc, argv);
}

DEFINE_JOB(quit) {
	_common_exit(argc, argv);
}
