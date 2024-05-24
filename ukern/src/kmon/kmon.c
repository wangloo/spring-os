#include <kernel.h>
#include <esr.h>
#include <ctx_arm64.h>
#include <exception.h>
#include <console.h>
#include <cpu.h>
#include <barrier.h>
#include <kmon/pmuv3.h>
#include <kmon/kmon.h>

#define MAXARGS  8
#define WHITESPACE " \t\n\r"


struct km_state cur_state;
struct econtext *cur_ectx;

// Return <0 on error
static int
handlecmd(char *cmd)
{
  int argc=0;
  char *argv[MAXARGS];

  if (!cmd ) {
    LOG_ERROR("Input cmd is NULL\n");
    return -1;
  }
  if (!(*cmd)) return 0;

  // For debug
  // LOG_DEBUG("Run cmd: %s\n", cmd);

  // Parse parameters
  while (1) {
    if (argc >= MAXARGS - 1) {
      LOG_ERROR("Too many parameter, limit is %d\n", MAXARGS);
      return -1;
    }
    // Filter whitespace and terminate last param
    while (*cmd && strchr(WHITESPACE, *cmd))
      *(cmd++) = 0;
    // No more parameters
    if (!*cmd)
      break;
    
    argv[argc++] = cmd;
    while (*cmd && !strchr(WHITESPACE, *cmd))
      cmd++;
  }
  argv[argc] = 0;

  if (runcmd(argc, argv) < 0)
    return -1;
  return 0;
}

static void
cmdloop(void)
{
  char *line;

  while (1) {
    line = readline(">");
    if (!line) {
      LOG_ERROR("Read null cmd\n");
      return;
    }

    if (handlecmd(line) < 0) {
      LOG_ERROR("A command return non-zero, exec failed!\n");
      return;
    }
  }
}

void
kmon_main(void)
{
  if (!cur_state.initok) {
    LOG_WARN("KMonitor is not available\n");
    return;
  }

  // Exception happends in kmon
  // Meaningless to enter again
  if (cur_state.inside) {
    LOG_ERROR("We already in kmon, but error happens\n");
    return;
  }

  functrace_disable();
  cur_state.inside = 1;
  cmdloop(); // Never return
}

void
kmon_sync(void *ectx, int returnable)
{
  cur_ectx = ectx;
  cur_state.returnable = returnable;
  cur_state.ftrace_disabled = functrace_is_disable();
}

int
kmon_return(void)
{
  if (!cur_state.returnable)
    return -1;

  if (!cur_state.ftrace_disabled)
    functrace_enable();
  cur_state.inside = 0;
  return 0;
}

// Enter KMonitor mannually
void
kmon_enter(void)
{
  asm("brk #0\n");
}

int
init_kmon(void)
{
  if (init_dbginfo() < 0)
    return -1;

  if (init_functrace() < 0) 
    return -1;
  if (init_ftrace_timer() < 0)
    return -1;

  pmuv3_enable();

  // Do architecture config
  write_sysreg(0x0, oslar_el1);
  write_sysreg(0xa000, mdscr_el1);
  write_sysreg(read_sysreg(dbgbcr0_el1) & ~(0xf << 20), dbgbcr0_el1);
  write_sysreg(read_sysreg(dbgbcr1_el1) & ~(0xf << 20), dbgbcr1_el1);
  cpu_debug_on();


  LOG_INFO("Kmonitor init ok\n");
  cur_state.initok = 1;
  return 0;
}