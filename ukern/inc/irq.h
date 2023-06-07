#pragma once

typedef enum sgi_mode {
  SGI_TO_LIST = 0,
  SGI_TO_OTHERS,
  SGI_TO_SELF,
} sgi_mode_t;


/* Interrupt Control */
#define pstate_irq_enable()    asm("msr	daifclr, #2" : : : "memory")
#define pstate_irq_disable()   asm("msr	daifset, #2" : : : "memory")
#define pstate_fiq_enable()    asm("msr	daifclr, #1" : : : "memory")
#define pstate_fiq_disable()   asm("msr	daifset, #1" : : : "memory")
#define pstate_async_enable()  asm("msr	daifclr, #4" : : : "memory")
#define pstate_async_disable() asm("msr	daifset, #4" : : : "memory")
#define pstate_debug_enable()  asm("msr	daifclr, #8" : : : "memory")
#define pstate_debug_disable() asm("msr	daifset, #8" : : : "memory")