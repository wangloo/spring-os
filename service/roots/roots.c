// #include <string.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <elf.h>

#include <minos/service.h>
#include <minos/types.h>
#include <minos/list.h>
#include <minos/utils.h>
#include <minos/debug.h>
#include <minos/compiler.h>
#include <halloc.h>
#include <ramdisk.h>
#include <exec.h>


char allzero[512];


// Load service: sh
static int
load_sh(void)
{
  char *argv[] = {"sh.elf", NULL};

  if (exec("sh.elf", argv) < 0) {
    return -1;
  }

  return 0;
}





int main(int argc, char *argv[], char *envp[])
{
    int i;
    unsigned long heapbase = 0, heapend = 0;
    unsigned long ramdisk_base = 0, ramdisk_end = 0;

    puts("\n\nRoots service start...\n\n");
    
	printf("Checking bss clean\n"); // BUG? first \n not work
	for (i = 0; i < nelem(allzero)/sizeof(long); i++) {
		assert(*((long *)allzero+i) == 0);
	}
	
    // Print all arguements
    printf("argc: %d\n", argc);
    for (i = 0; i < argc; i++) {
      printf("argv[%d]: %s\n", i, argv[i]);
    }

    // Print all enviroment variables
    for (i = 0; envp[i]; i++) {
      printf("env[%d]: %s\n", i, envp[i]);
    }

    // Print all auxv
    Elf64_auxv_t *auxv = (Elf64_auxv_t *)(envp+i+1);
    for (; auxv->a_type != AT_NULL; auxv++) {
      if (auxv->a_type == AT_PAGESZ) {
        printf("AT_PAGESZ: 0x%lx\n", auxv->a_un.a_val);
      } else if (auxv->a_type == 62) { // HEAP_BASE
        printf("AT_HEAP_BASE: 0x%lx\n", auxv->a_un.a_val);
        heapbase = auxv->a_un.a_val;
      } else if (auxv->a_type == 63) { // HEAP_END
        printf("AT_HEAP_END: 0x%lx\n", auxv->a_un.a_val);
        heapend = auxv->a_un.a_val;
      } else if (auxv->a_type == 64) { // RAMDISK_BASE
        printf("AT_RAMDISK_BASE: 0x%lx\n", auxv->a_un.a_val);
        ramdisk_base = auxv->a_un.a_val;
      } else if (auxv->a_type == 65) {
        printf("AT_RAMDISK_END: 0x%lx\n", auxv->a_un.a_val);
        ramdisk_end = auxv->a_un.a_val;
      }
        
    }
    
    if (halloc_init(heapbase, heapend) < 0)
      goto out;

    if (ramdisk_init(ramdisk_base, ramdisk_end) < 0)
      goto out;
  

    if (load_sh() < 0) {
      printf("Load service sh failed\n");
      goto out;
    }

out:
    return 0;
}