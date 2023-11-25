#include <list.h>
#include <spinlock.h>
#include <uapi/kobject_uapi.h>

#define KOBJ_FLAGS_NON_SHARED (1 << 0)


struct kobject_ops;
struct proc;
/*
 * Kernel object is a object than can provide some ability
 * to user space thread.
 *
 * type : the type of this kobj defined as above.
 * ref  : reference count of this kernel object, when
 *        0 can be released.
 * right_mask : the right mask for this kobj used for grant
 * list : list all the kernel object for a task or global.
 */
struct kobject {
  uint8_t type;
  uint8_t flags;
  uint16_t padding;

  right_t right_mask;
  atomic_t ref;

  struct spinlock lock;
  // struct poll_struct *poll_struct;

  struct kobject_ops *ops;
  unsigned long data;
  struct list_head list;
};


struct kobject_ops {
  long (*send)(struct kobject *kobj, void __user *data, size_t data_size,
               void __user *extra, size_t extra_size, uint32_t timeout);

  long (*recv)(struct kobject *kobj, void __user *data, size_t data_size,
               size_t *actual_data, void __user *extra, size_t extra_size,
               size_t *actual_extra, uint32_t timeout);

  void (*release)(struct kobject *kobj);

  int (*open)(struct kobject *kobj, handle_t handle, right_t right);

  int (*poll)(struct kobject *ksrc, struct kobject *kdst, int event,
              bool enable);

  int (*close)(struct kobject *kobj, right_t right, struct proc *proc);

  int (*reply)(struct kobject *kobj, right_t right, unsigned long token,
               long err_code, handle_t fd, right_t fd_right);

  int (*mmap)(struct kobject *kobj, right_t right, void **addr,
              unsigned long *msize);

  int (*munmap)(struct kobject *kobj, right_t right);

  long (*ctl)(struct kobject *kobj, int req, unsigned long data);
};

typedef int (*kobject_create_cb)(struct kobject **kobj, right_t *right,
                                 unsigned long data);

struct kobject_desc {
  char *name;
  int type;
  kobject_create_cb ops;
};

#define DEFINE_KOBJECT(kname, ktype, kops)                                     \
  static struct kobject_desc __kobject_##kname __used __section(               \
      ".__kobject_desc") = {                                                   \
      .name = #kname,                                                          \
      .type = ktype,                                                           \
      .ops = kops,                                                             \
  }


long kobject_send(struct kobject *kobj, void __user *data, size_t data_size,
                  void __user *extra, size_t extra_size, uint32_t timeout);

int kobject_get(struct kobject *kobj);
int kobject_put(struct kobject *kobj);

int kobject_close(struct kobject *kobj, right_t right, struct proc *proc);
void kobject_init(struct kobject *kobj, int type, right_t right_mask,
                  struct kobject_ops *ops, unsigned long data);


