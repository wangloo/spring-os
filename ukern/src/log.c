#include <print.h>

static const char *level_str[] = {
    "INFO", "WARN", "ERROR", "DEBUG"
};

static const char *level_color[] = {
  "\x1b[32m", "\x1b[33m", "\x1b[31m", "\x1b[36m"
};


void log(int level, const char *domain, 
          const char *file, int line, const char *fmt, ...)
{
  va_list ap;

  va_start(ap, fmt);
#if 0
  printf("%s%-5s\x1b[0m \x1b[90m%s:%d:\x1b[0m ",
            level_color[level], level_str[level], file, line);
#endif
  printf("%s%-5s\x1b[0m \x1b[90m(%s)\x1b[0m ",
            level_color[level], level_str[level], domain);
  vprintf(fmt, ap);
  va_end(ap);
//   printf("\n");
}
