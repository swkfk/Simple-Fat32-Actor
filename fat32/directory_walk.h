#ifndef _FAT32_DIRECTORY_WALK_H_
#define _FAT32_DIRECTORY_WALK_H_

#include "directory_entry.h"
#include "img.h"

#include <stdbool.h>

#define DirectoryWalkCallbackParams                                                                \
	struct Fat32_ShortDirectoryEntry *dir, size_t dir_offset, const char *short_basename,      \
	    const char *short_extname, const char *longname
#define DefDirWalkCb(fn) static bool fn(DirectoryWalkCallbackParams)

typedef bool(directory_walk_callback_t)(DirectoryWalkCallbackParams);

void walk_directory_on_fat(struct Fat32_Image *img, int start_cluster,
			   directory_walk_callback_t cb);
fat_entry_t search_directory_on_fat(struct Fat32_Image *img, int start_cluster,
				    const char *filename, struct Fat32_ShortDirectoryEntry *entry);

#endif
