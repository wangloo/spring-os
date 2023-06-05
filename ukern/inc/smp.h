#pragma once


static inline int cpu_id(void)
{
	int cpu;
#ifdef CONFIG_SMP_OK
	uint64_t v;
	__asm__ volatile (
		"mrs	%0, TPIDR_EL1\n"
		"ldrh	%w1, [%0, #0]\n"
		: "=r" (v), "=r" (cpu)
		:
		: "memory"
	);
#else
    cpu = 0;
#endif
	return cpu;
}