#pragma once
#include <types.h>

/* #include <list.h> */

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


void timer_setup_and_start(struct timer *timer, u64 ns);
void timer_setup(struct timer *timer, u64 ns);
void timer_start(struct timer *timer);
void timer_init(void);