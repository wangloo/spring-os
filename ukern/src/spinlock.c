#include <kernel.h>
#include <list.h>
#include <cpu.h>
#include <atomic.h>
#include <spinlock.h>

void
initlock(struct spinlock *lock, char *name)
{
    lock->locked = 0;
    lock->name = name;
    lock->cpu = 0;   
}

// Check whether this cpu is holding the lock.
// Interrupts must be off.
static int
holding(struct spinlock *lock)
{
  return (lock->locked && lock->cpu == cur_cpu());

}


// Acquire the lock.
// Loops (spins) until the lock is acquired.
void
acquire(struct spinlock *lock)
{
    // disable interrupts to avoid deadlock.
    cpu_intr_off();

    if (holding(lock))
        panic("acquire");

    while (atomic_cmpxchg(&lock->locked, 0, 1) != 0)
        ;

    // Record info about lock acquisition for holding() and debugging.
    lock->cpu = cur_cpu();
}

// Release the lock.
void
release(struct spinlock *lock)
{
    if (!holding(lock))
        panic("release"); 

    lock->cpu = 0;

    atomic_set(&lock->locked, 0);

    cpu_intr_on();
}

