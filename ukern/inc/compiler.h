#pragma once

#define __cache_line_size__ (64)
#define __section(S)        __attribute__((__section__(#S)))
#define __used              __attribute__((__used__))
#define __unused            __attribute__((__unused__))
#define __align(x)          __attribute__((__aligned__(x)))
#define __cache_line_align  __align(__cache_line_size__)
#define __packed            __attribute__((__packed__))
#define __noreturn          __attribute__((noreturn))
#define __notrace             __attribute__((no_instrument_function))
#define likely(x)           __builtin_expect(!!(x), 1)
#define unlikely(x)         __builtin_expect(!!(x), 0)
#define barrier()           __asm__ __volatile__("" ::: "memory")
#define unused(__arg__)     (void)(__arg__)

#ifndef __always_inline // Compiler might define it
#define __always_inline	    inline __attribute__((always_inline))
#endif

/* 仅表明这是一个位于用户地址空间的数据 */
#define __user