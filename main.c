#include <stdio.h>
#include <unistd.h>

#include "dispatch.h"
#include "info.h"
#include "interact/input.h"
#include "utils/utils.h"

#define MAX_LINE_LEN 1024
#define MAX_ARGS_COUNT 16

void parse_args(int argc, char **argv) {
	const char *optstr = "f:v";
	char ch;
	while ((ch = getopt(argc, argv, optstr)) != EOF) {
		switch (ch) {
		case 'v':
			set_verbose();
			Ltrace("Enable trace logs ...");
			break;
		case 'f': {
			char *new_argv[] = {"", optarg};
			Linfo("Load image from file %s", optarg);
			int r = CALL_JOB(load, 2, new_argv);
			if (r != 0) {
				Lwarn("Pre job exited with %d (%s)", r, get_error_reason(r));
			}
			break;
		}
		}
	}
}

int main(int argc, char **argv) {
	printf(">>> Simple Fat32 Actor v" VERSION " <<<\n");

	parse_args(argc, argv);

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
