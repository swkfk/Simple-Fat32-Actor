#include "job.h"

#include "../fat32/datetime.h"
#include "../fat32/directory_walk.h"

DefDirWalkCb(display_entry_info) {
	char short_basename[9], short_extname[4], longname[MAX_FILENAME_LENGTH];
	struct Fat32_ShortDirectoryEntry *dir;

	dump_short_name(dirs, short_basename, short_extname);
	dump_long_name(dirs, longname);
	dump_last_dir(dirs, &dir);

	// File name
	if (longname[0]) {
		display("%40s ", longname);
	} else {
		char shortname[13];
		concat_short_name(shortname, short_basename, short_extname);
		display("%40s ", shortname);
	}

	// Length
	if (DIR_ENTRY_IS_FILE(dir)) {
		display("%8d ", dir->FileLength);
	} else {
		display("%8s ", "DIR");
	}

	struct Fat32_Datetime dt;
	char dt_string[25];

	dt = parse_datetime(dir->CreateDate, dir->CreateTime, dir->CreateTime_10ms);
	datetime_string(dt_string, &dt);
	display(" %s; ", dt_string);

	dt = parse_datetime(dir->ModifyDate, dir->ModifyTime, 0);
	datetime_string(dt_string, &dt);
	display("%s ", dt_string);

	// Attributes
	if (DIR_ENTRY_IS_READONLY(dir)) {
		display("%s ", "READONLY");
	}
	if (DIR_ENTRY_IS_HIDDEN(dir)) {
		display("%s ", "HIDDEN");
	}

	// Show short name!
	if (longname[0]) {
		Dtrace("B: '%8s', E: '%3s' ", short_basename, short_extname);
	}

	for (int i = 0; i < dirs->position; i++) {
		struct DirectoryEntryWithOffset *dir = array_get_elem(dirs, i);
		Dtrace("%zx ", dir->offset);
	}

	display("\n");

	array_free(&dirs);

	return false;
}

DEFINE_JOB(ls) {
	fat_entry_t start_cluster = 0;
	DirectoryEntries dirs = NULL;
	struct Fat32_ShortDirectoryEntry *entry = NULL;

	int r = search_path(&img, (const char **)argv + 1, argc - 1, &dirs, &start_cluster);
	if (r) {
		return r;
	}

	if (dirs) {
		dump_last_dir(dirs, &entry);
	}

	if (start_cluster != img.header->RootClusterNumber && entry && DIR_ENTRY_IS_FILE(entry)) {
		display("[FILE] %s %d byte%s\n", argv[argc - 1], entry->FileLength,
			entry->FileLength ? "s" : "");

		// Read the file content
		char argv0[] = {""};
		char argv1[20], argv2[20];
		sprintf(argv1, "%d", start_cluster);
		sprintf(argv2, "%d", entry->FileLength);
		char *argv[] = {argv0, argv1, argv2};
		CALL_JOB(read_data, 3, argv);

	} else {
		walk_directory_on_fat(&img, start_cluster, display_entry_info);
	}

	if (dirs) {
		array_free(&dirs);
	}

	return 0;
}
