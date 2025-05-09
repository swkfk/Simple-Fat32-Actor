#ifndef _INTERACT_INPUT_H_
#define _INTERACT_INPUT_H_

#include <stddef.h>

int parse_command_line(char *, char **, size_t);
char *line_interact(char *, size_t);

#endif
