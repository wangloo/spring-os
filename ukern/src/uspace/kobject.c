#include <kernel.h>
#include <init.h>
#include <atomic.h>
#include <spinlock.h>
#include <proc.h>
#include <uspace/handle.h>


static kobject_create_cb kobj_create_cbs[KOBJ_TYPE_MAX];

static void 
register_kobject_type(kobject_create_cb cb, int type)
{
  assert(cb && type < KOBJ_TYPE_MAX);

  if (kobj_create_cbs[type])
    LOG_WARN("Overwrite kobject create callback for %d\n", type);
  kobj_create_cbs[type] = cb;
}


static void 
kobject_release(struct kobject *kobj)
{
	/*
	 * release poll_struct if needed.
	 */
	// release_poll_struct(kobj);

	if (kobj->ops && kobj->ops->release)
		kobj->ops->release(kobj);
}


int kobject_get(struct kobject *kobj)
{
	int old;

	if (!kobj)
		return 0;

	old = atomic_inc_if_postive(&kobj->ref);
	assert(old >= 0);

	return 1;
}

int kobject_put(struct kobject *kobj)
{
	int old;

	if (!kobj)
		return 0;

	old = atomic_dec_set_negtive_if_zero(&kobj->ref);
	assert(old > 0);

	/*
	 * if the old value is 1, then release the kobject.
	 */
	if (old == 1)
		kobject_release(kobj);

	return 1;
}

void 
kobject_init(struct kobject *kobj, int type, right_t right_mask,
              struct kobject_ops *ops, unsigned long data)
{
  assert(kobj);
	kobj->right_mask = right_mask;
	kobj->type = type;
	kobj->data = data;
  kobj->ops = ops;
  INIT_LIST_HEAD(&kobj->list);
	initlock(&kobj->lock, "kobj");
}



int kobject_create(int type, struct kobject **kobj, right_t *right, unsigned long data)
{
	kobject_create_cb ops;

	if ((type <= 0) || (type >= KOBJ_TYPE_MAX))
		return -ENOENT;

	ops = kobj_create_cbs[type];
	if (!ops) {
    LOG_ERROR("Unsupport create kobject tpye: %d\n", type);
		return -EOPNOTSUPP;
  }

	return ops(kobj, right, data);
}


int 
kobject_open(struct kobject *kobj, handle_t handle, right_t right)
{
	// int ret;

	// if (!kobj->ops || !kobj->ops->open)
	// 	return 0;

	// ret = kobj->ops->open(kobj, handle, right);
	// if (ret)
	// 	return ret;

	// if (right & KOBJ_RIGHT_WRITE)
	// 	poll_event_send(kobj->poll_struct, EV_WOPEN);
	// else
	// 	poll_event_send(kobj->poll_struct, EV_ROPEN);

	return 0;
}

int kobject_close(struct kobject *kobj, right_t right, struct proc *proc)
{
	// int ret = 0;

	// /*
	//  * just put this kobject if the right is 0.
	//  */
	// if (right == KOBJ_RIGHT_NONE) {
	// 	kobject_put(kobj);
	// 	return 0;
	// }

	// if (kobj->ops && kobj->ops->close)
	// 	ret = kobj->ops->close(kobj, right, proc);

	// /*
	//  * send the close event to the poller if need.
	//  */
	// if (right & KOBJ_RIGHT_WRITE)
	// 	poll_event_send(kobj->poll_struct, EV_WCLOSE);
	// else if (right & KOBJ_RIGHT_READ)
	// 	poll_event_send(kobj->poll_struct, EV_RCLOSE);

	// /*
	//  * dec the refcount which caused by kobject_init and
	//  * kobject_connect.
	//  */
	// kobject_put(kobj);

	// return ret;
  return 0;
}


long kobject_recv(struct kobject *kobj, void __user *data, size_t data_size,
		size_t *actual_data, void __user *extra, size_t extra_size,
		size_t *actual_extra, uint32_t timeout)
{
	// if (!kobj->ops || !kobj->ops->recv)
	// 	return -EACCES;

	// /*
	//  * before read, if there is task waitting the event
	//  * to be read, here can send EV_OUT event to notify
	//  * the target task to send new data.
	//  */
	// poll_event_send(kobj->poll_struct, EV_OUT);

	// return kobj->ops->recv(kobj, data, data_size, actual_data,
	// 		extra, extra_size, actual_extra, timeout);
  return 0;
}

long kobject_send(struct kobject *kobj, void __user *data, size_t data_size,
		void __user *extra, size_t extra_size, uint32_t timeout)
{
	if (!kobj->ops || !kobj->ops->send)
		return -EACCES;
	/*
	 * the poll event must called by the kobject itself
	 */
	return kobj->ops->send(kobj, data, data_size, extra,
			extra_size, timeout);
}

int kobject_reply(struct kobject *kobj, right_t right, unsigned long token,
		long err_code, handle_t fd, right_t fd_right)
{
	if (!kobj->ops || !kobj->ops->reply)
		return -EPERM;

	return kobj->ops->reply(kobj, right, token, err_code, fd, fd_right);
}

int kobject_munmap(struct kobject *kobj, right_t right)
{
	if (!kobj->ops || !kobj->ops->munmap)
		return -EPERM;

	return kobj->ops->munmap(kobj, right);
}

int kobject_mmap(struct kobject *kobj, right_t right,
		void **addr, unsigned long *msize)
{
	if (!kobj->ops || !kobj->ops->mmap)
		return -EPERM;

	return kobj->ops->mmap(kobj, right, addr, msize);
}

long kobject_ctl(struct kobject *kobj, right_t right,
		int req, unsigned long data)
{
	if (!kobj->ops || !kobj->ops->ctl)
		return -EPERM;

	return kobj->ops->ctl(kobj, req, data);
}

static int 
init_kobject_type(void)
{
  extern unsigned long __kobj_type_desc_start;
  extern unsigned long __kobj_type_desc_stop;
  struct kobject_type_desc *desc;

  section_for_each_item(__kobj_type_desc_start, __kobj_type_desc_stop, desc) {
    if (desc->type >= KOBJ_TYPE_MAX) {
      LOG_WARN("Unsupported kobject type [%d] name [%s]\n", desc->type, desc->name);
      continue;
    }
    LOG_INFO("Register kobject type[%2d] name [%s]\n", desc->type, desc->name);
    register_kobject_type(desc->ops, desc->type);
  }
  return 0;
}
uspace_initcall(init_kobject_type);









long sys_kobject_close(handle_t handle)
{
	struct kobject *kobj;
	right_t right;
	long ret;

	/*
	 * release the handle first, then other thread in
	 * this process can not see this kobject now. if one
	 * thread in this process called this successfully
	 * other thread can not get ok from release_handle.
	 */
	ret = handle_free(handle, &kobj, &right);
	if (ret)
		return ret;

	return kobject_close(kobj, right, cur_proc());
}

long 
sys_kobject_send(handle_t handle, void __user *data, size_t data_size,
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

long 
sys_kobject_recv(handle_t handle, void __user *data, size_t data_size,
		size_t *actual_data, void __user *extra, size_t extra_size,
		size_t *actual_extra, uint32_t timeout)
{
	// struct kobject *kobj;
	// right_t right;
	// long ret;

  // 	ret = get_kobject(handle, &kobj, &right);
  // 	if (ret)
  // 		return ret;

  // 	if (!(right & KOBJ_RIGHT_READ)) {
  // 		ret = -EPERM;
  // 		goto out;
  // 	}

  // 	ret = kobject_recv(kobj, data, data_size, actual_data, extra,
  // 			extra_size, actual_extra, timeout);
  // out:
  // 	put_kobject(kobj);
	// return ret;
  return 0;
}



handle_t 
sys_kobject_create(int type, unsigned long data)
{
	struct kobject *kobj;
	right_t right;
	int ret;

	ret = kobject_create(type, &kobj, &right, data);
	if (ret)
		return ret;

	/*
	 * visable for all the threads in this process, and
	 * the owner of this kobject have the GRANT right for
	 * this kobject.
	 */
	return handle_alloc(kobj, right);
}

int 
sys_kobject_open(handle_t handle)
{
	// struct kobject *kobj;
	// right_t right;
	// int ret;

	// ret = get_kobject(handle, &kobj, &right);
	// if (ret)
	// 	return ret;

	// ret = kobject_open(kobj, handle, right);
	// put_kobject(kobj);

	// return ret;
  return 0;
}

/*
 * kobject reply can reply a fd to the target process
 * who obtain this handle. if need to reply a fd.
 */
long sys_kobject_reply(handle_t handle, unsigned long token,
		long err_code, handle_t fd, right_t fd_right)
{
	// struct kobject *kobj;
	// right_t right;
	// long ret;

	// ret = get_kobject(handle, &kobj, &right);
	// if (ret)
	// 	return ret;

	// if (!(right & KOBJ_RIGHT_READ))
	// 	return -EPERM;

	// ret = kobject_reply(kobj, right, token, err_code, fd, fd_right);
	// put_kobject(kobj);

	// return ret;
  return 0;
}

long sys_kobject_ctl(handle_t handle, int req, unsigned long data)
{
// 	struct kobject *kobj;
// 	right_t right;
// 	long ret;

// 	ret = get_kobject(handle, &kobj, &right);
// 	if (ret)
// 		return ret;

// 	if (!(right & KOBJ_RIGHT_CTL)) {
// 		ret = -EPERM;
// 		goto out;
// 	}

// 	ret = kobject_ctl(kobj, right, req, data);
// out:
// 	put_kobject(kobj);

// 	return ret;
  return 0;
}

int sys_kobject_mmap(handle_t handle,
		void **addr, unsigned long *map_size)
{
// 	struct kobject *kobj;
// 	right_t right;
// 	long ret;

// 	ret = get_kobject(handle, &kobj, &right);
// 	if (ret)
// 		return ret;

// 	if (!(right & KOBJ_RIGHT_MMAP)) {
// 		ret = -EPERM;
// 		goto out;
// 	}

// 	ret = kobject_mmap(kobj, right, addr, map_size);
// out:
// 	put_kobject(kobj);
// 	return ret;
  return 0;
}

int sys_kobject_munmap(handle_t handle)
{
// 	struct kobject *kobj;
// 	right_t right;
// 	int ret;

// 	ret = get_kobject(handle, &kobj, &right);
// 	if (ret)
// 		return ret;

// 	if (!(right & KOBJ_RIGHT_MMAP)) {
// 		ret = -EPERM;
// 		goto out;
// 	}

// 	ret = kobject_munmap(kobj, right);
// out:
// 	put_kobject(kobj);
// 	return ret;
  return 0;
}


