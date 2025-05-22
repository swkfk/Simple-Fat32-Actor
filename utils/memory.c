#include <stdlib.h>

#include "../interact/logger.h"
#include "utils.h"

void *checked_malloc(size_t elem_size, size_t count) {
	void *p = malloc(elem_size * count);
	if (p == NULL) {
		Lerror("OOM! We need %zd bytes", elem_size * count);
		exit_program(1, 0);
	}
	return p;
}

void *checked_realloc(void *ptr, size_t new_size) {
	void *p = realloc(ptr, new_size);
	if (p == NULL) {
		Lerror("OOM! We need %zd bytes", new_size);
		exit_program(1, 0);
	}
	return p;
}
