#include "img.h"
#include "../utils/utils.h"

struct Fat32_Image open_fat32_img(const char *filename) {
	FILE *fp = fopen(filename, "rb+");
	if (fp == NULL) {
		return (struct Fat32_Image){.fp = NULL};
	}
	struct Fat32_Header *header = checked_malloc(sizeof(struct Fat32_Header), 1);
	struct Fat32_FsInfo *fsinfo = checked_malloc(sizeof(struct Fat32_FsInfo), 1);
	return (struct Fat32_Image){.fp = fp, .header = header, .fsinfo = fsinfo};
}

void close_fat32_img(struct Fat32_Image *img) {
	fclose(img->fp);
	free(img->header);
	free(img->fsinfo);
	img->fp = NULL;
	img->header = NULL;
	img->fsinfo = NULL;
}
