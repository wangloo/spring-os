#pragma once

#ifdef __ASSEMBLY__
#define _AC(X,Y)        X
#define _AT(T,X)        X
#else
#define __AC(X,Y)       (X##Y)
#define _AC(X,Y)        __AC(X,Y)
#define _AT(T,X)        ((T)(X))
#endif

#define _UL(x)          (_AC(x, UL))
#define _ULL(x)         (_AC(x, ULL))

#define UL(x)           (_UL(x))
#define ULL(x)          (_ULL(x))

#define BIT(x)          (1ul << (x))

#define div_round_up(n, d)  (((n) + (d)-1) / (d))
#define align_up(addr, x)   (((addr) + ((x)-1)) & (~((typeof(addr))(x)-1)))
#define align_down(addr, x) ((addr) & (~((typeof(addr))(x)-1)))