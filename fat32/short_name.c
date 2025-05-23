#include <ctype.h>
#include <stdbool.h>
#include <stddef.h>
#include <string.h>

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
			if (!isalpha(*it)) {
				// We delete all invalid characters here
				continue;
			}
			out->extname[ext_pos++] = toupper(*it);
		}
	}
	out->extname[ext_pos] = '\0';

	for (const char *it = longname; it < r_dot && *it && base_pos < 8; it++) {
		if (!isalpha(*it)) {
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
	for (tmp.index = 0; tmp.index < list->position; tmp.index++) {
		short_name_to_string(&tmp, tmp_short_name);
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

void short_name_to_string(const struct ShortName *sn, char *out) {
	if (sn->index == 0) {
		concat_short_name(out, sn->basename, sn->extname);
	} else {
		int number_length;
		char number[9], base_name[9];
		sprintf(number, "~%d%n", sn->index, &number_length);

		strcpy(base_name, sn->basename);
		strcpy(base_name + 8 - number_length, number);

		concat_short_name(out, base_name, sn->extname);
	}
}
