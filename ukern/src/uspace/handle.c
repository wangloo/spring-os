#include <kernel.h>
#include <proc.h>
#include <page.h>

#include <uspace/handle.h>


#define HANDLE_NULL	(-1)

#define NR_DESC_PER_PAGE (PAGE_SIZE / sizeof(struct handle_desc) - 1)
#define PROC_MAX_HANDLE	(NR_DESC_PER_PAGE * 128)

#define invalid_handle(handle)	\
	((handle == HANDLE_NULL) || (handle >= PROC_MAX_HANDLE))
#define to_handle_table_desc(hdesc)	\
	(struct handle_table_desc *)&hdesc[NR_DESC_PER_PAGE]


static struct handle_desc *
new_handle_table(int index)
{
  struct handle_desc *hd;
  struct handle_table_desc *htd;

  if (index >= PROC_MAX_HANDLE) {
    LOG_ERROR("HANDLE", "handle table too big exceed %d\n", PROC_MAX_HANDLE);
    return NULL;
  }
  
  hd = page_allocz();
  htd = to_handle_table_desc(hd);
  htd->next = NULL;
  htd->free = NR_DESC_PER_PAGE;
  htd->index = index;

  return hd;
}



static int 
lookup_handle_desc(struct proc *proc, handle_t handle,
    struct handle_desc **hd, struct handle_table_desc **htd)
{
	struct handle_desc *table = proc->handle_desc_table;
	struct handle_table_desc *tdesc = to_handle_table_desc(table);

	while (handle >= NR_DESC_PER_PAGE) {
		handle -= NR_DESC_PER_PAGE;
		table = tdesc->next;
		tdesc = to_handle_table_desc(table);
	}

	*hd = &table[handle];
	*htd = tdesc;

	return 0;
}



static int 
get_kobject_from_proc(struct proc *proc, handle_t handle,
			struct kobject **kobj, right_t *right)
{
	int ret;
	struct kobject *tmp;
	struct handle_desc *hd;
	struct handle_table_desc *htd;

	if (invalid_handle(handle) || !proc)
		return -ENOENT;

	acquire(&proc->lock);

	ret = lookup_handle_desc(proc, handle, &hd, &htd);
	if (ret)
		goto out;

	tmp = hd->kobj;

	if (tmp != NULL) {
		if (kobject_get(tmp)) {
			*kobj = tmp;
			*right = hd->right;
			ret = 0;
		}
	}
out:
	release(&proc->lock);
	return ret;
}

// Find kobject by handle id
int 
get_kobject(handle_t handle, struct kobject **kobj, right_t *right)
{
	return get_kobject_from_proc(cur_proc(), handle, kobj, right);
}

// No real effect, just in pairs with kobject_get()
int 
put_kobject(struct kobject *kobj)
{
	return kobject_put(kobj);
}



static handle_t
__handle_alloc(struct proc *proc, struct kobject *kobj, right_t right)
{
	struct handle_desc *hd = proc->handle_desc_table;
	struct handle_table_desc *htd = to_handle_table_desc(hd);
  handle_t handle = HANDLE_NULL;
  struct handle_desc *target;
  
  assert(kobj != NULL);
  assert(proc != NULL);

  acquire(&proc->lock);


  while ((handle == HANDLE_NULL) && hd) {
    if (htd->free == 0) {
      hd = htd->next;
      htd = to_handle_table_desc(hd);
      continue;
    }

    for (int i = 0; i < NR_DESC_PER_PAGE; i++) {
      if (hd[i].kobj == NULL) {
        handle = i + htd->index;
        target = &hd[i];
        break;
      }
    }
  }
  
  if (!hd) {
    // TODO: allocate a new handle table for this process
    goto out;
  }

  target->kobj = kobj;
  target->right = right;
  htd->free -= 1;
  kobject_get(kobj);

out:
  release(&proc->lock);
  return handle;
}

// Attach the kobj to current process
// Return a handle for control the kobj
handle_t
handle_alloc(struct kobject *kobj, right_t right)
{
  return __handle_alloc(cur_proc(), kobj, right);
}



static int
__handle_free(struct proc *proc, handle_t handle, 
                struct kobject **kobj, right_t *right)
{
  struct handle_desc *hd;
  struct handle_table_desc *htd;
  int ret = -ENOENT;

  if (invalid_handle(handle) || !proc)
    return ret;

  acquire(&proc->lock);
  ret = lookup_handle_desc(proc, handle, &hd, &htd);
  if (ret) 
    goto out;

  if (hd->kobj == NULL) {
    ret = -EPERM;
    goto out;
  }

  *kobj = hd->kobj;
  *right = hd->right;

  hd->kobj = NULL;
  hd->right = KOBJ_RIGHT_NONE;
  htd->free += 1;

out:
  release(&proc->lock);
  return ret;
}

int 
handle_free(handle_t handle, struct kobject **kobj, right_t *right)
{
  return __handle_free(cur_proc(), handle, kobj, right);
}

// Initialize process's handle table after being created
void 
init_proc_handles(struct proc *proc)
{
  extern struct kobject stdio_kobj;

  proc->handle_desc_table = new_handle_table(0);

	/*
	 * Main task kobj is 0, process can use this handle
	 * to control itself.
	 *
	 * stdout stdin and stderr will provided by kernel, for process
	 * handle [1] is stdout
	 * handle [2] is stdin
	 * handle [3] is stderr
	 */
  handle_t handle;
	handle = __handle_alloc(proc, &proc->kobj, KOBJ_RIGHT_WRITE | KOBJ_RIGHT_CTL);
	assert(handle == 0);
	handle = __handle_alloc(proc, &stdio_kobj, KOBJ_RIGHT_READ);
	assert(handle == 1);
	handle = __handle_alloc(proc, &stdio_kobj, KOBJ_RIGHT_WRITE);
	assert(handle == 2);
	handle = __handle_alloc(proc, &stdio_kobj, KOBJ_RIGHT_WRITE);
	assert(handle == 3);

}