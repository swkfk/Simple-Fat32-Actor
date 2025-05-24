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
	Ltrace("All directory entries were invalidated");

	// 2. Release the clusters
	release_cluster_chain(start_cluster);
	Ltrace("Clusters used were released");

	// 3. Change the free cluster
	if (start_cluster && start_cluster < img.fsinfo->NextEmptyClusterNumber) {
		img.fsinfo->NextEmptyClusterNumber = start_cluster;
		Ltrace("Modify the 'NextEmptyClusterNumber' as %x", start_cluster);
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

DEFINE_JOB(truncate) {
	int ret = 0;

	fat_entry_t start_cluster = 0;
	DirectoryEntries dirs = NULL;
	struct Fat32_ShortDirectoryEntry *entry = NULL;

	size_t new_size = atoll(argv[1]), old_size;
	size_t new_cluster_count, old_cluster_count;
	size_t cluster_byte = img.header->SectorsPerCluster * img.header->BytesPerSector;

	ret = search_path(&img, (const char **)argv + 2, argc - 2, &dirs, &start_cluster);
	if (ret) {
		goto out;
	}

	if (!dirs) {
		ret = E_PermissionDenied;
		goto out;
	}

	dump_last_dir(dirs, &entry);

	if (!entry || DIR_ENTRY_IS_DIR(entry)) {
		ret = E_PermissionDenied;
	}

	old_size = entry->FileLength;
	new_cluster_count = ROUND_UP(new_size, cluster_byte) / cluster_byte;
	old_cluster_count = ROUND_UP(old_size, cluster_byte) / cluster_byte;
	Ltrace("Truncate: %zd -> %zd, Cluster: %zd -> %zd", old_size, new_size, old_cluster_count,
	       new_cluster_count);

	if (old_size == new_size) {
		Ltrace("File size no changed");
		goto out;
	}
	if (old_cluster_count == new_cluster_count) {
		Ltrace("Cluster count no changed, just modify the size");
		goto entry_wb;
	} else if (old_cluster_count > new_cluster_count) {
		// Release some clusters
		truncate_cluster_chain(start_cluster, new_cluster_count);
	} else {
		// Allocate some clusters
		size_t needed_cluster = new_cluster_count - old_cluster_count;
		if (old_size == 0) {
			Ltrace("No origin clusters found, allocate a cluster first!");
			fat_entry_t allocated = 0;
			ret = allocate_orphan_cluster(&allocated);
			if (ret) {
				goto entry_wb;
			}

			Ltrace("Modify the file's start cluster to %x", allocated);
			entry->StartCluster_hi = (allocated >> 16) & 0xFFFF;
			entry->StartCluster_lo = (allocated) & 0xFFFF;
			write_file_directory_entry(img.fp, array_get_elem(dirs, -1));

			start_cluster = allocated;
			needed_cluster--;
		}
		if (needed_cluster) {
			allocate_cluster_chain(start_cluster, needed_cluster);
		}
	}

	if (new_size == 0) {
		Ltrace("Clear the start cluster in the file's directory entry");
		// Clear the start cluster!
		entry->StartCluster_hi = 0;
		entry->StartCluster_lo = 0;
		write_file_directory_entry(img.fp, array_get_elem(dirs, -1));
	}

	// Write back the fsinfo
	write_file(img.fp, img.fsinfo, img.header->FSINFO_SectorNumber * img.header->BytesPerSector,
		   sizeof(struct Fat32_FsInfo));

entry_wb:
	// TODO: Modify the datetime
	entry->FileLength = new_size;
	// Write back the directory entry
	write_file(img.fp, entry,
		   ((struct DirectoryEntryWithOffset *)array_get_elem(dirs, -1))->offset,
		   sizeof(struct Fat32_ShortDirectoryEntry));

out:
	if (dirs) {
		array_free(&dirs);
	}
	return ret;
}
