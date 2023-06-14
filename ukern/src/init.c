#include <console.h>
#include <print.h>
#include <pagetable.h>
#include <mm.h>
#include <panic.h>
#include <gic_v3.h>
#include <sched.h>
#include <task.h>
#include <pcpu.h>
#include <cfi.h>
#include <ramdisk.h>
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
  
  cfi_init();
  printf("cfi-pflash OK!\n");

  ramdisk_copy_from_flash();
  ramdisk_init();
  printf("ramdisk OK!\n");

  create_idle_task();

  extern int load_root_service(void);
  load_root_service();
  
  // debug
  // DBG_pagetable(kernel_pgd_base());
  // extern int DBG_bitmap(void);
  // DBG_bitmap();

  /* asm ("brk #0"); */
  
  panic("[SPRING-OS end, byebye]\n");
}
