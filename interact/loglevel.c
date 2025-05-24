#include <stdbool.h>

#include "logger.h"

int _global_log_level = _L_Info;
unsigned long _log_index = 0;

void set_verbose() {
	if (_global_log_level == _L_Trace) {
		_global_log_level = _L_Trace_Verbose;
		Lverbose("Enable trace verbose logs");
	} else {
		_global_log_level = _L_Trace;
		Ltrace("Enable trace logs");
	}
}

void set_quiet() {
	_global_log_level = -1;
}

const char *_log_hints[] = {"Error", "Warn", "Info", "Trace", "Trace"};
const int _log_colors[] = {31, 33, 35, 36, 36};
const bool _log_shadows[] = {false, false, false, false, true};
