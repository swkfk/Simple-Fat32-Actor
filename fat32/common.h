#ifndef _FAT32_COMMON_H_
#define _FAT32_COMMON_H_

#include <stdint.h>

#define Field(bits, name) uint##bits##_t name
#define ArrayField(bytes, name) uint8_t name[bytes];
#define ReservedField(bits, name) uint##bits##_t _##name
#define ReservedArrayField(bytes, name) uint8_t _##name[bytes];

#endif
