#pragma once

#include <atomic.h>

#ifdef CONFIG_SMP

void arch_ticket_lock(spinlock_t *lock);
void arch_ticket_unlock(spinlock_t *lock);
int arch_ticket_trylock(spinlock_t *lock);

#define DEFINE_SPIN_LOCK(name)                                                 \
  spinlock_t name = {                                                          \
      .current_ticket = 0,                                                     \
      .next_ticket = 0,                                                        \
  }

static void inline spin_lock_init(spinlock_t *lock)
{
  lock->current_ticket = 0;
  lock->next_ticket = 0;
}

static void inline raw_spin_lock(spinlock_t *lock)
{
  arch_ticket_lock(lock);
}

static void inline raw_spin_unlock(spinlock_t *lock)
{
  arch_ticket_unlock(lock);
}

static int inline raw_spin_trylock(spinlock_t *lock)
{
  return arch_ticket_trylock(lock);
}

#else

#define DEFINE_SPIN_LOCK(name) spinlock_t name

static void inline spin_lock_init(spinlock_t *lock)
{}

static void inline raw_spin_lock(spinlock_t *lock)
{}

static void inline raw_spin_unlock(spinlock_t *lock)
{}

static void inline raw_spin_trylock(spinlock_t *lock)
{
  // return 1;
}

#endif
