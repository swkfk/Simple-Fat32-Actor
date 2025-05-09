#include <stdlib.h>

#include "../interact/logger.h"

__attribute__((noreturn)) void exit_program(int exit_code, int pass_through) {
	if (exit_code == 0) {
		display("Bye!\n");
	} else {
		Lerror("Exit with non-zero code %d", exit_code);
	}
	if (pass_through) {
		exit(exit_code);
	} else {
		exit(0);
	}
}
