#include <kernel.h>
#include <kmem.h>
#include <time.h>
#include <kmon/kmon.h>
#include <kmon/tracebuf.h>

// Enter traced but not yet exited
struct ftrace_half {
  unsigned long tick_start;

  // Passed pc in ftrace_start and ftrace_end are same,
  // it's func's first instruction address.
  // So when we orignize all ftrace_incall with head-inserted,
  // the first matched is what we find.
  unsigned long pc;
  struct ftrace_half *next;
};
struct ftrace_half head_half_calls;

static int in_functrace = 0; // Avoid recursive
static int functrace_disabled = 1;
static struct tracebuf *functrace_buf;

static int
ftrace_half_call_add(unsigned long pc, unsigned long tick_start)
{
  struct ftrace_half *fh;
  if (!(fh = kalloc(sizeof(*fh))))
    return -1;

  fh->pc = pc;
  fh->tick_start = tick_start;
  fh->next = head_half_calls.next;
  head_half_calls.next = fh;
  return 0;
}

// Return value needs to be freed outside
static struct ftrace_half *
ftrace_half_call_pop(unsigned long pc)
{
  struct ftrace_half *fh, *pre;
  for (fh=head_half_calls.next, pre=&head_half_calls; 
      fh; pre=fh, fh=fh->next) {
    if (fh->pc == pc) {
      pre->next = fh->next;
      break;
    }
  }
  return fh;
}

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

__notrace int
functrace_is_disable(void)
{
  return functrace_disabled;
}

__notrace void
print_functrace()
{
  char *s;
  print_tracebuf_info(functrace_buf);
  // print_tracebuf_raw(functrace_buf);

  if (tracebuf_to_str(functrace_buf, &s) > 0)
    printf("Functrace Result:\n%s", s); 

}





  
__notrace void
ftrace_start(unsigned long pc, unsigned long lr, unsigned long fp)
{
  if (functrace_disabled || in_functrace)
    return;
  in_functrace = 1; 

  unsigned long tick = ftrace_timer_tick();

  if (ftrace_half_call_add(pc, tick) < 0) {
    assert(0);
  }

  in_functrace = 0;
}

// Main process of function trace
// [pc] function's pc
// [lr] function's lr(caller's pc)
// [fp] function's fp
__notrace void
ftrace_end(unsigned long pc, unsigned long lr, unsigned long fp)
{
  unsigned long execus, tick;
  struct ftrace_half *fh;
  char *buf, *funcname;
  char argstr[64];
  int argc, i, argslen=0;;
  int *argoff, *argsize;
  int showname=1, showparam=1;

  if (functrace_disabled || in_functrace)
    return;
  in_functrace = 1; 

  // Record tick as early as possible
  tick = ftrace_timer_tick();
  if (!(fh = ftrace_half_call_pop(pc))) {
    assert(0);
  }
  execus = tick2ns(tick-fh->tick_start)/1000;
  kfree(fh);

  if (dgbinfo_get_func_loc(pc, &funcname, NULL, NULL) < 0)
    showname = 0;

  if (dgbinfo_get_func_param(pc, &argc, &argoff, &argsize) < 0) {
    // LOG_ERROR("Get invalid func param info for %lx\n", pc);
    showparam = 0;
  }
  
  if (showparam) {
    unsigned long *argv;
    unsigned long callerfp;

    assert(argc >= 0 && argc <= 8);
    if (argc == 0) {
      strcpy(argstr, "void");
    } else {
      argv = kalloc((argc + 1) * sizeof(unsigned long));
      callerfp = *((unsigned long *)fp);
      assert(callerfp > fp);

      for (i = 0; i < argc; i++) {
        // printf("argoff[%d]: %d, argsize: %d\n", i, argoff[i], argsize[i]);
        if (argsize[i] == 1) {
          argv[i] = *(unsigned char *)(callerfp + argoff[i]);
        } else if (argsize[i] == 2) {
          argv[i] = *(unsigned short *)(callerfp + argoff[i]);
        } else if (argsize[i] == 4) {
          argv[i] = *(unsigned int *)(callerfp + argoff[i]);
        } else if (argsize[i] == 8) {
          argv[i] = *(unsigned long *)(callerfp + argoff[i]);
        }
        
        // TODO: support more format
        argslen += sprintf(argstr + argslen, "%ld", argv[i]);
        if (argslen >= nelem(argstr)) {
          LOG_ERROR("Arguments too long\n");
          break;
        }
        if (i != argc - 1) argstr[argslen++] = ',';
      }
      argv[argc] = 0;
      argstr[argslen] = 0;
      kfree(argv); // Not used actually
    }
  }

  buf = kalloc(128);
  if (showname) {
    if (showparam)
      sprintf(buf, "[%ld us]%s(%s)\n", execus, funcname, argstr);
    else
      sprintf(buf, "[%ld us]%s(<noinfo>)\n", execus, funcname);
  } else {
    if (showparam)
      sprintf(buf, "[%ld us]%lx(%s)\n", execus, pc, argstr);
    else 
      sprintf(buf, "[%ld us]%lx(<noinfo>)\n", execus, pc);
  }

  // printf("Tracebuf insert: %s", buf);
  tracebuf_insert(functrace_buf, buf);
  kfree(buf);

  in_functrace = 0;
}

__notrace void 
functrace (unsigned long pc, unsigned long lr, unsigned long fp)
{
  char *funcname;
  char argstr[64];
  int argc, i, argslen=0;;
  int *argoff, *argsize;
  int showname=1, showparam=1;

  if (functrace_disabled || in_functrace)
    return;
  in_functrace = 1; 

  if (dgbinfo_get_func_loc(pc, &funcname, NULL, NULL) < 0)
    showname = 0;

  if (dgbinfo_get_func_param(pc, &argc, &argoff, &argsize) < 0) {
    // LOG_ERROR("Get invalid func param info for %lx\n", pc);
    showparam = 0;
  }

  // LOG_DEBUG("For pc: %lx\n", pc);  
  // LOG_DEBUG("argc: %d\n", argc);

  if (showparam) {
    unsigned long *argv;
    unsigned long callerfp;

    assert(argc >= 0 && argc <= 8);
    if (argc == 0) {
      strcpy(argstr, "void");
    } else {
      argv = kalloc((argc + 1) * sizeof(unsigned long));
      callerfp = *((unsigned long *)fp);
      assert(callerfp > fp);

      for (i = 0; i < argc; i++) {
        // printf("argoff[%d]: %d, argsize: %d\n", i, argoff[i], argsize[i]);
        if (argsize[i] == 1) {
          argv[i] = *(unsigned char *)(callerfp + argoff[i]);
        } else if (argsize[i] == 2) {
          argv[i] = *(unsigned short *)(callerfp + argoff[i]);
        } else if (argsize[i] == 4) {
          argv[i] = *(unsigned int *)(callerfp + argoff[i]);
        } else if (argsize[i] == 8) {
          argv[i] = *(unsigned long *)(callerfp + argoff[i]);
        }
        
        // TODO: support more format
        argslen += sprintf(argstr + argslen, "%ld", argv[i]);
        if (argslen >= nelem(argstr)) {
          LOG_ERROR("Arguments too long\n");
          break;
        }
        if (i != argc - 1) argstr[argslen++] = ',';
      }
      argv[argc] = 0;
      argstr[argslen] = 0;
      kfree(argv); // Not used actually
    }
  }

  char *buf = kalloc(128);
  if (showname) {
    if (showparam)
      sprintf(buf, "%s(%s)\n", funcname, argstr);
    else
      sprintf(buf, "%s(<noinfo>)\n", funcname);
  } else {
    if (showparam)
      sprintf(buf, "%lx(%s)\n",pc, argstr);
    else 
      sprintf(buf, "%lx(<noinfo>)\n", pc);
  }


  // printf("Tracebuf insert: %s", buf);
  tracebuf_insert(functrace_buf, buf);
  kfree(buf);

  in_functrace = 0;
}







int
init_functrace(void)
{
  if ((functrace_buf = alloc_tracebuf(512)) == NULL) {
    return -1;
  }
  return 0;
}