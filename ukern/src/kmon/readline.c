#include <kernel.h>
#include <console.h>

#define BUFLEN 256
char buf[BUFLEN];


// TODO: https://github.com/oneiro-naut/readLine/blob/main/src/input.c

// Return null-terminated line str
char *
readline(void)
{
  char c;
  int cnt = 0;

  while (1) {
    c = console_getc();
    if (c == '\b' || c == '\x7f') { // Backspace
      if (cnt > 0) {
        console_putc('\b');
        console_putc(' ');
        console_putc('\b');
        cnt--;
      }
    } else if (cnt < BUFLEN - 1) {
      console_putc(c);
      // printf("%u\n", (unsigned int)c);
      buf[cnt++] = c;
    } else if (c == '\n' || c == '\r') { // End of line
      console_putc('\n');
      buf[cnt] = 0;
      return buf;
    } else {  // Error
      LOG_ERROR("Errno in getchar\n");
      return NULL;
    }

    // printf("cnt: %d\n", cnt);
  }
}
