#pragma once
#include <ctx_arm64.h>
#include <exception.h>
#include <spinlock.h>
#include <list.h>

enum procstate { UNUSED, SLEEPING, RUNNABLE, RUNNING, ZOMBIE };

// Return the base addr of econtext
// ----------------
//   struct econtext
// ---------------- < [return]
//   stack
// ---------------- < stack
#define proc_ectx(p) ((struct econtext *) \
                        ((u64)((p)->stack_base)+(p)->stack_size-sizeof(struct econtext)))

struct proc {
	struct spinlock lock;
	
	
	enum procstate state;        // Process state
    struct proc *parent;         // Parent process
	int pid;                     // Process ID
    int prio;                    // Process Priority
    struct list_head sched_list; // Link to the sched list
	
	struct pagetable *pagetable; // User page table
	void *stack_base;            // kernel stack base,
                                 // context in higher addr
    u64 stack_size;              // Size of kenrel stack

	char name[16];               // Process name (debugging)
};

struct proc*
cur_proc(void);
void 
proc_set_context(struct proc *p, void *entry, vaddr_t sp);
struct proc*
create_root_proc(void);
void
proc_ready(struct proc *p);