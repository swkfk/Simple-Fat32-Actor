#ifndef _FAT32_SHORT_NAME_H_
#define _FAT32_SHORT_NAME_H_

#include "../utils/utils.h"

struct ShortName {
	char basename[9];
	char extname[4];
	int index;
};

void short_name_basic_spawn(const char *longname, struct ShortName *out);
int short_name_find_index(const struct ShortName *sn, struct Array *list);
void short_name_to_string(const struct ShortName *sn, char *out, char *out_basename,
			  char *out_extname);
bool is_already_shortname(const char *filename);

#endif
