#pragma once

#include <types.h>
#include <valist.h>


int puts(char *buf);

int vprintf(const char *fmt, va_list ap);
int vsnprintf(char *buf, size_t size, const char *fmt, va_list ap);
int snprintf(char *buf, size_t size, const char *fmt, ...);
int sprintf(char *buf, const char *fmt, ...);
int printf(char *fmt, ...);
void print_space(int cnt);