#include "../job/job.h"

const char *error_reasons[E_MAXN] = {
    [E_Okay] = "Exit Normally",
    [E_NoImgLoaded] = "An img file shall be loaded first",
};

const char *get_error_reason(int errorn) {
	if (errorn >= 0 && errorn < E_MAXN) {
		return error_reasons[errorn];
	}
	return "Error number exceed!";
}
