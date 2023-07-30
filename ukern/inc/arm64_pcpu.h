#pragma once

static inline void arch_set_percpu_pcpu(void *pcpu)
{
	asm volatile ("msr tpidr_el1, %0" : : "r" (pcpu));
}

