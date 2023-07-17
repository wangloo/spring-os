
#include <uspace/kobject.h>
#include <uspace/proc.h>
#include <errno.h>

long __sys_kobject_send(handle_t handle, void __user *data, size_t data_size,
                        void __user *extra, size_t extra_size, uint32_t timeout)
{
  struct kobject *kobj;
  right_t right;
  long ret;

  ret = get_kobject(handle, &kobj, &right);
  if (ret) 
  	return ret;

  if (!(right & KOBJ_RIGHT_WRITE)) {
    ret = -EPERM;
    goto out;
  }

  ret = kobject_send(kobj, data, data_size, extra, extra_size, timeout);
out:
  put_kobject(kobj);
  return ret;
}