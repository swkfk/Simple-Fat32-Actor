#include <stdint.h>
#include <stdlib.h>
#include <string.h>

void strip_trailing(char *str, const char ch, size_t length) {
	str[length] = '\0';
	for (size_t i = length - 1; i >= 0 && str[i] == ch; i--) {
		str[i] = '\0';
	}
}

void unicode2char(uint8_t *dest, uint16_t *src, size_t length) {
	for (size_t i = 0; i < length; i++) {
		dest[i] = src[i] & 0xFF;
	}
	dest[length] = '\0';
}

void char2unicode(uint16_t *dest, uint8_t *src, size_t length) {
	for (size_t i = 0; i < length; i++) {
		dest[i] = ((uint16_t)src[i]) & 0xFF;
	}
}

void concat_short_name(char *dest, const char *basename, const char *extname) {
	strcpy(dest, basename);
	if (extname[0] != '\0') {
		strcat(dest, ".");
		strcat(dest, extname);
	}
}
