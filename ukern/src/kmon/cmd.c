#include <kernel.h>
#include <esr.h>
#include <ctx_arm64.h>
#include <exception.h>
#include <slab.h>
#include <kmon/kmon.h>

extern struct econtext *cur_ectx;
extern struct km_state cur_state;
extern void load_ectx(struct econtext *ectx);

struct km_cmd {
  char *name;
  char *desc;
  int (*exec)(int argc, char **argv);
};


#define DEFINE_FUNC_EXEC(cmdname) int exec_##cmdname(int argc, char **argv)
#define assert_no_param(argc) do {assert(argc==1);}while(0)
#define assert_has_param(argc) do {assert(argc>1);}while(0)
#define check_no_param(cmd, argc) do {if (argc!1) {return 0;}}while(0)
#define check_has_param(cmd, argc) do {if (argc<=1) {return 0;}}while(0)

static inline int
is_dbgsym(char *str)
{
  int len = strlen(str);

  // If use instruction address as parameter, 
  // only handle hex and must start with 0x.
  if (len > 2 && (str[0]=='0' && str[1]=='x'))
    return 0;
  return 1;
}


DEFINE_FUNC_EXEC(bp)
{
  unsigned long breakaddr;
  int breakid;
  check_has_param(bp, argc);

  if (argc == 1 || !argv[1]) {
    LOG_ERROR("parameter invalid\n");
    return 0;
  }

  if (is_dbgsym(argv[1])) {
    if (dbgsym_func_name2addr(argv[1], &breakaddr) < 0) {
      LOG_ERROR("Can't find address for symbol: %s\n", argv[1]);
      return 0;
    }
  } else {
    breakaddr = strtoul(argv[1], NULL, 16);
  }

  if ((breakid = brkpnt_add(breakaddr)) < 0)
    return -1;

  brkpnt_enable(breakid);
  return 0;
}

DEFINE_FUNC_EXEC(bl)
{
  print_brkpnts();
  return 0;
}

DEFINE_FUNC_EXEC(bt)
{
  assert_no_param(argc);

  backtrace(cur_ectx->ctx.elr, 
            cur_ectx->ctx.sp, 
            cur_ectx->ctx.gp_regs.lr);
  
  return 0;
}

// ft [opt]
DEFINE_FUNC_EXEC(ft)
{
  int enable;

  if (argc == 1) {
    print_functrace();
    return 0;
  }

  enable = (int)strtol(argv[1], NULL, 10);
  if (enable >= 2 || enable < 0) {
    return -1;
  }

  cur_state.ftrace_disabled = !enable;
  return 0;
}

DEFINE_FUNC_EXEC(hwt) { return 0; }
DEFINE_FUNC_EXEC(where) { return 0; }

DEFINE_FUNC_EXEC(ss)
{
  int count=1; // Step count

  if (argc > 1) 
    count = (int)strtol(argv[1], NULL, 10);
  if (count < 1 || count > 100)
    return -1;

  // If has breakpoint at elr, disable it temporarily,
  // re-enable in ss handler
  brkpnt_suspend(cur_ectx->ctx.elr);
  step_asm(count);

  if (kmon_return() < 0) {
    LOG_WARN("Context can't be reload, might in exception!\n");
    return 0;
  }
  load_ectx(cur_ectx); // Never return
  return 0;
}

DEFINE_FUNC_EXEC(go)
{
  assert_no_param(argc);

  if (kmon_return() < 0) {
    LOG_WARN("Context can't be reload, might in exception!\n");
    return 0;
  }
  load_ectx(cur_ectx); // Never return
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

DEFINE_FUNC_EXEC(slab)
{
  assert_no_param(argc);
  print_slab_info();
  return 0;
}


struct km_cmd allcmds[] = {
  {"bp", "Set breakpoints", exec_bp},
  {"bl", "List breakpoints", exec_bl},
  {"bt", "Backtrace", exec_bt},
  {"ft", "Function trace", exec_ft},
  {"hwt", "Hardware trace", exec_hwt},
  {"where", "Where i am", exec_where},
  {"ss", "Single step", exec_ss},
  {"go", "Go on", exec_go},
  {"regs", "Show registers", exec_regs},
  {"print", "Show var or func", exec_print},
  {"slab", "Show status of slab", exec_slab},
};


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