#include "../fat32/directory_walk.h"
#include "../fat32/fat_write.h"
#include "job.h"
#include <string.h>

static bool _has_content;

DefDirWalkCb(check_empty_callback) {
	char basename[9], extname[4], shortname[13];
	dump_short_name(dirs, basename, extname);
	concat_short_name(shortname, basename, extname);
	bool end = false;
	if (strcmp(shortname, ".") && strcmp(shortname, "..")) {
		// Neither '.' nor '..'
		_has_content = true;
		end = true;
	} else {
		end = false;
	}
	array_free(&dirs);
	return end;
}

DEFINE_JOB(rm) {
	int ret = 0;

	fat_entry_t start_cluster = 0;
	DirectoryEntries dirs = NULL;
	struct Fat32_ShortDirectoryEntry *entry = NULL;

	ret = search_path(&img, (const char **)argv + 1, argc - 1, &dirs, &start_cluster);
	if (ret) {
		goto out;
	}

	if (start_cluster == img.header->RootClusterNumber || !dirs) {
		ret = E_PermissionDenied;
		goto out;
	}

	dump_last_dir(dirs, &entry);

	if (entry && DIR_ENTRY_IS_FILE(entry)) {
		// File, just delete it!
	} else {
		// Directory, check its contents
		_has_content = false;
		walk_directory_on_fat(&img, start_cluster, check_empty_callback);
		if (_has_content) {
			ret = E_DirectoryNotEmpty;
			goto out;
		}
	}

	// 1. Mark the directory entries
	for (int i = 0; i < dirs->position; i++) {
		struct DirectoryEntryWithOffset *e = array_get_elem(dirs, i);
		uint8_t data = 0xE5;
		write_file(img.fp, &data, e->offset, 1);
	}
	// 2. Release the clusters
	release_cluster_chain(start_cluster);
	// 3. Change the free cluster
	if (start_cluster && start_cluster < img.fsinfo->NextEmptyClusterNumber) {
		img.fsinfo->NextEmptyClusterNumber = start_cluster;
	}
	// Write back the fsinfo
	write_file(img.fp, img.fsinfo, img.header->FSINFO_SectorNumber * img.header->BytesPerSector,
		   sizeof(struct Fat32_FsInfo));

out:
	if (dirs) {
		array_free(&dirs);
	}
	return ret;
}
