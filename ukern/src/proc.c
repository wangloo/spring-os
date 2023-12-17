#include <spinlock.h>
#include <kernel.h>
#include <list.h>
#include <page.h>
#include <pagetable.h>
#include <kmem.h>
#include <cpu.h>

#include <uspace/handle.h>
#include <proc.h>


int 
pid_alloc(void)
{
    static int pid;
    return pid++;
}

void 
proc_set_context(struct proc *p, void *entry, vaddr_t sp)
{
    struct econtext *ectx = proc_ectx(p);
    ectx->ctx.sp0 = sp;
    ectx->ctx.elr = (u64)entry;
}

int
proc_is_roots(struct proc *p)
{
  return (0 == strcmp(p->name, "roots"));
}

// Return the current struct proc *, or zero if none.
struct proc*
cur_proc(void) 
{
  cpu_intr_off();
  struct cpu *c = cur_cpu();
  struct proc *p = c->proc;
  cpu_intr_on();
  return p;
}

static struct proc*
proc_alloc(void)
{
  struct proc *p;
  struct econtext *ectx;
  
  if (!(p = kalloc(sizeof(*p)))) {
    return NULL;
  }
  
  initlock(&p->lock, "proc");

  // Insert to right shed queue when assigning
  // a real priority for it
  p->prio = -1;
  INIT_LIST_HEAD(&p->sched_list);
  
  p->pid = pid_alloc();
  p->state = UNUSED;


  // Build process's handle table
  init_proc_handles(p);

  


  // Allocate two pages for the process's stack,
  // low page for kernel stack, high page for user stack.
  // Map them high in memory
  p->stack_base = page_allocn(2);
  p->stack_size = 2 * PAGE_SIZE;
  
  
  // Allocate new process's pagetable.
  // Only a pgd initially, allocate other levels when used.
  p->pagetable = (struct pagetable *)page_allocz();
  
  // Set up new context to start executing at forkret,
  // which returns to user space.
  ectx = proc_ectx(p);
  memset(ectx, 0, sizeof(*ectx));
  ectx->ctx.sp0 = (uint64_t)page_alloc();
  ectx->ctx.elr = 0;    // FIXME: fake a right return addr
  ectx->ctx.spsr = AARCH64_SPSR_EL0t | \
                    AARCH64_SPSR_F | \
                    AARCH64_SPSR_I | \
                    AARCH64_SPSR_A;   
  
  return p;
}

static void 
proc_free(struct proc *p)
{
  struct econtext *ectx = proc_ectx(p);
  // Stack needs be freed
  page_free((void *)(ectx->ctx.sp0));
  page_free(p->stack_base);
  
  // Pagetable needs to be freed recursively.
  pagetable_free(p->pagetable);
  p->pagetable = 0;
  
  p->name[0] = 0;
  p->parent = 0;
  p->pid = 0;
  p->state = UNUSED;
  
  p->prio = -1;
  INIT_LIST_HEAD(&p->sched_list);
  
  kfree(p);
}


// Create a new process, copying the parent.
// Sets up child kernel stack to return as if from fork() system call.
int
fork(void)
{
  int pid;
  struct proc *np;
  struct proc *p = cur_proc();

  // Allocate process descriptor.
  if((np = proc_alloc()) == 0){
    return -1;
  }

  // Copy user memory from parent to child.
  if(pagetable_clone(np->pagetable, p->pagetable) < 0){
    proc_free(np);
    release(&np->lock);
    return -1;
  }

  // Copy process context
  struct econtext *old_ectx, *new_ectx;
  old_ectx = proc_ectx(p);
  new_ectx = proc_ectx(np);
  memcpy(new_ectx, old_ectx, sizeof(*old_ectx));

  // Child's priority == parent
  np->prio = p->prio;
  list_add_tail(&np->sched_list, &p->sched_list);

  np->parent = p;
  np->state = RUNNABLE;
  strcpy(np->name, p->name);

  pid = np->pid;

  release(&np->lock);
  return pid;
}



struct proc*
create_proc(char *name, int pid, int prio)
{
  struct proc *p;

  if (name && strlen(name) >= PROC_NAME_MAX) {
    LOG_ERROR("proc name [%s] too long\n", name);
    return NULL;
  }
  p = proc_alloc();
  p->prio = prio;
  p->pid = pid;
  p->state = RUNNABLE;
  if (name)
    strcpy(p->name, name);

  return p;
}

struct proc*
create_root_proc(void)
{
    struct proc *p;
    struct cpu *cpu = cur_cpu();

    p = create_proc("roots", 0, 0);
    if (!p)
      return NULL;

    // set cur_proc();
    cpu->proc = p;
    return p;
}

void
proc_ready(struct proc *p)
{
    struct cpu *c = cur_cpu();
    list_add_tail(&p->sched_list, &c->ready_list[p->prio]);
    c->local_rdy_grp |= bit(p->prio);
}