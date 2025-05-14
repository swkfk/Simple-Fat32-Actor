#ifndef _FAT32_FAT_READER_H_
#define _FAT32_FAT_READER_H_

#include "common.h"

#include <stdbool.h>
#include <stdint.h>

bool fat_is_valid_cluster(fat_entry_t this_cluster);
bool fat_has_next_cluster(fat_entry_t this_cluster);
fat_entry_t fat_next_cluster(fat_entry_t this_cluster);

#define FOR_FAT_ENTRY_CHAIN(var, start_cluster_number)                                             \
	for (fat_entry_t var = start_cluster_number; fat_is_valid_cluster(var);                    \
	     var = fat_next_cluster(var))

#define FOR_DIRECTORY_ENTRY(var, data, data_size)                                                  \
	for (void *var = data; var < ((void *)data + data_size);                                   \
	     var += sizeof(struct Fat32_ShortDirectoryEntry))

#endif
