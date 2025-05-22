#include <stdlib.h>

#include "../fat32/img.h"
#include "../interact/logger.h"

static void pre_exit_operations() {
	if (img.fp != NULL) {
		close_fat32_img(&img);
		Linfo("Unload the image file");
	}
}

__attribute__((noreturn)) void exit_program(int exit_code, int pass_through) {
	pre_exit_operations();

	if (exit_code == 0) {
		Linfo("Bye!");
	} else {
		Lerror("Exit with non-zero code %d", exit_code);
	}

	if (pass_through) {
		exit(exit_code);
	} else {
		exit(0);
	}
}
