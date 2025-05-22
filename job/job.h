#ifndef _JOB_JOB_H_
#define _JOB_JOB_H_

#define JOB_FN_NAME(name) _job_##name
#define DEFINE_JOB(name) int JOB_FN_NAME(name)(int argc, char **argv)
#define CALL_JOB(name, argc, argv) JOB_FN_NAME(name)(argc, argv)

// Exported Functions
// dummy.c
DEFINE_JOB(dummy);
// help.c
DEFINE_JOB(help);
// info.c
DEFINE_JOB(info);
// load.c
DEFINE_JOB(load);
DEFINE_JOB(unload);
// ls.c
DEFINE_JOB(ls);
// quit.c
DEFINE_JOB(exit);
// rm.c
DEFINE_JOB(rm);
DEFINE_JOB(truncate);
// debug/fat.c
DEFINE_JOB(read_data);
DEFINE_JOB(read_directory);

// Common includes
#include "../fat32/img.h"
#include "../interact/logger.h"
#include "../utils/utils.h"

// Error Codes
enum {
	E_Okay,
	E_NoImgLoaded,
	E_InvalidParam,
	E_ImgFileNotFound,
	E_FileReadError,
	E_FileOrDirectoryNotFound,
	E_ThisIsAFile,
	E_FileOrDirectoryExists,
	E_NoSpace,
	E_PermissionDenied,
	E_DirectoryNotEmpty,
	E_MAXN,
};

#define acquire_image_loaded()                                                                     \
	do {                                                                                       \
		if (img.fp == NULL) {                                                              \
			return E_NoImgLoaded;                                                      \
		}                                                                                  \
	} while (0)

#endif
