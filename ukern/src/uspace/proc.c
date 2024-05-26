#include <kernel.h>
#include <uapi/kobject_uapi.h>
#include <uspace/uaccess.h>
#include <proc.h>

#define PROC_RIGHT	(KOBJ_RIGHT_READ | KOBJ_RIGHT_CTL)
#define PROC_RIGHT_MASK	(KOBJ_RIGHT_CTL)

static long 
proc_send(struct kobject *kobj,
		void __user *data, size_t data_size,
		void __user *extra, size_t extra_size, uint32_t timeout)
{
	TODO(); return 0;
}

static long 
proc_recv(struct kobject *kobj, void __user *data,
		size_t data_size, size_t *actual_data, void __user *extra,
		size_t extra_size, size_t *actual_extra, uint32_t timeout)
{
  TODO(); return 0;
}

static int 
proc_reply(struct kobject *kobj, right_t right, 
    unsigned long token, long errno, handle_t fd, right_t fd_right)
{
	TODO(); return 0;
}

static void 
proc_release(struct kobject *kobj)
{
	TODO(); 
}

static long 
proc_ctl(struct kobject *kobj, int req, unsigned long data)
{
	TODO(); return 0;
}

static int 
proc_close(struct kobject *kobj, right_t right, struct proc *proc)
{
	TODO(); return 0;
}



static struct kobject_ops proc_kobj_ops = {
  .recv = proc_recv,
  .send = proc_send,
  .reply = proc_reply,
  .release = proc_release,
  .ctl = proc_ctl,
  .close = proc_close,
};
static int
kobject_create_proc(struct kobject **kobj, right_t *right, unsigned long data)
{
  struct process_create_arg args;
  struct proc *p;
  int ret;

  // Only root service can create process directly
  if (!proc_is_roots(cur_proc())) {
	LOG_ERROR("Only rootserver can create process\n");
    return -EPERM;
  }
  
  ret = copy_from_user(&args, (void *)data, sizeof(args));
  if (ret <= 0) {
	LOG_ERROR("can't get create args\n");
    return -EFAULT;
  }
  
  p = create_proc(NULL, args.pid, args.prio);
  if (!p)
    return -EFAULT;
  kobject_init(&p->kobj, KOBJ_TYPE_PROCESS, PROC_RIGHT_MASK, &proc_kobj_ops, 0);

  *kobj = &p->kobj;
  *right = PROC_RIGHT;
  return 0;
}


DEFINE_KOBJECT_TYPE_DESC(process, KOBJ_TYPE_PROCESS, kobject_create_proc);
