#include <string.h>

#include "../fat32/directory_walk.h"
#include "../fat32/short_name.h"
#include "job.h"

static struct Array *short_list;

DefDirWalkCb(collect_all_shortname_cb) {
	struct ShortName sn;
	struct Fat32_ShortDirectoryEntry *entry = array_get_elem(dirs, -1);
	bool has_index;

	strcpy(sn.extname, (const char *)&entry->BaseName);
	strcpy(sn.basename, (const char *)&entry->ExtName);

	for (const char *it = sn.basename; *it; it++) {
		if (*it == '~') {
			has_index = true;
			break;
		}
	}

	if (has_index) {
		sscanf(sn.basename, "%*[^~]~%d", &sn.index);
	} else {
		sn.index = 0;
	}

	array_append_elem(short_list, &sn);

	return false;
}

DEFINE_JOB(touch) {
	return 0;
}

DEFINE_JOB(mkdir) {
	return 0;
}
