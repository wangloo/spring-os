#include <kernel.h>
#include <kmon/kmon.h>

void 
backtrace(unsigned long pc, unsigned long sp)
{
  unsigned long prepc, presp;
  int level = 0;

  while (1) {
    char *file, *func;
    int line;

    if (findloc(pc, &func, &file, &line) < 0) {
      LOG_ERROR("Faild to get pc info\n");
      return;
    }

    printf("\nFrame level %d\n", level);
    printf("pc: %lx\nsp: %lx\n", pc, sp);
    printf("%s %s:%d\n", func, file, line);

    if (findcaller(pc, sp, &prepc, &presp) < 0)
      break;

    printf("prepc: %lx\npresp: %lx\n", prepc, presp);
    pc = prepc;
    sp = presp;
    level += 1;
  }
}