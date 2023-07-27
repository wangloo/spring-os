#pragma once
#include <raw_spinlock.h>

#ifdef CONFIG_SMP
#define spin_lock(l)                                                           \
  do {                                                                         \
    preempt_disable();                                                         \
    raw_spin_lock(l);                                                          \
  } while (0)

#define spin_unlock(l)                                                         \
  do {                                                                         \
    raw_spin_unlock(l);                                                        \
    preempt_enable();                                                          \
  } while (0)

#define spin_trylock(l)                                                        \
  ({                                                                           \
    preempt_disable();                                                         \
    raw_spin_trylock(l);                                                       \
  })

#define spin_lock_irqsave(l, flags)                                            \
  do {                                                                         \
    preempt_disable();                                                         \
    flags = arch_save_irqflags();                                              \
    arch_disable_local_irq();                                                  \
    raw_spin_lock(l);                                                          \
  } while (0)

#define spin_trylock_irqsave(l, flags)                                         \
  ({                                                                           \
    int ret;                                                                   \
    preempt_disable();                                                         \
    flags = arch_save_irqflags();                                              \
    arch_disable_local_irq();                                                  \
    ret = raw_spin_trylock(l);                                                 \
    if (!ret) {                                                                \
      arch_restore_irqflags(flags);                                            \
      preempt_enable();                                                        \
    }                                                                          \
    ret;                                                                       \
  })

#define spin_unlock_irqrestore(l, flags)                                       \
  do {                                                                         \
    raw_spin_unlock(l);                                                        \
    arch_restore_irqflags(flags);                                              \
    preempt_enable();                                                          \
  } while (0)


#else

/**
 * FIXME: 下面的这些没什么意义，趁早还是注释掉吧
 */
#define spin_lock(l)
#define spin_unlock(l)
#define spin_trylock(l) raw_spin_trylock()
#define spin_lock_irqsave(l, flags)   \
  do {                 \
    raw_spin_lock(l);  \
    flags = 0;         \
  } while (0) 
#define spin_trylock_irqsave(l, flags) ({ raw_spin_trylock(l); flags = 0;})
#define spin_lock_irqrestore(l, flags) ({ raw_spin_lock(l); })
#define spin_unlock_irqrestore(l, flags)  \
  do {                                                                         \
    raw_spin_unlock(l);                                                        \
    flags++;                                                                   \
  } while (0)

// static inline void spin_lock(spinlock_t *lock) {}
// static inline void spin_unlock(spinlock_t *lock) {}
// static inline void spin_trylock(spinlock_t *lock) {}
// static inline void spin_lock_irqsave(spinlock_t *lock, unsigned long flags) {}
// static inline void spin_trylock_irqsave(spinlock_t *lock, unsigned long flags) {}
// static inline void spin_lock_irqrestore(spinlock_t *lock, unsigned long flags) {}
// static inline void spin_unlock_irqrestore(spinlock_t *lock, unsigned long flags) {}


#endif
