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
