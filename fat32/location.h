#ifndef _FAT32_LOCATION_H_
#define _FAT32_LOCATION_H_

#include "img.h"

int loc_fat_start_sector(struct Fat32_Image *img, int fatno);
int loc_fat_entry_bytes(struct Fat32_Image *img, int cluster_no);
int loc_fat_nth_entry_bytes(struct Fat32_Image *img, int cluster_no, int fat_no);
int loc_data_start_sector(struct Fat32_Image *img);
int loc_data_sector_by_cluster(struct Fat32_Image *img, int cluster_no);
int loc_data_bytes_by_cluster(struct Fat32_Image *img, int cluster_no);

#endif
