#include <uspace/uvspace.h>
#include <string.h>
#include <asid.h>
#include <assert.h>
#include <page.h>
#include <errno.h>
#include <kmem.h>




int user_map_destory(struct process *proc, vaddr_t va, size_t size)
{
	// struct vspace *vs = &proc->vspace;
	// int ret, inuse;

	/*
	 * the process can be NULL when close the kobject
	 * by kernel, this is ok for kernel, since, kernel
	 * will unmap all the memory space for a process when
	 * the process exit.
	 */
	assert(proc != NULL);
	if (!IS_PAGE_ALIGN(va) || !IS_PAGE_ALIGN(size))
		return -EINVAL;

	/*
	 * cpu1: inc_vspace_usage
	 *       spin_lock
	 *       translate_va_to_pa
	 *
	 * cpu2 spin_lock
	 *      atomic_read
	 *      arch_host_unmap
	 *
	 * inuse value need after spin_lock.
	 */
    assert(0); // FIXME
	// spin_lock(&vs->lock);  // FIXME
	// inuse = atomic_read(&vs->inuse);
	// ret = pagetable_unmap(proc->vspace.pgdp, va, va + size, 0);
	// asm volatile("ic ialluis" : : );
	// if (inuse == 0)
	// 	release_vspace_pages(vs);
	// spin_unlock(&vs->lock);

	// return ret;
	return 0;
}


int user_map_create(struct process *proc, unsigned long vaddr,
		       size_t size, unsigned long phy, unsigned long flags)
{
	struct vspace *vs = &proc->vspace;
	int ret;

	if (!IS_PAGE_ALIGN(vaddr) || !IS_PAGE_ALIGN(phy) ||
			!IS_PAGE_ALIGN(size))
		return -EINVAL;

	// spin_lock(&vs->lock); // FIXME
	ret = pagetable_map(vs->pgdp, vaddr, phy, size, flags, get_free_page_q);
	// spin_unlock(&vs->lock);

	return ret;
}




int user_vspace_init(struct process *proc)
{
	struct vspace *vs = &proc->vspace;

	// spin_lock_init(&vs->lock); // FIXME

    
	vs->pgdp = get_free_pages(1, GFP_KERNEL);
	if (!vs->pgdp)
		return -ENOMEM;
    memset(vs->pgdp, 0, PAGE_SIZE);

	vs->asid = asid_alloc();
	vs->pdata = proc;
	// vs->notifier_ops = &user_mm_notifier_ops;

	return 0;
}

void user_vspace_deinit(struct process *proc)
{
	struct vspace *vs = &proc->vspace;

	user_map_destory(proc, 0, USER_PROCESS_ADDR_LIMIT);

	if (vs->pgdp)
		kfree(vs->pgdp);
	if (vs->asid != 0)
		asid_free(vs->asid);
}