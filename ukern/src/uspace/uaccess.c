#include <kernel.h>
#include <page.h>
#include <pagetable.h>
#include <proc.h>

int 
__copy_from_user(void *dst, struct pagetable *pgtbl, __user void *src, size_t size)
{
	int offset = (unsigned long)src - align_page_down((vaddr_t)src);
	int copy_size;
	size_t cnt = size;
	void *ksrc;

  // Copy most one page at a time
  // As the mapped va of kernel can be discontinuous
	while (size > 0) {
		copy_size = PAGE_SIZE - offset;
		copy_size = copy_size > size ? size : copy_size;

		ksrc = (void *)ptov(pgtbl_walk(pgtbl, (vaddr_t)src));
		memcpy(dst, ksrc, copy_size);
		offset = 0;
		size -= copy_size;
		src += copy_size;
		dst += copy_size;
	}

	return cnt;
}

int __copy_to_user(struct pagetable *pgtbl, void __user *dst, void *src, size_t size)
{
	int offset = (unsigned long)dst - align_page_down((vaddr_t)dst);
	int copy_size;
	size_t cnt = size;
	void *kdst;

  // Copy most one page at a time
  // As the mapped va of kernel can be discontinuous
	while (size > 0) {
		copy_size = PAGE_SIZE - offset;
		copy_size = copy_size > size ? size : copy_size;

		kdst = (void *)ptov(pgtbl_walk(pgtbl, (unsigned long)dst));

		memcpy(kdst, src, copy_size);
		offset = 0;
		size -= copy_size;
		src += copy_size;
		dst += copy_size;
	}

	return cnt;
}

int copy_from_user(void *dst, void __user *src, size_t size)
{
	return __copy_from_user(dst, cur_proc()->pagetable, src, size);
}

int copy_to_user(void __user *dst, void *src, size_t size)
{
	return __copy_to_user(cur_proc()->pagetable, dst, src, size);
}