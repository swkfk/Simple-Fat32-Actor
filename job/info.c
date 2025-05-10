#include <string.h>

#include "../fat32/location.h"
#include "job.h"

#define print_banner(l)                                                                            \
	do {                                                                                       \
		display("\n--------< %s >--------\n", l);                                          \
	} while (0)
#define print_kv(k, fmt, v)                                                                        \
	do {                                                                                       \
		display("\033[36m%30s\033[0m => " fmt "\n", k, v);                                 \
	} while (0)

DEFINE_JOB(info) {
	acquire_image_loaded();

	print_banner("System Information");
	char oem[9] = {0};
	memcpy(oem, img.header->OEM, 8);
	print_kv("OEM String", "%s", oem);

	print_banner("Sector Definition");
	print_kv("Bytes Per Sector", "%d", img.header->BytesPerSector);
	print_kv("Reserved Sector Count", "%d", img.header->ReservedSector);
	print_kv("Sector Count", "%d", img.header->TotalSector);

	print_banner("Cluster Definition");
	print_kv("Sectors Per Cluster", "%d", img.header->SectorsPerCluster);
	print_kv("Root Cluster Number", "%d", img.header->RootClusterNumber);

	print_banner("FAT Definition");
	print_kv("FAT Count", "%d", img.header->NumberOfFAT);
	print_kv("Sector Per FAT", "%d", img.header->SectorsPerFAT);
	for (int i = 0; i < img.header->NumberOfFAT; i++) {
		print_kv("FAT Start At Sector", "%d", loc_fat_start_sector(&img, i));
	}

	print_banner("Data Sector");
	print_kv("Data Sector Start At", "%d", loc_data_start_sector(&img));
	size_t data_sector_count = img.header->TotalSector - loc_data_start_sector(&img);
	print_kv("Data Sector Count", "%zd", data_sector_count);
	print_kv("Data Cluster Count", "%zd", data_sector_count / img.header->SectorsPerCluster);

	print_banner("FsInfo Data");
	print_kv("FsInfo Start At Sector", "%d", img.header->FSINFO_SectorNumber);
	print_kv("Empty Cluster Count", "%u", img.fsinfo->EmptyClusterCount);
	print_kv("Next Empty Cluster Number", "%u", img.fsinfo->NextEmptyClusterNumber);

	return 0;
}
