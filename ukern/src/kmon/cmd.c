#include <kernel.h>
#include <esr.h>
#include <ctx_arm64.h>
#include <exception.h>
#include <kmon/kmon.h>

extern struct econtext *cur_ectx;

struct km_cmd {
  char *name;
  char *desc;
  int (*exec)(int argc, char **argv);
};

struct km_cmd allcmds[] = {
  {"bt", "Backtrace", exec_bt},
  {"ft", "Function trace", exec_ft},
  {"hwt", "Hardware trace", exec_hwt},
  {"where", "Where i am", exec_where},
  {"cont", "Continue", exec_cont},
  {"regs", "Show registers", exec_regs},
  {"print", "Show var or func", exec_print},
};
  

#define DEFINE_FUNC_EXEC(cmdname) int exec_##cmdname(int argc, char **argv)

DEFINE_FUNC_EXEC(bt)
{
  assert(argc == 1);
  backtrace(cur_ectx->ctx.elr, 
            cur_ectx->ctx.sp, 
            cur_ectx->ctx.gp_regs.lr);
  
  return 0;
}

DEFINE_FUNC_EXEC(ft)
{
  return 0;
}

DEFINE_FUNC_EXEC(hwt)
{
  return 0;
}

DEFINE_FUNC_EXEC(where)
{
  return 0;
}

DEFINE_FUNC_EXEC(cont)
{
  return 0;
}

DEFINE_FUNC_EXEC(regs)
{
  return 0;
}

DEFINE_FUNC_EXEC(print)
{
  return 0;
}

int
runcmd(int argc, char **argv)
{
  int i;

  if (argc < 1)
    return -1;

  for (i = 0; i < nelem(allcmds); i++) {
    if (strcmp(argv[0], allcmds[i].name) == 0) {
      return allcmds[i].exec(argc, argv);
    }
  }

  LOG_ERROR("Unspported cmd: %s\n", argv[0]);
  return 0;
}