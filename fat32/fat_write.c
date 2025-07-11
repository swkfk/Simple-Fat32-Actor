#include "../job/job.h"
#include "../utils/utils.h"
#include "fat_reader.h"
#include "location.h"

static void write_cluster_in_fat(fat_entry_t cluster, fat_entry_t data) {
	for (int fat_no = 0; fat_no < img.header->NumberOfFAT; fat_no++) {
		int offset = loc_fat_nth_entry_bytes(&img, cluster, fat_no);
		write_file(img.fp, &data, offset, 4);
	}
}

static void free_cluster(fat_entry_t cluster) {
	Lverbose("Free cluster: %x", cluster);
	write_cluster_in_fat(cluster, 0);
}

static void terminate_cluster(fat_entry_t cluster) {
	write_cluster_in_fat(cluster, FAT_ENTRY_TERMINATE_LOW);
}

void release_cluster_chain(fat_entry_t start_cluster) {
	if (start_cluster == 0) {
		return;
	}
	FOR_FAT_ENTRY_CHAIN (cluster, start_cluster) {
		free_cluster(cluster);
		// Not write back here!
		img.fsinfo->EmptyClusterCount++;
	}
	invalidate_fat();
}

void truncate_cluster_chain(fat_entry_t start_cluster, size_t remained_count) {
	if (remained_count == 0) {
		Ltrace("Release all clusters from %x", start_cluster);
		release_cluster_chain(start_cluster);
		return;
	}

	// Assert remained_count > 0!
	fat_entry_t last_cluster = 0, current_cluster = 0;

	FOR_FAT_ENTRY_CHAIN (cluster, start_cluster) {
		current_cluster = cluster;
		if (!(remained_count--)) {
			break;
		}
		last_cluster = cluster;
	}
	Ltrace("Release cluster chain from %x", current_cluster);
	release_cluster_chain(current_cluster);
	Ltrace("The terminated cluster is %x now", last_cluster);
	terminate_cluster(last_cluster);
}

int allocate_cluster_chain(fat_entry_t start_cluster, size_t addition_count) {
	if (img.fsinfo->EmptyClusterCount < addition_count) {
		return E_NoSpace;
	}

	fat_entry_t last_cluster = 0;
	FOR_FAT_ENTRY_CHAIN (cluster, start_cluster) {
		last_cluster = cluster;
	}

	Ltrace("Allocate %zd clusters from origin terminated cluster %x", addition_count,
	       last_cluster);

	for (size_t i = 0; i < addition_count; i++) {

		fat_entry_t allocated = find_free_cluster();
		if (allocated == 0) {
			return E_NoSpace;
		}
		Lverbose("Allocate cluster: %x", allocated);

		img.fsinfo->NextEmptyClusterNumber = allocated;
		img.fsinfo->EmptyClusterCount--;

		// Write into file first, and then read it out? Ugly code here!
		write_cluster_in_fat(last_cluster, allocated);
		write_cluster_in_fat(allocated, FAT_ENTRY_TERMINATE_LOW);
		last_cluster = allocated;
		invalidate_fat();
	}

	Ltrace("The terminated cluster is %x now", last_cluster);

	invalidate_fat();
	return 0;
}

int allocate_orphan_cluster(fat_entry_t *out) {
	fat_entry_t allocated = find_free_cluster();
	if (allocated == 0) {
		return E_NoSpace;
	}
	Lverbose("Allocate cluster: %x", allocated);

	img.fsinfo->NextEmptyClusterNumber = allocated;
	img.fsinfo->EmptyClusterCount--;

	write_cluster_in_fat(allocated, FAT_ENTRY_TERMINATE_LOW);
	invalidate_fat();

	*out = allocated;
	return 0;
}
