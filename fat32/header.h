#ifndef _DAT32_HEADER_H_
#define _DAT32_HEADER_H_

#include "common.h"

#pragma pack(1)
struct Fat32_Header {
	struct {
		ReservedArrayField(3, JmpInstruction);
	};
	struct {
		ArrayField(8, OEM);
	};
	struct {
		Field(16, BytesPerSector);
		Field(8, SectorsPerCluster);
		Field(16, ReservedSector);
		Field(8, NumberOfFAT);
		ReservedField(16, RootEntries);
		ReservedField(16, SmallSector);
		ReservedField(8, MediaDescriptor);
		ReservedField(16, SectorsPerFAT);
		ReservedField(16, SectorsPerTrack);
		ReservedField(16, NumberOfHead);
		ReservedField(32, HiddenSector);
		Field(32, LargeSector);
		Field(32, SectorsPerFAT);
		Field(16, ExtendedFlag);
		ReservedField(16, FileSystemVersion);
		Field(32, RootClusterNumber);
		Field(16, FSINFO_SectorNumber);
		ReservedField(16, BackupBootSector);
		ReservedArrayField(12, Reserved_1);
	}; // BPB
	struct {
		ReservedField(8, PhysicalDriveNumber);
		ReservedField(8, Reserved_2);
		ReservedField(8, ExtendedBootSignature);
		ReservedField(32, VolumeSerialNumber);
		ReservedArrayField(11, VolumeLabel);
		ReservedArrayField(8, SystemID);
	}; // Extend BPB
	struct {
		ReservedArrayField(420, BootCode);
		ReservedField(16, EndMark);
	}; // Boot Program
};
#pragma pack()

#endif
