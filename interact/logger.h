#ifndef _INTERACT_LOGGER_H_
#define _INTERACT_LOGGER_H_

#include <stdio.h>
#include <time.h>

#define display printf

// Logging Level
enum {
	_L_Error,
	_L_Warning,
	_L_Info,
	_L_Trace,
};

extern int _global_log_level;
extern const char *_log_hints[];
extern const int _log_colors[];

void set_verbose();
void set_quiet();

// Logger time support
time_t init_start_time();
time_t get_time_delta_us();

extern unsigned long _log_index;

// Logger Core
#define __logger(level, format, ...)                                                               \
	do {                                                                                       \
		if (level <= _global_log_level) {                                                  \
			int pre_width;                                                             \
			printf("\033[%dm[%9.2lf](%03ld) %5s <%s:%d %s>%n", _log_colors[level],     \
			       get_time_delta_us() / 1000., ++_log_index, _log_hints[level],       \
			       __FILE__, __LINE__, __FUNCTION__, &pre_width);                      \
			for (; pre_width < 75; pre_width++) {                                      \
				printf(" ");                                                       \
			}                                                                          \
			printf(format "\033[0m\n", ##__VA_ARGS__);                                 \
		}                                                                                  \
	} while (0)
#define __display(level, format, ...)                                                              \
	do {                                                                                       \
		if (level <= _global_log_level) {                                                  \
			printf("\033[2;%dm" format "\033[0m", _log_colors[level], ##__VA_ARGS__);  \
		}                                                                                  \
	} while (0)

// Logger useful macros
#define Ltrace(format, ...) __logger(_L_Trace, format, ##__VA_ARGS__)
#define Linfo(format, ...) __logger(_L_Info, format, ##__VA_ARGS__)
#define Lwarn(format, ...) __logger(_L_Warning, format, ##__VA_ARGS__)
#define Lerror(format, ...) __logger(_L_Error, format, ##__VA_ARGS__)

// Display with levels
#define Dtrace(format, ...) __display(_L_Trace, format, ##__VA_ARGS__)
#define Dinfo(format, ...) __display(_L_Info, format, ##__VA_ARGS__)
#define Dwarn(format, ...) __display(_L_Warning, format, ##__VA_ARGS__)
#define Derror(format, ...) __display(_L_Error, format, ##__VA_ARGS__)

#endif
