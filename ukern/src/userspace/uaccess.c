#include <page.h>
#include <current.h>
#include <uspace/uvspace.h>
#include <errno.h>
#include <addrspace.h>
#include <string.h>
#include <task.h>

int __copy_from_user(void *dst, struct vspace *vsrc, void __user *src, size_t size)
{
	int offset = (unsigned long)src - align_page_down((vaddr_t)src);
	int copy_size;
	size_t cnt = size;
	void *ksrc;

	user_vspace_inc_usage(vsrc);

	while (size > 0) {
		copy_size = PAGE_SIZE - offset;
		copy_size = copy_size > size ? size : copy_size;

		ksrc = (void *)pagetable_va_to_pa(vsrc->pgdp, (vaddr_t)src);
		if ((paddr_t)ksrc == 0) {
			cnt = -EFAULT;
			goto out;
		}

		ksrc = (char *)ptov(ksrc);
		memcpy(dst, ksrc, copy_size);
		offset = 0;
		size -= copy_size;
		src += copy_size;
		dst += copy_size;
	}

out:
	user_vspace_dec_usage(vsrc);
	return cnt;
}

int __copy_to_user(struct vspace *vdst, void __user *dst, void *src, size_t size)
{
	int offset = (unsigned long)dst - align_page_down((vaddr_t)dst);
	int copy_size;
	size_t cnt = size;
	void *kdst;

	user_vspace_inc_usage(vdst);

	while (size > 0) {
		copy_size = PAGE_SIZE - offset;
		copy_size = copy_size > size ? size : copy_size;

		kdst = (void *)pagetable_va_to_pa(vdst->pgdp, (unsigned long)dst);
		if ((paddr_t)kdst == 0) {
			cnt = -EFAULT;
			goto out;
		}

		memcpy((void *)ptov(kdst), src, copy_size);
		offset = 0;
		size -= copy_size;
		src += copy_size;
		dst += copy_size;
	}

out:
	user_vspace_dec_usage(vdst);

	return cnt;
}

int copy_from_user(void *dst, void __user *src, size_t size)
{
	return __copy_from_user(dst, current()->vs, src, size);
}

int copy_to_user(void __user *dst, void *src, size_t size)
{
	return __copy_to_user(current()->vs, dst, src, size);
}