#include <kernel.h>
#include <kmem.h>
#include <kmon/kmon.h>
#include <kmon/tracebuf.h>

static int in_functrace = 0; // Avoid recursive
static int functrace_disabled = 1;
static struct tracebuf *functrace_buf;

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


// Main process of function trace
// [pc] function's pc
// [lr] function's lr(caller's pc)
// [fp] function's fp
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