#include <ctype.h>
#include <string.h>

#include "../../fat32/data_reader.h"
#include "../../fat32/datetime.h"
#include "../../fat32/directory_entry.h"
#include "../../fat32/directory_walk.h"
#include "../../fat32/fat_reader.h"
#include "../job.h"

#define print_kv(k, fmt, v)                                                                        \
	do {                                                                                       \
		display("\033[36m%15s\033[0m: " fmt "\n", k, v);                                   \
	} while (0)

#define print_bool(k, v) print_kv(k, "%s", (v) ? "\033[32myes\033[0m" : "\033[33mno\033[0m")

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

DefDirWalkCb(display_entry_info) {
	display("--> %8s.%3s <--\n", short_basename, short_extname);
	if (longname[0]) {
		print_kv("Long Name", "%s", longname);
	}
	if (!(dir->Attribute & DIR_ATTR_DIRECTORY)) {
		print_kv("File Size", "%d", dir->FileLength);
	}
	print_kv("Start Cluster", "%d",
		 JOIN_NUMBER(32, dir->StartCluster_hi, dir->StartCluster_lo));
	print_bool("Is Read Only", dir->Attribute & DIR_ATTR_READ_ONLY);
	print_bool("Is Hidden", dir->Attribute & DIR_ATTR_HIDDEN);
	print_bool("Is Directory", dir->Attribute & DIR_ATTR_DIRECTORY);

	char time_string[25];
	struct Fat32_Datetime create_time =
	    parse_datetime(dir->CreateDate, dir->CreateTime, dir->CreateTime_10ms);
	datetime_string(time_string, &create_time);
	print_kv("Create At", "%s", time_string);

	struct Fat32_Datetime modify_time = parse_datetime(dir->ModifyDate, dir->ModifyTime, 0);
	datetime_string(time_string, &modify_time);
	print_kv("Modify At", "%s", time_string);

	return false;
}

DEFINE_JOB(read_directory) {
	if (argc != 2) {
		return E_InvalidParam;
	}

	walk_directory_on_fat(&img, atoi(argv[1]), display_entry_info);

	return 0;
}
