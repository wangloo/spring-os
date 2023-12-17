#include <kernel.h>
#include <uapi/kobject_uapi.h>
#include <uspace/uaccess.h>
#include <proc.h>
#include <page.h>
#include <kmem.h>


#define PMA_RIGHT	(KOBJ_RIGHT_CTL | KOBJ_RIGHT_MMAP | KOBJ_RIGHT_RWX)
#define PMA_RIGHT_MASK	(KOBJ_RIGHT_CTL | KOBJ_RIGHT_MMAP | KOBJ_RIGHT_RWX)


struct pma {
  unsigned long pstart;
  unsigned long pend;
  unsigned long psize;
  struct kobject kobj;
};

static long 
pma_send(struct kobject *kobj,
		void __user *data, size_t data_size,
		void __user *extra, size_t extra_size, uint32_t timeout)
{
	TODO(); return 0;
}

static long 
pma_recv(struct kobject *kobj, void __user *data,
		size_t data_size, size_t *actual_data, void __user *extra,
		size_t extra_size, size_t *actual_extra, uint32_t timeout)
{
  TODO(); return 0;
}

static int 
pma_reply(struct kobject *kobj, right_t right, 
    unsigned long token, long errno, handle_t fd, right_t fd_right)
{
	TODO(); return 0;
}

static void 
pma_release(struct kobject *kobj)
{
	TODO(); 
}

static long 
pma_ctl(struct kobject *kobj, int req, unsigned long data)
{
	TODO(); return 0;
}

static int 
pma_close(struct kobject *kobj, right_t right, struct proc *proc)
{
	TODO(); return 0;
}


static struct kobject_ops pma_kobj_ops = {
  .recv = pma_recv,
  .send = pma_send,
  .reply = pma_reply,
  .release = pma_release,
  .ctl = pma_ctl,
  .close = pma_close,
};


// Support allocate continuous pma ONLY!
// Caller make sure size is page aligned
static int
create_pma_normal(struct kobject **kobj, right_t *right, 
                    struct pma_create_arg *args)
{
  right_t right_mask = PMA_RIGHT_MASK;
  right_t right_ret = PMA_RIGHT;
  struct pma *p;
  size_t cnt = args->size >> PAGE_SHIFT;

  if (args->size == 0 || !page_aligned(args->size))
    return -EINVAL;

  if (!(p = kallocz(sizeof(*p))))
    return -ENOMEM;
  
  p->pstart = (unsigned long)page_allocn(cnt);
  if (!p->pstart)
    return -ENOMEM;
  p->psize = cnt << PAGE_SHIFT;
  p->pend = p->psize + p->psize;

  kobject_init(&p->kobj, KOBJ_TYPE_PMA, right_mask, &pma_kobj_ops, (unsigned long)p);

  *kobj = &p->kobj;
  *right = right_ret;
  return 0;
}

// Create a kobject which type is pma
// Only need to create struct pma, it has
// a member kobj as it's kobject
static int
create_pma(struct kobject **kobj, right_t *right, unsigned long data)
{
  struct pma_create_arg args;
  // struct proc *p;
  int ret;

  // Only root service can create pma directly
  if (!proc_is_roots(cur_proc()))
    return -EPERM;
  
  ret = copy_from_user(&args, (void *)data, sizeof(args));
  if (ret <= 0)
    return -EFAULT;

  if (args.type >= PMA_TYPE_MAX) 
    return -EINVAL;

  switch (args.type) {
  case PMA_TYPE_DMA:
  case PMA_TYPE_MMIO:
  case PMA_TYPE_PMEM:
    TODO(); break;
  case PMA_TYPE_KCACHE:
    TODO(); break;
  case PMA_TYPE_NORMAL:
    return create_pma_normal(kobj, right, &args);
  default:
    break;
  }
  return -1;
}


DEFINE_KOBJECT_TYPE_DESC(pma, KOBJ_TYPE_PMA, create_pma);

