#include <kernel.h>
#include <ctx_arm64.h>
#include <exception.h>
#include <kmon/kmon.h>

enum step_type {
  STEP_ASM = 0,
  STEP_SRC,
};



// Describe a single-step task
struct step {
  int count;
  int grains;
  unsigned long startaddr;
};

struct step curstep;
extern struct econtext *cur_ectx;

struct step *
cur_step(void)
{
  return &curstep;
}

// TODO: Implement in dbginfo submodule
int 
same_srcline(unsigned long pc1, unsigned long pc2)
{
  return 1;
}
int 
step_handler(unsigned long pc)
{
  struct step *s=cur_step();
  
  printf("at %lx\n", pc);

  if (s->grains == STEP_SRC) {
    if (same_srcline(pc, s->startaddr)) 
      return STEP_MORE;
  }
  if (!(--s->count))
    return STEP_EXIT;
  assert(s->count > 0);
  return STEP_MORE;
}

int
step_asm(int count)
{
  assert(count > 0);

  // Update current step task
  curstep.grains = STEP_ASM;
  curstep.count = count;
  curstep.startaddr = cur_ectx->ctx.elr; 

  // Set hardware register
  write_sysreg(read_sysreg(mdscr_el1) | MDSCR_SS, mdscr_el1);
  cur_ectx->ctx.spsr |= SPSR_SS;
  return 0;
}

int
step_src(int count)
{
  // TODO
  return 0;
}