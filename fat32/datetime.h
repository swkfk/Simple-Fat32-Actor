#ifndef _FAT32_DATETIME_H_
#define _FAT32_DATETIME_H_

#include <stdint.h>

struct Fat32_Datetime {
	int year;
	uint8_t month, day;
	uint8_t hour, minute, second;
	uint8_t millisecond;
};

struct Fat32_Datetime parse_datetime(uint16_t date, uint16_t time, uint8_t time_ms_10);
void datetime_string(char *dest, const struct Fat32_Datetime *dt);

#endif
