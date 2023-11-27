// #include <string.h>
#include <stdio.h>
#include <elf.h>

char s[20] = "ROOTSERVICE";
char *p;
int main(int argc, char *argv[], char *envp[])
{
    int i;

    puts("\n\nRoots service start...\n\n");
    
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
      } else if (auxv->a_type == 63) { // HEAP_END
        printf("AT_HEAP_END: 0x%lx\n", auxv->a_un.a_val);
      }
    }

    return 0;
}