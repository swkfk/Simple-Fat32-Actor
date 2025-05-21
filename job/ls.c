#include "job.h"

#include "../fat32/datetime.h"
#include "../fat32/directory_walk.h"

static bool display_entry_info(struct Fat32_ShortDirectoryEntry *dir, const char *short_basename,
			       const char *short_extname, const char *longname) {
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
	display("%s  ", dt_string);

	// Attributes
	if (DIR_ENTRY_IS_READONLY(dir)) {
		display("%s ", "READONLY");
	}
	if (DIR_ENTRY_IS_HIDDEN(dir)) {
		display("%s ", "HIDDEN");
	}

	display("\n");

	return false;
}

DEFINE_JOB(ls) {
	fat_entry_t current = img.header->RootClusterNumber;
	struct Fat32_ShortDirectoryEntry entry;

	for (int i = 1; i < argc; i++) {
		fat_entry_t searched_result =
		    search_directory_on_fat(&img, current, argv[i], &entry);
		if (-1 == searched_result) {
			Lerror("No such file or directory '%s' in cluster %d", argv[i], current);
			return E_FileOrDirectoryNotFound;
		} else if (searched_result != 0) {
			current = searched_result;
		} else {
			// TODO: Why?
			current = img.header->RootClusterNumber;
		}
		if (i != argc - 1 && DIR_ENTRY_IS_FILE(&entry)) {
			Lerror("'%s' is a file", argv[i]);
			return E_ThisIsAFile;
		}
	}

	if (current != img.header->RootClusterNumber && DIR_ENTRY_IS_FILE(&entry)) {
		display("[FILE] %s %d byte%s\n", argv[argc - 1], entry.FileLength,
			entry.FileLength ? "s" : "");

		char argv0[] = {""};
		char argv1[20];
		sprintf(argv1, "%d", current);
		char *argv[] = {argv0, argv1};
		CALL_JOB(read_data, 2, argv);

	} else if (current != 0) {
		walk_directory_on_fat(&img, current, display_entry_info);
	} else {
		// TODO: Why?
		walk_directory_on_fat(&img, img.header->RootClusterNumber, display_entry_info);
	}

	return 0;
}
