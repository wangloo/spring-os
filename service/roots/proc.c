#include <stdio.h>
#include <stdlib.h>
#include <minos/service.h>
#include <minos/types.h>
#include <minos/utils.h>
#include <minos/list.h>
#include <minos/debug.h>
#include <minos/kobject.h>
#include <minos/kobject_uapi.h>
#include <memlayout.h>
#include <halloc.h>
#include <proc.h>

struct proc *
create_new_proc(char *name, struct proc *parent, unsigned long entry)
{
  struct proc *p;
  struct process_create_arg args = {0};
  

  p = halloc_zero(sizeof(*p));
  if (!p) 
    return NULL;

  args.stack = PROCESS_STACK_BASE;
  args.entry = entry;
  args.aff = -1;
  args.prio = -1;
  args.pid = 2;  // FIXME
  pr_debug("do syscall\n");
  p->handle = kobject_create(KOBJ_TYPE_PROCESS, (unsigned long)&args);
  return p;
}