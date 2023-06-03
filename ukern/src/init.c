#include <console.h>
#include <print.h>
#include <pagetable.h>

void kernel_init()
{
  console_init();

  printf("=============\n");
  printf("HI, SPRING-OS\n");
  printf("=============\n");

  // debug
  
  // DBG_pagetable(kernel_pgd_base());
}