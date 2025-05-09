#ifndef _UTILS_UTILS_H_
#define _UTILS_UTILS_H_

// exit.c
__attribute__((noreturn)) void exit_program(int, int);

// error.c
const char *get_error_reason(int);

#endif
