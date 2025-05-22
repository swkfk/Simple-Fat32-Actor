#include <stdlib.h>

#include "../interact/logger.h"

static void pre_exit_operations() {
	//
}

__attribute__((noreturn)) void exit_program(int exit_code, int pass_through) {
	display("\n");
	if (exit_code == 0) {
		Linfo("Bye!\n");
	} else {
		Lerror("Exit with non-zero code %d", exit_code);
	}

	pre_exit_operations();

	if (pass_through) {
		exit(exit_code);
	} else {
		exit(0);
	}
}
