#include <uspace/kobject.h>
#include <atomic.h>
#include <assert.h>
#include <list.h>
#include <errno.h>


static void kobject_release(struct kobject *kobj)
{
  /*
   * release poll_struct if needed.
   */
  assert(0);
  // release_poll_struct(kobj); // FIXME

  if (kobj->ops && kobj->ops->release) kobj->ops->release(kobj);
}


int kobject_get(struct kobject *kobj)
{
  int old;

  if (!kobj) return 0;

  old = atomic_inc_if_postive(&kobj->ref);
  assert(old >= 0);

  return 1;
}

int kobject_put(struct kobject *kobj)
{
  int old;

  if (!kobj) return 0;

  old = atomic_dec_set_negtive_if_zero(&kobj->ref);
  assert(old > 0);

  /*
   * if the old value is 1, then release the kobject.
   */
  if (old == 1) kobject_release(kobj);

  return 1;
}

/**
 * 所有类型的object共用的init接口，初始化一些共用的结构，
 * 但是最重要的，每个type都不同的ops成员却不是在这里初始化，
 * 而是在每个type的文件中。
 * 所以，这个init其实可以放到每个type的文件中做，但是这部分
 * 代码是重复的，放到这里可以减少重复。
 *
 * UPDATE: 我将op作为参数传进来是不是也行，更加减少了代码的
 * 重复度，试一试吧暂时。
 */
void kobject_init(struct kobject *kobj, int type, right_t right_mask,
                  struct kobject_ops *ops, unsigned long data)
{
  assert(kobj);
  kobj->right_mask = right_mask;
  kobj->type = type;
  kobj->data = data;
  kobj->ops = ops;
  kobj->list.prev = NULL;
  kobj->list.next = NULL;
  // INIT_LIST_HEAD(&kobj->list); // TODO: 改用这个接口是不是更好一些
  // spin_lock_init(&kobj->lock);
}


/**
 * @brief TODO
 *
 * @param [in] data TODO
 * @param [in] extra TODO
 * @return     long TODO
 */
long kobject_send(struct kobject *kobj, void __user *data, size_t data_size,
                  void __user *extra, size_t extra_size, uint32_t timeout)
{
  if (!kobj->ops || !kobj->ops->send) return -EACCES;
  /*
   * the poll event must called by the kobject itself
   */
  return kobj->ops->send(kobj, data, data_size, extra, extra_size, timeout);
}



int kobject_close(struct kobject *kobj, right_t right, struct process *proc)
{
  int ret = 0;

  /*
   * just put this kobject if the right is 0.
   */
  if (right == KOBJ_RIGHT_NONE) {
    kobject_put(kobj);
    return 0;
  }

  if (kobj->ops && kobj->ops->close) ret = kobj->ops->close(kobj, right, proc);

  /*
   * send the close event to the poller if need.
   */
  assert(0); // FIXME
  // if (right & KOBJ_RIGHT_WRITE)
  // 	poll_event_send(kobj->poll_struct, EV_WCLOSE);
  // else if (right & KOBJ_RIGHT_READ)
  // 	poll_event_send(kobj->poll_struct, EV_RCLOSE);

  /*
   * dec the refcount which caused by kobject_init and
   * kobject_connect.
   */
  kobject_put(kobj);

  return ret;
}

#if 0 // copy from minos, but 暂时不知道有什么用
int kobject_create(int type, struct kobject **kobj, right_t *right, unsigned long data)
{
	kobject_create_cb ops;

	if ((type <= 0) || (type >= KOBJ_TYPE_MAX))
		return -ENOENT;

	ops = kobj_create_cbs[type];
	if (!ops)
		return -EOPNOTSUPP;

	return ops(kobj, right, data);
}

#endif