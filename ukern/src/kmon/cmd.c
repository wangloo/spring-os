#include <kernel.h>
#include <esr.h>
#include <ctx_arm64.h>
#include <exception.h>
#include <slab.h>
#include <kmem.h>
#include <kmon/kmon.h>

extern struct econtext *cur_ectx;
extern struct km_state cur_state;
extern void load_ectx(struct econtext *ectx);

#define DEFINE_FUNC_EXEC(cmdname) int exec_##cmdname(int argc, char **argv)
#define assert_no_param(argc) do {assert(argc==1);}while(0)
#define assert_has_param(argc) do {assert(argc>1);}while(0)
#define check_no_param(cmd, argc) do {if (argc!1) {return 0;}}while(0)
#define check_has_param(cmd, argc) do {if (argc<=1) {return 0;}}while(0)

struct km_cmd {
  char *name;
  char *desc;
  int (*exec)(int argc, char **argv);
};

struct km_cmdmsg {
  int diag;
  char *msg;
};

#define KMCMDMSG(msgnum, text) \
	{ KM_##msgnum, text }

enum cmd_diag {
  KM_NOTFOUND = 4,
  KM_BADGRANS = 10,
  KM_BADRADIX,
  KM_BADCOUNT,
  KM_BADPARAM,
};

static struct km_cmdmsg cmdmsgs[] = {
  KMCMDMSG(NOTFOUND, "Command not found"),
  KMCMDMSG(BADGRANS, "Illegal value for GRANS"),
  KMCMDMSG(BADRADIX, "Illegal value for RADIX"),
  KMCMDMSG(BADCOUNT, "Illegal value for COUNT"),
  KMCMDMSG(BADPARAM, "Illegal param format or count"),
};

static void
print_cmderror(int diag)
{
  if (diag == 0) {
    printf("No error detected (diagnostic is %d\n", diag);
    return;
  }

  for (int i = 0; i < nelem(cmdmsgs); i++) {
    if (cmdmsgs[i].diag == diag) {
      printf("diag: %d: %s\n", diag, cmdmsgs[i].msg);
      return;
    }
  }
  printf("Unknow diag: %d\n", diag);
  assert(0);
}

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
static inline int
is_hexaddr(char *str)
{
  int len = strlen(str);

  // If use instruction address as parameter, 
  // only handle hex and must start with 0x.
  if (len > 2 && (str[0]=='0' && str[1]=='x'))
    return 1;
  return 0;
}



static int 
__exec_md(unsigned long addr, int grans, int radix, int count)
{
  int line;

  if (!strchr("dxo", (char)radix))
    return KM_BADRADIX;
  if (grans != 1 && grans != 4 && grans != 8)
    return KM_BADGRANS;
  if (count <= 0 || count > 32)
    return KM_BADCOUNT;
  
  // Round address down modulo grans
  addr &= ~(grans-1);

  // Display 16bytes one line
  for (line = 0; ; line ++) {
    char lstr[128];
    int lcount, i, lidx=0;
    if (!count) break;

    lcount = (16/grans <= count)? 16/grans : count;
    lidx += sprintf(lstr, "%lx: ", addr);
    for (i = 0; i < lcount; i++) {
      switch (grans) {
      case 1:
        lidx += sprintf(lstr+lidx, "%02x ", *((unsigned char *)(addr + i*grans)));
        break;
      case 4:
        lidx += sprintf(lstr+lidx, "%08x ", *((unsigned int *)(addr + i*grans)));
        break;
      case 8:
        lidx += sprintf(lstr+lidx, "%016lx ", *((unsigned long *)(addr + i*grans)));
        break;
      default: break;
      }
    }
    printf("%s\n", lstr);

    count -= lcount;
    addr += 16;
  }
  return 0;
}

DEFINE_FUNC_EXEC(bp)
{
  unsigned long breakaddr;
  int breakid;
  char *breakloc=NULL;
  char *funcname, *filename;
  int line;

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

  // Prepare location string
  if (dgbinfo_get_func_loc(breakaddr, &funcname, &filename,NULL) == 0 &&
      dbginfo_get_func_lineno(breakaddr, filename, &line) == 0) {
    breakloc = kalloc(128);
    sprintf(breakloc, "%s(%s:%d)", funcname, filename, line);
    assert(strlen(breakloc) < 100);
  }

  if ((breakid = brkpnt_add(breakaddr, breakloc)) < 0)
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


// md [[148][dxo]N] <vaddr>
DEFINE_FUNC_EXEC(md)
{
  // radix: format of display
  // grans: bytes of each count
  // count: display total grans*count bytes
  int radix='x', count=4, grans=8;
  unsigned long addr;
  check_has_param(md, argc);

  if (!is_hexaddr(argv[1])) {
    grans = (int)(argv[1][0] - '0');
    radix = (int)argv[1][1];
    count = strtoul(argv[1]+2, NULL, 10);
    if (argc != 3 || !is_hexaddr(argv[2]))
      return KM_BADPARAM;
    addr = strtoul(argv[2], NULL, 16);
  } else {
    addr = strtoul(argv[1], NULL, 16);
  }

  return __exec_md(addr, grans, radix, count);
}

// mdp <paddr>
DEFINE_FUNC_EXEC(mdp)
{
  // radix: format of display
  // grans: bytes of each count
  // count: display total grans*count bytes
  int radix='x', count=4, grans=8;
  unsigned long addr;
  check_has_param(mdp, argc);

  if (!is_hexaddr(argv[1])) {
    grans = (int)(argv[1][0] - '0');
    radix = (int)argv[1][1];
    count = strtoul(argv[1]+2, NULL, 10);
    if (argc != 3 || !is_hexaddr(argv[2]))
      return KM_BADPARAM;
    addr = strtoul(argv[2], NULL, 16);
  } else {
    addr = strtoul(argv[1], NULL, 16);
  }

  printf("pa:%lx ==> va: %lx\n", addr, ptov(addr));
  return __exec_md(ptov(addr), grans, radix, count);
}

// mm <vaddr> <value>
DEFINE_FUNC_EXEC(mm)
{
  unsigned long addr, val;
  check_has_param(mm, argc);
  
  if (argc != 3)
    return KM_BADPARAM;

  addr = strtoul(argv[1], NULL, 16);
  val = strtoul(argv[2], NULL, 16);
  *((unsigned long *)addr) = val;
  return 0;
}

// mmB <vaddr> <value>
DEFINE_FUNC_EXEC(mmB)
{
  unsigned long addr;
  unsigned char val;
  check_has_param(mmB, argc);
  if (argc != 3)
    return KM_BADPARAM;

  addr = strtoul(argv[1], NULL, 16);
  val = (unsigned char)strtoul(argv[2], NULL, 16);
  *((unsigned char *)addr) = val;
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
  {"bt", "Stack backtrace", exec_bt},
  {"md", "Memory display", exec_md},
  {"mdp", "Memory display(physical)", exec_mdp},
  {"mm", "Modify memory", exec_mm},
  {"mmB", "Modify memory(bytes)", exec_mmB},
  {"ft", "Function trace", exec_ft},
  {"hwt", "Hardware trace", exec_hwt},
  {"where", "Where i am", exec_where},
  {"ss", "Single step", exec_ss},
  {"go", "Go on execution", exec_go},
  {"regs", "Show registers", exec_regs},
  {"print", "Show var or func", exec_print},
  {"slab", "Show status of slab", exec_slab},
};


// Return 0 if can go on next cmd, <0 for IMPORTANT error, 
// must stop KMonitor and check.
int
runcmd(int argc, char **argv)
{
  int i, cmdret;

  if (argc < 1)
    return -1;

  for (i = 0; i < nelem(allcmds); i++) {
    if (strcmp(argv[0], allcmds[i].name) == 0) {
      if ((cmdret = allcmds[i].exec(argc, argv)) < 0) {
        // Serious err, KMonitor must exit
        return -1;
      } else if (cmdret > 0) {
        // Don't affect execute other cmds
        print_cmderror(cmdret);
      }
      return 0;
    }
  }

  print_cmderror(KM_NOTFOUND);
  return 0;
}