#include <kernel.h>
#include <kmon/kmon.h>
#include <console.h>

static int kmon_ok = 0;
static unsigned long elr = (unsigned long)printf;

// Return <0 on error
static int
runcmd(char *cmd)
{
  LOG_DEBUG("Run cmd: %s\n", cmd);

  char *func, *file;
  int line;
  if (kmon_get_inst_info(elr, &func, &file, &line) < 0) {
    return -1;
  }
  printf("%s (%s:%d)\n", func, file, line);
  return 0;
}

static void
cmdloop(void)
{
  char *line;

  while (1) {
    console_puts(">", 1);
    
    line = readline();
    if (!line) {
      LOG_ERROR("Read null cmd\n");
      return;
    }

    if (runcmd(line) < 0) {
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

  cmdloop();
}


int
init_kmon(void)
{

  if (kmon_symbol_init() < 0)
    return -1;


  kmon_ok = 1;
  return 0;
}