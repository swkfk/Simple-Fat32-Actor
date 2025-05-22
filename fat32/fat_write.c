#include "../utils/utils.h"
#include "fat_reader.h"
#include "location.h"

static void free_cluster(fat_entry_t cluster) {
	uint32_t data = 0;
	for (int fat_no = 0; fat_no < img.header->NumberOfFAT; fat_no++) {
		int offset = loc_fat_nth_entry_bytes(&img, cluster, fat_no);
		write_file(img.fp, &data, offset, 4);
	}
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
