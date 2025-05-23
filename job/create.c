#include <string.h>

#include "../fat32/directory_walk.h"
#include "../fat32/short_name.h"
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

static int create_item(bool is_file, const char *filename, char **pathes, int path_count) {
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

	_short_list = alloc_array(14, 0);
	_already_exist = false;
	_target_filename = filename;
	walk_directory_on_fat(&img, parent_dir_cluster, collect_all_shortname_cb);

	int ret = 0;

	if (_already_exist) {
		ret = E_FileOrDirectoryExists;
		goto out_free_array;
	}

	char shortname[14];
	bool only_shortname = is_already_shortname(filename);
	if (only_shortname) {
		strcpy(shortname, filename);
	} else {
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
		short_name_to_string(&sn, shortname);
	}

	int directory_needed = 1;
	if (!only_shortname) {
		directory_needed += (strlen(filename) + 12) / 13;
	}

	Ltrace("Create item: Longname('%s'), Shortname('%s'), need %d %s", filename, shortname,
	       directory_needed, directory_needed == 1 ? "entry" : "entries");

	//

out_free_array:
	array_free(&_short_list);

	return ret;
}

DEFINE_JOB(touch) {
	if (argc == 1) {
		return E_InvalidParam;
	}
	return create_item(true, argv[argc - 1], argv + 1, argc - 2);
}

DEFINE_JOB(mkdir) {
	if (argc == 1) {
		return E_InvalidParam;
	}
	return create_item(false, argv[argc - 1], argv + 1, argc - 2);
}
