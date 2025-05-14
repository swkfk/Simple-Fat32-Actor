#ifndef _FAT32_DATA_READER_H_
#define _FAT32_DATA_READER_H_

#include <stdint.h>

void read_cluster_content(uint32_t data_cluster_number, void *data);

#endif
