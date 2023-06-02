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
  DBG_page_table((page_table_t *)kernel_pgd_base());
}