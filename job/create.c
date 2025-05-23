#include <string.h>

#include "../fat32/directory_walk.h"
#include "../fat32/fat_write.h"
#include "../fat32/location.h"
#include "../fat32/short_name.h"
#include "../utils/array.h"
#include "job.h"

static struct Array *_short_list;
static const char *_target_filename;
static bool _already_exist;

DefDirWalkCb(collect_all_shortname_cb) {
	char shortname[14], longname[MAX_FILENAME_LENGTH];

	char basename[9] = {}, extname[4] = {};
	dump_short_name(dirs, basename, extname);
	dump_long_name(dirs, longname);
	concat_short_name(shortname, basename, extname);

	if (longname[0] != '\0') {
		if (!strcmp(longname, _target_filename)) {
			_already_exist = true;
			return true;
		}
	} else {
		if (!strcmp(shortname, _target_filename)) {
			_already_exist = true;
			return true;
		}
	}

	array_append_elem(_short_list, shortname);

	return false;
}

static int create_item(bool is_file, const char *filename, char **pathes, int path_count,
		       struct DirectoryEntryWithOffset *out_last_entry,
		       fat_entry_t *out_parent_start_cluster) {
	if (strlen(filename) > MAX_FILENAME_LENGTH - 1) {
		Lwarn("Filename too long, %d-1 allowed", MAX_FILENAME_LENGTH);
		return E_InvalidParam;
	}

	fat_entry_t parent_dir_cluster = 0;
	DirectoryEntries dirs = NULL;
	struct Fat32_ShortDirectoryEntry *entry = NULL;

	int r = search_path(&img, (const char **)pathes, path_count, &dirs, &parent_dir_cluster);
	if (r) {
		return r;
	}

	if (dirs) {
		dump_last_dir(dirs, &entry);
	}

	if (parent_dir_cluster != img.header->RootClusterNumber && entry &&
	    DIR_ENTRY_IS_FILE(entry)) {
		return E_ThisIsAFile;
	}

	if (out_parent_start_cluster) {
		*out_parent_start_cluster = parent_dir_cluster;
	}

	_short_list = alloc_array(14, 0);
	_already_exist = false;
	_target_filename = filename;
	walk_directory_on_fat(&img, parent_dir_cluster, collect_all_shortname_cb);

	int ret = 0;

	if (_already_exist) {
		ret = E_FileOrDirectoryExists;
		goto out_free_array;
	}

	char shortname[14], short_base[9], short_ext[4];

	struct ShortName sn;
	short_name_basic_spawn(filename, &sn);
	Dtrace("B: '%s', E: '%s'\n", sn.basename, sn.extname);
	int index = short_name_find_index(&sn, _short_list);
	if (index < 0) {
		Lwarn("Cannot find a correct index in shortname");
		ret = E_PermissionDenied;
		goto out_free_array;
	}
	sn.index = index;
	short_name_to_string(&sn, shortname, short_base, short_ext);

	int directory_needed = 1;
	if (!is_already_shortname(filename) || sn.index != 0) {
		directory_needed += (strlen(filename) + 12) / 13;
	}

	Ltrace("Create item: Longname('%s'), Shortname('%s'), need %d %s", filename, shortname,
	       directory_needed, directory_needed == 1 ? "entry" : "entries");

	// Alloc directory entries, but we do not use the entry inside
	DirectoryEntries new_entries = alloc_array(sizeof(struct DirectoryEntryWithOffset), 0);
	alloc_directory_entries(&img, parent_dir_cluster, directory_needed, new_entries);

	Ltrace("Allocated enteries: %zd", new_entries->position);
	for (size_t i = 0; i < new_entries->position; i++) {
		struct DirectoryEntryWithOffset *o = array_get_elem(new_entries, i);
		Dtrace("(O): %zx\n", o->offset);
	}

	// Write the last entry (The shortname entry)
	struct DirectoryEntryWithOffset *short_entry_off = array_get_elem(new_entries, -1);
	struct Fat32_ShortDirectoryEntry *short_entry = &short_entry_off->entry;

	// Fill short name
	memset(short_entry->BaseName, ' ', 8);
	memset(short_entry->ExtName, ' ', 3);
	memcpy(short_entry->BaseName, short_base, strlen(short_base));
	memcpy(short_entry->ExtName, short_ext, strlen(short_ext));

	Ltrace("Short name filled!");
	// TODO: Fill the datetime

	if (is_file) {
		// Do nothing!
	} else {
		short_entry->Attribute = DIR_ATTR_DIRECTORY;

		// Alloc a cluster for directory
		fat_entry_t allocated;
		if ((ret = allocate_orphan_cluster(&allocated))) {
			goto out_free_entries;
		}
		short_entry->StartCluster_hi = (allocated >> 16) & 0xFFFF;
		short_entry->StartCluster_lo = (allocated) & 0xFFFF;

		// Fill the '.' & '..' entry
		struct Fat32_ShortDirectoryEntry dot[2] = {};
		// Simple name
		memset(dot[0].BaseName, ' ', 8 + 3);
		dot[0].BaseName[0] = '.';
		memset(dot[1].BaseName, ' ', 8 + 3);
		dot[1].BaseName[0] = '.';
		dot[1].BaseName[1] = '.';
		// Attribute
		dot[0].Attribute = DIR_ATTR_DIRECTORY;
		dot[1].Attribute = DIR_ATTR_DIRECTORY;
		// Cluster
		dot[0].StartCluster_hi = short_entry->StartCluster_hi;
		dot[0].StartCluster_lo = short_entry->StartCluster_lo;
		if (parent_dir_cluster != img.header->RootClusterNumber) {
			dot[1].StartCluster_hi = (parent_dir_cluster >> 16) & 0xFFFF;
			dot[1].StartCluster_lo = (parent_dir_cluster) & 0xFFFF;
		}
		// TODO: Fill the datetime
		// Write into the cluster
		write_file(img.fp, dot, loc_data_bytes_by_cluster(&img, allocated),
			   2 * sizeof(struct Fat32_ShortDirectoryEntry));
	}

	// Write the short entry
	write_file_directory_entry(img.fp, short_entry_off);
	if (out_last_entry) {
		*out_last_entry = *short_entry_off;
	}

	// Fill the longname
	uint8_t checksum = calculate_checksum(short_entry);

	// A simple way to avoid out of boundary
	char longname[MAX_FILENAME_LENGTH + 26] = {};
	strcpy(longname, filename);

	for (int i = 1; i < directory_needed; i++) {
		struct DirectoryEntryWithOffset *long_entry_off =
		    array_get_elem(new_entries, -1 - i);
		struct Fat32_LongDirectoryEntry *long_entry =
		    (struct Fat32_LongDirectoryEntry *)&long_entry_off->entry;

		size_t offset_in_string = (i - 1) * 13;
		char2unicode((uint16_t *)long_entry->Unicode_1,
			     (uint8_t *)longname + offset_in_string, 5);
		char2unicode((uint16_t *)long_entry->Unicode_2,
			     (uint8_t *)longname + offset_in_string + 5, 6);
		char2unicode((uint16_t *)long_entry->Unicode_3,
			     (uint8_t *)longname + offset_in_string + 11, 2);

		long_entry->Sequence = i;
		long_entry->IsLastEntry = (i == directory_needed - 1);
		long_entry->Attribute = DIR_ATTR_LONG_NAME;
		long_entry->Checksum = checksum;
		// Write the long entry
		write_file_directory_entry(img.fp, long_entry_off);
	}

out_free_entries:
	// Write back the fsinfo. Even the error occurred, we still keep the allocated clusters for
	// the directories
	write_file(img.fp, img.fsinfo, img.header->FSINFO_SectorNumber * img.header->BytesPerSector,
		   sizeof(struct Fat32_FsInfo));
	array_free(&new_entries);

out_free_array:
	array_free(&_short_list);

	return ret;
}

DEFINE_JOB(touch) {
	if (argc == 1) {
		return E_InvalidParam;
	}
	return create_item(true, argv[argc - 1], argv + 1, argc - 2, NULL, NULL);
}

DEFINE_JOB(mkdir) {
	if (argc == 1) {
		return E_InvalidParam;
	}
	return create_item(false, argv[argc - 1], argv + 1, argc - 2, NULL, NULL);
}

DEFINE_JOB(mv) {
	int ret = 0;

	int pos_of_arrow = -1;
	for (int i = 1; i < argc; i++) {
		if (!strcmp(argv[i], "->")) {
			pos_of_arrow = i;
			break;
		}
	}
	if (pos_of_arrow == -1) {
		Lwarn("-> is needed in the arguments!");
		return E_InvalidParam;
	}
	if (pos_of_arrow == argc - 1) {
		Lwarn("-> shall not be the last argument!");
		return E_InvalidParam;
	}

	size_t src_count = pos_of_arrow - 1;
	size_t dest_count = argc - pos_of_arrow - 1;

	fat_entry_t src_start_cluster = 0;
	DirectoryEntries src_dirs = NULL;
	struct Fat32_ShortDirectoryEntry *src_last_entry = NULL;

	ret = search_path(&img, (const char **)argv + 1, src_count, &src_dirs, &src_start_cluster);
	if (ret) {
		goto out;
	}

	if (src_start_cluster == img.header->RootClusterNumber) {
		ret = E_PermissionDenied;
		goto out;
	}

	if (!src_dirs) {
		ret = E_FileOrDirectoryNotFound;
		goto out;
	}

	dump_last_dir(src_dirs, &src_last_entry);

	// Invalidate the old entries first to avoid move parent into its child
	for (size_t i = 0; i < src_dirs->position; i++) {
		struct DirectoryEntryWithOffset *o = array_get_elem(src_dirs, i);
		o->entry._Reserved_1 = o->entry.BaseName[0];
		o->entry.BaseName[0] = 0xE5;
		write_file_directory_entry(img.fp, o);
	}

	// Create the new entry first AS A FILE!
	fat_entry_t dest_parent_cluster = 0;
	struct DirectoryEntryWithOffset dest_last_entry;
	ret = create_item(true, argv[argc - 1], argv + pos_of_arrow + 1, dest_count - 1,
			  &dest_last_entry, &dest_parent_cluster);
	if (ret) {
		// Recover the invalidated items
		for (size_t i = 0; i < src_dirs->position; i++) {
			struct DirectoryEntryWithOffset *o = array_get_elem(src_dirs, i);
			o->entry.BaseName[0] = o->entry._Reserved_1;
			o->entry._Reserved_1 = 0;
			write_file_directory_entry(img.fp, o);
		}

		goto out;
	}

	// Modify the new entry
	dest_last_entry.entry.StartCluster_hi = src_last_entry->StartCluster_hi;
	dest_last_entry.entry.StartCluster_lo = src_last_entry->StartCluster_lo;
	dest_last_entry.entry.FileLength = src_last_entry->FileLength;
	dest_last_entry.entry.Attribute = src_last_entry->Attribute;
	write_file_directory_entry(img.fp, &dest_last_entry);
	// TODO: Fill the datetime

	// If is directory, modify the parent directory cluster
	if (DIR_ENTRY_IS_DIR(src_last_entry)) {
		struct Fat32_ShortDirectoryEntry dotdot;
		size_t offset = loc_data_bytes_by_cluster(&img, src_start_cluster) +
				sizeof(struct Fat32_ShortDirectoryEntry);
		read_file(img.fp, &dotdot, offset, sizeof(struct Fat32_ShortDirectoryEntry));
		dotdot.StartCluster_hi = (dest_parent_cluster >> 16) & 0xFF;
		dotdot.StartCluster_lo = (dest_parent_cluster) & 0xFF;
		write_file(img.fp, &dotdot, offset, sizeof(struct Fat32_ShortDirectoryEntry));
	}

out:
	if (src_dirs) {
		array_free(&src_dirs);
	}
	return ret;
}
