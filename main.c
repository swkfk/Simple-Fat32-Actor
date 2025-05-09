#include <stdio.h>

#include "dispatch.h"
#include "info.h"
#include "interact/input.h"
#include "utils/utils.h"

#define MAX_LINE_LEN 1024
#define MAX_ARGS_COUNT 16

int main() {
	printf(">>> Simple Fat32 Actor v" VERSION " <<<\n");

	char inputs[MAX_LINE_LEN], *input_p;
	int args_count, ret;
	char *args[MAX_ARGS_COUNT];

	while (1) {
		input_p = line_interact(inputs, MAX_LINE_LEN);
		args_count = parse_command_line(input_p, args, MAX_ARGS_COUNT);
		if (args_count == 0) {
			continue;
		}
		worker_fn_t fn = find_job_worker(args[0]);
		if (fn == NULL) {
			continue;
		}

		ret = fn(args_count, args);

		if (ret != 0) {
			Lwarn("Jobs exited with %d (%s)", ret, get_error_reason(ret));
		}
	}
	return 0;
}
