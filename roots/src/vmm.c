#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <sys/mman.h>

#include <minos/types.h>
#include <minos/page.h>
#include <minos/utils.h>
#include <minos/kobject_uapi.h>
#include <minos/debug.h>
#include <minos/list.h>
#include <minos/proto.h>
#include <proc.h>
#include <halloc.h>
#include <vmm.h>


#define vma_size(vma) ((vma)->end-(vma)->start)


static inline void
vma_init(struct vma *vma, unsigned long start, unsigned long end,
          int perm, int anon)
{
  vma->start = start;
  vma->end = end;
  vma->perm = perm;
  vma->anon = anon;
}

struct vma *
find_vma(struct proc *p, unsigned long addr)
{
  struct vma *vma;
  list_for_each_entry(vma, &p->vma_used, list) {
    if (addr >= vma->start && addr < vma->end)
      return vma;
  }
  return NULL;
}

struct vma *
split_vma(struct proc *proc, struct vma *vma, 
          unsigned long base, unsigned long end)
{
  struct vma *freevma;
  assert(base >= vma->start && end <= vma->end);

  if (base > vma->start) {
    freevma = halloc(sizeof(*freevma));
    vma_init(freevma, vma->start, base, 0, 0);
    list_add(&freevma->list, &proc->vma_free);
  }
  if (end < vma->end) {
    freevma = halloc(sizeof(*freevma));
    vma_init(freevma, end, vma->end, 0, 0);
    list_add(&freevma->list, &proc->vma_free);
  }
  vma->start = base;
  vma->end = end;
  return vma;
}

struct vma *
request_vma(struct proc *proc, unsigned long base, 
            size_t size, unsigned int perm, int anon)
{
  struct vma *vma, *out = NULL;
  unsigned long newbase = base, newend = base + size;

  if (base != 0 && !page_aligned(base)) {
    pr_err("%s invalid request address 0x%lx\n", __func__, base);
    return NULL;
  }

  list_for_each_entry(vma, &proc->vma_free, list) {
    if (base == 0) {
      newbase = vma->start;
      newend = vma->end;
    }
    if (newbase >= vma->start && newend <= vma->end) {
      out = vma;
      break;
    }
  }
  
  if (!out) 
    return NULL;

  list_del(&out->list);
  out = split_vma(proc, out, newbase, newend);
  out->perm = perm;
  out->anon = anon;
  list_add(&out->list, &proc->vma_used);
  return out;
}

int
roots_mmap(struct proc *proc, struct proto *proto)
{
  size_t len = proto->mmap.len;
  int prot = proto->mmap.prot;
  struct vma *vma;
  int perm = 0;
  void *addr = (void *)-1;

  if (proto->mmap.addr != NULL || proto->mmap.fd != -1) {
    pr_err("Only support map anon region for process\n");
    goto out;
  }
  
  if (prot & PROT_EXEC)
    perm |= KOBJ_RIGHT_EXEC;
  if (prot & PROT_WRITE)
    perm |= KOBJ_RIGHT_WRITE;
  if (prot & PROT_READ)
    perm |= KOBJ_RIGHT_READ;

  len = align_up(len, PAGE_SIZE);
  vma = request_vma(proc, 0, len, perm, 1);

  if (vma)
    addr = (void *)vma->start;

out:
  // kobject_reply_errcode(proc->handle, proc->token, (long)addr);
  return 0;
}

int
roots_brk(struct proc *proc, struct proto *proto)
{
  unsigned long addr = (unsigned long)proto->brk.addr;
  unsigned long reply = addr;

  if (addr == 0) {
    reply = proc->brk_cur;
    goto out;
  } 

  if (addr < proc->brk_start || addr >= proc->brk_end) {
    reply = -1;
    goto out;
  }
  
  proc->brk_cur = addr;
out:
  // kobject_reply_errcode(proc->handle, proc->token, (long)reply_addr);
  return 0;
}
