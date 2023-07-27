#include <uspace/syscall.h>
#include <panic.h>
#include <errno.h>
#include <types.h>
#include <compiler.h>


typedef void (*syscall_handler_t)(syscall_regs *regs);

extern long __sys_kobject_send(handle_t handle, void __user *data, size_t data_size,
		void __user *extra, size_t extra_size, uint32_t timeout);

static void aarch64_syscall_unsupport(syscall_regs *regs)
{
  u64 nr = *(u64 *)(regs + 1); // x8
  panic("Unsupported syscall:%d\n", nr);
  regs->a0 = -ENOENT;
}

static void sys_kobject_send(syscall_regs *regs)
{
    regs->a0 = __sys_kobject_send(
            (int)          regs->a0,
            (void __user *)regs->a1,
            (size_t)       regs->a2,
            (void __user *)regs->a3,
            (size_t)       regs->a4,
            (uint32_t)     regs->a5);
}

static syscall_handler_t __syscall_table[] = {
  [0 ... __NR_syscalls]     = aarch64_syscall_unsupport,

//   [__NR_kobject_open]    = __sys_kobject_open,
//   [__NR_kobject_create]  = __sys_kobject_create,
//   [__NR_kobject_reply]   = __sys_kobject_reply,
  [__NR_kobject_send]    = sys_kobject_send,
//   [__NR_kobject_recv]    = __sys_kobject_recv,
//   [__NR_kobject_close]   = __sys_kobject_close,
//   [__NR_kobject_ctl]     = __sys_kobject_ctl,
//   [__NR_kobject_mmap]    = __sys_kobject_mmap,
//   [__NR_kobject_munmap]  = __sys_kobject_munmap,

//   [__NR_grant]           = __sys_grant,

//   [__NR_yield]           = __sys_yield,
//   [__NR_futex]           = __sys_futex,

//   [__NR_map]             = __sys_map,
//   [__NR_unmap]           = __sys_unmap,
//   [__NR_trans]           = __sys_mtrans,

//   [__NR_clock_gettime]   = __sys_clock_gettime,
//   [__NR_clock_nanosleep] = __sys_clock_nanosleep,

//   [__NR_exit]            = __sys_exit,
//   [__NR_exitgroup]       = __sys_exitgroup,

//   [__NR_clone]           = __sys_clone,
};


void syscall_handler(syscall_regs *regs)
{
	int nr = *(u64 *)(regs + 1); // x8

	// arch_enable_local_irq(); // FIXME
  printf("in syscall handler, nr = %d\n", nr);
	if (nr >= __NR_syscalls) {
		regs->a0 = -EINVAL;
		return;
	}
	__syscall_table[nr](regs);

	// arch_disable_local_irq(); // FIXME
}