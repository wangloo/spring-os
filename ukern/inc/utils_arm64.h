#pragma once

/* Reverse the bit order. For example, rbit(0b1010) = 0b0101 */
static inline unsigned long rbit(unsigned long val)
{
    unsigned long ret;
    asm volatile("rbit %0, %1\n\t"
                 :"=&r"(ret)
                 :"r"(val));
    return ret;
}

/* Calculate the number of leading 0. For example, clz(0b10) = 62 */
static inline unsigned long clz(unsigned long val)
{
    unsigned long ret;
    asm volatile("clz %0, %1\n\t"
                 :"=&r"(ret)
                 :"r"(val));
    return ret;
}

/* Calculate the number of tailing 0. Arm instruction set doesn't provide
   a direct instruction to do it. For example, ctz(0b10) = 1 */
static inline unsigned long ctz(unsigned long val)
{
    return clz(rbit(val));
}



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