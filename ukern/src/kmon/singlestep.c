#include <kernel.h>
#include <ctx_arm64.h>
#include <exception.h>
#include <kmon/kmon.h>

enum step_type {
  STEP_ASM = 0,
  STEP_SRC,
};

enum step_action {
  STEP_EXIT = 0,
  STEP_MORE,
};

// Describe a single-step task
struct step {
  int count;
  int grains;
  unsigned long startaddr;
};

struct step curstep;

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
  struct step *s;
  
  s = cur_step();

  if (s->grains == STEP_SRC) {
    if (same_srcline(pc, s->startaddr)) 
      return STEP_MORE;
  }

  if (!(--s->count))
    return STEP_EXIT;
  return STEP_MORE;
}
int
step_asm(struct econtext *ectx, int count)
{
  // Update current step task
  curstep.grains = STEP_ASM;
  curstep.count = count;
  curstep.startaddr = ectx->ctx.elr; 

  // Set hardware register
  // TODO
  return 0;
}

int
step_src(struct econtext *ectx, int count)
{
  // TODO
  return 0;
}