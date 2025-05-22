#ifndef _UTILS_ARRAY_H_
#define _UTILS_ARRAY_H_

#include <stdlib.h>

struct Array {
	size_t capacity;
	size_t position;
	size_t elem_size;
	void *_data;
};

struct Array *alloc_array(size_t elem_size, size_t init_capacity);
void *array_get_elem(struct Array *array, int pos);
void array_append_elem(struct Array *array, void *data);
void array_free(struct Array **p);

#endif
