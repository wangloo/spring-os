#pragma once

#define __isb()    asm volatile("isb" : : : "memory")
#define __dmb(opt) asm volatile("dmb " #opt : : : "memory")
#define __dsb(opt) asm volatile("dsb " #opt : : : "memory")

#define isb()      __isb();

#define mb()       __dsb(sy)
#define dsb()      __dsb(sy)
#define dmb()      __dmb(sy)
#define rmb()      __dmb(ld)
#define wmb()      __dmb(st)

#define dma_rmb()  __dmb(oshld)
#define dma_wmb()  __dmb(oshst)

#define iormb()    dma_rmb()
#define iowmb()    dma_wmb()

#define smp_mb()   __dmb(ish)
#define smp_rmb()  __dmb(ishld)
#define smp_wmb()  __dmb(ishst)
