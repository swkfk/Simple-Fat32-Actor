#include "job.h"

DEFINE_JOB(dummy) {
	Lerror("This is an error log!");
	Lwarn("This is a warning log!");
	Linfo("This is an information log!");
	Ltrace("This is a trace log!");
	Lverbose("This is a verbose log!");

	display("Dummy Job Called! Args Count=%d\n", argc);
	for (int i = 0; i < argc; i++) {
		display("Args #%d: %s\n", i, argv[i]);
	}
	return 0;
}
