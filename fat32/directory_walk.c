#include <string.h>

#include "../interact/logger.h"
#include "../job/job.h"
#include "../utils/utils.h"
#include "common.h"
#include "data_reader.h"
#include "directory_entry.h"
#include "directory_walk.h"
#include "fat_reader.h"
#include "fat_write.h"
#include "img.h"
#include "location.h"

static size_t copy_into_buffer(void *buffer, struct Fat32_LongDirectoryEntry *dir) {
	void *dest = buffer;
	memcpy(dest, dir->Unicode_1, sizeof(dir->Unicode_1));
	dest += sizeof(dir->Unicode_1);
	memcpy(dest, dir->Unicode_2, sizeof(dir->Unicode_2));
	dest += sizeof(dir->Unicode_2);
	memcpy(dest, dir->Unicode_3, sizeof(dir->Unicode_3));
	dest += sizeof(dir->Unicode_3);
	return dest - buffer;
}

void dump_last_dir(DirectoryEntries dirs, struct Fat32_ShortDirectoryEntry **out) {
	struct Fat32_ShortDirectoryEntry *sdir = array_get_elem(dirs, -1);
	if (!sdir) {
		Lwarn("Unexpected emprt dirs!");
	}
	if (out) {
		*out = sdir;
	}
}

void dump_short_name(DirectoryEntries dirs, char *basename, char *extname) {
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

void dump_long_name(DirectoryEntries dirs, char *longname) {
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

	DirectoryEntries dirs = alloc_array(sizeof(struct DirectoryEntryWithOffset), 1);

	FOR_FAT_ENTRY_CHAIN (cluster, start_cluster) {
		read_cluster_content(cluster, cluster_data);

		FOR_DIRECTORY_ENTRY (dir, cluster_data, cluster_size) {

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

static DirectoryEntries _now_searched_entries;
static const char *_now_target_name;

DefDirWalkCb(search_directory_callback) {
	char short_basename[9], short_extname[4], longname[MAX_FILENAME_LENGTH];
	struct Fat32_ShortDirectoryEntry *dir;

	dump_short_name(dirs, short_basename, short_extname);
	dump_long_name(dirs, longname);
	dump_last_dir(dirs, &dir);

	Lverbose(
	    "Search Directory: _now_target_name: '%s', short_name: '%s', '%s', long_name: '%s'",
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
		Lverbose("Concated string: '%s' + '%s' ==> '%s'", short_basename, short_extname,
			 short_name);
		if (!strcmp(short_name, _now_target_name)) {
			goto found;
		} else {
			goto not_found;
		}
	}

found:
	_now_searched_entries = dirs;
	Ltrace("Find cluster: %d", JOIN_NUMBER(32, dir->StartCluster_hi, dir->StartCluster_lo));
	return true;

not_found:
	array_free(&dirs);
	return false;
}

int search_path(struct Fat32_Image *img, const char **pathes, int count, DirectoryEntries *out_dirs,
		fat_entry_t *out_start_cluster) {
	fat_entry_t current_start_cluster = img->header->RootClusterNumber;
	for (int i = 0; i < count; i++) {
		_now_target_name = pathes[i];
		_now_searched_entries = NULL;
		walk_directory_on_fat(img, current_start_cluster, search_directory_callback);

		if (!_now_searched_entries) {
			return E_FileOrDirectoryNotFound;
		}

		struct DirectoryEntryWithOffset *dir = array_get_elem(_now_searched_entries, -1);
		if (!dir) {
			return E_InvalidParam;
		}

		current_start_cluster =
		    JOIN_NUMBER(32, dir->entry.StartCluster_hi, dir->entry.StartCluster_lo);

		if (DIR_ENTRY_IS_DIR(&dir->entry)) {
			if (current_start_cluster == 0) {
				// TODO: Check it in the FAT spec!
				current_start_cluster = img->header->RootClusterNumber;
			}
		} else {
			// A file, must be the last entry!
			if (i != count - 1) {
				Lerror("'%s' is a file", _now_target_name);
				array_free(&_now_searched_entries);
				return E_ThisIsAFile;
			}
		}

		// Only the last one will not be freed here
		if (i != count - 1) {
			array_free(&_now_searched_entries);
		}
	}

	if (out_start_cluster) {
		*out_start_cluster = current_start_cluster;
	}

	if (out_dirs) {
		*out_dirs = _now_searched_entries;
		_now_searched_entries = NULL;
	} else {
		array_free(&_now_searched_entries);
	}

	return 0;
}

int alloc_directory_entries(struct Fat32_Image *img, fat_entry_t start_cluster, size_t count,
			    DirectoryEntries entries) {
	const size_t cluster_size = img->header->BytesPerSector * img->header->SectorsPerCluster;
	void *cluster_data =
	    checked_malloc(img->header->BytesPerSector, img->header->SectorsPerCluster);

	fat_entry_t last_cluster = 0;

	FOR_FAT_ENTRY_CHAIN (cluster, start_cluster) {
		read_cluster_content(cluster, cluster_data);
		last_cluster = cluster;
		for (void *entry = cluster_data; entry < ((void *)cluster_data + cluster_size);
		     entry += sizeof(struct Fat32_ShortDirectoryEntry)) {
			uint8_t leading_byte = ((unsigned char *)entry)[0];
			if (leading_byte != 0xE5 && leading_byte != 0x00) {
				array_drop_all(entries);
				continue;
			}
			struct DirectoryEntryWithOffset item = {
			    .offset =
				loc_data_bytes_by_cluster(img, cluster) + (entry - cluster_data),
			};
			array_append_elem(entries, &item);

			if (entries->position == count) {
				goto alloc_over;
			}
		}
	}

	size_t entry_per_cluster = cluster_size / sizeof(struct Fat32_ShortDirectoryEntry);
	size_t new_cluster_needed =
	    (count - entries->position + entry_per_cluster - 1) / entry_per_cluster;

	allocate_cluster_chain(start_cluster, new_cluster_needed);

	for (fat_entry_t cluster = fat_next_cluster(last_cluster); fat_is_valid_cluster(cluster);
	     cluster = fat_next_cluster(cluster)) {

		for (size_t offset = 0; offset < cluster_size && entries->position < count;
		     offset += sizeof(struct Fat32_ShortDirectoryEntry)) {

			struct DirectoryEntryWithOffset item = {
			    .offset = loc_data_bytes_by_cluster(img, cluster) + offset,
			};
			array_append_elem(entries, &item);
		}
	}

alloc_over:
	Ltrace("Allocated %zd (%zd really) %s from start cluster %d", count, entries->position,
	       count <= 1 ? "entry" : "entries", start_cluster);

	free(cluster_data);
	return 0;
}

uint8_t calculate_checksum(struct Fat32_ShortDirectoryEntry *sdir) {
	uint8_t i, j = 0, chksum = 0;
	for (i = 11; i > 0; i--) {
		chksum = ((chksum & 1) ? 0x80 : 0) + (chksum >> 1) +
			 (j < 8 ? sdir->BaseName[j] : sdir->ExtName[j - 8]);
		j++;
	}
	return chksum;
}
