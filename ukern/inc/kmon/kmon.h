struct km_state {
  // Enter from kernel exception can't return.
  // Otherwise, from hotkey or breakpoint can return normally
  int returnable;
  int ftrace_disabled;
  int initok;
  // Flag we already in KMonitor to avoid recursive
  int inside;
};

char *
readline(const char *prompt);
int
init_kmon(void);
void
kmon_sync(void *ectx, int returnable);
int
kmon_return(void);
void
kmon_main();

int
init_dbginfo(void);
int
dgbinfo_get_func_param(unsigned long pc, int *argc, int **offset, int **size);
int
dgbinfo_get_func_loc(unsigned long pc, char **name, char **file, int *line);
int
dbginfo_get_caller(u64 curpc, u64 cursp, u64 *callerpc, u64 *callersp);
int
dbginfo_get_func_lineno(unsigned long pc, char *file, int *line);



// Tmp
void 
backtrace(unsigned long pc, unsigned long sp, unsigned long x30);

__notrace void
functrace_enable();
__notrace void
functrace_disable();
__notrace int
functrace_is_disable(void);
int
init_functrace(void);
__notrace void
print_functrace();

void
ftrace_timer_setup(u64 ns);
void 
ftrace_timer_start(void);
void 
ftrace_timer_stop(void);
int 
init_ftrace_timer(void);
unsigned long
ftrace_timer_tick(void);


int
runcmd(int argc, char **argv);
int exec_bt(int argc, char **argv);
int exec_ft(int argc, char **argv);
int exec_hwt(int argc, char **argv);
int exec_where(int argc, char **argv);
int exec_cont(int argc, char **argv);
int exec_regs(int argc, char **argv);
int exec_print(int argc, char **argv);
