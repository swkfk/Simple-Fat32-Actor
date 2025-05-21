#include "../job/job.h"

const char *error_reasons[E_MAXN] = {
    [E_Okay] = "Exit Normally",
    [E_NoImgLoaded] = "An img file shall be loaded first",
    [E_InvalidParam] = "Arguments error or invalid",
    [E_ImgFileNotFound] = "Cannot find or open the image file",
    [E_FileReadError] = "Cannot read the image contents, or the image ended too early",
    [E_FileOrDirectoryNotFound] = "File or directory not found in the image file",
    [E_ThisIsAFile] = "This is a file instead of a directory",
    [E_FileOrDirectoryExists] = "File or directory is already exist",
    [E_NoSpace] = "There is no enough space for the image file",
};

const char *get_error_reason(int errorn) {
	if (errorn >= 0 && errorn < E_MAXN) {
		return error_reasons[errorn];
	}
	return "Error number exceed!";
}
