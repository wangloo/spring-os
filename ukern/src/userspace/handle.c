#include <uspace/kobject.h>
#include <uspace/handle.h>
#include <task.h>
#include <uspace/proc.h>
#include <spinlock.h>
#include <page.h>
#include <errno.h>
#include <string.h>

#define KOBJ_PLACEHOLDER (struct kobject *)(-1)

#define to_handle_table_desc(hdesc)                                            \
  (struct handle_table_desc *)&hdesc[NR_DESC_PER_PAGE]


static struct handle_desc *new_handle_desc_table(uint32_t index)
{
  struct handle_desc *dt;
  struct handle_table_desc *htd;

  if (index >= PROC_MAX_HANDLE) {
    panic("handle table too big exceed %d\n", PROC_MAX_HANDLE);
    return NULL;
  }

  dt = get_free_page(GFP_KERNEL);
  if (!dt) return NULL;
  memset(dt, 0, PAGE_SIZE);

  htd = to_handle_table_desc(dt);
  htd->left = NR_DESC_PER_PAGE;
  htd->index = index;
  htd->next = NULL;

  return dt;
}

static int __alloc_new_handle(struct handle_table_desc *htd, handle_t *handle,
                              struct handle_desc **hd)
{
  struct handle_desc *table;
  struct handle_table_desc *new_htd;

  table = new_handle_desc_table(htd->index + NR_DESC_PER_PAGE);
  if (!table) return -ENOMEM;

  htd->next = table;
  new_htd = to_handle_table_desc(table);
  new_htd->left -= 1;

  *handle = new_htd->index;
  *hd = &table[0];

  return 0;
}

static int lookup_handle_desc(struct process *proc, handle_t handle,
                              struct handle_desc **hd,
                              struct handle_table_desc **htd)
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

void __release_handle(struct process *proc, handle_t handle)
{
  struct handle_desc *hd;
  struct handle_table_desc *htd;
  int ret;

  spin_lock(&proc->lock);
  ret = lookup_handle_desc(proc, handle, &hd, &htd);
  if (ret) goto out;

  if (hd->kobj == NULL) goto out;

  hd->kobj = NULL;
  hd->right = KOBJ_RIGHT_NONE;
  htd->left++;
out:
  spin_unlock(&proc->lock);
}

int release_process_handle(struct process *proc, handle_t handle,
                           struct kobject **kobj, right_t *right)
{
  struct handle_desc *hd;
  struct handle_table_desc *htd;
  int ret = -ENOENT;

  if (WRONG_HANDLE(handle) || !proc) return -ENOENT;

  spin_lock(&proc->lock);
  ret = lookup_handle_desc(proc, handle, &hd, &htd);
  if (ret) goto out;

  if (hd->kobj == NULL || hd->kobj == KOBJ_PLACEHOLDER) {
    ret = -EPERM;
    goto out;
  }

  *kobj = hd->kobj;
  *right = hd->right;

  hd->kobj = NULL;
  hd->right = KOBJ_RIGHT_NONE;
  htd->left++;
out:
  spin_unlock(&proc->lock);

  return ret;
}

int release_handle(handle_t handle, struct kobject **kobj, right_t *right)
{
  return release_process_handle(current_proc(), handle, kobj, right);
}

static inline int __alloc_handle_internal(struct handle_desc *desc,
                                          struct handle_table_desc *htd,
                                          handle_t *handle,
                                          struct handle_desc **hd)
{
  int i;

  if (htd->left == 0) return -ENOSPC;

  for (i = 0; i < NR_DESC_PER_PAGE; i++) {
    if (desc[i].kobj == NULL) {
      *handle = i + htd->index;
      *hd = &desc[i];
      return 0;
    }
  }

  assert(0);
  return -ENOSPC;
}

handle_t __alloc_handle(struct process *proc, struct kobject *kobj,
                        right_t right)
{
  struct handle_desc *hd = proc->handle_desc_table;
  struct handle_table_desc *htd = to_handle_table_desc(hd);
  handle_t handle = HANDLE_NULL;
  struct handle_desc *hdesc;
  int ret = -ENOSPC;

  assert(kobj != NULL);
  assert(proc != NULL);

  spin_lock(&proc->lock);

  do {
    ret = __alloc_handle_internal(hd, htd, &handle, &hdesc);
    if (ret == 0) break;
    hd = htd->next;
  } while (hd != NULL);

  if (ret != 0) {
    ret = __alloc_new_handle(htd, &handle, &hdesc);
    if (ret) {
      handle = HANDLE_NULL;
      goto out;
    }
  }

  hdesc->kobj = kobj;
  hdesc->right = right;

  if (kobj != KOBJ_PLACEHOLDER) kobject_get(kobj);
out:
  spin_unlock(&proc->lock);

  return handle;
}

handle_t alloc_handle(struct kobject *kobj, right_t right)
{
  return __alloc_handle(current_proc(), kobj, right);
}

static int setup_handle(struct process *proc, handle_t handle,
                        struct kobject *kobj, right_t right)
{
  struct handle_desc *hd;
  struct handle_table_desc *htd;
  int ret;

  assert(!WRONG_HANDLE(handle));

  spin_lock(&proc->lock);
  ret = lookup_handle_desc(proc, handle, &hd, &htd);
  if (ret) goto out;

  assert(hd->kobj == KOBJ_PLACEHOLDER);
  hd->kobj = kobj;
  hd->right = right;
  kobject_get(kobj);
out:
  spin_unlock(&proc->lock);
  return ret;
}

handle_t send_handle(struct process *proc, struct process *pdst,
                     handle_t handle, right_t right_send)
{
  struct handle_table_desc *htd;
  struct handle_desc *hdesc;
  struct kobject *kobj;
  int handle_ret;
  int right;
  int ret;

  if (WRONG_HANDLE(handle)) return -EINVAL;

  /*
   * first get a empty handle from the target process, then
   * do the transfer, the reason why take this logic is there
   * are two spinlocks need to request, and may cause dead lock
   * in some case.
   */
  handle_ret = __alloc_handle(pdst, KOBJ_PLACEHOLDER, 0);
  if (handle_ret < 0) return handle_ret;

  spin_lock(&proc->lock);
  ret = lookup_handle_desc(proc, handle, &hdesc, &htd);
  if (ret) goto out;

  kobj = hdesc->kobj;
  right = hdesc->right;
  if (!kobj || (kobj->flags & KOBJ_FLAGS_NON_SHARED)
      || (kobj == KOBJ_PLACEHOLDER) || !right) {
    ret = -EPERM;
    goto out;
  }

  /*
   * if the right is 0, means need send all the right
   * it has to target process, then check whether this
   * kobject has the required rights. then clear the right
   * which need to transfer to other process.
   */
  right_send = (right_send == 0) ? right : right_send;
  if ((right & right_send) != right_send) {
    ret = -EPERM;
    goto out;
  }

  hdesc->right = right & (~right_send | kobj->right_mask);
  spin_unlock(&proc->lock);

  setup_handle(pdst, handle_ret, kobj, right_send);

  return handle_ret;

out:
  spin_unlock(&proc->lock);
  __release_handle(pdst, handle_ret);
  panic("send handle %d with 0x%x fail\n", handle, right_send);

  return ret;
}

handle_t sys_grant(handle_t proc_handle, handle_t handle, right_t right)
{
  struct kobject *kobj_proc;
  right_t right_proc;
  int ret;

  if (WRONG_HANDLE(proc_handle) || WRONG_HANDLE(handle)) return -ENOENT;

  ret = get_kobject(proc_handle, &kobj_proc, &right_proc);
  if (ret) return -ENOENT;

  if (kobj_proc->type != KOBJ_TYPE_PROCESS) {
    ret = -EBADF;
    goto out;
  }

  ret = send_handle(current_proc(), (struct process *)kobj_proc->data, handle,
                    right);
out:
  put_kobject(kobj_proc);

  return ret;
}

int get_kobject_from_process(struct process *proc, handle_t handle,
                             struct kobject **kobj, right_t *right)
{
  int ret;
  struct kobject *tmp;
  struct handle_desc *hd;
  struct handle_table_desc *htd;

  if (WRONG_HANDLE(handle) || !proc) return -ENOENT;

  spin_lock(&proc->lock);
  ret = lookup_handle_desc(proc, handle, &hd, &htd);
  if (ret) goto out;

  tmp = hd->kobj;

  if ((tmp != NULL) && (tmp != KOBJ_PLACEHOLDER)) {
    if (kobject_get(tmp)) {
      *kobj = tmp;
      *right = hd->right;
      ret = 0;
    }
  }
out:
  spin_unlock(&proc->lock);
  return ret;
}

int get_kobject(handle_t handle, struct kobject **kobj, right_t *right)
{
  return get_kobject_from_process(current_proc(), handle, kobj, right);
}

int put_kobject(struct kobject *kobj)
{
  return kobject_put(kobj);
}

void process_handles_deinit(struct process *proc)
{
  struct handle_desc *table = proc->handle_desc_table;
  struct handle_table_desc *tdesc = to_handle_table_desc(table);
  struct handle_desc *tmp;

  while (table != NULL) {
    tmp = tdesc->next;
    tdesc = to_handle_table_desc(tmp);
    free_pages(table);
    table = tmp;
  }
}

static void release_handle_table(struct process *proc,
                                 struct handle_desc *table)
{
  struct handle_desc *hdesc = table;
  int i;

  for (i = 0; i < NR_DESC_PER_PAGE; i++) {
    if ((hdesc->kobj) && (hdesc->kobj != KOBJ_PLACEHOLDER))
      kobject_close(hdesc->kobj, hdesc->right, proc);
    hdesc++;
  }
}

void release_proc_kobjects(struct process *proc)
{
  struct handle_desc *table = proc->handle_desc_table;
  struct handle_table_desc *tdesc = to_handle_table_desc(table);
  struct handle_desc *tmp;

  while (table != NULL) {
    tmp = tdesc->next;
    tdesc = to_handle_table_desc(tmp);
    release_handle_table(proc, table);
    table = tmp;
  }
}

int init_proc_handles(struct process *proc)
{
  extern struct kobject stdio_kobj;
  handle_t handle;

  proc->handle_desc_table = new_handle_desc_table(0);
  if (!proc->handle_desc_table) return -ENOMEM;

  /*
   * Main task kobj is 0, process can use this handle
   * to control itself.
   *
   * stdout stdin and stderr will provided by kernel, for process
   * handle [1] is stdout
   * handle [2] is stdin
   * handle [3] is stderr
   */
  handle = __alloc_handle(proc, &proc->kobj, KOBJ_RIGHT_WRITE | KOBJ_RIGHT_CTL);
  assert(handle == 0);
  handle = __alloc_handle(proc, &stdio_kobj, KOBJ_RIGHT_READ);
  assert(handle == 1);
  handle = __alloc_handle(proc, &stdio_kobj, KOBJ_RIGHT_WRITE);
  assert(handle == 2);
  handle = __alloc_handle(proc, &stdio_kobj, KOBJ_RIGHT_WRITE);
  assert(handle == 3);

  return 0;
}
