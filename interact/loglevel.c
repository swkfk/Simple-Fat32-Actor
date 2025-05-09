#include "logger.h"

int _global_log_level = _L_Info;

void change_log_level(int new_level) {
	_global_log_level = new_level;
}

const char _log_hints[] = {'E', 'W', 'I', 'T'};
const int _log_colors[] = {31, 33, 35, 36};
