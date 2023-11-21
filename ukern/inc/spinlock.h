#pragma once

struct pcpu;
// Mutual exclusion lock.
struct spinlock {
  int locked;        // Is the lock held?

  // For debugging:
  char *name;        // Name of lock.
  struct cpu *cpu;  // The cpu holding the lock.    
};


void initlock(struct spinlock *lock, char *name);
void acquire(struct spinlock *lock);
void release(struct spinlock *lock);