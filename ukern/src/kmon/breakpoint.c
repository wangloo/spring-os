#include <kernel.h>
#include <kmem.h>
#include <kmon/kmon.h>

#define BRKPNT_MAX  4  // 最多同时支持4个断点

enum brkpnt_state {
  BRKPNT_OFF = 0,
  BRKPNT_ON,
};

struct brkpnt {
  int id;             // Id of breakpoint
  int state;          // State of breakpoint
  unsigned long addr; // Binded virtual addr
};

struct brkpnt *allbrks[BRKPNT_MAX];


void
print_brkpnts(void)
{
  struct brkpnt *b;
  int i;

  printf("=============================\n");
  for (i = 0; i < BRKPNT_MAX; i++) {
    if ((b = allbrks[i])) {
      if (b->state == BRKPNT_ON)
        printf("id: %u, ON, %lx\n", b->id, b->addr);
      else
        printf("id: %u, OFF, %lx\n", b->id, b->addr);
    }
  }
  printf("\n");
}

static int
addr_can_brk(unsigned long addr)
{
  return 1;
}

// Return breakpoint Id if success, -1 for error
int 
brkpnt_add(unsigned long addr)
{ 
  int id;
  struct brkpnt *b;
  
  if (!addr_can_brk(addr))
    return -1;

  for (id = 0; id < BRKPNT_MAX; id++) {
    if (!allbrks[id])
      break;
  }
  if (id == BRKPNT_MAX) {
    LOG_ERROR("Too many breakpoints\n");
    return -1;
  }
  
  if (!(b = kalloc(sizeof(*b))))
    return -1;
  b->addr = addr;
  b->id = id;
  b->state = BRKPNT_OFF;
  allbrks[id] = b;

  return id;
}

int
brkpnt_enable(int id)
{
  assert(id >= 0 && id < BRKPNT_MAX);
  assert(allbrks[id]);
  
  if (allbrks[id]->state == BRKPNT_ON)
    return 0;

  allbrks[id]->state = BRKPNT_ON;

  // Set hardward breakpoint register
  switch (id) {
  case 0:
  case 1:
    break;
  }
  return 0;
}

int
brkpnt_disable(int id)
{
  assert(id >= 0 && id < BRKPNT_MAX);
  assert(allbrks[id]);

  if (allbrks[id]->state == BRKPNT_OFF)
    return 0;

  allbrks[id]->state = BRKPNT_OFF;

  // Set hardward breakpoint register
  switch (id) {
  case 0:
  case 1:
    break;
  }
  return 0;
}

int
brkpnt_del(int id)
{ 
  if (!allbrks[id]) {
    LOG_ERROR("Try to delete a null breakpoint\n");
    return -1;
  }

  if (brkpnt_disable(id) < 0)
    return -1;

  kfree(allbrks[id]);
  allbrks[id] = NULL;
  return 0;
}

int
init_breakpoints(void)
{
  int i;
  for (i = 0; i < BRKPNT_MAX; i++) {
    allbrks[i] = NULL;
  }
  return 0;
}