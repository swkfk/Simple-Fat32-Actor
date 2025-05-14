#ifndef _FAT32_COMMON_H_
#define _FAT32_COMMON_H_

#include <stdint.h>

#define Field(bits, name) uint##bits##_t name
#define ArrayField(bytes, name) uint8_t name[bytes];
#define ReservedField(bits, name) uint##bits##_t _##name
#define ReservedArrayField(bytes, name) uint8_t _##name[bytes];

#define FAT_ENTRY_VALID_LOW 0x00000002
#define FAT_ENTRY_VALID_HIGH (0x0FFFFFEF + 1)
#define FAT_ENTRY_TERMINATE_LOW 0x0FFFFFF8
#define FAT_ENTRY_TERMINATE_HIGH 0x0FFFFFFF + 1
#define FAT_ENTRY_BAD 0x0FFFFFF7

typedef uint32_t fat_entry_t;

// filename length is 'sizeof(long_name_entry_t) * MAX_FILENAME_ENTRY_COUNT'
#define MAX_FILENAME_ENTRY_COUNT 16

#endif
