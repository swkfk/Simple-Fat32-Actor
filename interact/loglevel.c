#include "logger.h"

int _global_log_level = _L_Info;
unsigned long _log_index = 0;

void set_verbose() {
	_global_log_level = _L_Trace;
}

void set_quiet() {
	_global_log_level = -1;
}

const char *_log_hints[] = {"Error", "Warn", "Info", "Trace"};
const int _log_colors[] = {31, 33, 35, 36};
