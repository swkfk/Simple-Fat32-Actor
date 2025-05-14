#include <stdint.h>
#include <stdlib.h>

void strip_trailing(char *str, const char ch, size_t length) {
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
