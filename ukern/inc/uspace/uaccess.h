#pragma once

#include <compiler.h>

struct vspace;

int __copy_from_user(void *dst, struct vspace *vsrc, void __user *src, size_t size);
int __copy_to_user(struct vspace *vdst, void __user *dst, void *src, size_t size);
int copy_from_user(void *dst, void __user *src, size_t size);
int copy_to_user(void __user *dst, void *src, size_t size);
