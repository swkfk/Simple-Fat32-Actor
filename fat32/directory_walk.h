#ifndef _FAT32_DIRECTORY_WALK_H_
#define _FAT32_DIRECTORY_WALK_H_

#include "directory_entry.h"
#include "img.h"

#include <stdbool.h>

typedef bool(directory_walk_callback_t)(struct Fat32_ShortDirectoryEntry *dir,
					const char *short_basename, const char *short_extname,
					const char *longname);

void walk_directory_on_fat(struct Fat32_Image *img, int start_cluster,
			   directory_walk_callback_t cb);

#endif
