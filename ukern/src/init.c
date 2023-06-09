#include <console.h>
#include <print.h>
#include <pagetable.h>
#include <mm.h>
#include <panic.h>
#include <gic_v3.h>
#include <sched.h>
#include <task.h>
#include <pcpu.h>
void kernel_init()
{
  console_init();

  printf("=============\n");
  printf("HI, SPRING-OS\n");
  printf("=============\n");

  mm_init();

  gicv3_init();

  sched_init();
  sched_local_init();

  percpu_init();

  create_idle_task();
  
  // debug
  // DBG_pagetable(kernel_pgd_base());
  // extern int DBG_bitmap(void);
  // DBG_bitmap();

  /* asm ("brk #0"); */
  
  panic("[SPRING-OS end, byebye]\n");
}
