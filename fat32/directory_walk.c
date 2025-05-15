#include <string.h>

#include "../interact/logger.h"
#include "../utils/utils.h"
#include "common.h"
#include "data_reader.h"
#include "directory_entry.h"
#include "directory_walk.h"
#include "fat_reader.h"
#include "img.h"

static void copy_into_buffer(long_name_entry_t *buffer, struct Fat32_LongDirectoryEntry *dir) {
	void *dest = buffer;
	memcpy(dest, dir->Unicode_1, sizeof(dir->Unicode_1));
	dest += sizeof(dir->Unicode_1);
	memcpy(dest, dir->Unicode_2, sizeof(dir->Unicode_2));
	dest += sizeof(dir->Unicode_2);
	memcpy(dest, dir->Unicode_3, sizeof(dir->Unicode_3));
}

static void copy_into_string(char *dest, long_name_entry_t *src, size_t count) {
	size_t len = 0;
	dest[0] = '\0';
	while (count--) {
		unicode2char((uint8_t *)dest + len, (uint16_t *)(src + count),
			     sizeof(long_name_entry_t) / 2);
		len += sizeof(long_name_entry_t) / 2;
	}
}

void walk_directory_on_fat(struct Fat32_Image *img, int start_cluster,
			   directory_walk_callback_t cb) {
	const size_t cluster_size = img->header->BytesPerSector * img->header->SectorsPerCluster;
	void *cluster_data =
	    checked_malloc(img->header->BytesPerSector, img->header->SectorsPerCluster);
	long_name_entry_t *filename_buffer =
	    checked_malloc(sizeof(long_name_entry_t), MAX_FILENAME_ENTRY_COUNT);
	char *filename =
	    checked_malloc(1, sizeof(long_name_entry_t) * MAX_FILENAME_ENTRY_COUNT / 2);

	size_t pos = 0;
	bool warned_too_long = false;

	FOR_FAT_ENTRY_CHAIN (cluster, start_cluster) {
		read_cluster_content(cluster, cluster_data);

		FOR_DIRECTORY_ENTRY(dir, cluster_data, cluster_size) {

			if (((unsigned char *)dir)[0] == 0xE5) {
				continue;
			}

			struct Fat32_ShortDirectoryEntry *sdir = dir;
			struct Fat32_LongDirectoryEntry *ldir = dir;

			uint8_t attr = sdir->Attribute;
			if (attr == DIR_ATTR_LONG_NAME) {
				if (pos == MAX_FILENAME_ENTRY_COUNT) {
					if (!warned_too_long) {
						copy_into_string(filename, filename_buffer, pos);
						Lwarn("filename too long: %s", filename);
						warned_too_long = true;
					}
				} else {
					copy_into_buffer(filename_buffer + pos, ldir);
					pos++;
				}
			} else {
				char base_name[9], ext_name[4];
				memcpy(base_name, sdir->BaseName, 8);
				memcpy(ext_name, sdir->ExtName, 3);
				strip_trailing(base_name, ' ', 8);
				strip_trailing(ext_name, ' ', 3);

				// Handle the long file name
				copy_into_string(filename, filename_buffer, pos);
				pos = 0;
				warned_too_long = false;

				if (cb(sdir, base_name, ext_name, filename)) {
					goto out;
				}
			}
		}
	}

out:
	free(cluster_data);
	free(filename_buffer);
	free(filename);
}

static fat_entry_t _now_searched_cluster;
static struct Fat32_ShortDirectoryEntry _now_searched_entry;
static const char *_now_target_name;

static bool search_directory_callback(struct Fat32_ShortDirectoryEntry *dir,
				      const char *short_basename, const char *short_extname,
				      const char *longname) {
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
		if (!strcmp(short_name, _now_target_name)) {
			goto found;
		} else {
			goto not_found;
		}
	}

found:
	_now_searched_cluster = JOIN_NUMBER(32, dir->StartCluster_hi, dir->StartCluster_lo);
	_now_searched_entry = *dir;
	return true;

not_found:
	return false;
}

fat_entry_t search_directory_on_fat(struct Fat32_Image *img, int start_cluster,
				    const char *filename, struct Fat32_ShortDirectoryEntry *entry) {
	_now_target_name = filename;
	_now_searched_cluster = 0;

	walk_directory_on_fat(img, start_cluster, search_directory_callback);

	if (entry && _now_searched_cluster) {
		*entry = _now_searched_entry;
	}
	return _now_searched_cluster;
}
