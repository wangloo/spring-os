#include <console.h>
#include <print.h>
#include <pagetable.h>
#include <mm.h>

void kernel_init()
{
  console_init();

  printf("=============\n");
  printf("HI, SPRING-OS\n");
  printf("=============\n");

  mm_init();

  // debug
  // DBG_pagetable(kernel_pgd_base());
  // extern int DBG_bitmap(void);
  // DBG_bitmap();
}