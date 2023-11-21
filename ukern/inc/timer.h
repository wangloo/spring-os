#pragma once
#include <types.h>

/* #include <list.h> */
#define SECONDS(s)     		((uint64_t)((s)  * 1000000000ULL))
#define MILLISECS(ms)  		((uint64_t)((ms) * 1000000ULL))
#define MICROSECS(us)  		((uint64_t)((us) * 1000ULL))

typedef void (*timer_func_t)(unsigned long);

struct timer {
  /* int cpu; */
  /* int stop; */
  uint64_t expires;
  uint64_t timeout;
  unsigned long data;
  timer_func_t hook;
  /* struct list_head entry; */
  /* struct raw_timer *raw_timer; */
};


void
timer_setup(u64 ns);
void 
timer_start(void);
void 
timer_stop(void);
void 
init_timer(void);