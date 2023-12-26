#include <kernel.h>
#include <esr.h>
#include <ctx_arm64.h>
#include <exception.h>
#include <kmon/kmon.h>
#include <console.h>

#define MAXARGS  8
#define WHITESPACE " \t\n\r"

static int kmon_ok = 0;
static int in_kmon = 0; // Avoid recursive

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
  if (!(*cmd)) {
    return 0;
  }

  // For debug
  LOG_DEBUG("Run cmd: %s\n", cmd);

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
      LOG_ERROR("A command return non-zero, exit!\n");
      return;
    }
  }
}

void
kmon_main(void)
{
  if (!kmon_ok) {
    LOG_WARN("KMonitor is not available\n");
    return;
  }

  // Exception happends in kmon
  // Meaningless to enter again
  if (in_kmon) {
    LOG_ERROR("We already in kmon, but error happens\n");
    return;
  }

  in_kmon = 1;
  cmdloop(); // Never return
}

void
kmon_sync(void *ectx)
{
  cur_ectx = ectx;
}

void
kmon_exit(void)
{
  in_kmon = 0;
  cur_ectx = NULL; // Clear will be better
}


int
init_kmon(void)
{
  if (init_dbginfo() < 0)
    return -1;

  if (init_functrace() < 0) 
    return -1;

  LOG_INFO("Kmonitor init ok\n");
  kmon_ok = 1;
  return 0;
}