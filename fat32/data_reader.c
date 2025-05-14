#include <stdint.h>

#include "../interact/logger.h"
#include "../utils/utils.h"
#include "location.h"

void read_cluster_content(uint32_t data_cluster_number, void *data) {
	int start_sector = loc_data_sector_by_cluster(&img, data_cluster_number);
	int r = read_file(img.fp, data, start_sector * img.header->BytesPerSector,
			  img.header->BytesPerSector * img.header->SectorsPerCluster);
	if (r != 0) {
		Lerror("Cannot read the data cluster: %d", data_cluster_number);
	}
}
