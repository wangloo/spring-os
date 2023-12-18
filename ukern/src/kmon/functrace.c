#include <kernel.h>
#include <kmon/tracebuf.h>

static int in_functrace = 0;
static int functrace_disabled = 1;
long functrace_count = 0;

struct tracebuf *functrace_buf;

__notrace void
functrace_enable()
{
  functrace_disabled = 0;
}

__notrace void
functrace_disable()
{
  functrace_disabled = 1;
}

__notrace void
print_functrace()
{
  char *s;
  print_tracebuf_info(functrace_buf);
  print_tracebuf_raw(functrace_buf);

  tracebuf_to_str(functrace_buf, &s);
  printf("Functrace Result:\n%s", s); 

}

__notrace void 
functrace (unsigned long pc, unsigned long lr)
{
  if (functrace_disabled)
    return;

  if (in_functrace)
    return;

  in_functrace = 1;

  functrace_count ++;
  char buf[32];
  sprintf(buf, "%lx\n", pc);
  printf("Tracebuf insert: %s", buf);
  tracebuf_insert(functrace_buf, buf);

  in_functrace = 0;
}

int
init_functrace(void)
{
  if ((functrace_buf = alloc_tracebuf(512)) == NULL) {
    return -1;
  }

}