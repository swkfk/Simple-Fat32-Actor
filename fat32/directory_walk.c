#include <string.h>

#include "../interact/logger.h"
#include "../utils/utils.h"
#include "common.h"
#include "data_reader.h"
#include "directory_entry.h"
#include "directory_walk.h"
#include "fat_reader.h"
#include "img.h"
#include "location.h"

static size_t copy_into_buffer(void *buffer, struct Fat32_LongDirectoryEntry *dir) {
	void *dest = buffer;
	memcpy(dest, dir->Unicode_1, sizeof(dir->Unicode_1));
	dest += sizeof(dir->Unicode_1);
	memcpy(dest, dir->Unicode_2, sizeof(dir->Unicode_2));
	dest += sizeof(dir->Unicode_2);
	memcpy(dest, dir->Unicode_3, sizeof(dir->Unicode_3));
	return dest - buffer;
}

void dump_last_dir(struct Array *dirs, struct Fat32_ShortDirectoryEntry **out) {
	struct Fat32_ShortDirectoryEntry *sdir = array_get_elem(dirs, -1);
	if (!sdir) {
		Lwarn("Unexpected emprt dirs!");
	}
	if (out) {
		*out = sdir;
	}
}

void dump_short_name(struct Array *dirs, char *basename, char *extname) {
	struct Fat32_ShortDirectoryEntry *sdir;
	dump_last_dir(dirs, &sdir);

	if (!sdir) {
		basename[0] = extname[0] = '\0';
		return;
	}
	memcpy(basename, sdir->BaseName, 8);
	memcpy(extname, sdir->ExtName, 3);
	strip_trailing(basename, ' ', 8);
	strip_trailing(extname, ' ', 3);
}

void dump_long_name(struct Array *dirs, char *longname) {
	char buffer[MAX_FILENAME_LENGTH * 2] = {};
	size_t offset = 0;
	for (int i = dirs->position - 2; i >= 0; i--) {
		offset += copy_into_buffer(buffer + offset, array_get_elem(dirs, i));
	}
	unicode2char((uint8_t *)longname, (uint16_t *)buffer, MAX_FILENAME_LENGTH - 1);
}

void walk_directory_on_fat(struct Fat32_Image *img, int start_cluster,
			   directory_walk_callback_t cb) {
	const size_t cluster_size = img->header->BytesPerSector * img->header->SectorsPerCluster;
	void *cluster_data =
	    checked_malloc(img->header->BytesPerSector, img->header->SectorsPerCluster);

	struct Array *dirs = alloc_array(sizeof(struct DirectoryEntryWithOffset), 1);

	FOR_FAT_ENTRY_CHAIN (cluster, start_cluster) {
		read_cluster_content(cluster, cluster_data);

		FOR_DIRECTORY_ENTRY(dir, cluster_data, cluster_size) {

			if (((unsigned char *)dir)[0] == 0xE5) {
				continue;
			}

			struct Fat32_ShortDirectoryEntry *sdir = dir;

			struct DirectoryEntryWithOffset item = {
			    .entry = *sdir,
			    .offset =
				loc_data_bytes_by_cluster(img, cluster) + (dir - cluster_data),
			};
			array_append_elem(dirs, &item);

			if (sdir->Attribute != DIR_ATTR_LONG_NAME) {
				// The cb shall free the dirs!
				bool end = cb(dirs);
				dirs = alloc_array(sizeof(struct DirectoryEntryWithOffset), 1);
				if (end) {
					goto out;
				}
			}
		}
	}

out:
	free(cluster_data);
	array_free(&dirs);
}

static fat_entry_t _now_searched_cluster;
static struct Fat32_ShortDirectoryEntry _now_searched_entry;
static const char *_now_target_name;

DefDirWalkCb(search_directory_callback) {
	bool end = false;

	char short_basename[9], short_extname[4], longname[MAX_FILENAME_LENGTH];
	struct Fat32_ShortDirectoryEntry *dir;

	dump_short_name(dirs, short_basename, short_extname);
	dump_long_name(dirs, longname);
	dump_last_dir(dirs, &dir);

	Ltrace("Search Directory: _now_target_name: '%s', short_name: '%s', '%s', long_name: '%s'",
	       _now_target_name, short_basename, short_extname, longname);
	if (longname[0] != '\0') {
		// Just compare the long name
		if (!strcmp(longname, _now_target_name)) {
			goto found;
		} else {
			goto not_found;
		}
	} else {
		// TODO: Check the FAT spec to find whether there is need to consider all the
		// situations!

		// Build the short name, and compare it!
		char short_name[13];
		concat_short_name(short_name, short_basename, short_extname);
		Ltrace("Concated string: '%s' + '%s' ==> '%s'", short_basename, short_extname,
		       short_name);
		if (!strcmp(short_name, _now_target_name)) {
			goto found;
		} else {
			goto not_found;
		}
	}

found:
	_now_searched_cluster = JOIN_NUMBER(32, dir->StartCluster_hi, dir->StartCluster_lo);
	Ltrace("Set _now_searched_cluster = %d", _now_searched_cluster);
	_now_searched_entry = *dir;
	end = true;
	goto out;

not_found:
	end = false;
	goto out;

out:
	array_free(&dirs);
	return end;
}

fat_entry_t search_directory_on_fat(struct Fat32_Image *img, int start_cluster,
				    const char *filename, struct Fat32_ShortDirectoryEntry *entry) {
	_now_target_name = filename;
	_now_searched_cluster = -1;

	walk_directory_on_fat(img, start_cluster, search_directory_callback);

	if (entry && _now_searched_cluster) {
		*entry = _now_searched_entry;
	}
	return _now_searched_cluster;
}
