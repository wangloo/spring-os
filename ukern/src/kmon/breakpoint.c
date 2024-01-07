#include <kernel.h>
#include <kmem.h>
#include <kmon/kmon.h>

#define BRKPNT_MAX  2  // 最多同时支持4个断点

enum brkpnt_state {
  BRKPNT_OFF = 0,
  BRKPNT_ON,
  BRKPNT_SUSPEND_SS, // suspended for single step
};

struct brkpnt {
  int id;             // Id of breakpoint
  int state;          // State of breakpoint
  int hit;            // Count of breakpoint hit
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
        printf("id: %u, ON, hit: %d, addr: %lx\n", b->id, b->hit, b->addr);
      else
        printf("id: %u, OFF, hit: %d, addr: %lx\n", b->id, b->hit, b->addr);
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
  b->hit = 0;
  b->state = BRKPNT_OFF;
  allbrks[id] = b;

  LOG_INFO("Breakpoint add: #%d at %lx\n", id, addr);
  return id;
}

// Restore all breakpoints suspended by single step.
void
brkpnt_restore(void)
{
  int id;

  for (id=0; id<BRKPNT_MAX; id++) {
    if (allbrks[id] &&
        allbrks[id]->state == BRKPNT_SUSPEND_SS) {
      allbrks[id]->state = BRKPNT_ON;
      switch (id) {
      case 0:
        write_sysreg((read_sysreg(dbgbcr0_el1) | 0x1), dbgbcr0_el1);
        break;
      case 1:
        write_sysreg((read_sysreg(dbgbcr1_el1) | 0x1), dbgbcr1_el1);
        break;
      default: assert(0);
      }
    }
  }
}

void
brkpnt_suspend(unsigned long pc)
{
  int id;
  unsigned long val;

  for (id=0; id < BRKPNT_MAX; id++) {
    if (allbrks[id] && 
        allbrks[id]->addr == pc && 
        allbrks[id]->state == BRKPNT_ON) {
      break;
    }
  }
  if (id == BRKPNT_MAX) 
    return;

  assert(allbrks[id]->hit > 0);
  allbrks[id]->state= BRKPNT_SUSPEND_SS;

  switch(id) {
  case 0:
    val = read_sysreg(dbgbcr0_el1);
    write_sysreg((val & ~0x1), dbgbcr0_el1);
    break;
  case 1:
    val = read_sysreg(dbgbcr1_el1);
    write_sysreg((val & ~0x1), dbgbcr1_el1);
    break;
  default:
    assert(0);
  }
}

int
brkpnt_enable(int id)
{
  unsigned long val;

  assert(id >= 0 && id < BRKPNT_MAX);
  assert(allbrks[id]);
  
  if (allbrks[id]->state == BRKPNT_ON)
    return 0;

  allbrks[id]->state = BRKPNT_ON;

  // Set hardward breakpoint register
  switch (id) {
  case 0:
    val = read_sysreg(dbgbcr0_el1);
    write_sysreg(allbrks[id]->addr, dbgbvr0_el1);
    write_sysreg((val | ((0xf<<5) | (0x3<<1) | 0x1)), dbgbcr0_el1);
    
    LOG_DEBUG("bcr: %lx\nbvr: %lx\n", read_sysreg(dbgbcr0_el1), read_sysreg(dbgbvr0_el1));
    break;
  case 1:
    val = read_sysreg(dbgbcr1_el1);
    write_sysreg(allbrks[id]->addr, dbgbvr1_el1);
    write_sysreg((val | ((0xf<<5) | (0x3<<1) | 0x1)), dbgbcr1_el1);
    break;
  default:
    assert(0);
  }
  return 0;
}

int
brkpnt_disable(int id)
{
  unsigned long val;

  assert(id >= 0 && id < BRKPNT_MAX);
  assert(allbrks[id]);

  if (allbrks[id]->state == BRKPNT_OFF)
    return 0;

  allbrks[id]->state = BRKPNT_OFF;

  // Set hardward breakpoint register
  switch (id) {
  case 0:
    val = read_sysreg(dbgbcr0_el1);
    write_sysreg((val & ~0x1), dbgbcr0_el1);
    break;
  case 1:
    val = read_sysreg(dbgbcr1_el1);
    write_sysreg((val & ~0x1), dbgbcr1_el1);
    break;
  default:
    assert(0);
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

void
brkpnt_hit_handler(unsigned long breakaddr)
{
  int id;

  for (id = 0; id < BRKPNT_MAX; id++) {
    if (allbrks[id] && allbrks[id]->addr == breakaddr) {
      allbrks[id]->hit += 1;
      printf("Hit breakpoint #%d, %lx\n", id, allbrks[id]->addr);
      break;
    }
  }
  assert(id != BRKPNT_MAX);
}

// int
// init_breakpoints(void)
// {
//   int i;
//   for (i = 0; i < BRKPNT_MAX; i++) {
//     allbrks[i] = NULL;
//   }
//   return 0;
// }