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
	Ltrace("Free cluster: %x", cluster);
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
	// Assert remained_count > 0!
	fat_entry_t last_cluster, current_cluster;

	FOR_FAT_ENTRY_CHAIN (cluster, start_cluster) {
		current_cluster = cluster;
		if (!(remained_count--)) {
			break;
		}
		last_cluster = cluster;
	}
	release_cluster_chain(current_cluster);
	terminate_cluster(last_cluster);
}

int allocate_cluster_chain(fat_entry_t start_cluster, size_t addition_count) {
	if (img.fsinfo->EmptyClusterCount < addition_count) {
		return E_NoSpace;
	}

	fat_entry_t last_cluster;
	FOR_FAT_ENTRY_CHAIN (cluster, start_cluster) {
		last_cluster = cluster;
	}

	for (size_t i = 0; i < addition_count; i++) {

		fat_entry_t allocated = find_free_cluster();
		if (allocated == 0) {
			return E_NoSpace;
		}
		Ltrace("Allocate cluster: %x", allocated);

		img.fsinfo->NextEmptyClusterNumber = allocated;
		img.fsinfo->EmptyClusterCount--;

		// Write into file first, and then read it out? Ugly code here!
		write_cluster_in_fat(last_cluster, allocated);
		write_cluster_in_fat(allocated, FAT_ENTRY_TERMINATE_LOW);
		last_cluster = allocated;
		invalidate_fat();
	}

	invalidate_fat();
	return 0;
}
