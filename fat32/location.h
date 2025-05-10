#ifndef _FAT32_LOCATION_H_
#define _FAT32_LOCATION_H_

#include "img.h"

int loc_fat_start_sector(struct Fat32_Image *img, int fatno);
int loc_data_start_sector(struct Fat32_Image *img);

#endif
