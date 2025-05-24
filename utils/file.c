#include <stdio.h>

#include "../fat32/directory_entry.h"
#include "../interact/logger.h"

int read_file(FILE *fp, void *out, size_t start, size_t len) {
	if (fp == NULL || out == NULL) {
		return -1; // Invalid arguments
	}

	if (fseek(fp, start, SEEK_SET) != 0) {
		return -1; // Failed to seek to the start position
	}

	size_t read = fread(out, 1, len, fp);

	Lverbose("Read 0x%04zx bytes start @%08zx, length of 0x%04zx expected", read, start, len);

	if (read != len) {
		if (feof(fp)) {
			return 1; // Reached end of file
		} else if (ferror(fp)) {
			return -1; // Read error
		}
	}

	return 0; // Success
}

int write_file(FILE *fp, void *data, size_t start, size_t len) {
	if (fp == NULL || data == NULL) {
		return -1; // Invalid arguments
	}

	if (fseek(fp, start, SEEK_SET) != 0) {
		return -1; // Failed to seek to the start position
	}

	size_t written = fwrite(data, 1, len, fp);
	fflush(fp);

	Lverbose("Wrote 0x%04zx bytes start @%08zx, length of 0x%04zx expected", written, start,
		 len);

	if (written != len) {
		return -1; // Write error
	}

	return 0; // Success
}

int write_file_directory_entry(FILE *fp, struct DirectoryEntryWithOffset *o) {
	return write_file(fp, &o->entry, o->offset, sizeof(struct Fat32_ShortDirectoryEntry));
}
