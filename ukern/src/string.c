#include <string.h>

char *strncpy(char *des, const char *src, int len)
{
	char *tmp = des;
	int i;

	if (des == NULL || src == NULL)
		return NULL;

	for (i = 0; i < len; i++) {
		des[i] = src[i];
	}

	return tmp;
}
