#pragma once

#define va_start(v, s)    __builtin_va_start(v, s)
#define va_end(v)         __builtin_va_end(v)
#define va_arg(v, t)      __builtin_va_arg(v, t)
#define va_copy(d,s)      __builtin_va_copy(d,s)

typedef __builtin_va_list va_list;