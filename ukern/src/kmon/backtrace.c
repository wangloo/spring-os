#include <kernel.h>
#include <kmon/kmon.h>

void 
backtrace(unsigned long pc, unsigned long sp)
{
  unsigned long prepc, presp;
  char *file, *func;
  int line, level=0;

  while (1) {
    // Normal return address(in x30 or Elr) is next instr 
    // after which cause jump or exception.
    // But dump the current instr is easier to understand.
    // One TODO is: not all exception push next instr to Elr 
    pc -= 4;

    if (dgbinfo_get_func_loc(pc, &func, &file, NULL) < 0) {
      LOG_ERROR("Faild to get loc of %lx\n", pc);
      break;
    }
    if (dbginfo_get_func_lineno(pc, file, &line) < 0) {
      LOG_ERROR("Faild to get lineno of %lx\n", pc);
      break;
    }

    printf("\nFrame level %d\n", level);
    printf("pc: %lx\nsp: %lx\n", pc, sp);
    printf("%s %s:%d\n", func, file, line);

    if (dbginfo_get_caller(pc, sp, &prepc, &presp) < 0)
      break;
    // printf("prepc: %lx\npresp: %lx\n", prepc, presp);

    pc = prepc;
    sp = presp;
    level += 1;
  }
}