#pragma once
#include <types.h>

#define TIMER_PRECISION 1000000 // 1000000ns = 1ms

extern uint64_t boot_tick;
extern uint32_t cpu_khz;

void arch_timer_enable(void);
void arch_timer_setalarm(u32 ns);