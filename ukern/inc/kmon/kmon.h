
char *
readline(const char *prompt);
int
init_kmon(void);
void
kmon_main(void);
int 
kmon_symbol_init(void);
int
findloc(unsigned long pc, char **func, char **file, int *line);
int 
findcaller(u64 curpc, u64 cursp, u64 *callerpc, u64 *callersp);

// Tmp
void 
backtrace(unsigned long pc, unsigned long sp);