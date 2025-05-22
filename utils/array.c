#include "array.h"
#include "utils.h"
#include <string.h>

struct Array *alloc_array(size_t elem_size, size_t init_capacity) {
	init_capacity = init_capacity ? init_capacity : 4;

	struct Array *array = checked_malloc(sizeof(struct Array), 1);
	array->capacity = init_capacity;
	array->position = 0;
	array->elem_size = elem_size;
	array->_data = checked_malloc(elem_size, init_capacity);

	return array;
}

void *array_get_elem(struct Array *array, int pos) {
	if (pos < 0) {
		pos = array->position + pos;
	}
	if (pos < 0 || pos >= array->position) {
		return NULL;
	}
	return array->_data + pos * array->elem_size;
}

void array_append_elem(struct Array *array, void *data) {
	if (array->position == array->capacity) {
		array->capacity = array->capacity * 3 / 2 + 1;
		array->_data = checked_realloc(array->_data, array->capacity * array->elem_size);
	}
	memcpy(array->_data + array->position * array->elem_size, data, array->elem_size);
	array->position++;
}

void array_free(struct Array **p) {
	free((*p)->_data);
	free(*p);
	*p = NULL;
}
