#ifndef _FAT32_IMG_H_
#define _FAT32_IMG_H_

#include <stdint.h>
#include <stdio.h>

#include "fsinfo.h"
#include "header.h"

struct Fat32_Image {
	FILE *fp;
	struct Fat32_Header *header;
	struct Fat32_FsInfo *fsinfo;
};

struct Fat32_Image open_fat32_img(const char *filename);
void close_fat32_img(struct Fat32_Image *img);

// global.h
extern struct Fat32_Image img;

#endif
