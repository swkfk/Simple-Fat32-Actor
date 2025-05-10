#ifndef _FAT32_FSINFO_H_
#define _FAT32_FSINFO_H_

#include "common.h"

#pragma pack(1)
struct Fat32_FsInfo {
	ReservedField(32, ExtendedTag);
	ReservedArrayField(480, Reserved_1);
	ReservedField(32, FileSystemSignature);
	Field(32, EmptyClusterCount);
	Field(32, NextEmptyClusterNumber);
	ReservedArrayField(14, Reserved_2);
	ReservedField(16, EndMark);
};
#pragma pack()

#endif
