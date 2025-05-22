#ifndef _FAT32_DIRECTORY_WALK_H_
#define _FAT32_DIRECTORY_WALK_H_

#include "directory_entry.h"
#include "img.h"

#include <stdbool.h>

#define DirectoryWalkCallbackParams struct Array *dirs
// Callback shall free the array!
#define DefDirWalkCb(fn) static bool fn(DirectoryWalkCallbackParams)

typedef bool(directory_walk_callback_t)(DirectoryWalkCallbackParams);

void walk_directory_on_fat(struct Fat32_Image *img, int start_cluster,
			   directory_walk_callback_t cb);

void dump_last_dir(struct Array *dirs, struct Fat32_ShortDirectoryEntry **out);
void dump_short_name(struct Array *dirs, char *basename, char *extname);
void dump_long_name(struct Array *dirs, char *longname);

int search_path(struct Fat32_Image *img, const char **pathes, int count, DirectoryEntries *out_dirs,
		fat_entry_t *out_start_cluster);

#endif
