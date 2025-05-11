#ifndef _FAT32_DIRECTORY_ENTRY_H_
#define _FAT32_DIRECTORY_ENTRY_H_

#include "common.h"

#pragma pack(1)
struct Fat32_ShortDirectoryEntry {
	ArrayField(8, BaseName);
	ArrayField(3, ExtName);
	Field(8, Attribute);
	ReservedField(8, Reserved_1);
	Field(8, CreateTime_10ms);
	Field(16, CreateTime);
	Field(16, CreateDate);
	Field(16, AccessDate);
	Field(16, StartCluster_hi);
	Field(16, ModifyTime);
	Field(16, ModifyDate);
	Field(16, StartCluster_lo);
	Field(32, FileLength);
};
#pragma pack()

#pragma pack(1)
struct Fat32_LongDirectoryEntry {
	struct {
		uint8_t Reserved_1 : 1;
		uint8_t IsLastEntry : 1;
		uint8_t Reserved_2 : 1;
		uint8_t Sequence : 5;
	};
	ArrayField(10, Unicode_1);
	Field(8, Attribute);
	ReservedField(8, Reserved_3);
	Field(8, Checksum);
	ArrayField(12, Unicode_2);
	Field(16, StartCluster);
	ArrayField(4, Unicode_3);
};
#pragma pack()

#endif
