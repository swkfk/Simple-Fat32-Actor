#include <ctype.h>

#include "../../fat32/data_reader.h"
#include "../../fat32/fat_reader.h"
#include "../job.h"

DEFINE_JOB(read_data) {
	if (argc != 2 && argc != 3) {
		return E_InvalidParam;
	}

	int start_cluster = atoi(argv[1]), limit_bytes;

	if (argc == 3) {
		limit_bytes = atoi(argv[2]);
	} else {
		limit_bytes = 0x7FFFFFFF;
	}

	const size_t cluster_size = img.header->BytesPerSector * img.header->SectorsPerCluster;
	const int line_count = 16;
	char *cluster_data =
	    checked_malloc(img.header->BytesPerSector, img.header->SectorsPerCluster);

	uint32_t offset = 0;
	FOR_FAT_ENTRY_CHAIN (cluster, start_cluster) {
		read_cluster_content(cluster, cluster_data);
		for (uint32_t inner_offset = 0; inner_offset < cluster_size;
		     inner_offset += line_count) {
			if (offset + inner_offset >= limit_bytes) {
				goto out;
			}
			display("%08X ", offset + inner_offset);
			for (uint8_t i = 0; i < line_count; i++) {
				display("%02X ", cluster_data[inner_offset + i] & 0xFF);
			}
			display(" |  ");
			for (uint8_t i = 0; i < line_count; i++) {
				if (isprint(cluster_data[inner_offset + i])) {
					display("%c ", cluster_data[inner_offset + i]);
				} else {
					display(". ");
				}
			}
			display("\n");
		}
		offset += cluster_size;
	}

out:
	free(cluster_data);

	return 0;
}
