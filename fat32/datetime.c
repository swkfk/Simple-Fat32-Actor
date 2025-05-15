#include "datetime.h"
#include <stdio.h>

struct Fat32_Datetime parse_datetime(uint16_t date, uint16_t time, uint8_t time_ms_10) {
	uint16_t millisecond = 10 * ((time_ms_10 >> 1) & 0x7F);
	return (struct Fat32_Datetime){
	    .year = 1980 + ((date >> 9) & 0x7F),
	    .month = (date >> 5) & 0x0F,
	    .day = date & 0x1F,
	    .hour = (time >> 11) & 0x1F,
	    .minute = (time >> 5) & 0x3F,
	    .second = ((time << 1) & 0x3F) + millisecond / 1000,
	    .millisecond = millisecond % 1000,
	};
}

void datetime_string(char *dest, const struct Fat32_Datetime *dt) {
	sprintf(dest, "%4d-%02d-%02d %02d:%02d:%02d.%03d", dt->year, dt->month, dt->day, dt->hour,
		dt->minute, dt->second, dt->millisecond);
}
