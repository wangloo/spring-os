
char *
readline(const char *prompt);
int
init_kmon(void);
void
kmon_main(void);
int 
kmon_symbol_init(void);
int
kmon_get_inst_info(unsigned long pc, 
                char **func, char **file, int *line);