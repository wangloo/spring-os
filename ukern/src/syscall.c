#include <kernel.h>
#include <ctx_arm64.h>
#include <syscall.h>
#include <proc.h>

#include <uspace/syscall.h>

typedef void (*syscall_handler_t)(struct gp_regs *regs);



static void __sys_kobject_close(struct gp_regs *regs)
{
	regs->x0 = sys_kobject_close((handle_t)regs->x0);
}

static void __sys_kobject_create(struct gp_regs *regs)
{
	regs->x0 = sys_kobject_create(
    (int)regs->x0, 
    (unsigned long)regs->x1);
}

static void 
__sys_kobject_recv(struct gp_regs *regs)
{
	size_t data = 0, extra = 0;

	regs->x0 = sys_kobject_recv(
			(int)regs->x0,
			(void __user *)regs->x1,
			(size_t)regs->x2,
			&data,
			(void __user *)regs->x3,
			(size_t)regs->x4,
			&extra,
			(uint32_t)regs->x5);
	regs->x1 = data;
	regs->x2 = extra;
}

static void 
__sys_kobject_send(struct gp_regs *regs)
{
  // LOG_DEBUG("SYSCALL", "sys_kobject_send x0: %d, x1: 0x%lx, x2: %d\n", 
  //             regs->x0, regs->x1, regs->x2);
	regs->x0 = sys_kobject_send(
			(int)regs->x0,
			(void __user *)regs->x1,
			(size_t)regs->x2,
			(void __user *)regs->x3,
			(size_t)regs->x4,
			(uint32_t)regs->x5);
  // LOG_DEBUG("sys_kobject_send ret: %ld\n", regs->x0);
}

static void 
__sys_kobject_reply(struct gp_regs *regs)
{
	regs->x0 = sys_kobject_reply(
			(int)regs->x0,
			(unsigned long)regs->x1,
			(long)regs->x2,
			(handle_t)regs->x3,
			(right_t)regs->x4);
}

static void 
__sys_kobject_ctl(struct gp_regs *regs)
{
	regs->x0 = sys_kobject_ctl((handle_t)regs->x0,
			(int)regs->x1,
			(unsigned long)regs->x2);
}

static void 
__sys_kobject_mmap(struct gp_regs *regs)
{
	void *addr = 0;
	unsigned long mapsz = 0;

	regs->x0 = (unsigned long)sys_kobject_mmap((handle_t)regs->x0, &addr, &mapsz);
	regs->x1 = (unsigned long)addr;
	regs->x2 = mapsz;
}

static void 
__sys_kobject_munmap(struct gp_regs *regs)
{
	regs->x0 = sys_kobject_munmap((handle_t)regs->x0);
}

static void 
__sys_kobject_open(struct gp_regs *regs)
{
	regs->x0 = sys_kobject_open((handle_t)regs->x0);
}


static void
__sys_unsupport(struct gp_regs *regs)
{
  int nr = regs->x8;
  
  regs->x0 = -ENOENT;
  LOG_ERROR("Unsupported syscall:%d\n", nr);

  // Exit system for easy debug
  exit(); 
}

static syscall_handler_t __syscall_table[] = {
  [0 ... __NR_syscalls]     = __sys_unsupport,

	[__NR_kobject_open]		= __sys_kobject_open,
	[__NR_kobject_create]		= __sys_kobject_create,
	[__NR_kobject_reply]		= __sys_kobject_reply,
	[__NR_kobject_send]		= __sys_kobject_send,
	[__NR_kobject_recv]		= __sys_kobject_recv,
	[__NR_kobject_close]		= __sys_kobject_close,
	[__NR_kobject_ctl]		= __sys_kobject_ctl,
	[__NR_kobject_mmap]		= __sys_kobject_mmap,
	[__NR_kobject_munmap]		= __sys_kobject_munmap,
};

void 
syscall_handler(struct gp_regs *regs)
{
  int nr = regs->x8;

  // LOG_DEBUG("in syscall handler, nr = %d\n", nr);

  if (nr >= __NR_syscalls) {
    regs->x0 = -EINVAL;
    return;
  }
  __syscall_table[nr](regs);
}
