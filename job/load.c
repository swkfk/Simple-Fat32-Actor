#include "../fat32/img.h"
#include "../utils/utils.h"
#include "job.h"

DEFINE_JOB(load) {
	int r;

	if (argc != 2) {
		return E_InvalidParam;
	}

	// Open the image file and alloc the in-memory structs
	struct Fat32_Image image = open_fat32_img(argv[1]);
	if (image.fp == NULL) {
		return E_ImgFileNotFound;
	}

	// Close the now img if necessary
	if (img.fp != NULL) {
		close_fat32_img(&img);
		Linfo("Closed the old image file");
	}
	img = image;

	// Load the basic information
	r = read_file(img.fp, img.header, 0, sizeof(struct Fat32_Header));
	if (r) {
		return E_FileReadError;
	}

	// The header struct
	size_t fsinfo_start = img.header->FSINFO_SectorNumber * img.header->BytesPerSector;
	Linfo("FsInfo started @%08zx bytes", fsinfo_start);

	if (img.header->BytesPerSector != 512) {
		Lwarn("Bytes-Per-Sector(%hx) != 512 bytes, I may make some mistakes",
		      img.header->BytesPerSector);
	}

	// The fsinfo struct
	r = read_file(img.fp, img.fsinfo, fsinfo_start, sizeof(struct Fat32_FsInfo));
	if (r) {
		return E_FileReadError;
	}

	Linfo("Empty Cluster Count: %d, Next Empty Cluster Number: %d",
	      img.fsinfo->EmptyClusterCount, img.fsinfo->NextEmptyClusterNumber);

	return 0;
}

DEFINE_JOB(unload) {
	if (img.fp != NULL) {
		close_fat32_img(&img);
		Linfo("Closed the old image file");
	} else {
		Lwarn("No valid image file opened");
	}
	return 0;
}
