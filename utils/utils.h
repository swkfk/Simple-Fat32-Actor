#ifndef _UTILS_UTILS_H_
#define _UTILS_UTILS_H_

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "macros.h"

// exit.c
__attribute__((noreturn)) void exit_program(int, int);

// error.c
const char *get_error_reason(int);

// file.c
int read_file(FILE *fp, void *out, size_t start, size_t len);

// memory.c
void *checked_malloc(size_t elem_size, size_t count);

// string.c
void strip_trailing(char *str, const char ch, size_t length);
void unicode2char(uint8_t *dest, uint16_t *src, size_t length);

#endif
