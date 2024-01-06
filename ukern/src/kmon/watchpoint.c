#include <kernel.h>
#include <kmem.h>
#include <kmon/kmon.h>

#define WCHPNT_MAX  1  // 最多同时支持1个监视点

enum wchpnt_state {
  WCHPNT_OFF = 0,
  WCHPNT_ON,
};

enum wchpnt_width {
  WCHPNT_BYTE = 0,
  WCHPNT_WORD,
  WCHPNT_DWORD,
};

enum wchpnt_access {
  WCHPNT_RD = 0,
  WCHPNT_WR,
};

struct wchpnt {
  int id;     // Id of watchpoint
  int state;  // State of watchpoint
  int width;  // Watched data width
  int access; // Watched access type
  unsigned long addr;  // Watched address
  unsigned long value; // Keeped value
};
struct wchpnt *allwches[WCHPNT_MAX];

static int
addr_can_wch(unsigned long addr)
{
  return 1;
}

// Return breakpoint Id if success, -1 for error
int 
wchpnt_add(unsigned long addr, unsigned long val, int width, int access)
{
  int id;
  struct wchpnt *w;

  if (!addr_can_wch(addr))
    return -1;
  
  for (id = 0; id < WCHPNT_MAX; id++) {
    if (!allwches[id])
      break;
  }
  if (id == WCHPNT_MAX) {
    LOG_ERROR("Too many watchpoints\n");
    return -1;
  }

  if (!(w = kalloc(sizeof(*w))))
    return -1;
  w->access = access;
  w->width = width;
  w->addr = addr;
  w->value = val;
  w->id = id;
  w->state = WCHPNT_OFF;
  allwches[id] = w;
  return id;
}

int
wchpnt_enable(int id)
{
  assert(id >= 0 && id < WCHPNT_MAX);
  assert(allwches[id]);
  
  if (allwches[id]->state == WCHPNT_ON)
    return 0;

  allwches[id]->state = WCHPNT_ON;

  // Set hardward breakpoint register
  switch (id) {
  case 0:
  case 1:
    break;
  }
  return 0;
}

int
wchpnt_disable(int id)
{
  assert(id >= 0 && id < WCHPNT_MAX);
  assert(allwches[id]);

  if (allwches[id]->state == WCHPNT_OFF)
    return 0;

  allwches[id]->state = WCHPNT_OFF;

  // Set hardward breakpoint register
  switch (id) {
  case 0:
  case 1:
    break;
  }
  return 0;
}

int
wchpnt_del(int id)
{ 
  if (!allwches[id]) {
    LOG_ERROR("Try to delete a null watchpoint\n");
    return -1;
  }

  if (wchpnt_disable(id) < 0)
    return -1;

  kfree(allwches[id]);
  allwches[id] = NULL;
  return 0;
}

int
init_watchpoints(void)
{
  int i;
  for (i = 0; i < WCHPNT_MAX; i++) {
    allwches[i] = NULL;
  }
  return 0;
}