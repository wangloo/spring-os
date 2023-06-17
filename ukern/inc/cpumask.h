#pragma once
#include <bitmap.h>
#include <types.h>
#include <config/config.h>

typedef struct cpumask {
	 bitmap_t bits[bits_to_long(CONFIG_NR_CPUS)];
} cpumask_t;

/* verify cpu argument to cpumask_* operators */
static inline unsigned int cpumask_check(int cpu)
{
	/*
	 * do some check
	 */
	return cpu;
}

static inline void cpumask_set_cpu(int cpu, cpumask_t *dstp)
{
	bitmap_set_bit(dstp->bits, cpumask_check(cpu));
}

static inline void cpumask_clear_cpu(int cpu, cpumask_t *dstp)
{
	bitmap_clear_bit(dstp->bits, cpumask_check(cpu));
}

static inline void cpumask_setall(cpumask_t *dstp)
{
	bitmap_init(dstp->bits, BITMAP_SIZE(CONFIG_NR_CPUS), BITMAP_FULL);
}

static inline void cpumask_clearall(cpumask_t *dstp)
{
	bitmap_init(dstp->bits, BITMAP_SIZE(CONFIG_NR_CPUS), BITMAP_EMPTY);
}

static inline int cpumask_first(const cpumask_t *srcp)
{
	return bitmap_find_first_1(srcp->bits, BITMAP_SIZE(CONFIG_NR_CPUS));
}

static inline int cpumask_next(int n, const cpumask_t *srcp)
{
	/* -1 is a legal arg here. */
	if (n != -1)
		cpumask_check(n);

	return bitmap_find_next_1(srcp->bits, BITMAP_SIZE(CONFIG_NR_CPUS), n + 1);
}

#if CONFIG_NR_CPUS > 1
#define for_each_cpu(cpu, mask)			\
	for ((cpu) = cpumask_first(mask);	\
	     (cpu) >= 0 && (cpu) < CONFIG_NR_CPUS;		\
	     (cpu) = cpumask_next(cpu, mask))
#else /* CONFIG_NR_CPUS == 1 */
#define for_each_cpu(cpu, mask)			\
	for ((cpu) = 0; (cpu) < 1; (cpu)++, (void)(mask))
#endif /* CONFIG_NR_CPUS */
