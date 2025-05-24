#include <ctype.h>
#include <stdbool.h>
#include <stddef.h>
#include <string.h>

#include "../interact/logger.h"
#include "../utils/utils.h"
#include "short_name.h"

void short_name_basic_spawn(const char *longname, struct ShortName *out) {
	size_t long_length = strlen(longname), base_pos = 0, ext_pos = 0;

	// Find the rightesr dot
	const char *r_dot = longname + long_length;
	for (const char *it = longname; *it; it++) {
		if (*it == '.') {
			r_dot = it;
		}
	}

	out->index = 0;

	if (r_dot != longname + long_length) {
		for (const char *it = r_dot + 1; *it && ext_pos < 3; it++) {
			if (!isalpha(*it) && !isdigit(*it)) {
				// We delete all invalid characters here
				continue;
			}
			out->extname[ext_pos++] = toupper(*it);
		}
	}
	out->extname[ext_pos] = '\0';

	for (const char *it = longname; it < r_dot && *it && base_pos < 8; it++) {
		if (!isalpha(*it) && !isdigit(*it)) {
			// We delete all invalid characters here
			continue;
		}
		out->basename[base_pos++] = toupper(*it);
	}
	out->basename[base_pos] = '\0';
}

int short_name_find_index(const struct ShortName *sn, struct Array *list) {
	struct ShortName tmp = *sn;
	char tmp_short_name[13];
	bool this_index_ok;

	// The condition to avoid endless loop!
	for (tmp.index = 0; tmp.index <= list->position; tmp.index++) {
		short_name_to_string(&tmp, tmp_short_name, NULL, NULL);
		Dverbose("Try shortname: '%s'\n", tmp_short_name);
		this_index_ok = true;
		for (int i = 0; i < list->position; i++) {
			if (!strcmp(tmp_short_name, array_get_elem(list, i))) {
				this_index_ok = false;
				break;
			}
		}
		if (this_index_ok) {
			return tmp.index;
		}
	}

	return -1;
}

void short_name_to_string(const struct ShortName *sn, char *out, char *out_basename,
			  char *out_extname) {
	if (out_extname) {
		strcpy(out_extname, sn->extname);
	}
	if (sn->index == 0) {
		concat_short_name(out, sn->basename, sn->extname);
		if (out_basename) {
			strcpy(out_basename, sn->basename);
		}
	} else {
		int number_length;
		char number[13], base_name[9];
		sprintf(number, "~%d%n", sn->index, &number_length);

		strcpy(base_name, sn->basename);
		if (strlen(base_name) + number_length <= 8) {
			strcat(base_name, number);
		} else {
			strcpy(base_name + 8 - number_length, number);
		}

		concat_short_name(out, base_name, sn->extname);
		if (out_basename) {
			strcpy(out_basename, base_name);
		}
	}
}

bool is_already_shortname(const char *filename) {
	size_t len = strlen(filename);
	const char *dot = strchr(filename, '.');
	size_t base_len = dot ? (size_t)(dot - filename) : len;
	size_t ext_len = dot ? strlen(dot + 1) : 0;

	// Check the length: 1-8.0-3
	if (base_len == 0 || base_len > 8 || ext_len > 3) {
		return false;
	}

	// Check base name: 1-8 uppercase letters or digits, no spaces
	for (size_t i = 0; i < base_len; i++) {
		if (!isalnum(filename[i]) || !isupper(filename[i])) {
			return false;
		}
	}

	// Check extension: 0-3 uppercase letters or digits, no spaces
	for (size_t i = 0; i < ext_len; i++) {
		if (!isalnum(dot[1 + i]) || !isupper(dot[1 + i])) {
			return false;
		}
	}

	return true;
}
