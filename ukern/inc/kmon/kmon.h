
char *
readline(const char *prompt);
int
init_kmon(void);
void
kmon_main(void);

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
int
init_functrace(void);
__notrace void
print_functrace();