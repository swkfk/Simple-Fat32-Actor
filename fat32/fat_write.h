#ifndef _FAT32_FAT_WRITE_H_
#define _FAT32_FAT_WRITE_H_

#include "common.h"

void release_cluster_chain(fat_entry_t start_cluster);

#endif
