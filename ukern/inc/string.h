#pragma once
#include <types.h>

void memset(void *base, char ch, int size);
void *memcpy(void *dest, const void *src, size_t n);
int memcmp(const void *s1, const void *s2, size_t n);
void *memmove(void *dest, const void *src, size_t n);
void *memchr(const void *s, int c, size_t n);
int strcmp(const char *s1, const char *s2);
int strncmp(const char *s1, const char *s2, size_t n);
char *strcpy(char *des, const char *src);
char *strncpy(char *des, const char *src, int len);
size_t strlen(const char *s);
char *strchr(const char *src, char ch);
char *strstr(const char *haystack, const char *needle);
long  strtol(const char *s, char **endptr, int base);
unsigned long strtoul(const char * nptr, char ** endptr, int base);
void bzero(void *s, size_t n);