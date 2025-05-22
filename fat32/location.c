#include "../interact/logger.h"
#include "img.h"

int loc_fat_start_sector(struct Fat32_Image *img, int fatno) {
	if (fatno < 0 || fatno >= img->header->NumberOfFAT) {
		Lerror("Invalid FAT No. %d", fatno);
		return -1;
	}
	return img->header->ReservedSector + fatno * img->header->SectorsPerFAT;
}

int loc_fat_entry_bytes(struct Fat32_Image *img, int cluster_no) {
	return loc_fat_start_sector(img, 0) * img->header->BytesPerSector +
	       cluster_no * sizeof(fat_entry_t);
}

int loc_data_start_sector(struct Fat32_Image *img) {
	return img->header->ReservedSector + img->header->NumberOfFAT * img->header->SectorsPerFAT;
}

int loc_data_sector_by_cluster(struct Fat32_Image *img, int cluster_no) {
	return loc_data_start_sector(img) + (cluster_no - 2) * img->header->SectorsPerCluster;
}

int loc_data_bytes_by_cluster(struct Fat32_Image *img, int cluster_no) {
	return loc_data_sector_by_cluster(img, cluster_no) * img->header->BytesPerSector;
}
