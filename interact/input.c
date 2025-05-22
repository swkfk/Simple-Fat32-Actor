#include <ctype.h>
#include <stdio.h>
#include <string.h>

#include "../utils/utils.h"
#include "logger.h"

int parse_command_line(char *input_str, char *argv[], size_t args_maxn) {
	const char *delimiters = " ";
	char *token;
	int argc = 0;

	if (input_str == NULL) {
		// EOF Caught!
		display("\n");
		exit_program(0, 0);
		return 0;
	}

	while (isspace(*input_str)) {
		input_str++;
	}
	size_t len = strlen(input_str);
	while (len >= 0 && isspace(input_str[len - 1])) {
		input_str[len - 1] = '\0';
		len--;
	}

	if (*input_str == '\0') {
		return 0;
	}

	token = strtok(input_str, delimiters);
	if (token != NULL) {
		argv[0] = token;
		argc++;
	}

	while (argc < args_maxn && (token = strtok(NULL, delimiters)) != NULL) {
		argv[argc] = token;
		argc++;
	}

	if (argc == args_maxn) {
		Lerror("Too many arguments: the count of %zd is allowed", args_maxn);
		return 0;
	}

	return argc;
}

char *line_interact(char *response, size_t maxn) {
	printf("\033[34m>>> ");
	fflush(stdout);
	char *ret = fgets(response, maxn, stdin);
	printf("\033[0m");
	fflush(stdout);

	return ret;
}
