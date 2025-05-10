#include "../job/job.h"

const char *error_reasons[E_MAXN] = {
    [E_Okay] = "Exit Normally",
    [E_NoImgLoaded] = "An img file shall be loaded first",
    [E_InvalidParam] = "Arguments error or invalid",
    [E_ImgFileNotFound] = "Cannot find or open the image file",
    [E_FileReadError] = "Cannot read the image contents, or the image ended too early",
};

const char *get_error_reason(int errorn) {
	if (errorn >= 0 && errorn < E_MAXN) {
		return error_reasons[errorn];
	}
	return "Error number exceed!";
}
