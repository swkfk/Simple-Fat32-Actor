#include <ctype.h>
#include <string.h>

#include "../../fat32/data_reader.h"
#include "../../fat32/directory_entry.h"
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

static void copy_into_buffer(long_name_entry_t *buffer, struct Fat32_LongDirectoryEntry *dir) {
	void *dest = buffer;
	memcpy(dest, dir->Unicode_1, sizeof(dir->Unicode_1));
	dest += sizeof(dir->Unicode_1);
	memcpy(dest, dir->Unicode_2, sizeof(dir->Unicode_2));
	dest += sizeof(dir->Unicode_2);
	memcpy(dest, dir->Unicode_3, sizeof(dir->Unicode_3));
}

static void copy_into_string(char *dest, long_name_entry_t *src, size_t count) {
	size_t len = 0;
	while (count--) {
		unicode2char((uint8_t *)dest + len, (uint16_t *)(src + count),
			     sizeof(long_name_entry_t) / 2);
		len += sizeof(long_name_entry_t) / 2;
	}
}

DEFINE_JOB(read_directory) {
	if (argc != 2 && argc != 3) {
		return E_InvalidParam;
	}

	int start_cluster = atoi(argv[1]);
	const size_t cluster_size = img.header->BytesPerSector * img.header->SectorsPerCluster;
	void *cluster_data =
	    checked_malloc(img.header->BytesPerSector, img.header->SectorsPerCluster);

	long_name_entry_t *filename_buffer =
	    checked_malloc(sizeof(long_name_entry_t), MAX_FILENAME_ENTRY_COUNT);
	char *filename =
	    checked_malloc(1, sizeof(long_name_entry_t) * MAX_FILENAME_ENTRY_COUNT / 2);

	size_t pos = 0;
	bool warned_too_long = false;

	FOR_FAT_ENTRY_CHAIN (cluster, start_cluster) {
		read_cluster_content(cluster, cluster_data);

		FOR_DIRECTORY_ENTRY(dir, cluster_data, cluster_size) {

			if (((unsigned char *) dir)[0] == 0xE5) {
				continue;
			}

			struct Fat32_ShortDirectoryEntry *sdir = dir;
			struct Fat32_LongDirectoryEntry *ldir = dir;

			uint8_t attr = sdir->Attribute;
			if (attr == DIR_ATTR_LONG_NAME) {
				if (pos == MAX_FILENAME_ENTRY_COUNT) {
					if (!warned_too_long) {
						copy_into_string(filename, filename_buffer, pos);
						Lwarn("filename too long: %s", filename);
						warned_too_long = true;
					}
				} else {
					copy_into_buffer(filename_buffer + pos, ldir);
					pos++;
				}
			} else {
				char base_name[9], ext_name[4];
				memcpy(base_name, sdir->BaseName, 8);
				memcpy(ext_name, sdir->ExtName, 3);
				strip_trailing(base_name, ' ', 8);
				strip_trailing(ext_name, ' ', 3);

				// Handle the long file name
				copy_into_string(filename, filename_buffer, pos);
				pos = 0;
				warned_too_long = false;

				// Display informations
				display("--> %8s.%3s <--\n", base_name, ext_name);
				if (((char *)filename_buffer)[0]) {
					print_kv("Long Name", "%s", filename);
				}
				if (!(sdir->Attribute & DIR_ATTR_DIRECTORY)) {
					print_kv("File Size", "%d", sdir->FileLength);
				}
				print_kv(
				    "Start Cluster", "%d",
				    JOIN_NUMBER(32, sdir->StartCluster_hi, sdir->StartCluster_lo));
				print_bool("Is Read Only", sdir->Attribute & DIR_ATTR_READ_ONLY);
				print_bool("Is Hidden", sdir->Attribute & DIR_ATTR_HIDDEN);
				print_bool("Is Directory", sdir->Attribute & DIR_ATTR_DIRECTORY);
			}
		}
	}

	free(cluster_data);

	return 0;
}
