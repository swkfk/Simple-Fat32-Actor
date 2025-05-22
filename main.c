#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "dispatch.h"
#include "info.h"
#include "interact/input.h"
#include "utils/utils.h"

#define MAX_LINE_LEN 1024
#define MAX_ARGS_COUNT 16
#define MAX_PRE_EXEC_COUNT 32

static void parse_args(int argc, char **argv, char **pre_exec, int *pre_exec_count) {
	const char *optstr = "f:e:vq";
	char ch;
	*pre_exec_count = 0;
	while ((ch = getopt(argc, argv, optstr)) != EOF) {
		switch (ch) {
		case 'v':
			set_verbose();
			Ltrace("Enable trace logs ...");
			break;
		case 'q':
			set_quiet();
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
		case 'e':
			if (*pre_exec_count >= MAX_PRE_EXEC_COUNT) {
				Lwarn("Too many pre jobs, '%s' will be ignored", optarg);
			} else {
				char *string = checked_malloc(1, strlen(optarg));
				strcpy(string, optarg);
				pre_exec[(*pre_exec_count)++] = string;
			}
			break;
		}
	}
}

static void do_job_from_line(char *input) {
	int argc, ret;
	char *args[MAX_ARGS_COUNT];

	argc = parse_command_line(input, args, MAX_ARGS_COUNT);
	if (argc == 0) {
		return;
	}
	worker_fn_t fn = find_job_worker(args[0]);
	if (fn == NULL) {
		return;
	}

	ret = fn(argc, args);

	if (ret != 0) {
		Lwarn("Jobs exited with %d (%s)", ret, get_error_reason(ret));
	}
}

int main(int argc, char **argv) {
	char inputs[MAX_LINE_LEN], *input_p;

	int pre_exec_count = 0;
	char *pre_exec[MAX_PRE_EXEC_COUNT];

	parse_args(argc, argv, pre_exec, &pre_exec_count);

	for (int i = 0; i < pre_exec_count; i++) {
		do_job_from_line(pre_exec[i]);
		free(pre_exec[i]);
	}

	if (pre_exec_count != 0) {
		exit_program(0, 0);
	}

	while (1) {
		input_p = line_interact(inputs, MAX_LINE_LEN);
		do_job_from_line(input_p);
	}

	exit_program(0, 0);
}
