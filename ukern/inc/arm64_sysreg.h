#pragma once
#include <utils.h>

/**
 * TODO: 32bit read and write sysreg must use 64bit
 *       general registers, but will cause compiler WARNING!!
 *       NO IDEA NOW!
 */
#define read_sysreg32(name) ({                        \
    uint32_t _r;                            \
    asm volatile("mrs  %0, "stringify(name) : "=r" (_r));        \
    _r; })

#define write_sysreg32(v, name)                        \
    do {                                \
        uint32_t _r = v;                    \
        asm volatile("msr "stringify(name)", %0" : : "r" (_r));    \
    } while (0)

#define write_sysreg64(v, name)                        \
    do {                                \
        uint64_t _r = v;                    \
        asm volatile("msr "stringify(name)", %0" : : "r" (_r));    \
    } while (0)

#define read_sysreg64(name) ({                        \
    uint64_t _r;                            \
    asm volatile("mrs  %0, "stringify(name) : "=r" (_r));        \
    _r; })

#define read_sysreg(name)     read_sysreg64(name)
#define write_sysreg(v, name) write_sysreg64(v, name)
