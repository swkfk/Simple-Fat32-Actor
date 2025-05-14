#include <stdint.h>

#include "../interact/logger.h"
#include "../job/job.h"
#include "../utils/macros.h"
#include "../utils/utils.h"
#include "fat_reader.h"
#include "img.h"
#include "location.h"

#define CACHE_FAT_NUMBER 128

struct {
	bool present;
	fat_entry_t start;
	fat_entry_t cached_fat_entry[CACHE_FAT_NUMBER];
} _fat;

static int fat_ensure_present(fat_entry_t cluster) {
	fat_entry_t begin_cluster = ROUND_DOWN(cluster, CACHE_FAT_NUMBER);
	if (_fat.present && _fat.start == begin_cluster) {
		return 0;
	}

	_fat.present = false;

	int position = loc_fat_entry_bytes(&img, begin_cluster);

	Ltrace("Read the fat content for the %u-th cluster, offsets: 0x%x", begin_cluster,
	       position);

	int r = read_file(img.fp, &_fat.cached_fat_entry, position,
			  sizeof(fat_entry_t) * CACHE_FAT_NUMBER);

	if (r) {
		return E_FileReadError;
	}

	_fat.present = true;
	_fat.start = begin_cluster;

	return 0;
}

static fat_entry_t read_fat_content(fat_entry_t cluster) {
	int r = fat_ensure_present(cluster);
	if (r) {
		Lerror("Cannot read the fat entry, returned %d (%s)", r, get_error_reason(r));
		return FAT_ENTRY_BAD;
	}
	return _fat.cached_fat_entry[cluster - ROUND_DOWN(cluster, CACHE_FAT_NUMBER)];
}

bool fat_is_valid_cluster(fat_entry_t this_cluster) {
	fat_entry_t this_content = read_fat_content(this_cluster);
	return IN_RANGE(this_content, FAT_ENTRY_VALID) ||
	       IN_RANGE(this_content, FAT_ENTRY_TERMINATE);
}

bool fat_has_next_cluster(fat_entry_t this_cluster) {
	fat_entry_t this_content = read_fat_content(this_cluster);
	return !(this_content == FAT_ENTRY_BAD || IN_RANGE(this_content, FAT_ENTRY_TERMINATE) ||
		 this_content <= 1);
}

fat_entry_t fat_next_cluster(fat_entry_t this_cluster) {
	if (fat_has_next_cluster(this_cluster)) {
		return read_fat_content(this_cluster);
	} else {
		return FAT_ENTRY_BAD;
	}
}
