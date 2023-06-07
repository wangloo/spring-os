#pragma once

#include <types.h>
#include <barrier.h>

/*
 * Generic IO read/write.  These perform native-endian accesses.
 */
#define __raw_writeb __raw_writeb
static inline void __raw_writeb(uint8_t val, volatile void *addr)
{
	asm volatile("strb %w0, [%1]" : : "r" (val), "r" (addr));
}

#define __raw_writew __raw_writew
static inline void __raw_writew(uint16_t val, volatile void *addr)
{
	asm volatile("strh %w0, [%1]" : : "r" (val), "r" (addr));
}

#define __raw_writel __raw_writel
static inline void __raw_writel(uint32_t val, volatile void *addr)
{
	asm volatile("str %w0, [%1]" : : "r" (val), "r" (addr));
}

#define __raw_writeq __raw_writeq
static inline void __raw_writeq(uint64_t val, volatile void *addr)
{
	asm volatile("str %0, [%1]" : : "r" (val), "r" (addr));
}

#define __raw_readb __raw_readb
static inline uint8_t __raw_readb(const volatile void *addr)
{
	uint8_t val;
	asm volatile("ldrb %w0, [%1]" : "=r" (val) : "r" (addr));
	return val;
}

#define __raw_readw __raw_readw
static inline uint16_t __raw_readw(const volatile void *addr)
{
	uint16_t val;
	asm volatile("ldarh %w0, [%1]" : "=r" (val) : "r" (addr));
	return val;
}

#define __raw_readl __raw_readl
static inline uint32_t __raw_readl(const volatile void *addr)
{
	uint32_t val;
	asm volatile("ldar %w0, [%1]" : "=r" (val) : "r" (addr));
	return val;
}

#define __raw_readq __raw_readq
static inline uint64_t __raw_readq(const volatile void *addr)
{
	uint64_t val;
	asm volatile("ldar %0, [%1]" : "=r" (val) : "r" (addr));
	return val;
}

/*
 * Relaxed I/O memory access primitives. These follow the Device memory
 * ordering rules but do not guarantee any ordering relative to Normal memory
 * accesses.
 */
#define readb_relaxed(c)	({ uint8_t  __v = __raw_readb(c); iormb(); __v; })
#define readw_relaxed(c)	({ uint16_t __v = __raw_readw(c); iormb(); __v; })
#define readl_relaxed(c)	({ uint32_t __v = __raw_readl(c); iormb(); __v; })
#define readq_relaxed(c)	({ uint64_t __v = __raw_readq(c); iormb(); __v; })

#define writeb_relaxed(v,c)	({ iowmb(); (void)__raw_writeb((v), (c)); })
#define writew_relaxed(v,c)	({ iowmb(); (void)__raw_writew((v), (c)); })
#define writel_relaxed(v,c)	({ iowmb(); (void)__raw_writel((v), (c)); })
#define writeq_relaxed(v,c)	({ iowmb(); (void)__raw_writeq((v), (c)); })
                                            
#define ioread8(addr)		readb_relaxed(addr)
#define ioread16(addr)		readw_relaxed(addr)
#define ioread32(addr)		readl_relaxed(addr)
#define ioread64(addr)		readq_relaxed(addr)

#define iowrite8(v, addr)	writeb_relaxed((v), (addr))
#define iowrite16(v, addr)	writew_relaxed((v), (addr))
#define iowrite32(v, addr)	writel_relaxed((v), (addr))
#define iowrite64(v, addr)	writeq_relaxed((v), (addr))

// #define readb(addr)		readb_relaxed(addr)
// #define readw(addr)		readw_relaxed(addr)
// #define readl(addr)		readl_relaxed(addr)
// #define readq(addr)		readq_relaxed(addr)

// #define writeb(v, addr)		writeb_relaxed((v), (addr))
// #define writew(v, addr)		writew_relaxed((v), (addr))
// #define writel(v, addr)		writel_relaxed((v), (addr))
// #define writeq(v, addr)		writeq_relaxed((v), (addr))
