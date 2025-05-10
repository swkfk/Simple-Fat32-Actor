#ifndef _JOB_JOB_H_
#define _JOB_JOB_H_

#define JOB_FN_NAME(name) _job_##name
#define DEFINE_JOB(name) int JOB_FN_NAME(name)(int argc, char **argv)

// Exported Functions
// dummy.c
DEFINE_JOB(dummy);
// help.c
DEFINE_JOB(help);
// load.c
DEFINE_JOB(load);
DEFINE_JOB(unload);
// quit.c
DEFINE_JOB(exit);
DEFINE_JOB(quit);

// Common includes
#include "../interact/logger.h"

// Error Codes
enum {
	E_Okay,
	E_NoImgLoaded,
	E_InvalidParam,
	E_ImgFileNotFound,
	E_FileReadError,
	E_MAXN,
};

#endif
