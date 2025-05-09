#ifndef _INTERACT_LOGGER_H_
#define _INTERACT_LOGGER_H_

#include <stdio.h>

#define display printf

// Logging Level
enum {
	_L_Error,
	_L_Warning,
	_L_Info,
	_L_Trace,
};

extern int _global_log_level;
extern const char _log_hints[];
extern const int _log_colors[];

void change_log_level(int new_level);

// Logger Core
#define __logger(level, format, ...)                                                               \
	do {                                                                                       \
		if (level <= _global_log_level) {                                                  \
			printf("\033[%dm[%c] " format "\033[0m\n", _log_colors[level],             \
			       _log_hints[level], ##__VA_ARGS__);                                  \
		}                                                                                  \
	} while (0)

// Logger useful macros
#define Ltrace(format, ...) __logger(_L_Trace, format, ##__VA_ARGS__)
#define Linfo(format, ...) __logger(_L_Info, format, ##__VA_ARGS__)
#define Lwarn(format, ...) __logger(_L_Warning, format, ##__VA_ARGS__)
#define Lerror(format, ...) __logger(_L_Error, format, ##__VA_ARGS__)

#endif
