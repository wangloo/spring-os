#pragma once

#define DIV_ROUND_UP(n, d) (((n) + (d) - 1) / (d))
#define BITS_TO_LONGS(nr) DIV_ROUND_UP(nr, BITS_PER_BYTE * sizeof(long))
#define BITS_PER_BYTE	(8)
#define BITS_PER_LONG 64
#define BIT_ULL(nr)		(1ULL << (nr))
#define BIT_MASK(nr)		(1UL << ((nr) % BITS_PER_LONG))
#define BIT_WORD(nr)		((nr) / BITS_PER_LONG)
#define BIT_ULL_MASK(nr)	(1ULL << ((nr) % BITS_PER_LONG_LONG))
#define BIT_ULL_WORD(nr)	((nr) / BITS_PER_LONG_LONG)


#define __round_mask(x, y) 	((__typeof__(x))((y)-1))
#define round_up(x, y) 		((((x)-1) | __round_mask(x, y))+1)
#define round_down(x, y) 	((x) & ~__round_mask(x, y))