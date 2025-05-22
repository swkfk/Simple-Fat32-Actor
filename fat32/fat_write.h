#ifndef _FAT32_FAT_WRITE_H_
#define _FAT32_FAT_WRITE_H_

#include <stddef.h>

#include "common.h"

void release_cluster_chain(fat_entry_t start_cluster);
void truncate_cluster_chain(fat_entry_t start_cluster, size_t remained_count);
int allocate_cluster_chain(fat_entry_t start_cluster, size_t addition_count);

#endif
