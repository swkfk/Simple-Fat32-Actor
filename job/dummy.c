#include "job.h"

DEFINE_JOB(dummy) {
	display("Dummy Job Called! Args Count=%d\n", argc);
	for (int i = 0; i < argc; i++) {
		display("Args #%d: %s\n", i, argv[i]);
	}
	return 0;
}
